#ifndef LF_BRIO_AVIPLAYER_H
#define LF_BRIO_AVIPLAYER_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		AVIPlayer.h
//
// Description:
//		Defines the AVIPlayer class derived from VideoPlayer. 
//
//==============================================================================
#include <SystemTypes.h>
#include <VideoTypes.h>
#include <VideoPlayer.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Constants
//==============================================================================

//==============================================================================
// Typedefs
//==============================================================================

//==============================================================================
class CAVIPlayer : public CVideoPlayer {
public:	
	CAVIPlayer();
	~CAVIPlayer();
	Boolean			InitVideo(tVideoHndl hVideo);
    Boolean			DeInitVideo(tVideoHndl hVideo);
	Boolean 		GetVideoFrame(tVideoHndl hVideo, void* pCtx);
	Boolean 		PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pCtx);
	Boolean 		GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo);
	Boolean 		GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime);
	Boolean 		SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop);
	Boolean 		SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bExact);
	
private:
	AVFormatContext*	pFormatCtx;			// container context
	AVCodecContext*		pCodecCtx;			// codec context
	AVCodec*			pCodec;				// video codec
	AVFrame*			pFrame;				// video frame buffer
    int					iVideoStream;		// index of video stream
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_AVIPLAYER_H

// EOF
