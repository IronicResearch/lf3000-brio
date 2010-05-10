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

#include <AVIWrapper.h>

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// Local task state variables
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	tTaskHndl				hCameraThread	= kNull;
	volatile bool			bRunning		= false;
	volatile bool			bStopping		= false;
	tVidCapHndl				hndl			= kInvalidVidCapHndl;
	class CCameraModule*	cam				= NULL;
	volatile bool			timeout			= false;
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
static void TimerCallback(tTimerHndl arg)
{
	timeout = true;
	cam->StopVideoCapture(hndl);
}

//----------------------------------------------------------------------------
void* CameraTaskMain(void* arg)
{
	CDebugMPI			dbg(kGroupCamera);
	CKernelMPI			kernel;
	CDisplayMPI			display;
	CAudioMPI			audiomgr;
	Boolean				bSpeakerState = true;

	avi_t				*avi		= NULL;
	int					keyframe	= 0;
	U32					start, end;

	unsigned int		audio_rate	= 0;
	unsigned int		audio_chans	= 0;
	int					audio_width = 0;
	int					audio_fmt;

	tCameraContext		*pCtx 	= static_cast<tCameraContext*>(arg);
	tFrameInfo			frame;
	tBitmapInfo			image	= {kBitmapFormatError, 0, 0, 0, NULL, 0 };

	Boolean				bRet, bFile = false, bScreen = false;

	tTimerProperties	props	= {TIMER_RELATIVE_SET, {0, 0, 0, 0}};
	tTimerHndl 			timer 	= kInvalidTimerHndl;

	tVideoSurf*			pSurf = pCtx->surf;
	tVideoSurf			aSurf[2];
	tDisplayHandle		aHndl[2];
	int					ibuf = 0;
	bool				bDoubleBuffered = false;

	// these are needed to stop the recording asynchronously
	// globals are not ideal, but the timer callback doesn't take a custom parameter
	cam 	= pCtx->module;
	hndl	= pCtx->hndl;

	// set up save-to-file
	if(pCtx->path.length())
	{
		bFile	= true;

		audio_rate	= cam->micCtx_.rate;
		audio_chans	= cam->micCtx_.channels;
		audio_width	= cam->micCtx_.sbits;
		audio_fmt	= cam->XlateAudioFormatAVI(cam->micCtx_.format);

		avi	= AVI_open_output_file(const_cast<char*>(pCtx->path.c_str()));

		// fps will be reset upon completion
		AVI_set_video(avi, pCtx->fmt.fmt.pix.width, pCtx->fmt.fmt.pix.height, pCtx->fps, "MJPG");
		AVI_set_audio(avi, audio_chans, audio_rate, audio_width, audio_fmt, audio_rate * audio_width / 1000);
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

		// Support double buffering by replicating YUV planar video contexts
		if (pSurf->format == kPixelFormatYUV420 && pSurf->pitch == 4096)
		{
			aSurf[0] = aSurf[1] = *pSurf;
			aSurf[1].buffer += 1024;
			aHndl[0] = display.CreateHandle(aSurf[0].height, aSurf[0].width, aSurf[0].format, aSurf[0].buffer);
			aHndl[1] = display.CreateHandle(aSurf[1].height, aSurf[1].width, aSurf[1].format, aSurf[1].buffer);
			bDoubleBuffered = true;
		}
	}

	bRunning = pCtx->bStreaming = true;
	dbg.DebugOut( kDbgLvlImportant, "CameraTask Started...\n" );

	start = kernel.GetElapsedTimeAsMSecs();

	timeout = false;
	timer = kernel.CreateTimer(TimerCallback, props, NULL);
	props.timeout.it_value.tv_sec = pCtx->maxLength;
	props.timeout.it_value.tv_nsec = 0;
	kernel.StartTimer(timer, props);

	if(bFile)
	{
		pCtx->module->StartAudio();
	}

	// Hack to reduce audio streaming interference with video streaming
	bSpeakerState = audiomgr.GetSpeakerEqualizer();
	audiomgr.SetSpeakerEqualizer(false);

	/*
	 * This is intentionally an assignment, not a comparison.
	 * End loop when bRunning is false.
	 */
	while(bRunning = pCtx->bStreaming)
	{
		if(bFile && !pCtx->bPaused)
		{
			pCtx->module->WriteAudio(avi);
		}

		dbg.Assert((kNoErr == kernel.LockMutex(pCtx->mThread)),\
											  "Couldn't lock mutex.\n");

		if(!pCtx->module->PollFrame(pCtx->hndl))
		{
			dbg.Assert((kNoErr == kernel.UnlockMutex(pCtx->mThread)),\
													  "Couldn't unlock mutex.\n");
			continue;
		}

		if(!(bRet = pCtx->module->GetFrame(pCtx->hndl, &frame)))
		{
			dbg.Assert((kNoErr == kernel.UnlockMutex(pCtx->mThread)),\
													"Couldn't unlock mutex.\n");
			continue;
		}

		if(bScreen && !pCtx->bVPaused)
		{
			bRet = pCtx->module->RenderFrame(&frame, pSurf, &image);
			if(bRet)
			{
				if (bDoubleBuffered) {
					// Update double buffered YUV video contexts
					display.SwapBuffers(aHndl[ibuf], false);
					ibuf ^= 1;
					pSurf = &aSurf[ibuf];
				}
				else
					display.Invalidate(0);
			}
		}

		/*
		 * Write the AVI frame only if it rendered correctly.
		 */
		if(bFile && !pCtx->bPaused && bRet)
		{
			AVI_write_frame(avi, static_cast<char*>(frame.data), frame.size, keyframe++);
		}

		bRet = pCtx->module->ReturnFrame(pCtx->hndl, &frame);

		dbg.Assert((kNoErr == kernel.UnlockMutex(pCtx->mThread)),\
											  "Couldn't unlock mutex.\n");
	}

	kernel.DestroyTimer(timer);

	end = kernel.GetElapsedTimeAsMSecs();
	if(end > start)
	{
		end -= start;
	}
	else
	{
		end += (kU32Max - start);
	}

	// Restore speaker equalizer state prior to video streaming
	audiomgr.SetSpeakerEqualizer(bSpeakerState);

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
			data.length 			= end;
			msg = new CCameraEventMessage(data);
		}
		else if(pCtx->reqLength <= pCtx->maxLength)	// normal timeout
		{
			tCaptureTimeoutMsg		data;
			data.vhndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= end;
			msg = new CCameraEventMessage(data);
		}
		else										// file system capacity timeout
		{
			tCaptureQuotaHitMsg		data;
			data.vhndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= end;
			msg = new CCameraEventMessage(data);
		}

		evntmgr.PostEvent(*msg, 0, pCtx->pListener);
		delete msg;
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

	if (bDoubleBuffered)
	{
		display.DestroyHandle(aHndl[1], false);
		display.DestroyHandle(aHndl[0], false);
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
