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

#if USE_VPU

#include <SystemTypes.h>
#include <VideoTypes.h>
#include <VideoPlayer.h>
#include <AVIPlayer.h>

#include <nx_video_api.h>

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

private:
	NX_VID_DEC_HANDLE 	hDec;
	NX_VID_SEQ_IN 		seqIn;
	NX_VID_SEQ_OUT 		seqOut;
	NX_VID_DEC_IN 		decIn;
	NX_VID_DEC_OUT 		decOut;
	unsigned char*		pStreamBuffer;
	int           		reqSize;
};

LF_END_BRIO_NAMESPACE()

#endif // USE_VPU

#endif // LF_BRIO_VPUPLAYER_H

// EOF
