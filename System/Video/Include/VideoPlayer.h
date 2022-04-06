#ifndef LF_BRIO_VIDEOPLAYER_H
#define LF_BRIO_VIDEOPLAYER_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		VideoPlayer.h
//
// Description:
//		Defines the base VideoPlayer class. 
//
//==============================================================================
#include <SystemTypes.h>
#include <VideoTypes.h>
#include <DebugMPI.h>
#include <KernelMPI.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Constants
//==============================================================================

//==============================================================================
// Typedefs
//==============================================================================

//==============================================================================
class CVideoPlayer {
public:	
	CVideoPlayer();
	virtual ~CVideoPlayer();
	virtual Boolean			InitVideo(tVideoHndl hVideo);
    virtual Boolean			DeInitVideo(tVideoHndl hVideo);
	virtual Boolean 		GetVideoFrame(tVideoHndl hVideo, void* pCtx);
	virtual Boolean 		PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pCtx);
	virtual Boolean 		GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo);
	virtual Boolean 		GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime);
	virtual Boolean 		SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop);
	virtual Boolean 		SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bExact, Boolean bUpdateVideoDisplay);
	virtual S64 			GetVideoLength(tVideoHndl hVideo);

protected:
	CDebugMPI			dbg_;
	CKernelMPI			kernel_;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_VIDEOPLAYER_H

// EOF
