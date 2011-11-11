#ifndef AVIWRAPPER_H
#define AVIWRAPPER_H

//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		AVIWrapper.h
//
// Description:
//		AVI wrapper module for compatibility with avilib. 
//
//==============================================================================

#include <SystemTypes.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

//==============================================================================
// Defines for compatibility with avilib
//==============================================================================
#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_UNKNOWN             (0x0000)
#define WAVE_FORMAT_PCM                 (0x0001)
#define WAVE_FORMAT_ADPCM               (0x0002)
#define WAVE_FORMAT_ALAW                (0x0006)
#define WAVE_FORMAT_MULAW               (0x0007)
#define WAVE_FORMAT_DVI_ADPCM           (0x0011)
#define WAVE_FORMAT_GSM610              (0x0031)
#endif

//==============================================================================
// Typedefs for compatibility with avilib
//==============================================================================
typedef struct
{
	AVFormatContext*	pFormatCtx;			// container format context
	AVStream*			pVideoStrm;			// video stream
	AVStream*			pAudioStrm;			// audio stream
	AVCodecContext*		pVideoCodecCtx;		// video codec context
	AVCodecContext*		pAudioCodecCtx;		// audio codec context
	AVCodec*			pVideoCodec;		// video codec
	AVCodec*			pAudioCodec;		// audio codec
	AVFrame*			pVideoFrame;		// video frame buffer
	int16_t*			pAudioFrame;		// audio frame buffer
	uint8_t*			pEncoderBuf;		// encoder frame buffer
	int					iEncoderLength;		// length of encoder buffer
    int					iVideoStream;		// index of video stream
    int					iAudioStream;		// index of audio stream
    bool				bVideoConfig;		// video config set?
    bool				bAudioConfig;		// audio config set?
    bool				bAudioPresent;		// contains audio track?
} avi_t;


//==============================================================================
// Functions for compatibility with avilib
//==============================================================================
avi_t* AVI_open_output_file(char * filename, bool audio);
void AVI_set_video(avi_t *AVI, int width, int height, double fps, const char *compressor);
void AVI_set_audio(avi_t *AVI, int channels, long rate, int bits, int format, long mp3rate);
int  AVI_write_frame(avi_t *AVI, char *data, long bytes, int keyframe);
int  AVI_write_audio(avi_t *AVI, char *data, long bytes);
int  AVI_append_audio(avi_t *AVI, char *data, long bytes);
int  AVI_close(avi_t *AVI);

#endif // AVIWRAPPER_H
