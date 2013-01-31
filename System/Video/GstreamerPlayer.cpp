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

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Local Implementation for VideoSink derived classes
//==============================================================================

static GstVideoSinkClass*   parentClass;

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
        // memcpy(frame.data(), buf->data, buf->size);
        printf("render: data=%p, size=%d\n", buf->data, buf->size);
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
	g_print("Dynamic pad created, linking demuxer/decoder\n");

	sinkpad = gst_element_get_static_pad(decoder, "sink");

	gst_pad_link(pad, sinkpad);

	gst_object_unref(sinkpad);
}

//==============================================================================
// Implementation
//==============================================================================

//----------------------------------------------------------------------------
CGStreamerPlayer::CGStreamerPlayer()
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
    }

    // Create video pipeline
    m_videoBin = gst_bin_new(NULL);
    gst_object_ref(GST_OBJECT(m_videoBin)); // Take ownership
    gst_object_sink(GST_OBJECT(m_videoBin));

    // The videoplug element is the final element before the pluggable videosink
    m_videoplug = gst_element_factory_make("identity", NULL);

    // Colorspace ensures that the output of the stream matches the input format accepted by our video sink
    m_colorspace = gst_element_factory_make("ffmpegcolorspace", NULL);

    // Video scale is used to prepare the correct aspect ratio and scale.
    GstElement *videoScale = gst_element_factory_make("videoscale", NULL);

    // We need a queue to support the tee from parent node
    GstElement *queue = gst_element_factory_make("queue", NULL);
    gst_bin_add_many(GST_BIN(m_videoBin), queue, m_colorspace, m_videoplug, videoScale, m_videoSink,(const char*)NULL);
    gst_element_link_many(queue, m_colorspace, videoScale, m_videoplug, m_videoSink, (const char*)NULL);

    GstPad *videopad = gst_element_get_pad(queue, "sink");
    gst_element_add_pad(m_videoBin, gst_ghost_pad_new("sink", videopad));
    gst_object_unref(videopad);

    // Create audio pipeline (per gstreamer example)
    GstElement *pipeline, *source, *demuxer, *decoder, *conv, *sink;
    GstBus *bus;
    guint bus_watch_id;
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);

    // Create gstreamer elements
    pipeline = gst_pipeline_new("gstreamer-player");
    source   = gst_element_factory_make("filesrc",       "file-source");
    demuxer  = gst_element_factory_make("oggdemux",      "ogg-demuxer");
    decoder  = gst_element_factory_make("vorbisdec",     "vorbis-decoder");
    conv     = gst_element_factory_make("audioconvert",  "converter");
    sink     = gst_element_factory_make("autoaudiosink", "audio-output");

    if (!pipeline || !source || !demuxer || !decoder || !conv || !sink) {
        g_printerr("One element could not be created. Exiting.\n");
        return false; //-1;
    }

    // we set the input filename to the source element
    g_object_set(G_OBJECT(source), "location", path->c_str(), NULL);

    // we add a message handler
    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    bus_watch_id = gst_bus_add_watch(bus, bus_call, loop);
    gst_object_unref(bus);

    // we add all elements into the pipeline
    // file-source | ogg-demuxer | vorbis-decoder | converter | alsa-output
    gst_bin_add_many(GST_BIN(pipeline),
                     source, demuxer, decoder, conv, sink, NULL);

    // we link the elements together
    // file-source -> ogg-demuxer ~> vorbis-decoder -> converter -> alsa-output
    gst_element_link(source, demuxer);
    gst_element_link_many(decoder, conv, sink, NULL);
    g_signal_connect(demuxer, "pad-added", G_CALLBACK(on_pad_added), decoder);

    // Set the pipeline to "playing" state
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

	return true;
}

//----------------------------------------------------------------------------
Boolean	CGStreamerPlayer::DeInitVideo(tVideoHndl hVideo)
{
	gst_element_set_state(m_videoBin, GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(m_videoBin));
	return true;
}

//----------------------------------------------------------------------------
Boolean CGStreamerPlayer::GetVideoFrame(tVideoHndl hVideo, void* pCtx)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CGStreamerPlayer::PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pCtx)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CGStreamerPlayer::GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CGStreamerPlayer::GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CGStreamerPlayer::SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CGStreamerPlayer::SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bExact, Boolean bUpdateVideoDisplay)
{
	return false;
}

S64 CGStreamerPlayer::GetVideoLength(tVideoHndl hVideo)
{
	return 0;
}
LF_END_BRIO_NAMESPACE()	

// EOF
