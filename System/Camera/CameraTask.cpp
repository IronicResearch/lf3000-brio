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
#include <DisplayMPI.h>
//#include <EventMPI.h>
#include <CameraPriv.h>

#include <avilib.h>

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
	CDisplayMPI			display;
	CCameraMPI			cammgr;

	avi_t				*avi		= NULL;
	int					keyframe	= 0;
	U32					start, end;

	tCameraContext		*pCtx 	= static_cast<tCameraContext*>(arg);
	tFrameInfo			frame;
	tBitmapInfo			image	= {kBitmapFormatError, 0, 0, 0, NULL, 0 };

	Boolean				bRet, bFile = false, bScreen = false;

	// set up save-to-file
	if(pCtx->path.length())
	{
		bFile	= true;

		avi		= AVI_open_output_file(const_cast<char*>(pCtx->path.c_str()));
		// fps will be reset upon completion
		AVI_set_video(avi, pCtx->fmt.fmt.pix.width, pCtx->fmt.fmt.pix.height, pCtx->fps, "MJPG");
	}

	// set up render-to-screen
	if(pCtx->surf)
	{
		bScreen			= true;

		image.width		= pCtx->surf->width;
		image.height	= pCtx->surf->height;

		switch(pCtx->surf->format)
		{
		case kPixelFormatYUV420:
			image.format	= kBitmapFormatYCbCr888;
			break;
		case kPixelFormatRGB888:
			image.format	= kBitmapFormatRGB888;
			break;
		}
		image.depth		= 3;

		image.size		= (image.width * image.height * image.depth * sizeof(U8));
		image.data		= static_cast<U8*>(kernel.Malloc(image.size));

		image.buffer	= static_cast<U8**>(kernel.Malloc(image.height * sizeof(U8*)));
		for(int i = 0; i < image.height; i++)
		{
			image.buffer[i] = &image.data[i* image.width * image.depth];
		}
	}

	bRunning = pCtx->bStreaming = true;
	dbg.DebugOut( kDbgLvlImportant, "CameraTask Started...\n" );

	start = kernel.GetElapsedTimeAsMSecs();

	while(bRunning)
	{
		dbg.Assert((kNoErr == kernel.LockMutex(pCtx->mThread)),\
											  "Couldn't lock mutex.\n");

		if(!pCtx->module->PollFrame(pCtx->hndl))
		{
			dbg.Assert((kNoErr == kernel.UnlockMutex(pCtx->mThread)),\
													  "Couldn't unlock mutex.\n");
			//kernel.TaskSleep(1);
			bRunning = pCtx->bStreaming;
			continue;
		}

		bRet = pCtx->module->GetFrame(pCtx->hndl, &frame);

		if(bFile && !pCtx->bPaused && bRet)
		{
			AVI_write_frame(avi, static_cast<char*>(frame.data), frame.size, keyframe++);
		}

		if(bScreen && !pCtx->bPaused && bRet)
		{
			bRet = pCtx->module->RenderFrame(&frame, pCtx->surf, &image);
			display.Invalidate(0);
		}

		bRet = pCtx->module->ReturnFrame(pCtx->hndl, &frame);

		dbg.Assert((kNoErr == kernel.UnlockMutex(pCtx->mThread)),\
											  "Couldn't unlock mutex.\n");

		bRunning = pCtx->bStreaming;
	}

	end = kernel.GetElapsedTimeAsMSecs();
	if(end > start)
	{
		end -= start;
	}
	else
	{
		end += (kU32Max - start);
	}

	if(image.data)
	{
		kernel.Free(image.data);
		image.data = NULL;
	}
	if(image.buffer)
	{
		kernel.Free(image.buffer);
		image.buffer = NULL;
	}

	if(bFile)
	{
		float fps = keyframe / (end / 1000);
		AVI_set_video(avi, pCtx->fmt.fmt.pix.width, pCtx->fmt.fmt.pix.height, fps, "MJPG");
		AVI_close(avi);
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
