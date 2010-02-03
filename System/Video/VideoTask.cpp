//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		Video.cpp
//
// Description:
//		Video module task thread implementation.
//
//==============================================================================

#include <DebugMPI.h>
#include <VideoMPI.h>
#include <AudioMPI.h>
#include <KernelMPI.h>
#include <DisplayMPI.h>
#include <EventMPI.h>
#include <VideoPriv.h>

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// Local task state variables
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	tTaskHndl	hVideoThread = kNull;
	volatile bool		bRunning = false;
	volatile bool		bStopping = false;
	tMutex				gThreadMutex = PTHREAD_MUTEX_INITIALIZER;
	tCond				gThreadCond  = PTHREAD_COND_INITIALIZER;
	class VideoListener*	pVideoListener = NULL;
	volatile U32			gAudioTime = 0;
	volatile bool			gAudioDone = false;
}

//============================================================================
// Local event listener
//============================================================================
namespace
{
	const tEventType LocalVideoEvents[] = {kAllAudioEvents};
	
	class VideoListener : public IEventListener
	{
		tVideoContext*	mpVidCtx;
	public:
		VideoListener(tVideoContext* pCtx):
			IEventListener(LocalVideoEvents, ArrayCount(LocalVideoEvents)),
			mpVidCtx(pCtx)
			{}
		
		tEventStatus Notify(const IEventMessage& msg)
		{
			tEventType event_type = msg.GetEventType();
			if (event_type == kAudioTimeEvent)
			{
				const CAudioEventMessage& audmsg = dynamic_cast<const CAudioEventMessage&>(msg);
				const tAudioMsgTimeEvent& data = audmsg.audioMsgData.timeEvent;
				gAudioTime = data.playtime;
			}
			else if (event_type == kAudioCompletedEvent)
			{
				const CAudioEventMessage& audmsg = dynamic_cast<const CAudioEventMessage&>(msg);
				const tAudioMsgAudioCompleted& data = audmsg.audioMsgData.audioCompleted;
				gAudioDone = data.count;
			}
			return kEventStatusOK;
		}
	};
}

//============================================================================
// Video task thread Implementation
//============================================================================

//----------------------------------------------------------------------------
inline U32 GetMSecs(void)
{
	CKernelMPI	kernel;
#if 0
	U32			usec;
	kernel.GetHRTAsUsec(usec);
	return usec / 1000;
#else
	return kernel.GetElapsedTimeAsMSecs();
#endif
}

//----------------------------------------------------------------------------
void* VideoTaskMain( void* arg )
{
	CDebugMPI	dbg(kGroupVideo);
	CVideoMPI	vidmgr;
	CAudioMPI	audmgr;
	CKernelMPI	kernel;
	CDisplayMPI dispmgr;
	CEventMPI	evntmgr;
	tVideoContext*	pctx = static_cast<tVideoContext*>(arg);
	tVideoTime		vtm,vtm0 = {0, 0};
	U32				basetime,marktime,nexttime,lapsetime = pctx->uFrameTime;
	U32				flags = (pctx->pListener) ? kAudioOptionsDoneMsgAfterComplete : 0;
	Boolean			bAudio = false;
	Boolean			bListener = false; //pctx->pPathAudio != NULL;
	IEventListener*	pListener = pctx->pListener;
	tAudioPayload	payload = 0;
	
	// Sync video thread startup with InitVideoTask()
	bRunning = pctx->bPlaying = true;
	dbg.DebugOut( kDbgLvlImportant, "VideoTask Started...\n" );

	// Register our local listener to handle audio time events
	if (bListener) {
		pVideoListener = new VideoListener(pctx);
		evntmgr.RegisterEventListener(pVideoListener);
		flags |= kAudioOptionsTimeEvent | kAudioOptionsDoneMsgAfterComplete;
		payload = lapsetime;
		pListener = pVideoListener;
	}
	
	while (bRunning)
	{
		// Pre-render the 1st video frame prior to audio startup
		vidmgr.GetVideoFrame(pctx->hVideo, &vtm);
		vidmgr.PutVideoFrame(pctx->hVideo, pctx->pSurfVideo);
		dispmgr.Invalidate(0, NULL);
		// Start audio playback and sync each video frame to audio time stamp
		if (pctx->pPathAudio != NULL)
			pctx->hAudio = audmgr.StartAudio(*pctx->pPathAudio, 100, 1, 0, pListener, payload, flags);
		bAudio = (pctx->hAudio != kNoAudioID) ? true : false; // for drop-frame sync
		vtm.time = basetime = marktime = nexttime = gAudioTime = 0;
		if (!bAudio)
			basetime = marktime = nexttime = kernel.GetElapsedTimeAsMSecs();
		marktime += lapsetime;
		while (bRunning && vidmgr.SyncVideoFrame(pctx->hVideo, &vtm, false))
		{	
			vidmgr.PutVideoFrame(pctx->hVideo, pctx->pSurfVideo);
			dispmgr.Invalidate(0, NULL);
			while (bRunning) {
				static U32 lasttime = 0xFFFFFFFF;
				static U32 counter = 0;
				pthread_testcancel();
				if (pVideoListener)
					nexttime = gAudioTime;
				else if (bAudio)
					nexttime = audmgr.GetAudioTime(pctx->hAudio);
				else
					nexttime = kernel.GetElapsedTimeAsMSecs();
				if (nexttime >= marktime)
					break;
				// Detect frozen audio time stamps at end of audio
				if (bAudio) {
					if (lasttime == nexttime)
						counter++;
					else
						counter = 0;
					if (counter > lapsetime)
						break;
					lasttime = nexttime;
				}
				else if (abs(nexttime - marktime) > 60000) {
					nexttime = kernel.GetElapsedTimeAsMSecs();
					basetime = nexttime - vtm.time;
					marktime = nexttime + lapsetime;
				}
				kernel.TaskSleep(1);
			}
			// Next target time is relative to current frame time stamp
			marktime = vtm.time + basetime + lapsetime;
			if (bAudio)
				vtm.time = nexttime;
			if (pctx->bPaused)
			{
				if (pctx->hAudio != kNoAudioID)
					audmgr.PauseAudio(pctx->hAudio);
				while (bRunning && pctx->bPaused)
					kernel.TaskSleep(1);
				if (pctx->hAudio != kNoAudioID)
					audmgr.ResumeAudio(pctx->hAudio);
				if (!bAudio) {
					nexttime = kernel.GetElapsedTimeAsMSecs();
					basetime = nexttime - vtm.time;
					marktime = nexttime + lapsetime;
				}
			}
		}
		if (pctx->hAudio != kNoAudioID)
			audmgr.StopAudio(pctx->hAudio, false);
		// Reloop from 1st video frame if selected, or exit thread
		if (bRunning && pctx->bLooped)
			pctx->bPlaying = vidmgr.SeekVideoFrame(pctx->hVideo, &vtm0);
		else
			pctx->bPlaying = false;
		if (!bRunning)
			break;
		bRunning = pctx->bPlaying;
	}

	// Post done message to event listener
	if (pctx->pListener) 
	{
		tVideoMsgData	data;
		data.hVideo = pctx->hVideo;
		data.isDone = true;
		data.timeStamp = vtm;
		CVideoEventMessage msg(data);
		evntmgr.PostEvent(msg, 0, pctx->pListener);
	}

	// Unregister our local listener
	if (pVideoListener) {
		evntmgr.UnregisterEventListener(pVideoListener);
		delete pVideoListener;
		pVideoListener = NULL;
	}

	// Sync video thread shutdown with DeInitVideoTask(), unless we exit ourself normally
	bStopping = true;
	dbg.DebugOut( kDbgLvlImportant, "VideoTask Stopping...\n" );
	return kNull;
}

//----------------------------------------------------------------------------
tErrType InitVideoTask( tVideoContext* pCtx )
{
	CDebugMPI	dbg(kGroupVideo);
	CKernelMPI	kernel;
	tErrType	r;
	tTaskHndl 	hndl;
	tTaskProperties prop;

	// Set thread state prior to task creation
	bRunning = bStopping = false;
	
	// Setup task properties
	prop.TaskMainFcn = (void* (*)(void*))VideoTaskMain;
	prop.taskMainArgCount = 1;
	prop.pTaskMainArgValues = pCtx;
	r = kernel.CreateTask( hndl, prop, NULL );
	dbg.Assert( kNoErr == r, "InitVideoTask: Failed to create VideoTask!\n" );

	// Save task handle for cleanup
	pCtx->hVideoThread = hVideoThread = hndl;
	while (!bRunning)
		kernel.TaskSleep(1);

	return r;
}

//----------------------------------------------------------------------------
tErrType DeInitVideoTask( tVideoContext* pCtx )
{
	CKernelMPI	kernel;
	void* 		retval;
	int			count = 10;

	if (hVideoThread == kNull)
		return kNoErr;

	// Stop running task, if it hasn't already stopped itself
	bRunning = false;
	while (!bStopping && --count)
		kernel.TaskSleep(10);
	if (!bStopping)
		kernel.CancelTask(hVideoThread);
	kernel.JoinTask(hVideoThread, retval);
	pCtx->hVideoThread = hVideoThread = kNull;
	
	return kNoErr;
}

LF_END_BRIO_NAMESPACE()

// EOF
