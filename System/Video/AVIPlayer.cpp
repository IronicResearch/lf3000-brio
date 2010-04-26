//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		AVIPlayer.cpp
//
// Description:
//		Implementation of the AVIPlayer class derived from VideoPlayer. 
//
//==============================================================================
#include <SystemTypes.h>
#include <VideoTypes.h>
#include <VideoPlayer.h>
#include <VideoPriv.h>
#include <AVIPlayer.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
//#include <libswscale/avswscale.h>
}

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Constants
//==============================================================================

//==============================================================================
// Typedefs
//==============================================================================

//==============================================================================
// Local Variables
//==============================================================================
namespace
{
	AVFormatContext*	pFormatCtx = NULL;
	AVCodecContext*		pCodecCtx = NULL;
	AVCodec*			pCodec = NULL;
    AVFrame*			pFrame = NULL;
    int					iVideoStream = -1;
}

//==============================================================================
// Local Functions
//==============================================================================
bool GetNextFrame(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, 
    int iVideoStream, AVFrame *pFrame)
{
    static AVPacket packet;
    static int      bytesRemaining=0;
    static uint8_t  *rawData;
    static bool     fFirstTime=true;
    int             bytesDecoded;
    int             frameFinished;

    // First time we're called, set packet.data to NULL to indicate it
    // doesn't have to be freed
    if (fFirstTime)
    {
        fFirstTime=false;
        packet.data=NULL;
    }

    // Decode packets until we have decoded a complete frame
    while (true)
    {
        // Work on the current packet until we have decoded all of it
        while (bytesRemaining > 0)
        {
            // Decode the next chunk of data
            bytesDecoded = avcodec_decode_video(pCodecCtx, pFrame,
                &frameFinished, rawData, bytesRemaining);

            // Was there an error?
            if (bytesDecoded < 0)
            {
                fprintf(stderr, "Error while decoding frame\n");
                return false;
            }

            bytesRemaining -= bytesDecoded;
            rawData += bytesDecoded;

            // Did we finish the current frame? Then we can return
            if (frameFinished)
                return true;
        }

        // Read the next packet, skipping all packets that aren't for this
        // stream
        do
        {
            // Free old packet
            if (packet.data != NULL)
                av_free_packet(&packet);

            // Read new packet
            if (av_read_packet(pFormatCtx, &packet) < 0)
                goto loop_exit;
        } while(packet.stream_index != iVideoStream);

        bytesRemaining = packet.size;
        rawData = packet.data;
    }

loop_exit:

    // Decode the rest of the last frame
    bytesDecoded = avcodec_decode_video(pCodecCtx, pFrame, &frameFinished, 
        rawData, bytesRemaining);

    // Free last packet
    if (packet.data != NULL)
        av_free_packet(&packet);

    return frameFinished != 0;
}

//==============================================================================
// Implementation
//==============================================================================

//----------------------------------------------------------------------------
CAVIPlayer::CAVIPlayer()
{
}

//----------------------------------------------------------------------------
CAVIPlayer::~CAVIPlayer()
{
}

//----------------------------------------------------------------------------
Boolean	CAVIPlayer::InitVideo(tVideoHndl hVideo)
{
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);
	FILE*			file = pVidCtx->pFileVideo;
	CPath*			path = pVidCtx->pPathVideo;
	
    // Register all formats and codecs
    av_register_all();

    // Open video file
    if (av_open_input_file(&pFormatCtx, path->c_str(), NULL, 0, NULL) != 0)
        return false; // Couldn't open file

    // Retrieve stream information
    if (av_find_stream_info(pFormatCtx) < 0)
        return false; // Couldn't find stream information

    // Dump information about file onto standard error
    dump_format(pFormatCtx, 0, path->c_str(), 0);

    // Find the first video stream
    iVideoStream = -1;
    for (int i=0; i < pFormatCtx->nb_streams; i++) {
        pCodecCtx = pFormatCtx->streams[i]->codec;
        if(pCodecCtx->codec_type == CODEC_TYPE_VIDEO)
        {
            iVideoStream = i;
            break;
        }
        pCodecCtx = NULL;
    }
    if (iVideoStream == -1)
        return false; // Didn't find a video stream

    // Get a pointer to the codec context for the video stream
    pCodecCtx = pFormatCtx->streams[iVideoStream]->codec;

    // Find the decoder for the video stream
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL)
        return false; // Codec not found
    
    // Inform the codec that we can handle truncated bitstreams -- i.e.,
    // bitstreams where frame boundaries can fall in the middle of packets
    if (pCodec->capabilities & CODEC_CAP_TRUNCATED)
        pCodecCtx->flags |= CODEC_FLAG_TRUNCATED;

    // Open codec
    if (avcodec_open(pCodecCtx, pCodec) < 0)
        return false; // Could not open codec

    // Allocate video frame
    pFrame = avcodec_alloc_frame();

    // Cache video info
	pVidCtx->info.width 	= pCodecCtx->width;
	pVidCtx->info.height 	= pCodecCtx->height;
	pVidCtx->info.fps 		= pCodecCtx->time_base.den / pCodecCtx->time_base.num;
	pVidCtx->uFrameTime 	= 1000 * pCodecCtx->time_base.num / pCodecCtx->time_base.den;
 
	pVidCtx->bCodecReady 	= true;
	return true;
}

//----------------------------------------------------------------------------
Boolean	CAVIPlayer::DeInitVideo(tVideoHndl hVideo)
{
	if (pCodecCtx == NULL)
		return false;
	
    // Free the YUV frame
    av_free(pFrame);
    pFrame = NULL;

    // Close the codec
    avcodec_close(pCodecCtx);
    pCodecCtx = NULL;

    // Close the video file
    av_close_input_file(pFormatCtx);
    pFormatCtx = NULL;
    
	return true;
}

//----------------------------------------------------------------------------
Boolean CAVIPlayer::GetVideoFrame(tVideoHndl hVideo, void* pCtx)
{
	if (GetNextFrame(pFormatCtx, pCodecCtx, iVideoStream, pFrame))
		return true;
	return false;
}

//----------------------------------------------------------------------------
Boolean CAVIPlayer::PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pCtx)
{
	if (pCodecCtx && pFrame) {
		// YUV420 planar format supported as is
		if (pCodecCtx->pix_fmt == PIX_FMT_YUV420P 
			|| pCodecCtx->pix_fmt == PIX_FMT_YUV422P
			|| pCodecCtx->pix_fmt == PIX_FMT_YUVJ420P 
			|| pCodecCtx->pix_fmt == PIX_FMT_YUVJ422P) 
		{
			tVideoSurf*	surf = pCtx;
			U8* 		sy = pFrame->data[0];
			U8*			su = pFrame->data[1];
			U8*			sv = pFrame->data[2];
			U8*			dy = surf->buffer; // + (y * surf->pitch) + (x * 2);
			U8*			du = surf->buffer + surf->pitch/2; // U,V in double-width buffer
			U8*			dv = surf->buffer + surf->pitch/2 + surf->pitch * (surf->height/2);
			int			i,j,k,m;
			bool		is422 = pCodecCtx->pix_fmt == PIX_FMT_YUVJ422P || pCodecCtx->pix_fmt == PIX_FMT_YUV422P;
			// Pack into separate YUV planar surface regions
			if (surf->format == kPixelFormatYUV420)
			{
				for (i = 0; i < pCodecCtx->height; i++) 
				{
					memcpy(dy, sy, pCodecCtx->width);
					sy += pFrame->linesize[0];
					dy += surf->pitch;
					if (i % 2) 
					{
						memcpy(du, su, pCodecCtx->width/2);
						memcpy(dv, sv, pCodecCtx->width/2);
						su += pFrame->linesize[1];
						sv += pFrame->linesize[2];
						du += surf->pitch;
						dv += surf->pitch;
					}
					else if (is422)
					{
						su += pFrame->linesize[1];
						sv += pFrame->linesize[2];
					}
				}
			}
			return true;
		}
#if 0	// SW scaler option
		// Setup destination context for SW scaler conversion
        int dst_pix_fmt = PIX_FMT_YUV420P;
	    AVPicture pict;
        pict.data[0] = pCtx->buffer;
        pict.data[1] = pict.data[0] + pCtx->pitch/2;
        pict.data[2] = pict.data[1] + pCtx->pitch * pCtx->height/2;
        pict.linesize[0] = 
        pict.linesize[1] = 
        pict.linesize[2] = pCtx->pitch;

        // Convert via SW scaler context
        static struct SwsContext *img_convert_ctx;
        int sws_flags = av_get_int(sws_opts, "sws_flags", NULL);
        img_convert_ctx = sws_getCachedContext(img_convert_ctx,
            pCodecCtx->width, pCodecCtx->height,
            pCodecCtx->pix_fmt,
            pCodecCtx->width, pCodecCtx->height,
            dst_pix_fmt, sws_flags, NULL, NULL, NULL);
        if (img_convert_ctx == NULL) {
            fprintf(stderr, "Cannot initialize the conversion context\n");
            return false;
        }
        sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize,
                  0, pCodecCtx->height, pict.data, pict.linesize);
		return true;
#endif
	}
	return false;
}

//----------------------------------------------------------------------------
Boolean CAVIPlayer::GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo)
{
	if (pCodecCtx) {
		pInfo->width 	= pCodecCtx->width;
		pInfo->height 	= pCodecCtx->height;
		pInfo->fps 		= pCodecCtx->time_base.den / pCodecCtx->time_base.num;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------
Boolean CAVIPlayer::GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime)
{
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);
	if (pCodecCtx) {
		pTime->frame 	= pCodecCtx->frame_number - 1;	// not 0-based
		pTime->time		= pTime->frame * pVidCtx->uFrameTime;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------
Boolean CAVIPlayer::SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop)
{
	if (GetNextFrame(pFormatCtx, pCodecCtx, iVideoStream, pFrame))
		return true;
	return false;
}

//----------------------------------------------------------------------------
Boolean CAVIPlayer::SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bExact)
{
	// TODO
	return false;
}

LF_END_BRIO_NAMESPACE()	

// EOF
