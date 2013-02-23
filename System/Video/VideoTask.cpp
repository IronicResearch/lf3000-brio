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

#if USE_GSTREAMER
#include <GStreamerPlayer.h>
#endif

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
	CKernelMPI	kernel;
	CDisplayMPI dispmgr;
	tVideoContext*	pctx = static_cast<tVideoContext*>(arg);
	CVideoModule*	vidmgr = pctx->pModule;
	CEventMPI*		evntmgr = pctx->pEventMPI;
	CAudioMPI*		audmgr = pctx->pAudioMPI;
	tVideoTime		vtm,vtm0 = {0, 0};
	U32				basetime,marktime,nexttime,lapsetime = pctx->uFrameTime;
	U32				flags = (pctx->pListener) ? kAudioOptionsDoneMsgAfterComplete : 0;
	Boolean			bAudio = false;
	Boolean			bListener = false; //pctx->pPathAudio != NULL;
	IEventListener*	pListener = pctx->pListener;
	tAudioPayload	payload = 0;
	tVideoSurf*		pSurf = pctx->pSurfVideo;
	tVideoSurf		aSurf[2];
	tDisplayHandle	aHndl[2];
	int				ibuf = 0;
	bool			bDoubleBuffered = false;
	U32				zerotime = kernel.GetElapsedTimeAsMSecs();
	
	// Sync video thread startup with InitVideoTask()
	bRunning = pctx->bPlaying = true;
	dbg.DebugOut( kDbgLvlImportant, "VideoTask Started...(%p)\n", pctx );

	// Register our local listener to handle audio time events
	if (bListener) {
		pVideoListener = new VideoListener(pctx);
		evntmgr->RegisterEventListener(pVideoListener);
		flags |= kAudioOptionsTimeEvent | kAudioOptionsDoneMsgAfterComplete;
		payload = lapsetime;
		pListener = pVideoListener;
	}

	// Support double buffering by replicating YUV planar video contexts
	if (pSurf->format == kPixelFormatYUV420 && pSurf->pitch == 4096) {
		aSurf[0] = aSurf[1] = *pSurf;
		aSurf[1].buffer += 1024;
		aHndl[0] = dispmgr.CreateHandle(aSurf[0].height, aSurf[0].width, aSurf[0].format, aSurf[0].buffer);
		aHndl[1] = dispmgr.CreateHandle(aSurf[1].height, aSurf[1].width, aSurf[1].format, aSurf[1].buffer);
		bDoubleBuffered = true;
	}
	
	while (bRunning)
	{
		// Pre-render the 1st video frame prior to audio startup
		vidmgr->GetVideoFrame(pctx->hVideo, &vtm);
		vidmgr->PutVideoFrame(pctx->hVideo, pctx->pSurfVideo);
		dispmgr.Invalidate(0, NULL);
		// Start audio playback and sync each video frame to audio time stamp
		if (pctx->pPathAudio != NULL)
			pctx->hAudio = audmgr->StartAudio(*pctx->pPathAudio, 100, 1, 0, pListener, payload, flags);
		bAudio = (pctx->hAudio != kNoAudioID) ? true : false; // for drop-frame sync
		vtm.time = basetime = marktime = nexttime = gAudioTime = 0;
		zerotime = kernel.GetElapsedTimeAsMSecs();
		marktime += lapsetime;

		// Main video rendering loop
		while (bRunning && vidmgr->SyncVideoFrame(pctx->hVideo, &vtm, bAudio))
		{	
			vidmgr->PutVideoFrame(pctx->hVideo, pSurf);
			if (bDoubleBuffered) {
				// Update double buffered YUV video contexts
				dispmgr.SwapBuffers(aHndl[ibuf], false);
				ibuf ^= 1;
				pSurf = &aSurf[ibuf];
			}
			else
				dispmgr.Invalidate(0, NULL);
			pctx->bUpdateVideoDisplay = false;

			// Elapsed Timestamp loop
			while (bRunning) {
				static U32 lasttime = 0xFFFFFFFF;
				static U32 counter = 0;
				pthread_testcancel();
				if (pVideoListener)
					nexttime = gAudioTime;
				else if (bAudio && counter < lapsetime)
					nexttime = audmgr->GetAudioTime(pctx->hAudio);
				else
					nexttime = kernel.GetElapsedTimeAsMSecs() - zerotime;
				if (nexttime >= marktime)
					break;
				if(pctx->bSeeked)
					break;
				// Detect frozen audio time stamps at end of audio
				if (bAudio) {
					if (lasttime == nexttime)
						counter++;
					else
						counter = 0;
					lasttime = nexttime;
				}
				else if (abs(nexttime - marktime) > 60000) {
					zerotime = kernel.GetElapsedTimeAsMSecs() - vtm.time;
					nexttime = vtm.time;
					basetime = nexttime - vtm.time;
					marktime = nexttime + lapsetime;
				}
				kernel.TaskSleep(1);
			}
			
			//Seeking Audio here in conjunction with video before update of marktime
			if(pctx->bSeeked)
			{
				pctx->bSeeked = false;
				vidmgr->GetVideoTime(pctx->hVideo, &vtm);
				if(bAudio)
				{
					audmgr->SeekAudioTime( pctx->hAudio, vtm.time);
					nexttime = vtm.time;
					marktime = (vtm.frame + 1) * pctx->uFrameTimeNum / pctx->uFrameTimeDen + basetime;
					zerotime = kernel.GetElapsedTimeAsMSecs() - vtm.time;
				}
				else
				{
					zerotime = kernel.GetElapsedTimeAsMSecs() - vtm.time;
					nexttime = vtm.time;
					basetime = nexttime - vtm.time;
					marktime = nexttime + lapsetime;
				}
			}
			else
			{
				// Next target time is relative to current frame time stamp
				if (pctx->bFrameTimeFract)
					marktime = (vtm.frame + 1) * pctx->uFrameTimeNum / pctx->uFrameTimeDen + basetime;
				else
					marktime = vtm.time + basetime + lapsetime;
			}
			if (bAudio)
				vtm.time = nexttime;
			
			// Paused video state loop
			if (pctx->bPaused)
			{
				if (pctx->hAudio != kNoAudioID)
					audmgr->PauseAudio(pctx->hAudio);
				while (bRunning && pctx->bPaused)
				{
					kernel.TaskSleep(1);
					if(pctx->bUpdateVideoDisplay)
					{
						vidmgr->PutVideoFrame(pctx->hVideo, pSurf);
						if (bDoubleBuffered) {
							// Update double buffered YUV video contexts
							dispmgr.SwapBuffers(aHndl[ibuf], true);
							ibuf ^= 1;
							pSurf = &aSurf[ibuf];
						}
						else
							dispmgr.Invalidate(0, NULL);
						pctx->bUpdateVideoDisplay = false;
					}
				}
				
				//Double check for seek during pause
				if(pctx->bSeeked)
				{
					pctx->bSeeked = false;
					vidmgr->GetVideoTime(pctx->hVideo, &vtm);
					if(bAudio)
					{
						audmgr->SeekAudioTime( pctx->hAudio, vtm.time);
						nexttime = vtm.time;
						marktime = (vtm.frame + 1) * pctx->uFrameTimeNum / pctx->uFrameTimeDen + basetime;
						zerotime = kernel.GetElapsedTimeAsMSecs() - vtm.time;
					}
				}
				
				if (bAudio)
					vtm.time = nexttime;
				
				if (pctx->hAudio != kNoAudioID)
					audmgr->ResumeAudio(pctx->hAudio);

				// Reset relative elapsed time in any case
				zerotime = kernel.GetElapsedTimeAsMSecs() - vtm.time;
				if (!bAudio) {
					nexttime = vtm.time;
					basetime = nexttime - vtm.time;
					marktime = nexttime + lapsetime;
				}
			}
		}
		if (pctx->hAudio != kNoAudioID)
			audmgr->StopAudio(pctx->hAudio, false);
		// Reloop from 1st video frame if selected, or exit thread
		if (bRunning && pctx->bLooped)
			pctx->bPlaying = vidmgr->SeekVideoFrame(pctx->hVideo, &vtm0, true, false);
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
		evntmgr->PostEvent(msg, kVideoEventPriority, pctx->pListener);
	}

	// Unregister our local listener
	if (pVideoListener) {
		evntmgr->UnregisterEventListener(pVideoListener);
		delete pVideoListener;
		pVideoListener = NULL;
	}

	// Release double buffered contexts, if any
	if (bDoubleBuffered) {
		dispmgr.DestroyHandle(aHndl[0], true);
		dispmgr.DestroyHandle(aHndl[1], true);
	}
	
	// Sync video thread shutdown with DeInitVideoTask(), unless we exit ourself normally
	bStopping = true;
	dbg.DebugOut( kDbgLvlImportant, "VideoTask Stopping...(%p)\n", pctx );
	return pctx;
}

//----------------------------------------------------------------------------
#if USE_GSTREAMER
void* VideoTaskMainGStreamer( void* arg )
{
	CDebugMPI		dbg(kGroupVideo);
	CKernelMPI		kernel;
	CDisplayMPI 	dispmgr;
	tVideoContext*	pctx = static_cast<tVideoContext*>(arg);
	CVideoModule*	vidmgr = pctx->pModule;
	CGStreamerPlayer* player = dynamic_cast<CGStreamerPlayer*>(pctx->pPlayer);
	GstMessage* 	msg;
	tVideoTime		vtm0 = {0, 0};

	// Sync video thread startup with InitVideoTask()
	bRunning = pctx->bPlaying = true;
	dbg.DebugOut( kDbgLvlImportant, "VideoTask Started...(%p)\n", pctx );
	dispmgr.Invalidate(0, NULL);

	while (bRunning)
	{
		// GStreamer message loop
		msg = gst_bus_timed_pop_filtered(player->bus, 100 * GST_MSECOND, GST_MESSAGE_ANY);
		if (msg) {
			switch (GST_MESSAGE_TYPE(msg)) {
			case GST_MESSAGE_ERROR: {
				gchar  *debug;
				GError *error;
				gst_message_parse_error(msg, &error, &debug);
				dbg.DebugOut( kDbgLvlImportant, "Error: %s\n", error->message );
				g_free(debug);
				g_error_free(error);
				pctx->bPlaying = false;
				break;
				}
			case GST_MESSAGE_EOS:
				dbg.DebugOut( kDbgLvlImportant, "End of stream\n" );
				pctx->bPlaying = false;
				break;
			case GST_MESSAGE_STATE_CHANGED:
				if (GST_MESSAGE_SRC(msg) == GST_OBJECT(player->pipeline)) {
					GstState old_state, new_state, pending_state;
					gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
					dbg.DebugOut( kDbgLvlImportant, "State changed %d to %d\n", old_state, new_state );
				}
				break;
			}
			gst_message_unref(msg);
		}

		// VideoMPI Paused state?
		if (pctx->bPaused) {
			gst_element_set_state(player->pipeline, GST_STATE_PAUSED);
			while (pctx->bPaused)
				kernel.TaskSleep(10);
			gst_element_set_state(player->pipeline, GST_STATE_PLAYING);
		}

		// VideoMPI Playing state?
		if (pctx->bPlaying)
			continue;

		// Reloop from 1st video frame if selected, or exit thread
		if (bRunning && pctx->bLooped)
			bRunning = pctx->bPlaying = vidmgr->SeekVideoFrame(pctx->hVideo, &vtm0, true, false);
		else
			bRunning = pctx->bPlaying = false;
	}

	// Post done message to event listener
	if (pctx->pListener)
	{
		tVideoMsgData	data;
		data.hVideo = pctx->hVideo;
		data.isDone = true;
		vidmgr->GetVideoTime(data.hVideo, &data.timeStamp);
		CVideoEventMessage msg(data);
		pctx->pEventMPI->PostEvent(msg, kVideoEventPriority, pctx->pListener);
	}

	// Sync video thread shutdown with DeInitVideoTask(), unless we exit ourself normally
	bStopping = true;
	dbg.DebugOut( kDbgLvlImportant, "VideoTask Stopping...(%p)\n", pctx );
	return pctx;
}
#endif //USE_GSTREAMER

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

	pCtx->pEventMPI = new CEventMPI();
	pCtx->pAudioMPI = new CAudioMPI();
	
	// Setup task properties
	prop.TaskMainFcn = VideoTaskMain;
#if USE_GSTREAMER
	if (dynamic_cast<CGStreamerPlayer*>(pCtx->pPlayer))
		prop.TaskMainFcn = VideoTaskMainGStreamer;
#endif
	prop.taskMainArgCount = 1;
	prop.pTaskMainArgValues = pCtx;
	prop.stackSize = 4 * PTHREAD_STACK_MIN; // 64 * 1024;
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
	CDebugMPI	dbg(kGroupVideo);
	CKernelMPI	kernel;
	void* 		retval = NULL;
	int			count = 50;

	if (hVideoThread == kNull)
		return kNoErr;

	// Stop running task, if it hasn't already stopped itself
	bRunning = false;
	while (!bStopping && --count) {
		kernel.UnlockMutex(*pCtx->pMutex);
		kernel.TaskSleep(10);
		kernel.LockMutex(*pCtx->pMutex);
	}
	if (!bStopping)
		kernel.CancelTask(hVideoThread);
	kernel.JoinTask(hVideoThread, retval);
	dbg.DebugOut(kDbgLvlImportant, "%s: count=%d, retval=%p\n", __FUNCTION__, count, retval);
	delete pCtx->pAudioMPI;
	delete pCtx->pEventMPI;
	pCtx->hVideoThread = hVideoThread = kNull;
	
	return kNoErr;
}

LF_END_BRIO_NAMESPACE()

// EOF
