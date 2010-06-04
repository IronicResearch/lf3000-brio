//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		MicrophoneTask.cpp
//
// Description:
//		Microphone module task thread implementation.
//
//==============================================================================

#include <DebugMPI.h>
#include <CameraMPI.h>
#include <AudioMPI.h>
#include <KernelMPI.h>
#include <EventMPI.h>
#include <CameraPriv.h>

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// Local task state variables
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	tTaskHndl				hMicThread		= kNull;
	volatile bool			bRunning		= false;
	volatile bool			bStopping		= false;
	volatile bool			bInited			= false;
	tAudCapHndl				hndl			= kInvalidAudCapHndl;
	class CCameraModule*	cam				= NULL;
	volatile bool			timeout			= false;
}

//============================================================================
// Microphone task thread Implementation
//============================================================================

//----------------------------------------------------------------------------
static void TimerCallback(tTimerHndl arg)
{
	timeout = true;
	cam->StopAudioCapture(hndl);
}

//----------------------------------------------------------------------------
void* MicTaskMain(void* arg)
{
	struct SF_INFO		sf_info;
	SNDFILE*			sndfile;
	U32					start, end;

	cam					=	static_cast<CCameraModule*>(arg);
	tMicrophoneContext	*pCtx 	= &cam->micCtx_;

	Boolean				bRet;

	tTimerProperties	props	= {TIMER_RELATIVE_SET, {0, 0, 0, 0}};
	tTimerHndl 			timer 	= kInvalidTimerHndl;

	// these are needed to stop the recording asynchronously
	// globals are not ideal, but the timer callback doesn't take a custom parameter
	hndl	= pCtx->hndl;

	sf_info.samplerate	= pCtx->rate;
	sf_info.channels	= pCtx->channels;
	sf_info.format		= SF_FORMAT_WAV | cam->XlateAudioFormatSF(pCtx->format);

	sndfile = sf_open(const_cast<char*>(pCtx->path.c_str()), SFM_WRITE, &sf_info);

	// Paused state set by StartAudioCapture() API now
	// pCtx->bPaused = false;

	bRunning = true;
	cam->dbg_.DebugOut( kDbgLvlImportant, "MicrophoneTask Started...\n" );

	timeout = false;
	timer = cam->kernel_.CreateTimer(TimerCallback, props, NULL);
	props.timeout.it_value.tv_sec = pCtx->reqLength;
	props.timeout.it_value.tv_nsec = 0;
	cam->kernel_.StartTimer(timer, props);

	start = cam->kernel_.GetElapsedTimeAsMSecs();

	if (!pCtx->bPaused)
		cam->StartAudio();

	while(bRunning)
	{
		if(!pCtx->bPaused)
		{
			bRet = cam->WriteAudio(sndfile);
			if (!bRet)
				cam->kernel_.TaskSleep(10);
		}
	}

	cam->kernel_.DestroyTimer(timer);

	end = cam->kernel_.GetElapsedTimeAsMSecs();
	if(end > start)
	{
		end -= start;
	}
	else
	{
		end += (kU32Max - start);
	}

	cam->StopAudio();

	sf_close(sndfile);

	// Post done message to event listener
	if(pCtx->pListener)
	{
		CCameraEventMessage *msg;

		if(!cam->valid)								// camera hot-unplugged
		{
			tCameraRemovedMsg		data;
			data.vhndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= end;
			msg = new CCameraEventMessage(data);
		}
		else if(!timeout)							// manually stopped by StopVideoCapture()
		{
			tCaptureStoppedMsg		data;
			data.vhndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= end;
			msg = new CCameraEventMessage(data);
		}
		else if(pCtx->reqLength <= pCtx->maxLength)	// normal timeout
		{
			tCaptureTimeoutMsg		data;
			data.vhndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= end;
			msg = new CCameraEventMessage(data);
		}
		else										// file system capacity timeout
		{
			tCaptureQuotaHitMsg		data;
			data.vhndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= end;
			msg = new CCameraEventMessage(data);
		}

		cam->event_.PostEvent(*msg, 0, pCtx->pListener);
		delete msg;
	}

	bStopping = true;
	cam->dbg_.DebugOut( kDbgLvlImportant, "MicTask Stopping...\n" );

	return kNull;
}

//----------------------------------------------------------------------------
tErrType InitMicTask(CCameraModule* module)
{
	CDebugMPI		dbg(kGroupCamera);
	CKernelMPI		kernel;
	tErrType		r;
	tTaskHndl 		hndl;
	tTaskProperties	prop;

	// Set thread state prior to task creation
	bRunning = bStopping = false;

	// Setup task properties
	prop.TaskMainFcn			= (void* (*)(void*))MicTaskMain;
	prop.taskMainArgCount		= 1;
	prop.pTaskMainArgValues		= (void*)module;

	r = kernel.CreateTask( hndl, prop, NULL );
	dbg.Assert( kNoErr == r, "InitMicTask: Failed to create MicTask!\n" );

	// Save task handle for cleanup
	module->micCtx_.hMicThread = hMicThread = hndl;
	while (!bRunning)
		kernel.TaskSleep(1);

	bInited = true;
	return r;
}

//----------------------------------------------------------------------------
tErrType DeInitMicTask(CCameraModule* module)
{
	CKernelMPI	kernel;
	void* 		retval;

	if (hMicThread == kNull)
		return kNoErr;

	while (!bInited)
		kernel.TaskSleep(1);

	// Stop running task, if it hasn't already stopped itself
	bRunning = false;
	while (!bStopping)
		kernel.TaskSleep(10);

	if (!bStopping)
		kernel.CancelTask(hMicThread);
	kernel.JoinTask(hMicThread, retval);
	module->micCtx_.hMicThread = hMicThread = kNull;

	bInited = false;

	return kNoErr;
}

LF_END_BRIO_NAMESPACE()

// EOF
