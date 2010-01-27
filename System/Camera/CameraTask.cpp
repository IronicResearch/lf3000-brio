//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		CameraTask.cpp
//
// Description:
//		Camera module task thread implementation.
//
//==============================================================================

#include <DebugMPI.h>
#include <CameraMPI.h>
//#include <AudioMPI.h>
#include <KernelMPI.h>
//#include <DisplayMPI.h>
//#include <EventMPI.h>
#include <CameraPriv.h>

#include <jpeglib.h>

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// Local task state variables
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	tTaskHndl		hCameraThread	= kNull;
	volatile bool	bRunning		= false;
	volatile bool	bStopping		= false;
//	tMutex			gThreadMutex = PTHREAD_MUTEX_INITIALIZER;
//	tCond			gThreadCond  = PTHREAD_COND_INITIALIZER;
//	class VideoListener*	pVideoListener = NULL;
//	volatile U32			gAudioTime = 0;
//	volatile bool			gAudioDone = false;
}

//============================================================================
// Local event listener
//============================================================================
namespace
{
	const tEventType LocalVideoEvents[] = {kAllAudioEvents};
}

//============================================================================
// Camera task thread Implementation
//============================================================================

//----------------------------------------------------------------------------
void* CameraTaskMain(void* arg)
{
	CDebugMPI			dbg(kGroupCamera);
	CKernelMPI			kernel;
	CCameraMPI			cammgr;

	tCameraContext		*pCtx 	= static_cast<tCameraContext*>(arg);
	tFrameInfo			frame;
	tBitmapInfo			image	= {kBitmapFormatError, 0, 0, 0, NULL, 0 };

	Boolean				bRet, bFile, bScreen;

	// set up save-to-file
	if(/*pCtx->path &&*/ pCtx->path.length())
	{
		bFile = true;
	}

	// set up render-to-screen
	if(pCtx->surf)
	{
		bScreen			= true;

		image.width		= pCtx->surf->width;
		image.height	= pCtx->surf->height;

		dbg.Assert((pCtx->surf->format == kPixelFormatYUV420), "CameraTask: Can only render to YUV layer!\n");
		image.format	= kBitmapFormatYCbCr888;
		image.depth		= 3;

		image.size		= (image.width * image.height * image.depth * sizeof(U8));
		image.data		= static_cast<U8*>(kernel.Malloc(image.size));
	}

	bRunning = pCtx->bStreaming = true;
	dbg.DebugOut( kDbgLvlImportant, "CameraTask Started...\n" );

	while(bRunning)
	{
		dbg.DebugOut( kDbgLvlImportant, "CameraTask running...\n" );


		bRet = cammgr.GetFrame(pCtx->hndl, &frame);

		if(bFile)
		{

		}

		if(bScreen)
		{
			bRet = cammgr.RenderFrame(&frame, pCtx->surf, &image);
		}

		bRet = cammgr.ReturnFrame(pCtx->hndl, &frame);

		bRunning = pCtx->bStreaming;
	}

	if(image.data)
	{
		kernel.Free(image.data);
		image.data = NULL;
	}

	bStopping = true;
	dbg.DebugOut( kDbgLvlImportant, "CameraTask Stopping...\n" );

	return kNull;
}

//----------------------------------------------------------------------------
tErrType InitCameraTask(tCameraContext* pCtx)
{
	CDebugMPI		dbg(kGroupCamera);
	CKernelMPI		kernel;
	tErrType		r;
	tTaskHndl 		hndl;
	tTaskProperties	prop;

	// Set thread state prior to task creation
	bRunning = bStopping = false;

	// Setup task properties
	prop.TaskMainFcn			= (void* (*)(void*))CameraTaskMain;
	prop.taskMainArgCount		= 1;
	prop.pTaskMainArgValues		= pCtx;

	r = kernel.CreateTask( hndl, prop, NULL );
	dbg.Assert( kNoErr == r, "InitCameraTask: Failed to create CameraTask!\n" );

	// Save task handle for cleanup
	pCtx->hCameraThread = hCameraThread = hndl;
	while (!bRunning)
		kernel.TaskSleep(1);

	return r;
}

//----------------------------------------------------------------------------
tErrType DeInitCameraTask(tCameraContext* pCtx)
{
	CKernelMPI	kernel;
	void* 		retval;

	if (hCameraThread == kNull)
		return kNoErr;

	// Stop running task, if it hasn't already stopped itself
	bRunning = pCtx->bStreaming = false;
	while (!bStopping)
		kernel.TaskSleep(10);
	if (!bStopping)
		kernel.CancelTask(hCameraThread);
	kernel.JoinTask(hCameraThread, retval);
	pCtx->hCameraThread = hCameraThread = kNull;

	return kNoErr;
}

LF_END_BRIO_NAMESPACE()

// EOF
