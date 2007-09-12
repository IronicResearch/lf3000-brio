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
	bool		bRunning = false;
}

//============================================================================
// Video task thread Implementation
//============================================================================

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
	tVideoMsgData	data;
	U32				marktime,nexttime,lapsetime = 30;
	Boolean			bAudio = false;
	
	bRunning = true;
	dbg.DebugOut( kDbgLvlImportant, "VideoTask Started...\n" );

	while (bRunning)
	{
		// Start audio playback and sync each video frame to audio time stamp
		pctx->bPlaying = true;
		if (bAudio)
			pctx->hAudio = audmgr.StartAudio(pctx->pathAudio, 100, 1, 0, pctx->pListener, 0, 0);
		vtm.time = marktime = nexttime = 0;
		if (!bAudio)
			marktime = nexttime = kernel.GetElapsedTimeAsMSecs();
		marktime += lapsetime;
		while (bRunning && vidmgr.SyncVideoFrame(pctx->hVideo, &vtm, bAudio))
		{	
			vidmgr.PutVideoFrame(pctx->hVideo, pctx->pSurfVideo);
			dispmgr.Invalidate(0, NULL);
			while (bRunning) {
				if (bAudio)
					nexttime = audmgr.GetAudioTime(pctx->hAudio);
				else
					nexttime = kernel.GetElapsedTimeAsMSecs();
				if (nexttime >= marktime)
					break;
				kernel.TaskSleep(1);
			}
			marktime = nexttime + lapsetime;
			if (bAudio)
				vtm.time = nexttime;	
			if (pctx->bPaused)
			{
				if (bAudio)
					audmgr.PauseAudio(pctx->hAudio);
				while (bRunning && pctx->bPaused)
					kernel.TaskSleep(1);
				if (bAudio)
					audmgr.ResumeAudio(pctx->hAudio);
			}
		}
		if (bAudio)
			audmgr.StopAudio(pctx->hAudio, false);
		// Reloop from 1st video frame if selected, or exit thread
		if (pctx->bLooped)
			pctx->bPlaying = vidmgr.SeekVideoFrame(pctx->hVideo, &vtm0);
		else
			pctx->bPlaying = false;
		bRunning = pctx->bPlaying;
	}

	// Post done message to event listener
	data.hVideo = pctx->hVideo;
	data.isDone = true;
	data.timeStamp = vtm;
	CVideoEventMessage msg(data);
	evntmgr.PostEvent(msg, 0, pctx->pListener);

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

	// Setup task properties
	prop.TaskMainFcn = (void* (*)(void*))VideoTaskMain;
	prop.taskMainArgCount = 1;
	prop.pTaskMainArgValues = pCtx;
#ifndef EMULATION
	prop.priority = 1;
	prop.schedulingPolicy = SCHED_FIFO;
#endif
	r = kernel.CreateTask( hndl, prop, NULL );
	dbg.Assert( kNoErr == r, "InitVideoTask: Failed to create VideoTask!\n" );

	// Save task handle for cleanup
	hVideoThread = hndl;
	while (!bRunning)
		kernel.TaskSleep(1);

	return r;
}

//----------------------------------------------------------------------------
tErrType DeInitVideoTask( void )
{
	CKernelMPI	kernel;

	if (hVideoThread == kNull)
		return kNoErr;

	// TODO: Need real sync protection via mutexes when killing task
	//		 Letting task exit itself works most times on embedded target
	
	// Stop running task
	bRunning = false;
	kernel.TaskSleep(2);
//	kernel.CancelTask(hVideoThread);
	hVideoThread = kNull;
	
	return kNoErr;
}

LF_END_BRIO_NAMESPACE()

// EOF
