#ifndef LF_BRIO_VPUPLAYER_H
#define LF_BRIO_VPUPLAYER_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		VPUPlayer.h
//
// Description:
//		Defines the VPUPlayer class derived from AVIPlayer.
//
//==============================================================================
#include <SystemTypes.h>
#include <VideoTypes.h>
#include <VideoPlayer.h>
#include <AVIPlayer.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Constants
//==============================================================================

//==============================================================================
// Typedefs
//==============================================================================

//==============================================================================
class CVPUPlayer : public CAVIPlayer {
public:	
	CVPUPlayer();
	~CVPUPlayer();
	Boolean			InitVideo(tVideoHndl hVideo);
    Boolean			DeInitVideo(tVideoHndl hVideo);
	Boolean 		GetVideoFrame(tVideoHndl hVideo, void* pCtx);
	Boolean 		PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pCtx);
	Boolean 		GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo);
	Boolean 		GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime);
	Boolean 		SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop);
	Boolean 		SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bExact, Boolean bUpdateVideoDisplay);
	S64 			GetVideoLength(tVideoHndl hVideo);
	bool 			GetNextFrame(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, int iVideoStream, AVFrame *pFrame);
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_VPUPLAYER_H

// EOF
