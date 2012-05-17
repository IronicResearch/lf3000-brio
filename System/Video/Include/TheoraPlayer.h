#ifndef LF_BRIO_THEORAPLAYER_H
#define LF_BRIO_THEORAPLAYER_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		TheoraPlayer.h
//
// Description:
//		Defines the TheoraPlayer class derived from VideoPlayer. 
//
//==============================================================================
#include <SystemTypes.h>
#include <VideoTypes.h>
#include <VideoPlayer.h>

#include <ogg/ogg.h>
#include <theora/theora.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Constants
//==============================================================================

//==============================================================================
// Typedefs
//==============================================================================

//==============================================================================
class CTheoraPlayer : public CVideoPlayer {
public:	
	CTheoraPlayer();
	~CTheoraPlayer();
	Boolean			InitVideo(tVideoHndl hVideo);
    Boolean			DeInitVideo(tVideoHndl hVideo);
	Boolean 		GetVideoFrame(tVideoHndl hVideo, void* pCtx);
	Boolean 		PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pCtx);
	Boolean 		GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo);
	Boolean 		GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime);
	Boolean 		SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop);
	Boolean 		SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bExact, Boolean bUpdateVideoDisplay);
	S64 			GetVideoLength(tVideoHndl hVideo);
	
private:
	// Theora global state vars
	ogg_sync_state   	oy;
	ogg_page         	og;
	ogg_stream_state 	vo;
	ogg_stream_state 	to;
	theora_info      	ti;
	theora_comment   	tc;
	theora_state     	td;
	
	ogg_int64_t BinarySeekFrame(FILE* file, ogg_int64_t target_framepos, long int &left_pos, long int &right_pos);
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_THEORAPLAYER_H

// EOF
