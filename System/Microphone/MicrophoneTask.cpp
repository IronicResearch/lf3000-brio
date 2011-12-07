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
#include <MicrophoneMPI.h>
#include <AudioMPI.h>
#include <KernelMPI.h>
#include <EventMPI.h>
#include <MicrophonePriv.h>

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
	class CMicrophoneModule*	cam				= NULL;
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
	SNDFILE*			sndfile = NULL;
	U32					elapsed = 0;

	cam					=	static_cast<CMicrophoneModule*>(arg);
	tMicrophoneContext	*pCtx 	= &cam->micCtx_;

	Boolean				bRet, bWasPaused = false;
	Boolean				bTriggered = false;

	tTimerProperties	props	= {TIMER_RELATIVE_SET, {0, 0, 0, 0}};
	tTimerHndl 			timer 	= kInvalidTimerHndl;
	saveTimerSettings	oldTimer;

	// these are needed to stop the recording asynchronously
	// globals are not ideal, but the timer callback doesn't take a custom parameter
	hndl	= pCtx->hndl;

	sf_info.samplerate	= pCtx->rate;
	sf_info.channels	= pCtx->channels;
	sf_info.format		= SF_FORMAT_WAV | cam->XlateAudioFormatSF(pCtx->format);

	// Writing to WAV file is conditional on valid path string
	if (pCtx->path.length())
		sndfile = sf_open(const_cast<char*>(pCtx->path.c_str()), SFM_WRITE, &sf_info);

	// Paused state set by StartAudioCapture() API now
	// pCtx->bPaused = false;

	timeout = false;
	timer = cam->kernel_.CreateTimer(TimerCallback, props, NULL);
	props.timeout.it_value.tv_sec = pCtx->maxLength;
	props.timeout.it_value.tv_nsec = 0;
	cam->kernel_.StartTimer(timer, props);
	elapsed = cam->kernel_.GetElapsedTimeAsMSecs();

	if (!pCtx->bPaused)
		cam->StartAudio(true);

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
			if (sndfile)
				bRet = cam->WriteAudio(sndfile);
			else
				bRet = cam->FlushAudio();	// Does not actually flush audio if threshold zeroed by CameraMPI

			if (!bRet)
				cam->kernel_.TaskSleep(10);

			if (!sndfile && pCtx->pListener && bRet != bTriggered && cam->kernel_.GetElapsedTimeAsMSecs() > elapsed)
			{
				tMicrophoneAudioTriggeredMsg		data;
				data.ahndl				= pCtx->hndl;
				data.threshold			= bTriggered = bRet;
				data.timestamp 			= cam->kernel_.GetElapsedTimeAsMSecs();
				CMicrophoneEventMessage msg = CMicrophoneEventMessage(data);
				cam->event_.PostEvent(msg, 128, pCtx->pListener);
				elapsed 				= data.timestamp + pCtx->duration;
			}
		}
	}

	cam->kernel_.DestroyTimer(timer);

	cam->StopAudio();

	if (sndfile)
		sf_close(sndfile);

	elapsed = pCtx->bytesWritten / (pCtx->rate * pCtx->channels * sizeof(short));

	// Post done message to event listener
	if(pCtx->pListener)
	{
		CMicrophoneEventMessage *msg;

		if(!cam->valid)								// camera hot-unplugged
		{
			tMicrophoneRemovedMsg		data;
			data.ahndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= elapsed;
			msg = new CMicrophoneEventMessage(data);
		}
		else if(!timeout)							// manually stopped by StopVideoCapture()
		{
			tMicrophoneCaptureStoppedMsg		data;
			data.ahndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= elapsed;
			msg = new CMicrophoneEventMessage(data);
		}
		else if(pCtx->reqLength && pCtx->reqLength <= pCtx->maxLength)	// normal timeout
		{
			tMicrophoneCaptureTimeoutMsg		data;
			data.ahndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= elapsed;
			msg = new CMicrophoneEventMessage(data);
		}
		else										// file system capacity timeout
		{
			tMicrophoneCaptureQuotaHitMsg		data;
			data.ahndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= elapsed;
			msg = new CMicrophoneEventMessage(data);
		}

		cam->event_.PostEvent(*msg, 0, pCtx->pListener);
		delete msg;
	}

	bStopping = true;
	cam->dbg_.DebugOut( kDbgLvlImportant, "MicTask Stopping...\n" );

	return kNull;
}

//----------------------------------------------------------------------------
tErrType InitMicTask(CMicrophoneModule* module)
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
tErrType DeInitMicTask(CMicrophoneModule* module)
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