//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		VideoPlayer.cpp
//
// Description:
//		Implementation of the GStreamerPlayer class derived from VideoPlayer.
//
//==============================================================================

#include <SystemTypes.h>
#include <VideoTypes.h>
#include <VideoPlayer.h>
#include <VideoPriv.h>
#include <GStreamerPlayer.h>

#include <gst/gst.h>
#include <gst/video/gstvideosink.h>
#include <gst/video/video.h>
#include <string.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Local Implementation for VideoSink derived classes
//==============================================================================

static GstVideoSinkClass*   parentClass = NULL;

enum VideoFormat {
    VideoFormat_YUV,
    VideoFormat_RGB
};

class VideoSinkBase
{
public:
    GstVideoSink    videoSink;
    gint            width;
    gint            height;
    gint            bpp;
    gint            depth;
    gint			pitch;
    guchar*			buffer;
};

template <VideoFormat FMT>
class VideoSink : public VideoSinkBase
{
public:
    static GstCaps* get_caps(GstBaseSink* sink);
    static gboolean set_caps(GstBaseSink* sink, GstCaps* caps);
    static GstStateChangeReturn change_state(GstElement* element, GstStateChange transition);
    static GstFlowReturn render(GstBaseSink* sink, GstBuffer* buf);
    static void base_init(gpointer g_class);
    static void instance_init(GTypeInstance *instance, gpointer g_class);
};

template <VideoFormat FMT>
struct VideoSinkClass
{
    GstVideoSinkClass   parent_class;
    static void class_init(gpointer g_class, gpointer class_data);
    static GType get_type();
    static const char* get_name();
};

template <VideoFormat FMT>
GstCaps* VideoSink<FMT>::get_caps(GstBaseSink* sink)
{
    return 0;
}

template <>
const char* VideoSinkClass<VideoFormat_YUV>::get_name()
{
    return "VideoSinkYUV";
}

template <>
const char* VideoSinkClass<VideoFormat_RGB>::get_name()
{
    return "VideoSinkRGB";
}

template <VideoFormat FMT>
gboolean VideoSink<FMT>::set_caps(GstBaseSink* sink, GstCaps* caps)
{
    GstStructure*       data;
    VideoSink<FMT> *self = G_TYPE_CHECK_INSTANCE_CAST(sink, VideoSinkClass<FMT>::get_type(), VideoSink<FMT>);

    data = gst_caps_get_structure(caps, 0);

    gst_structure_get_int(data, "width", &self->width);
    gst_structure_get_int(data, "height", &self->height);
    gst_structure_get_int(data, "bpp", &self->bpp);
    gst_structure_get_int(data, "depth", &self->depth);
    return TRUE;
}

template <VideoFormat FMT>
GstStateChangeReturn VideoSink<FMT>::change_state(GstElement* element, GstStateChange transition)
{
    return GST_ELEMENT_CLASS(parentClass)->change_state(element, transition);
}

template <VideoFormat FMT>
GstFlowReturn VideoSink<FMT>::render(GstBaseSink* sink, GstBuffer* buf)
{
    GstFlowReturn rc = GST_FLOW_OK;

    if (buf != 0)
    {
        VideoSink<FMT> *self = G_TYPE_CHECK_INSTANCE_CAST(sink, VideoSinkClass<FMT>::get_type(), VideoSink<FMT>);
        guchar* psrc = buf->data;
        guchar* pdst = self->buffer;
        for (int y = 0; y < self->height; y++) {
        	memcpy(pdst, psrc, self->width);
        	psrc += self->width;
        	pdst += self->pitch;
        }
        pdst = self->buffer + self->pitch/2;
        for (int u = 0; u < self->height/2; u++) {
        	memcpy(pdst, psrc, self->width/2);
        	psrc += self->width/2;
        	pdst += self->pitch;
        }
        pdst = self->buffer + self->pitch/2 + self->pitch*self->height/2;
        for (int v = 0; v < self->height/2; v++) {
        	memcpy(pdst, psrc, self->width/2);
        	psrc += self->width/2;
        	pdst += self->pitch;
        }
    }
    else
        rc = GST_FLOW_ERROR;
    return rc;
}

static GstStaticPadTemplate template_factory_yuv =
    GST_STATIC_PAD_TEMPLATE("sink",
                            GST_PAD_SINK,
                            GST_PAD_ALWAYS,
                            GST_STATIC_CAPS("video/x-raw-yuv, "
                                            "framerate = (fraction) [ 0, MAX ], "
                                            "width = (int) [ 1, MAX ], "
                                            "height = (int) [ 1, MAX ],"
                                            "bpp = (int) 32"));

static GstStaticPadTemplate template_factory_rgb =
    GST_STATIC_PAD_TEMPLATE("sink",
                            GST_PAD_SINK,
                            GST_PAD_ALWAYS,
#if 0
                            GST_STATIC_CAPS("video/x-raw-rgb, "
                                            "framerate = (fraction) [ 0, MAX ], "
                                            "width = (int) [ 1, MAX ], "
                                            "height = (int) [ 1, MAX ],"
                                            "bpp = (int) 32"));
#else
                            GST_STATIC_CAPS(GST_VIDEO_CAPS_xRGB_HOST_ENDIAN));
#endif

template <VideoFormat FMT>
struct template_factory;

template <>
struct template_factory<VideoFormat_YUV>
{
    static GstStaticPadTemplate *getFactory()
    {
        return &template_factory_yuv;
    }
};

template <>
struct template_factory<VideoFormat_RGB>
{
    static GstStaticPadTemplate *getFactory()
    {
        return &template_factory_rgb;
    }
};

template <VideoFormat FMT>
void VideoSink<FMT>::base_init(gpointer g_class)
{
    gst_element_class_add_pad_template(GST_ELEMENT_CLASS(g_class),
                                       gst_static_pad_template_get(template_factory<FMT>::getFactory()));
}

template <VideoFormat FMT>
void VideoSink<FMT>::instance_init(GTypeInstance *instance, gpointer g_class)
{
    VideoSink<FMT>* self = reinterpret_cast<VideoSink<FMT>*>(instance);

    self->width = 0;
    self->height = 0;
    self->bpp = 0;
    self->depth = 0;
    self->pitch = 0;
    self->buffer = NULL;
}

// VideoSinkClass
template <VideoFormat FMT>
void VideoSinkClass<FMT>::class_init(gpointer g_class, gpointer class_data)
{
    GstBaseSinkClass*   gstBaseSinkClass = (GstBaseSinkClass*)g_class;
    GstElementClass*    gstElementClass = (GstElementClass*)g_class;

    parentClass = reinterpret_cast<GstVideoSinkClass*>(g_type_class_peek_parent(g_class));

    // base
    gstBaseSinkClass->set_caps = VideoSink<FMT>::set_caps;
    gstBaseSinkClass->preroll = VideoSink<FMT>::render;
    gstBaseSinkClass->render = VideoSink<FMT>::render;

    // element
    gstElementClass->change_state = VideoSink<FMT>::change_state;
}

template <VideoFormat FMT>
GType VideoSinkClass<FMT>::get_type()
{
    static GType type = 0;

    if (type == 0)
    {
        static const GTypeInfo info =
        {
            sizeof(VideoSinkClass<FMT>),                 // class_size
            VideoSink<FMT>::base_init,                   // base init
            NULL,                                        // base_finalize

            VideoSinkClass<FMT>::class_init,             // class_init
            NULL,                                        // class_finalize
            NULL,                                        // class_data

            sizeof(VideoSink<FMT>),                      // instance_size
            0,                                           // n_preallocs
            VideoSink<FMT>::instance_init,               // instance_init
            0                                            // value_table
        };

        type = g_type_register_static(GST_TYPE_VIDEO_SINK,
                                      VideoSinkClass<FMT>::get_name(),
                                      &info,
                                      GTypeFlags(0));
    }
    return type;
}

GType get_type_YUV()
{
    return VideoSinkClass<VideoFormat_YUV>::get_type();
}

GType get_type_RGB()
{
    return VideoSinkClass<VideoFormat_RGB>::get_type();
}

//==============================================================================
// Local Implementation for callback handling
//==============================================================================

/* playbin2 plugin flags */
typedef enum {
  GST_PLAY_FLAG_VIDEO         = (1 << 0), /* We want video output */
  GST_PLAY_FLAG_AUDIO         = (1 << 1), /* We want audio output */
  GST_PLAY_FLAG_TEXT          = (1 << 2)  /* We want subtitle output */
} GstPlayFlags;

//----------------------------------------------------------------------------
static gboolean bus_call(GstBus *bus,
          GstMessage *msg,
          gpointer    data)
{
	GMainLoop *loop = (GMainLoop *) data;

	switch (GST_MESSAGE_TYPE(msg)) {

	case GST_MESSAGE_EOS:
		g_print("End of stream\n");
		g_main_loop_quit(loop);
		break;

	case GST_MESSAGE_ERROR: {
		gchar  *debug;
		GError *error;

		gst_message_parse_error(msg, &error, &debug);
		g_free(debug);

		g_printerr("Error: %s\n", error->message);
		g_error_free(error);

		g_main_loop_quit(loop);
		break;
		}

	case GST_MESSAGE_STATE_CHANGED: {
		GstState old_state, new_state, pending_state;
		gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
		g_print("State change: %d to %d, pending %d\n", old_state, new_state, pending_state);
		break;
		}

	default:
		break;
	}

	return TRUE;
}

//----------------------------------------------------------------------------
static void on_pad_added(GstElement *element,
              GstPad     *pad,
              gpointer    data)
{
	GstPad *sinkpad;
	GstElement *decoder = (GstElement *) data;

	// We can now link this pad with the vorbis-decoder sink pad
	g_print("Dynamic pad created, linking demuxer %s to decoder %s\n",
			GST_ELEMENT_NAME(element), GST_PAD_NAME(pad));

	sinkpad = gst_element_get_static_pad(decoder, "sink");

	if (!gst_pad_is_linked(sinkpad))
		gst_pad_link(pad, sinkpad);

	gst_object_unref(sinkpad);
}

//==============================================================================
// Implementation
//==============================================================================

//----------------------------------------------------------------------------
CGStreamerPlayer::CGStreamerPlayer()
	: m_videoSink(NULL), m_videoBin(NULL), m_audioBin(NULL), pipeline(NULL), bus(NULL)
{
	gst_init(NULL, NULL);
}

//----------------------------------------------------------------------------
CGStreamerPlayer::~CGStreamerPlayer()
{
	gst_deinit();
}

//----------------------------------------------------------------------------
Boolean	CGStreamerPlayer::InitVideo(tVideoHndl hVideo)
{
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);
	FILE*			file = pVidCtx->pFileVideo;
	CPath*			path = pVidCtx->pPathVideo;
	tVideoSurf*		surf = pVidCtx->pSurfVideo;

    // Create custom video sink for YUV rendering
    if ((m_videoSink = GST_ELEMENT(g_object_new(get_type_YUV(), NULL)))) {
        gst_object_ref(GST_OBJECT(m_videoSink)); // Take ownership
        gst_object_sink(GST_OBJECT(m_videoSink));
        VideoSinkBase*  sink = reinterpret_cast<VideoSinkBase*>(m_videoSink);
        if (surf) {
			sink->width  = surf->width;
			sink->height = surf->height;
			sink->pitch  = surf->pitch;
			sink->buffer = surf->buffer;
        }
    }
    else {
    	dbg_.DebugOut(kDbgLvlCritical, "GStreamerPlayer::InitVideo: video sink failed\n");
    	return false;
    }

    // Create video pipeline
    m_videoBin = gst_bin_new(NULL);
    if (!m_videoBin) {
    	dbg_.DebugOut(kDbgLvlCritical, "GStreamerPlayer::InitVideo: video bin failed\n");
    	return false;
    }
    gst_object_ref(GST_OBJECT(m_videoBin)); // Take ownership
    gst_object_sink(GST_OBJECT(m_videoBin));

    // The videoplug element is the final element before the pluggable videosink
    m_videoplug = gst_element_factory_make("identity", NULL);

    // Colorspace ensures that the output of the stream matches the input format accepted by our video sink
    m_colorspace = gst_element_factory_make("ffmpegcolorspace", NULL);

    // We need a queue to support the tee from parent node
    GstElement *queue = gst_element_factory_make("queue", NULL);

    if (!m_videoBin || !m_videoplug || !m_colorspace || !queue) {
    	dbg_.DebugOut(kDbgLvlCritical, "GStreamerPlayer::InitVideo: video pipeline elements missing\n");
    	return false;
    }

    // Link video pipeline elements as single bin element
    gst_bin_add_many(GST_BIN(m_videoBin), queue, m_colorspace, m_videoplug, m_videoSink, (const char*)NULL);
    gst_element_link_many(queue, m_colorspace, m_videoplug, m_videoSink, (const char*)NULL);

    // Expose sink pad on video bin element
    GstPad *videopad = gst_element_get_pad(queue, "sink");
    gst_element_add_pad(m_videoBin, gst_ghost_pad_new("sink", videopad));
    gst_object_unref(videopad);

    // Create audio pipeline (per gstreamer example)
    // http://docs.gstreamer.com/display/GstSDK/Playback+tutorial+7%3A+Custom+playbin2+sinks
    GstElement *conv, *sink;

    // Create gstreamer playbin pipeline to auto-construct demuxer/decoders
    pipeline = gst_element_factory_make("playbin2", "gstreamer-player");

    // Set playbin for audio & video streams only
    gint flags = 0;
    g_object_get(pipeline, "flags", &flags, NULL);
    flags |= GST_PLAY_FLAG_VIDEO | GST_PLAY_FLAG_AUDIO;
    flags &= ~GST_PLAY_FLAG_TEXT;
    g_object_set(pipeline, "flags", flags, NULL);

    // we set the input filename to the source element
    CURI uri = CURI("file://") + CURI(*path);
    g_object_set(G_OBJECT(pipeline), "uri", uri.c_str(), NULL);

    // we add a message handler
    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));

    // Create audio bin element for audio output pipeline
    m_audioBin = gst_bin_new(NULL);
    if (!m_audioBin) {
    	dbg_.DebugOut(kDbgLvlCritical, "GStreamerPlayer::InitVideo: audio bin failed\n");
    	return false;
    }
    gst_object_ref(GST_OBJECT(m_audioBin)); // Take ownership
    gst_object_sink(GST_OBJECT(m_audioBin));

    GstElement* audioplug = gst_element_factory_make("identity", NULL);
    conv     = gst_element_factory_make("audioconvert",  "converter");
    sink     = gst_element_factory_make("autoaudiosink", "audio-output");

    if (!m_audioBin || !audioplug || !conv || !sink) {
    	dbg_.DebugOut(kDbgLvlCritical, "GStreamerPlayer::InitVideo: audio pipeline elements missing\n");
    	return false;
    }

    // we add all elements into the pipeline
    // file-source | ogg-demuxer | vorbis-decoder | converter | alsa-output
    gst_bin_add_many(GST_BIN(m_audioBin),
                     audioplug, conv, sink, NULL);

    // we link the elements together
    // file-source -> ogg-demuxer ~> vorbis-decoder -> converter -> alsa-output
    gst_element_link_many(audioplug, conv, sink, NULL);

    // Expose sink pad on audio bin element
    GstPad *audiopad = gst_element_get_pad(audioplug, "sink");
    gst_element_add_pad(m_audioBin, gst_ghost_pad_new("sink", audiopad));
    gst_object_unref(audiopad);

    // Connect audio pipeline bin as playbin sink
    g_object_set(G_OBJECT(pipeline), "audio-sink", m_audioBin, (const char*)NULL);

    // Connect video pipeline bin as playbin sink
    g_object_set(G_OBJECT(pipeline), "video-sink", m_videoBin, (const char*)NULL);

    // Set the pipeline to "playing" state
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Wait for pipeline to change state before querying video stream info
	GstState old_state, new_state, pending_state;
	for (int i = 0; i < 10; ) {
		GstMessage* msg = gst_bus_timed_pop_filtered(bus, 100 * GST_MSECOND, GST_MESSAGE_STATE_CHANGED);
		if (msg && GST_MESSAGE_SRC(msg) == GST_OBJECT(pipeline)) {
			gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
			if (new_state == GST_STATE_PLAYING)
				break;
		}
		if (msg == NULL)
			i++;
	}

	// GStreamer pipeline should be ready
	if (new_state != GST_STATE_PLAYING) {
    	dbg_.DebugOut(kDbgLvlCritical, "GStreamerPlayer::InitVideo: pipeline not ready\n");
    	return false;
	}

    // Cache video info
    GetVideoInfo(hVideo, &pVidCtx->info);

    return true;
}

//----------------------------------------------------------------------------
Boolean	CGStreamerPlayer::DeInitVideo(tVideoHndl hVideo)
{
	if (pipeline) {
		gst_element_set_state(pipeline, GST_STATE_NULL);
		gst_object_unref(GST_OBJECT(pipeline));
		pipeline = NULL;
	}
	if (m_videoBin) {
		gst_object_unref(GST_OBJECT(m_videoBin));
		m_videoBin = NULL;
	}
	if (m_audioBin) {
		gst_object_unref(GST_OBJECT(m_audioBin));
		m_audioBin = NULL;
	}
	if (m_videoSink) {
		gst_object_unref(GST_OBJECT(m_videoSink));
		m_videoSink = NULL;
	}
	if (bus) {
		gst_object_unref(bus);
		bus = NULL;
	}
	return true;
}

//----------------------------------------------------------------------------
Boolean CGStreamerPlayer::GetVideoFrame(tVideoHndl hVideo, void* pCtx)
{
    gst_element_send_event(m_videoBin,
    		gst_event_new_step(GST_FORMAT_BUFFERS, 1, 1.0, TRUE, FALSE));
	return true;
}

//----------------------------------------------------------------------------
Boolean CGStreamerPlayer::PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pCtx)
{
	// handled in VideoSink.render()
	return false;
}

//----------------------------------------------------------------------------
Boolean CGStreamerPlayer::GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo)
{
	GstPad *pad = gst_element_get_static_pad(m_videoBin, "sink");
	GstCaps *caps = gst_pad_get_negotiated_caps(pad);
	if (!caps)
		caps = gst_pad_get_caps_reffed(pad);
	if (caps) {
		GstStructure *data = gst_caps_get_structure(caps, 0);
		gint x, y;
		if (gst_structure_get_int(data, "width", &x))
			pInfo->width = x;
		if (gst_structure_get_int(data, "height", &y))
			pInfo->height = y;
		if (gst_structure_get_fraction(data, "framerate", &x, &y))
			pInfo->fps = x / y;
		gst_caps_unref(caps);
		gst_object_unref(pad);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------
Boolean CGStreamerPlayer::GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime)
{
	Boolean ret = false;
	gint64 position;
	GstFormat format = GST_FORMAT_TIME;
	if (ret = gst_element_query_position(pipeline, &format, &position))
		pTime->time = GST_TIME_AS_MSECONDS(position);
	format = GST_FORMAT_DEFAULT;
	if (ret = gst_element_query_position(pipeline, &format, &position))
		pTime->frame = position;
	return ret;
}

//----------------------------------------------------------------------------
Boolean CGStreamerPlayer::SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop)
{
	gint64 position = pCtx->frame;
	GstFormat format = GST_FORMAT_DEFAULT;
	GstSeekFlags flags = GST_SEEK_FLAG_FLUSH | (bDrop) ? GST_SEEK_FLAG_SKIP : GST_SEEK_FLAG_ACCURATE;
	if (gst_element_seek_simple(pipeline, format, flags, position))
		return true;
	return false;
}

//----------------------------------------------------------------------------
Boolean CGStreamerPlayer::SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bExact, Boolean bUpdateVideoDisplay)
{
	gint64 position = pCtx->frame;
	GstFormat format = GST_FORMAT_DEFAULT;
	GstSeekFlags flags = GST_SEEK_FLAG_FLUSH | (bExact) ? GST_SEEK_FLAG_ACCURATE : GST_SEEK_FLAG_KEY_UNIT;
	if (gst_element_seek_simple(pipeline, format, flags, position))
		return true;
	return false;
}

//----------------------------------------------------------------------------
S64 CGStreamerPlayer::GetVideoLength(tVideoHndl hVideo)
{
	gint64 duration;
	GstFormat format = GST_FORMAT_TIME;
	if (gst_element_query_duration(pipeline, &format, &duration))
		return GST_TIME_AS_MSECONDS(duration);
	return 0;
}

LF_END_BRIO_NAMESPACE()	

// EOF
