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
	tVideoContext*	pctx = static_cast<tVideoContext*>(arg);
	tVideoTime		vtm;
	U32				marktime,nexttime,lapsetime = 30;
	
	bRunning = true;
	dbg.DebugOut( kDbgLvlImportant, "VideoTask Started...\n" );

	while (bRunning)
	{
		pctx->hAudio = audmgr.StartAudio(pctx->hRsrcAudio, 100, 1, 0, pctx->pListener, 0, 0);
		vtm.time = marktime = audmgr.GetAudioTime(pctx->hAudio);
		marktime += lapsetime;
		while (vidmgr.SyncVideoFrame(pctx->hVideo, &vtm, true))
		{	
			vidmgr.PutVideoFrame(pctx->hVideo, pctx->pSurfVideo);
			dispmgr.Invalidate(0, NULL);
			while ((nexttime = audmgr.GetAudioTime(pctx->hAudio)) < marktime)
				kernel.TaskSleep(1);
			marktime = nexttime + lapsetime;
			vtm.time = nexttime;	
		}
		audmgr.StopAudio(pctx->hAudio, false);
		while (bRunning)
			kernel.TaskSleep(1);
	}

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
	prop.priority = 1;
	prop.schedulingPolicy = SCHED_FIFO;
		
	r = kernel.CreateTask( hndl, prop, NULL );
	dbg.Assert( kNoErr == r, "InitVideoTask: Failed to create VideoTask!\n" );

	// Save task handle for cleanup
	hVideoThread = hndl;

	return r;
}

//----------------------------------------------------------------------------
tErrType DeInitVideoTask( void )
{
	CKernelMPI	kernel;

	if (hVideoThread == kNull)
		return kNoErr;

	// Stop running task
	kernel.CancelTask(hVideoThread);
	hVideoThread = kNull;
	
	return kNoErr;
}

LF_END_BRIO_NAMESPACE()

// EOF
