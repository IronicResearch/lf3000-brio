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
	U32					elapsed = 0;

	cam					=	static_cast<CCameraModule*>(arg);
	tMicrophoneContext	*pCtx 	= &cam->micCtx_;

	Boolean				bRet, bWasPaused = false;

	tTimerProperties	props	= {TIMER_RELATIVE_SET, {0, 0, 0, 0}};
	tTimerHndl 			timer 	= kInvalidTimerHndl;
	saveTimerSettings	oldTimer;

	// these are needed to stop the recording asynchronously
	// globals are not ideal, but the timer callback doesn't take a custom parameter
	hndl	= pCtx->hndl;

	sf_info.samplerate	= pCtx->rate;
	sf_info.channels	= pCtx->channels;
	sf_info.format		= SF_FORMAT_WAV | cam->XlateAudioFormatSF(pCtx->format);

	sndfile = sf_open(const_cast<char*>(pCtx->path.c_str()), SFM_WRITE, &sf_info);

	// Paused state set by StartAudioCapture() API now
	// pCtx->bPaused = false;

	timeout = false;
	timer = cam->kernel_.CreateTimer(TimerCallback, props, NULL);
	props.timeout.it_value.tv_sec = pCtx->maxLength;
	props.timeout.it_value.tv_nsec = 0;
	cam->kernel_.StartTimer(timer, props);

	if (!pCtx->bPaused)
		cam->StartAudio();

	bRunning = true;
	cam->dbg_.DebugOut( kDbgLvlImportant, "MicrophoneTask Started...\n" );

	while(bRunning)
	{
		if(pCtx->bPaused && !bWasPaused)
		{
			cam->kernel_.PauseTimer(timer, oldTimer);
			bWasPaused = true;

			continue;
		}

		if(bWasPaused && !pCtx->bPaused)
		{
			cam->kernel_.ResumeTimer(timer, oldTimer);
			bWasPaused = false;
		}

		if(!pCtx->bPaused)
		{
			bRet = cam->WriteAudio(sndfile);
			if (!bRet)
				cam->kernel_.TaskSleep(10);
		}
	}

	cam->kernel_.DestroyTimer(timer);

	cam->StopAudio();

	sf_close(sndfile);

	elapsed = pCtx->bytesWritten / (pCtx->rate * pCtx->channels * sizeof(short));

	// Post done message to event listener
	if(pCtx->pListener)
	{
		CCameraEventMessage *msg;

		if(!cam->valid)								// camera hot-unplugged
		{
			tCameraRemovedMsg		data;
			data.ahndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= elapsed;
			msg = new CCameraEventMessage(data);
		}
		else if(!timeout)							// manually stopped by StopVideoCapture()
		{
			tCaptureStoppedMsg		data;
			data.ahndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= elapsed;
			msg = new CCameraEventMessage(data);
		}
		else if(pCtx->reqLength && pCtx->reqLength <= pCtx->maxLength)	// normal timeout
		{
			tCaptureTimeoutMsg		data;
			data.ahndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= elapsed;
			msg = new CCameraEventMessage(data);
		}
		else										// file system capacity timeout
		{
			tCaptureQuotaHitMsg		data;
			data.ahndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= elapsed;
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
