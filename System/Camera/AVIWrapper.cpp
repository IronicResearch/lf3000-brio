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

#include <AVIWrapper.h>

//----------------------------------------------------------------------------
avi_t* AVI_open_output_file(char * filename)
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

	// Create video stream
	avi->pVideoStrm = av_new_stream(avi->pFormatCtx, avi->iVideoStream = 0);
	if (avi->pVideoStrm == NULL)
		return NULL;
	
	// Create audio stream
	avi->pAudioStrm = av_new_stream(avi->pFormatCtx, avi->iAudioStream = 1);
	if (avi->pAudioStrm == NULL)
		return NULL;

	// Init AVI file, video & audio config pending
	av_set_parameters(avi->pFormatCtx, NULL);
	avi->bVideoConfig = avi->bAudioConfig = false;
	
	return avi;
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
void AVI_set_video(avi_t *AVI, int width, int height, double fps, const char *compressor)
{
	// Rewrite AVI header with actual FPS rate if previously set
	if (AVI->bVideoConfig && AVI->bAudioConfig) {
		AVI->pVideoCodecCtx->time_base = av_d2q(1.0/fps, 1000);
		write_header(AVI, true);
		return;
	}
	
	// Set video codec and context (MJPEG)
	AVI->pVideoCodec = avcodec_find_encoder(CODEC_ID_MJPEG);
	if (AVI->pVideoCodec == NULL)
		return;

	AVCodecContext *c 	= AVI->pVideoStrm->codec;
	c->codec_id 		= CODEC_ID_MJPEG;
	c->codec_type 		= CODEC_TYPE_VIDEO;
	c->bit_rate 		= 400000; // suggested?
	c->width 			= width;
	c->height 			= height;
	c->time_base.den 	= (int)fps;
	c->time_base.num 	= 1;
	c->gop_size 		= 1; // intraframe
	c->pix_fmt 			= PIX_FMT_YUVJ422P;
	// some formats want stream headers to be separate
	if (AVI->pFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;
	
	int r = avcodec_open(AVI->pVideoCodecCtx = c, AVI->pVideoCodec);
	if (r < 0)
		return;

	// Write initial AVI header with nominal audio & video settings
	AVI->bVideoConfig = true;
	if (AVI->bVideoConfig && AVI->bAudioConfig)
		write_header(AVI, false);
	return;
}

//----------------------------------------------------------------------------
void AVI_set_audio(avi_t *AVI, int channels, long rate, int bits, int format, long mp3rate)
{
	// Set audio codec and context (PCM)
	AVI->pAudioCodec = avcodec_find_encoder(CODEC_ID_PCM_S16LE);
	if (AVI->pAudioCodec == NULL)
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
	av_init_packet(&pkt);
	
	// Supplied frame is already in JPEG format
	//	AVStream *st		= AVI->pVideoStrm;
	//	AVCodecContext *c 	= AVI->pVideoStrm->codec;
	
	//	pkt.size = avcodec_encode_video(c, video_outbuf, video_outbuf_size, picture);
	//	if (c->coded_frame->pts != AV_NOPTS_VALUE)
	//		pkt.pts = av_rescale_q(c->coded_frame->pts, c->time_base, st->time_base);
	//	if (c->coded_frame->key_frame)
	//		pkt.flags |= PKT_FLAG_KEY;

	pkt.flags 			|= PKT_FLAG_KEY;
	pkt.stream_index 	= AVI->iVideoStream;
	pkt.data 			= (uint8_t*)data;
	pkt.size 			= bytes;

	int r = av_interleaved_write_frame(AVI->pFormatCtx, &pkt);
	
	return 0;
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
	
	int r = av_interleaved_write_frame(AVI->pFormatCtx, &pkt);
	
	return 0;
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
	
	// Close codecs
	avcodec_close(AVI->pAudioCodecCtx);
	avcodec_close(AVI->pVideoCodecCtx);
	
	// Release resources
	av_free(AVI->pAudioStrm);
	av_free(AVI->pVideoStrm);
	av_free(AVI->pFormatCtx);
	delete AVI;

	return 0;
}

// EOF
