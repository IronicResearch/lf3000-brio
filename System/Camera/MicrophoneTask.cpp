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

	pCtx->bPaused = false;

	bRunning = pCtx->bStreaming = true;
	cam->dbg_.DebugOut( kDbgLvlImportant, "MicrophoneTask Started...\n" );

	timeout = false;
	timer = cam->kernel_.CreateTimer(TimerCallback, props, NULL);
	props.timeout.it_value.tv_sec = pCtx->reqLength;
	props.timeout.it_value.tv_nsec = 0;
	cam->kernel_.StartTimer(timer, props);

	cam->StartAudio();

	/*
	 * This is intentionally an assignment, not a comparison.
	 * End loop when bRunning is false.
	 */
	while(bRunning = pCtx->bStreaming)
	{
		if(!pCtx->bPaused)
		{
			cam->WriteAudio(sndfile);
		}
	}

	cam->kernel_.DestroyTimer(timer);

	cam->StopAudio();

	sf_close(sndfile);

	// Post done message to event listener
	if(pCtx->pListener)
	{
		CEventMPI			evntmgr;
		CCameraEventMessage *msg;

		if(!timeout)								// manually stopped by StopVideoCapture()
		{
			tCaptureStoppedMsg		data;
			data.vhndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= 0;
			msg = new CCameraEventMessage(data);
		}
		else if(pCtx->reqLength <= pCtx->reqLength)	// normal timeout
		{
			tCaptureTimeoutMsg		data;
			data.vhndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= 0;
			msg = new CCameraEventMessage(data);
		}
		else										// file system capacity timeout
		{
			tCaptureQuotaHitMsg		data;
			data.vhndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= 0;
			msg = new CCameraEventMessage(data);
		}

		evntmgr.PostEvent(*msg, 0, pCtx->pListener);
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

	return r;
}

//----------------------------------------------------------------------------
tErrType DeInitMicTask(CCameraModule* module)
{
	CKernelMPI	kernel;
	void* 		retval;

	if (hMicThread == kNull)
		return kNoErr;

	// Stop running task, if it hasn't already stopped itself
	bRunning = module->micCtx_.bStreaming = false;
	while (!bStopping)
		kernel.TaskSleep(10);

	if (!bStopping)
		kernel.CancelTask(hMicThread);
	kernel.JoinTask(hMicThread, retval);
	module->micCtx_.hMicThread = hMicThread = kNull;

	return kNoErr;
}

LF_END_BRIO_NAMESPACE()

// EOF
