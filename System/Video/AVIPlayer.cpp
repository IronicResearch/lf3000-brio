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
#include <DisplayPriv.h>
#include <AVIPlayer.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#undef ENABLE_PROFILING
#include <FlatProfiler.h>

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
	//----------------------------------------------------------------------------
	inline void PROFILE_BEGIN(int tag)
	{
	#ifdef ENABLE_PROFILING
		TimeStampOn(tag);
	#endif
	}

	//----------------------------------------------------------------------------
	inline void PROFILE_END(int tag, const char* msg)
	{
	#ifdef ENABLE_PROFILING
		TimeStampOff(tag);
	#endif
	}
}

//==============================================================================
// Local Functions
//==============================================================================
bool CAVIPlayer::GetNextFrame(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx,
    int iVideoStream, AVFrame *pFrame)
{
    static AVPacket packet;
    static int      bytesRemaining=0;
    static uint8_t  *pRawData = NULL;
    static bool     bFirstTime = true;
    int             bytesDecoded;
    int             frameFinished;

    // First time we're called, set packet.data to NULL to indicate it
    // doesn't have to be freed
    if (bFirstTime)
    {
        bFirstTime = false;
        av_init_packet(&packet);
        packet.data = NULL;
    }

    // Decode packets until we have decoded a complete frame
    while (true)
    {
        // Work on the current packet until we have decoded all of it
        while (bytesRemaining > 0)
        {
            // Decode the next chunk of data
            AVPacket packet2 = packet;
            packet2.size = bytesRemaining;
            packet2.data = pRawData;
            PROFILE_BEGIN(1);
            bytesDecoded = avcodec_decode_video2(pCodecCtx, pFrame,
            		&frameFinished, &packet2);
            PROFILE_END(1,"avcodec_decode_video2");

            // Was there an error?
            if (bytesDecoded < 0)
            {
            	bytesRemaining = 0;
            	break;
            }

            bytesRemaining -= bytesDecoded;
            pRawData       += bytesDecoded;

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
            if (av_read_frame(pFormatCtx, &packet) < 0)
                goto loop_exit;
        } while(packet.stream_index != iVideoStream);

        bytesRemaining = packet.size;
        pRawData = packet.data;
    }

loop_exit:

    // Decode the rest of the last frame
    PROFILE_BEGIN(1);
    bytesDecoded = avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
    PROFILE_END(1,"avcodec_decode_video2");

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
	pFormatCtx = NULL;
	pCodecCtx = NULL;
	pCodec = NULL;
    pFrame = NULL;
    iVideoStream = -1;
#ifdef ENABLE_PROFILING
	FlatProfilerInit(4, 1000);
#endif
}

//----------------------------------------------------------------------------
CAVIPlayer::~CAVIPlayer()
{
#ifdef ENABLE_PROFILING
	FlatProfilerDone();
#endif
}

//----------------------------------------------------------------------------
Boolean	CAVIPlayer::InitVideo(tVideoHndl hVideo)
{
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);
	FILE*			file = pVidCtx->pFileVideo;
	CPath*			path = pVidCtx->pPathVideo;
	AVDictionary*	pOpts = NULL;
	
    // Register all formats and codecs
    av_register_all();

    // Open video file
    if (avformat_open_input(&pFormatCtx, path->c_str(), NULL, NULL) != 0)
        return false; // Couldn't open file

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
        return false; // Couldn't find stream information

    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, path->c_str(), 0);

    // Find the first video stream
    iVideoStream = -1;
    for (int i=0; i < pFormatCtx->nb_streams; i++) {
        pCodecCtx = pFormatCtx->streams[i]->codec;
        if(pCodecCtx->codec_type == AVMEDIA_TYPE_VIDEO)
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
    if (avcodec_open2(pCodecCtx, pCodec, &pOpts) < 0)
        return false; // Could not open codec

    // Allocate video frame
    pFrame = avcodec_alloc_frame();

    // Cache video info
	pVidCtx->info.width 	= pCodecCtx->width;
	pVidCtx->info.height 	= pCodecCtx->height;
	pVidCtx->info.fps 		= pCodecCtx->time_base.den / pCodecCtx->time_base.num;
	pVidCtx->uFrameTime 	= 1000 * pCodecCtx->time_base.num / pCodecCtx->time_base.den;
	pVidCtx->uFrameTimeNum	= 1000 * pCodecCtx->time_base.num;
	pVidCtx->uFrameTimeDen	= pCodecCtx->time_base.den;
	pVidCtx->bFrameTimeFract = (pVidCtx->uFrameTimeNum % pVidCtx->uFrameTimeDen) != 0;
	dbg_.DebugOut(kDbgLvlImportant, "%dx%d, @%d fps (%d:%d)\n", (int)pVidCtx->info.width, (int)pVidCtx->info.height, (int)pVidCtx->info.fps, (int)pCodecCtx->time_base.den, (int)pCodecCtx->time_base.num);
	// FIXME: some MP4, FLV frame rates funky
	if (pVidCtx->uFrameTime <= 1) {
		pVidCtx->info.fps 		= 24;
		pVidCtx->uFrameTime 	= 1000 / 24;
		pVidCtx->uFrameTimeNum	= 1000;
		pVidCtx->uFrameTimeDen	= 24;
		pVidCtx->bFrameTimeFract = (pVidCtx->uFrameTimeNum % pVidCtx->uFrameTimeDen) != 0;
	}
	dbg_.DebugOut(kDbgLvlImportant, "%dx%d, @%d fps (%d:%d)\n", (int)pVidCtx->info.width, (int)pVidCtx->info.height, (int)pVidCtx->info.fps, (int)pVidCtx->uFrameTimeDen, (int)pVidCtx->uFrameTimeNum);
	
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
		tVideoSurf*	surf = pCtx;
		// YUV420 planar format supported as is
		if (pCodecCtx->pix_fmt == PIX_FMT_YUV420P 
			|| pCodecCtx->pix_fmt == PIX_FMT_YUV422P
			|| pCodecCtx->pix_fmt == PIX_FMT_YUVJ420P 
			|| pCodecCtx->pix_fmt == PIX_FMT_YUVJ422P) 
		{
			U8* 		sy = pFrame->data[0];
			U8*			su = pFrame->data[1];
			U8*			sv = pFrame->data[2];
			U8*			dy = surf->buffer; // + (y * surf->pitch) + (x * 2);
			U8*			du = dy + surf->pitch/2; // U,V in double-width buffer
			U8*			dv = du + surf->pitch * (surf->height/2);
			int			i,j,k,m;
			bool		is422 = pCodecCtx->pix_fmt == PIX_FMT_YUVJ422P || pCodecCtx->pix_fmt == PIX_FMT_YUV422P;
			// Pack into separate YUV planar surface regions
			if (surf->format == kPixelFormatYUV420)
			{
				dv = du + surf->pitch * (pCodecCtx->height/2);
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
				return true;
			}
			else if (surf->format == kPixelFormatARGB8888)
			{
				// Convert YUV to RGB format surface
				for (i = 0; i < pCodecCtx->height; i++) 
				{
					for (j = k = m = 0; k < pCodecCtx->width; j++, k+=2, m+=8) 
					{
						U8 y0 = sy[k];
						U8 y1 = sy[k+1];
						U8 u0 = su[j];
						U8 v0 = sv[j];
						dy[m+0] = B(y0,u0,v0);
						dy[m+1] = G(y0,u0,v0);
						dy[m+2] = R(y0,u0,v0);
						dy[m+3] = 0xFF;
						dy[m+4] = B(y1,u0,v0);
						dy[m+5] = G(y1,u0,v0);
						dy[m+6] = R(y1,u0,v0);
						dy[m+7] = 0xFF;
					}
					sy += pFrame->linesize[0];
					dy += surf->pitch;
					if (i % 2 || is422) 
					{
						su += pFrame->linesize[1];
						sv += pFrame->linesize[2];
					}
				}
				return true;
			}
			else if (surf->format == kPixelFormatRGB888)
			{
				// Convert YUV to RGB format surface
				for (i = 0; i < pCodecCtx->height; i++) 
				{
					for (j = k = m = 0; k < pCodecCtx->width; j++, k+=2, m+=6) 
					{
						U8 y0 = sy[k];
						U8 y1 = sy[k+1];
						U8 u0 = su[j];
						U8 v0 = sv[j];
						dy[m+0] = B(y0,u0,v0);
						dy[m+1] = G(y0,u0,v0);
						dy[m+2] = R(y0,u0,v0);
						dy[m+3] = B(y1,u0,v0);
						dy[m+4] = G(y1,u0,v0);
						dy[m+5] = R(y1,u0,v0);
					}
					sy += pFrame->linesize[0];
					dy += surf->pitch;
					if (i % 2 || is422) 
					{
						su += pFrame->linesize[1];
						sv += pFrame->linesize[2];
					}
				}
				return true;
			}
			return false;
		}
		// RGB888 format support
		if (pCodecCtx->pix_fmt == PIX_FMT_RGB24 
			|| pCodecCtx->pix_fmt == PIX_FMT_BGR24)
		{
			U8* 		s = pFrame->data[0];
			U8*			d = surf->buffer;
			U8*			du = d  + surf->pitch/2; // U,V in double-width buffer
			U8*			dv = du + surf->pitch * (surf->height/2);
			int			i,j,m,n;
			if (surf->format == kPixelFormatRGB888)
			{
				// Copy RGB triplets as is
				n = 3 * pCodecCtx->width; 
				for (i = 0; i < pCodecCtx->height; i++)
				{
					memcpy(d, s, n);
					s += pFrame->linesize[0];
					d += surf->pitch;
				}
				return true;
			}
			else if (surf->format == kPixelFormatARGB8888)
			{
				// Repack RGB triplets into ARGB surface
				for (i = 0; i < pCodecCtx->height; i++) 
				{
					for (j = m = n = 0; j < pCodecCtx->width; j++, m+=3, n+=4) 
					{
						d[n+0] = s[m+0];
						d[n+1] = s[m+1];
						d[n+2] = s[m+2];
						d[n+3] = 0xFF;
					}
					s += pFrame->linesize[0];
					d += surf->pitch;
				}
				return true;
			}
			else if (surf->format == kPixelFormatYUV420)
			{
				// Convert RGB triplets to YUV format surface
				for (i = 0; i < pCodecCtx->height; i++) 
				{
					for (j = m = n = 0; j < pCodecCtx->width; j++, m+=3) 
					{
						U8 b = s[m+0];
						U8 g = s[m+1];
						U8 r = s[m+2];
						d [j] = Y(r,g,b);
						du[n] = U(r,g,b);
						dv[n] = V(r,g,b);
						if (j % 2)
							n++;
					}
					s += pFrame->linesize[0];
					d += surf->pitch;
					if (i % 2)
					{
						du += surf->pitch;
						dv += surf->pitch;
					}
				}
				return true;
			}
			return false;
		}
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
		pTime->time		= pTime->frame * pVidCtx->uFrameTimeNum / pVidCtx->uFrameTimeDen; //pVidCtx->uFrameTime;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------
Boolean CAVIPlayer::SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop)
{
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);

	// FIXME: MPEG seeking flaky
	// Seeking to closest keyframe is needed, similar to Theora drop-frame logic.
	// MPEG codecs have trickier timestamp problem keeping PTS in order with DTS.
	// http://www.hackerfactor.com/blog/index.php?/archives/307-Picture-Go-Back.html

	if (bDrop && pCodecCtx->codec_id == CODEC_ID_MJPEG) {
		pCtx->frame = pCtx->time * pVidCtx->uFrameTimeDen / pVidCtx->uFrameTimeNum;
		if (SeekVideoFrame(hVideo, pCtx, true, false)) {
			pVidCtx->bSeeked = false;
			GetVideoTime(hVideo, pCtx);
			return true;
		}
	}
	else if (bDrop && pCodecCtx->codec_id == CODEC_ID_THEORA) {
		pCtx->frame = pCtx->time * pVidCtx->uFrameTimeDen / pVidCtx->uFrameTimeNum;
		if (SeekVideoFrame(hVideo, pCtx, false, false)) {
			pVidCtx->bSeeked = false;
			GetVideoTime(hVideo, pCtx);
			return true;
		}
	}
	if (GetNextFrame(pFormatCtx, pCodecCtx, iVideoStream, pFrame)) {
		GetVideoTime(hVideo, pCtx);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------
Boolean CAVIPlayer::SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bExact, Boolean bUpdateVideoDisplay)
{
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);
	if (pCodecCtx) {
		int flags = (bExact) ? AVSEEK_FLAG_ANY : 0; //AVSEEK_FLAG_FRAME;
		if (pCtx->frame < pCodecCtx->frame_number - 1)
			flags |= AVSEEK_FLAG_BACKWARD;
		int64_t timestamp = pCtx->frame + 1; //* pVidCtx->uFrameTime;
		int r = av_seek_frame(pFormatCtx, iVideoStream, timestamp, flags);
		if (r < 0)
			return false;
		pCodecCtx->frame_number = timestamp;
		pVidCtx->bSeeked = true;
		if (bUpdateVideoDisplay)
			pVidCtx->bUpdateVideoDisplay = true;
		return GetNextFrame(pFormatCtx, pCodecCtx, iVideoStream, pFrame);
	}
	return false;
}

S64 CAVIPlayer::GetVideoLength(tVideoHndl hVideo)
{
	tVideoContext* 	pVidCtx = reinterpret_cast<tVideoContext*>(hVideo);

	return pFormatCtx->duration * 1000 / AV_TIME_BASE;
}
LF_END_BRIO_NAMESPACE()	

// EOF
