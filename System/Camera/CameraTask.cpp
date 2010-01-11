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
#include <AudioMPI.h>
#include <KernelMPI.h>
#include <DisplayMPI.h>
#include <EventMPI.h>
#include <CameraPriv.h>

#include <jpeglib.h>

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// Local task state variables
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	tTaskHndl hCameraThread		= kNull;
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
}

//============================================================================
// Camera task thread Implementation
//============================================================================

//----------------------------------------------------------------------------
void* CameraTaskMain(void* arg)
{
	CCameraMPI cammgr;
	tCameraContext* pCtx	= static_cast<tCameraContext*>(arg);
	tFrameInfo frame;
	FILE *stream 			= NULL;

	JSAMPLE **buf			= &pCtx->surf->buffer;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

//	bRunning = true;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	cammgr.GetFrame(pCtx->hndl, &frame);

	stream = fmemopen(frame.data, frame.size, "rb");
	jpeg_stdio_src(&cinfo, stream);

	jpeg_read_header(&cinfo, TRUE);

	jpeg_start_decompress(&cinfo);
bRunning = true;
	while(cinfo.output_scanline < cinfo.output_height)
	{
		jpeg_read_scanlines(&cinfo, buf, cinfo.output_height);
	    /* Assume put_scanline_someplace wants a pointer and sample count. */
	    //put_scanline_someplace(buffer[0], row_stride);
	}

	jpeg_destroy_decompress(&cinfo);

	cammgr.PutFrame(pCtx->hndl, &frame);
	return kNull;
}

//----------------------------------------------------------------------------
tErrType InitCameraTask(tCameraContext* pCtx)
{
	CDebugMPI	dbg(kGroupCamera);
	CKernelMPI	kernel;
	tErrType	r;
	tTaskHndl 	hndl;
	tTaskProperties prop;

	// Set thread state prior to task creation
	bRunning = bStopping = false;

	// Setup task properties
	prop.TaskMainFcn = (void* (*)(void*))CameraTaskMain;
	prop.taskMainArgCount = 1;
	prop.pTaskMainArgValues = pCtx;
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
	bRunning = false;
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
