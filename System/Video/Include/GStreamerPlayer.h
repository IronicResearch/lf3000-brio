#ifndef LF_BRIO_GSTPLAYER_H
#define LF_BRIO_GSTPLAYER_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		GStreamerPlayer.h
//
// Description:
//		Defines the GStreamerPlayer class derived from VideoPlayer.
//
//==============================================================================
#include <SystemTypes.h>
#include <VideoTypes.h>
#include <VideoPlayer.h>

#include <glib.h>
#include <glib/gmacros.h>
#include <glib/gtypes.h>
#undef  G_GNUC_NULL_TERMINATED
#define G_GNUC_NULL_TERMINATED
#undef  G_GNUC_MALLOC
#define G_GNUC_MALLOC
#undef  G_GNUC_WARN_UNUSED_RESULT
#define G_GNUC_WARN_UNUSED_RESULT
typedef const gchar *   (*GTranslateFunc) (const gchar *str, gpointer data);
#include <glib/goption.h>
#include <gst/gst.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Constants
//==============================================================================

//==============================================================================
// Typedefs
//==============================================================================

//==============================================================================
class CGStreamerPlayer : public CVideoPlayer {
public:	
	CGStreamerPlayer();
	~CGStreamerPlayer();
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
    GstElement 		*m_videoBin;
    GstElement 		*m_videoBalance;
    GstElement 		*m_colorspace;
    GstElement 		*m_videoplug;
    GstElement 		*m_videoSink;
    GstElement  	*m_audioBin;
    GstElement  	*m_audioplug;
public:
    GstElement  	*pipeline;
    GstBus 			*bus;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_GSTPLAYER_H

// EOF
