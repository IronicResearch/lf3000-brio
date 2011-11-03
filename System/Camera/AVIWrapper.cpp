//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		AVIWrapper.cpp
//
// Description:
//		AVI wrapper module for compatibility with avilib. 
//
//==============================================================================

#include <CameraPriv.h>
#include <AVIWrapper.h>

#include <fcntl.h>
#include <errno.h>

//----------------------------------------------------------------------------
avi_t* AVI_open_output_file(char * filename, bool audio)
{
	// Create AVI compatible context
	avi_t* avi = new(avi_t);
	if (avi == NULL)
		return NULL;
	
	// Register all formats and codecs
	av_register_all();
	
	// Create AVI container format context
	avi->pFormatCtx = avformat_alloc_context();
	if (avi->pFormatCtx == NULL)
		return NULL;

	// Select AVI container format
	AVOutputFormat *fmt = guess_format("avi", filename, NULL);
	if (fmt == NULL)
		return NULL;
	
	// Select file for writing
	avi->pFormatCtx->oformat = fmt;
	snprintf(avi->pFormatCtx->filename, sizeof(avi->pFormatCtx->filename), "%s", filename);
    if (url_fopen(&avi->pFormatCtx->pb, avi->pFormatCtx->filename, URL_WRONLY) < 0)
    	return NULL;

    // Get internal file descriptor
    URLContext* pctx = url_fileno(avi->pFormatCtx->pb);
    int fd = (int)pctx->priv_data;
    
    // Change file writes to non-blocking mode
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, O_NONBLOCK | flags);
    
	// Create video stream
	avi->pVideoStrm = av_new_stream(avi->pFormatCtx, avi->iVideoStream = 0);
	if (avi->pVideoStrm == NULL)
		return NULL;
	
	// Create audio stream
	if(avi->bAudioPresent = audio)
	{
		avi->pAudioStrm = av_new_stream(avi->pFormatCtx, avi->iAudioStream = 1);
		if (avi->pAudioStrm == NULL)
			return NULL;
	}

	// Init AVI file, video & audio config pending
	av_set_parameters(avi->pFormatCtx, NULL);
	avi->bVideoConfig = avi->bAudioConfig = false;
	avi->pEncoderBuf = NULL;
	avi->iEncoderLength = 0;

	return avi;
}

//----------------------------------------------------------------------------
void fixup_header(avi_t *avi)
{
	// Fixup AVI header with actual FPS rate manually.

	// Only AVI header fields at offset 32 (avih[0]) and 132 (strh[28]) seem significant:
	// http://www.fastgraph.com/help/avi_header_format.html

	// Fractional FPS rate saved as dwRate:dwScale strh[] fields:
	// http://msdn.microsoft.com/en-us/library/aa451198.aspx
	// http://www.virtualdub.org/blog/pivot/entry.php?id=27
	
	uint32_t scale = avi->pVideoCodecCtx->time_base.num;
	uint32_t rate = avi->pVideoCodecCtx->time_base.den;
	uint32_t time = avi->pVideoCodecCtx->time_base.den ?
			1000000 * avi->pVideoCodecCtx->time_base.num / avi->pVideoCodecCtx->time_base.den
			: 0;

	FILE* fp = fopen(avi->pFormatCtx->filename, "r+");
	fseek(fp, 0, SEEK_END);
	long eof = ftell(fp);
	fseek(fp, 32, SEEK_SET);					// AVIMainHeader.dwMicroSecPerFrame
	fwrite(&time, sizeof(uint32_t), 1, fp);
	fseek(fp, 128, SEEK_SET);					// AVIStreamHeader.dwScale
	fwrite(&scale, sizeof(uint32_t), 1, fp);
	fseek(fp, 132, SEEK_SET);					// AVIStreamHeader.dwRate
	fwrite(&rate, sizeof(uint32_t), 1, fp);
	fseek(fp, eof, SEEK_SET);
	fclose(fp);
}

//----------------------------------------------------------------------------
void write_header(avi_t *avi, bool rewrite)
{
	int64_t	filepos = url_ftell(avi->pFormatCtx->pb);
	
	// Rewind file if rewriting AVI header
	if (rewrite)
		url_fseek(avi->pFormatCtx->pb, 0, SEEK_SET);

	// Write AVI header
	av_write_header(avi->pFormatCtx);
	
	// Reposition file after rewriting AVI header
	if (rewrite)
		url_fseek(avi->pFormatCtx->pb, filepos, SEEK_SET);
	
	dump_format(avi->pFormatCtx, 0, avi->pFormatCtx->filename, 1);
}

//----------------------------------------------------------------------------
void AVI_set_video(avi_t *AVI, int width, int height, double fps, const __u32 camfmt)
{
	// Rewrite AVI header with actual FPS rate if previously set
	if (AVI->bVideoConfig && AVI->bAudioConfig) {
		AVI->pVideoCodecCtx->time_base = av_d2q(1.0/fps, 1000);
//		write_header(AVI, true);
		return;
	}
	
	// Set video codec and context (MJPEG)
	AVI->pVideoCodec = avcodec_find_encoder(CODEC_ID_MJPEG);
	if (AVI->pVideoCodec == NULL)
		return;

	AVCodecContext *c 	= AVI->pVideoStrm->codec;
	c->codec_id 		= CODEC_ID_MJPEG;
	c->codec_type 		= CODEC_TYPE_VIDEO;
	c->bit_rate 		= (int)fps * 10 * 1024 * 8; //400000; // suggested?
	c->width 			= width;
	c->height 			= height;
	c->time_base.den 	= (int)fps;
	c->time_base.num 	= 1;
	c->gop_size 		= 1; // intraframe

	// FIXME: incoming images have clamped color space, codec requires full gamut
	// use libavfilter to adjust colorspace, instead of spoofing it here?
	switch(camfmt) {
	case V4L2_PIX_FMT_YUYV:
		c->pix_fmt 		= PIX_FMT_YUYV422;
		break;
	case V4L2_PIX_FMT_YUV420:
		c->pix_fmt 		= PIX_FMT_YUVJ420P;
		break;
	case V4L2_PIX_FMT_YUV422P:
		c->pix_fmt		= PIX_FMT_YUVJ422P;
		break;
	}

	// some formats want stream headers to be separate
	if (AVI->pFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;
	
	int r = avcodec_open(AVI->pVideoCodecCtx = c, AVI->pVideoCodec);
	if (r < 0)
		return;

	// Encoding will be required
	if(camfmt != V4L2_PIX_FMT_MJPEG) {
		AVI->pVideoFrame = avcodec_alloc_frame();
		if(AVI->pVideoFrame == NULL)
			return;

		// FIXME proper size estimate
		AVI->iEncoderLength = c->width * c->height * 3;
		AVI->pEncoderBuf = (uint8_t*)av_malloc(AVI->iEncoderLength);
		if(AVI->pEncoderBuf == NULL)
			return;

		// FIXME magic numbers
		switch(camfmt) {
		case V4L2_PIX_FMT_YUYV:
			AVI->pVideoFrame->linesize[0] = c->width;

			break;
		case V4L2_PIX_FMT_YUV420:
			AVI->pVideoFrame->linesize[0] = 4096;
			AVI->pVideoFrame->linesize[1] = 4096;
			AVI->pVideoFrame->linesize[2] = 4096;

			break;
		case V4L2_PIX_FMT_YUV422P:
			AVI->pVideoFrame->linesize[0] = 4096;
			AVI->pVideoFrame->linesize[1] = 4096;
			AVI->pVideoFrame->linesize[2] = 4096;

			break;
		}
	}
	else
		AVI->pVideoFrame = NULL;

	// Write initial AVI header with nominal audio & video settings
	AVI->bVideoConfig = true;
	if (AVI->bVideoConfig && (AVI->bAudioConfig || !AVI->bAudioPresent))
		write_header(AVI, false);
	return;
}

//----------------------------------------------------------------------------
void AVI_set_audio(avi_t *AVI, int channels, long rate, int bits, int format, long mp3rate)
{
	// Set audio codec and context (PCM)
	AVI->pAudioCodec = avcodec_find_encoder(CODEC_ID_PCM_S16LE);
	if (AVI->pAudioCodec == NULL || !AVI->bAudioPresent)
		return;

	AVCodecContext *c 	= AVI->pAudioStrm->codec;
	c->codec_id 		= CODEC_ID_PCM_S16LE;
	c->codec_type 		= CODEC_TYPE_AUDIO;
	c->bit_rate 		= mp3rate; //64000;	// suggested?
	c->sample_rate 		= rate;
	c->channels 		= channels;

	int r = avcodec_open(AVI->pAudioCodecCtx = c, AVI->pAudioCodec);
	if (r < 0)
		return;

	// Write initial AVI header with nominal audio & video settings
	AVI->bAudioConfig = true;
	if (AVI->bVideoConfig && AVI->bAudioConfig)
		write_header(AVI, false);
	return;
}

//----------------------------------------------------------------------------
int  AVI_write_frame(avi_t *AVI, char *data, long bytes, int keyframe)
{
	AVPacket pkt;
	int out_size;

	av_init_packet(&pkt);

	// Encode frame
	if(AVI->pVideoFrame)
	{
		AVStream *st		= AVI->pVideoStrm;
		AVCodecContext *c 	= AVI->pVideoStrm->codec;
		AVFrame *pict		= AVI->pVideoFrame;

		// FIXME ???, magic numbers
		switch(c->pix_fmt) {
		case PIX_FMT_YUYV422:
			pict->data[0] = (uint8_t *)data;
			break;
		case PIX_FMT_YUVJ420P:
			pict->data[0] = (uint8_t *)data;
			pict->data[1] = pict->data[0] + 4096/2;
			pict->data[2] = pict->data[1] + 4096 * c->height/2;
			break;
		case PIX_FMT_YUVJ422P:
			pict->data[0] = (uint8_t *)data;
			pict->data[1] = pict->data[0] + 4096/2;
			pict->data[2] = pict->data[1] + 4096/4;
			break;
		}

		pkt.size = avcodec_encode_video(c, AVI->pEncoderBuf, AVI->iEncoderLength, pict);

		if (c->coded_frame->pts != AV_NOPTS_VALUE)
			pkt.pts = av_rescale_q(c->coded_frame->pts, c->time_base, st->time_base);
		if (c->coded_frame->key_frame)
			pkt.flags |= PKT_FLAG_KEY;

		pkt.data = AVI->pEncoderBuf;
	}
	else
	{
		pkt.flags 			|= PKT_FLAG_KEY;
		pkt.data 			= (uint8_t*)data;
		pkt.size 			= bytes;
		pkt.pts				= keyframe; // time_base units
	}

	pkt.stream_index 	= AVI->iVideoStream;

	int r = av_write_frame(AVI->pFormatCtx, &pkt);
	
	if (r < 0)
		printf("%s: r=%d, errno=%d: %s\n", __FUNCTION__, r, errno, strerror(errno));

	return r;
}

//----------------------------------------------------------------------------
int  AVI_write_audio(avi_t *AVI, char *data, long bytes)
{
	AVPacket pkt;
	av_init_packet(&pkt);

	// Supplied frames are already in raw PCM format
	//	AVStream *st		= AVI->pAudioStrm;
	//	AVCodecContext *c 	= AVI->pAudioStrm->codec;
	
	//	pkt.size = avcodec_encode_audio(c, audio_outbuf, audio_outbuf_size, samples);
	//	if (c->coded_frame->pts != AV_NOPTS_VALUE)
	//		pkt.pts = av_rescale_q(c->coded_frame->pts, c->time_base, st->time_base);

	pkt.flags 			|= PKT_FLAG_KEY;
	pkt.stream_index	= AVI->iAudioStream;
	pkt.data			= (uint8_t*)data;
	pkt.size			= bytes;
	
	int r = av_write_frame(AVI->pFormatCtx, &pkt);
	
	if (r < 0)
		printf("%s: r=%d, errno=%d: %s\n", __FUNCTION__, r, errno, strerror(errno));

	return r;
}

//----------------------------------------------------------------------------
int  AVI_append_audio(avi_t *AVI, char *data, long bytes)
{
	return AVI_write_audio(AVI, data, bytes);
}

//----------------------------------------------------------------------------
int  AVI_close(avi_t *AVI)
{
	// Close AVI file
	av_write_trailer(AVI->pFormatCtx);
	url_fclose(AVI->pFormatCtx->pb);
	
	// Fixup AVI header for FPS rate manually
	fixup_header(AVI);
	
	// Close codecs
	if(AVI->bAudioPresent)
		avcodec_close(AVI->pAudioCodecCtx);
	avcodec_close(AVI->pVideoCodecCtx);
	
	// Release resources
	if(AVI->bAudioPresent)
		av_free(AVI->pAudioStrm);
	if(AVI->pVideoFrame)
		av_free(AVI->pVideoFrame);
	if(AVI->pEncoderBuf)
		av_free(AVI->pEncoderBuf);
	av_free(AVI->pVideoStrm);
	av_free(AVI->pFormatCtx);
	delete AVI;

	return 0;
}

// EOF
