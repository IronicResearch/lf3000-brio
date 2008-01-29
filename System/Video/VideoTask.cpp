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
	
	bRunning = true;
	dbg.DebugOut( kDbgLvlImportant, "VideoTask Started...\n" );

	while (bRunning)
	{
		// Start audio playback and sync each video frame to audio time stamp
		pctx->bPlaying = true;
		if (pctx->pPathAudio != NULL)
			pctx->hAudio = audmgr.StartAudio(*pctx->pPathAudio, 100, 1, 0, pctx->pListener, 0, flags);
		bAudio = (pctx->hAudio != kNoAudioID) ? true : false; // for drop-frame sync
		vtm.time = basetime = marktime = nexttime = 0;
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
				if (bAudio)
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
		if (pctx->bLooped)
			pctx->bPlaying = vidmgr.SeekVideoFrame(pctx->hVideo, &vtm0);
		else
			pctx->bPlaying = false;
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

	bStopping = false;
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

	// Locked by mutex on entry
	
	// Setup task properties
	prop.TaskMainFcn = (void* (*)(void*))VideoTaskMain;
	prop.taskMainArgCount = 1;
	prop.pTaskMainArgValues = pCtx;
	r = kernel.CreateTask( hndl, prop, NULL );
	dbg.Assert( kNoErr == r, "InitVideoTask: Failed to create VideoTask!\n" );

	// Save task handle for cleanup
	pCtx->hVideoThread = hVideoThread = hndl;
	bRunning = bStopping = false;
	while (!bRunning)
		kernel.TaskSleep(1);

	return r;
}

//----------------------------------------------------------------------------
tErrType DeInitVideoTask( tVideoContext* pCtx )
{
	CKernelMPI	kernel;

	if (hVideoThread == kNull)
		return kNoErr;

	// Locked by mutex on entry
	
	// Stop running task
	bStopping = true;
	bRunning = false;
	int counter = 0;
	while (bStopping != bRunning) {
		kernel.TaskSleep(2);
		if (counter++ > 10)
			break;
	}
	pCtx->hVideoThread = hVideoThread = kNull;
	
	return kNoErr;
}

LF_END_BRIO_NAMESPACE()

// EOF
