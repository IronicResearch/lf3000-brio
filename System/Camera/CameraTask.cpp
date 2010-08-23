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

#define USE_RENDER_THREAD	1	// for separate rendering thread

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// Local task state variables
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	tTaskHndl				hCameraThread	= kNull;
	tTaskHndl				hRenderThread	= kNull;
	volatile bool			bRunning		= false;
	volatile bool			bStopping		= false;
	volatile bool			bInited			= false;
	tVidCapHndl				hndl			= kInvalidVidCapHndl;
	class CCameraModule*	cam				= NULL;
	volatile bool			timeout			= false;
	volatile bool			bRendering		= false;
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
void* CameraTaskRender(void* arg)
{
	CKernelMPI			kernel;
	CDisplayMPI			display;
	tCameraContext*		pCtx = static_cast<tCameraContext*>(arg);
	tFrameInfo			frame;
	tFrameInfo*			pFrame = &frame;
	tVideoSurf*			pSurf = pCtx->surf;
	Boolean				bRet = false;
	int					ibuf = 0;
	
	while (bRunning)
	{
		if (!pCtx->qframes.empty() && !pCtx->bVPaused)
		{
			// Sync rendering thread with StopVideoCapture() and GrabFrame()
			if (kernel.TryLockMutex(pCtx->mThread2))
				continue;
			
			// Remove next frame to render from queue
			*pFrame = pCtx->qframes.front();
			pCtx->qframes.pop();
			
			bRendering = true;
			bRet = pCtx->module->RenderFrame(pFrame, pSurf, pCtx->image, pCtx->method);
			if (bRet)
			{
				if (pCtx->bDoubleBuffered) {
					// Update double buffered YUV video contexts
					display.SwapBuffers(pCtx->paHndl[ibuf], false);
					ibuf ^= 1;
					pSurf = &pCtx->paSurf[ibuf];
				}
				else
					display.Invalidate(0);
			}
			bRendering = false;
			
			// Done with queued frame and associated V4L buffer 
			bRet = pCtx->module->ReturnFrame(pCtx->hndl, pFrame);

			kernel.UnlockMutex(pCtx->mThread2);
		}
		kernel.TaskSleep(10);
	}
	
	return kNull;
}

//----------------------------------------------------------------------------
void* CameraTaskMain(void* arg)
{
	CDebugMPI			dbg(kGroupCamera);
	CKernelMPI			kernel;
	CDisplayMPI			display;
	CAudioMPI			audiomgr;

	avi_t				*avi		= NULL;
	int					keyframe	= 0;
	U32					elapsed = 0;

	unsigned int		audio_rate	= 0;
	unsigned int		audio_chans	= 0;
	int					audio_width = 0;
	int					audio_fmt;

	tCameraContext		*pCtx 	= static_cast<tCameraContext*>(arg);
	tFrameInfo			frame;
	tBitmapInfo			image	= {kBitmapFormatError, 0, 0, 0, NULL, 0 };

	Boolean				bRet, bFile = false, bScreen = false, bWasPaused = false;

	tTimerProperties	props	= {TIMER_RELATIVE_SET, {0, 0, 0, 0}};
	tTimerHndl 			timer 	= kInvalidTimerHndl;
	saveTimerSettings	oldTimer;

	tVideoSurf*			pSurf = pCtx->surf;
	tVideoSurf			aSurf[2];
	tDisplayHandle		aHndl[2];
	int					ibuf = 0;
	bool				bDoubleBuffered = false;

	JPEG_METHOD			method = JPEG_FAST;
	
	bool				bFirst = false;
	struct timeval		tv0, tvn, tvt = {0, 0};

	bool				bQueued = false;

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

		avi	= AVI_open_output_file(const_cast<char*>(pCtx->path.c_str()), pCtx->bAudio);

		// fps will be reset upon completion
		AVI_set_video(avi, pCtx->fmt.fmt.pix.width, pCtx->fmt.fmt.pix.height, pCtx->fps, "MJPG");
		if(pCtx->bAudio)
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
			/* TODO: libjpeg QVGA->QQVGA rendering + HW scaler is faster than
			 * HW IDCT QVGA->QVGA rendering
			 */
			if(pCtx->fmt.fmt.pix.width == pCtx->surf->width)
			{
				method		= JPEG_HW2;
			}
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

		// Cache pointers for use in CameraTaskRender() thread
		pCtx->frame = &frame;
		pCtx->image = &image;
		pCtx->method = method;
		pCtx->paSurf = &aSurf[0];
		pCtx->paHndl = &aHndl[0];
		pCtx->bDoubleBuffered = bDoubleBuffered;
	}

	timeout = false;
	timer = kernel.CreateTimer(TimerCallback, props, NULL);
	props.timeout.it_value.tv_sec = pCtx->maxLength;
	props.timeout.it_value.tv_nsec = 0;
	kernel.StartTimer(timer, props);

	if(bFile && pCtx->bAudio)
	{
		pCtx->module->StartAudio();
	}

	bRunning = true;
	dbg.DebugOut( kDbgLvlImportant, "CameraTask Started...\n" );

	while(bRunning)
	{
		if(0 != kernel.TryLockMutex(pCtx->mThread))
		{
			continue;
		}

		if(pCtx->bPaused && !bWasPaused)
		{
			kernel.PauseTimer(timer, oldTimer);
			bWasPaused = true;

			if(bFirst)
			{
				// Calculate difference in first and last video timestamps
				if (tvn.tv_usec < tv0.tv_usec) {
					tvn.tv_usec += 1000000;
					tvn.tv_sec--;
				}
				tvt.tv_sec	+= (tvn.tv_sec - tv0.tv_sec);
				tvt.tv_usec += (tvn.tv_usec - tv0.tv_usec);
				if(tvt.tv_usec > 1000000)
				{
					tvt.tv_usec -= 1000000;
					tvt.tv_sec++;
				}
			}
		}

		bRet = false;
		if(bFile && pCtx->bAudio && !pCtx->bPaused && bFirst)
		{
			bRet = pCtx->module->WriteAudio(avi);
		}

		if(!pCtx->module->PollFrame(pCtx->hndl))
		{
			dbg.Assert((kNoErr == kernel.UnlockMutex(pCtx->mThread)),\
													  "Couldn't unlock mutex.\n");
			// Yield timeslice if neither audio nor video input ready 
			if (!bRet)
				kernel.TaskSleep(10);
			continue;
		}

		if(!(bRet = pCtx->module->GetFrame(pCtx->hndl, &frame)))
		{
			dbg.Assert((kNoErr == kernel.UnlockMutex(pCtx->mThread)),\
													"Couldn't unlock mutex.\n");
			// Yield timeslice if neither audio nor video input ready 
			if (!bRet)
				kernel.TaskSleep(10);
			continue;
		}

		if(bScreen && !pCtx->bVPaused
				&& (!pCtx->bAudio || keyframe+1 >= cam->micCtx_.counter))
		{
#if USE_RENDER_THREAD
			// Add frame to be rendered into CameraTaskRender() queue
			pCtx->qframes.push(frame);
			bQueued = true;
#else
			bRet = pCtx->module->RenderFrame(&frame, pSurf, &image, method);
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
#endif
		}

		/*
		 * Write the AVI frame only if it rendered correctly.
		 */
		if(!pCtx->bPaused)
		{
			if(bFile && bRet)
			{
				// Duplicate video frame(s) if video frame count lags behind audio block count
				do {
					AVI_write_frame(avi, static_cast<char*>(frame.data), frame.size, keyframe++);
				} while (pCtx->bAudio && keyframe < cam->micCtx_.counter);
			}
			if (!bFirst) {
				bFirst = true;
				tv0 = pCtx->buf.timestamp;
			}
			tvn = pCtx->buf.timestamp;

			if(bWasPaused)
			{
				bWasPaused = false;
				kernel.ResumeTimer(timer, oldTimer);

				tv0 = tvn;
			}
		}

		if (!bQueued)
			bRet = pCtx->module->ReturnFrame(pCtx->hndl, &frame);
		bQueued = false;

		dbg.Assert((kNoErr == kernel.UnlockMutex(pCtx->mThread)),\
											  "Couldn't unlock mutex.\n");
	}

	if (bFile && pCtx->bAudio)
	{
		pCtx->module->StopAudio();
	}

	kernel.DestroyTimer(timer);

	if(bFirst && !bWasPaused)
	{
		// Calculate difference in first and last video timestamps
		if (tvn.tv_usec < tv0.tv_usec) {
			tvn.tv_usec += 1000000;
			tvn.tv_sec--;
		}
		tvt.tv_sec	+= (tvn.tv_sec - tv0.tv_sec);
		tvt.tv_usec += (tvn.tv_usec - tv0.tv_usec);
		if(tvt.tv_usec > 1000000)
		{
			tvt.tv_usec -= 1000000;
			tvt.tv_sec++;
		}
	}
	elapsed = 1000000 * (tvt.tv_sec) + (tvt.tv_usec);

	// Post done message to event listener
	if(pCtx->pListener)
	{
		CCameraEventMessage *msg;

		if(!pCtx->module->valid)					// camera hot-unplugged
		{
			tCameraRemovedMsg		data;
			data.vhndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= elapsed / 1000000;
			msg = new CCameraEventMessage(data);
		}
		else if(!timeout)							// manually stopped by StopVideoCapture()
		{
			tCaptureStoppedMsg		data;
			data.vhndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= elapsed / 1000000;
			msg = new CCameraEventMessage(data);
		}
		else if(pCtx->reqLength && pCtx->reqLength <= pCtx->maxLength)	// normal timeout
		{
			tCaptureTimeoutMsg		data;
			data.vhndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= elapsed / 1000000;
			msg = new CCameraEventMessage(data);
		}
		else										// file system capacity timeout
		{
			tCaptureQuotaHitMsg		data;
			data.vhndl				= pCtx->hndl;
			data.saved				= true;
			data.length 			= elapsed / 1000000;
			msg = new CCameraEventMessage(data);
		}

		pCtx->module->event_.PostEvent(*msg, 0, pCtx->pListener);
		delete msg;
	}

	while (bRendering)
		kernel.TaskSleep(10);
	
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
		float fps = (float)keyframe / ((float)elapsed / 1000000);
		if (pCtx->bAudio)
			fps = (float)keyframe * ((float)(audio_rate * audio_chans * sizeof(short)) / (float)cam->micCtx_.bytesWritten);

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

#if USE_RENDER_THREAD
	// Create additional thread just for rendering
	prop.TaskMainFcn			= (void* (*)(void*))CameraTaskRender;
	r = kernel.CreateTask( hndl, prop, NULL );
	dbg.Assert( kNoErr == r, "InitCameraTask: Failed to create CameraTaskRender!\n" );
	hRenderThread = hndl;
#endif
	
	bInited = true;
	return r;
}

//----------------------------------------------------------------------------
tErrType DeInitCameraTask(tCameraContext* pCtx)
{
	CKernelMPI	kernel;
	void* 		retval;

	if (hCameraThread == kNull)
		return kNoErr;

	while (!bInited)
		kernel.TaskSleep(1);

	// Stop running task, if it hasn't already stopped itself
	bRunning = false;
	while (!bStopping)
		kernel.TaskSleep(10);

#if USE_RENDER_THREAD
	kernel.JoinTask(hRenderThread, retval);
	hRenderThread = kNull;
#endif
	
	if (!bStopping)
		kernel.CancelTask(hCameraThread);
	kernel.JoinTask(hCameraThread, retval);
	pCtx->hCameraThread = hCameraThread = kNull;

	bInited = false;

	return kNoErr;
}

LF_END_BRIO_NAMESPACE()

// EOF
