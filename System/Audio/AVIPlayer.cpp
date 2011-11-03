//==============================================================================
// Copyright (c) 2002-2010 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// AVIPlayer.cpp
//
// Description:
//		AudioPlayer-derived class for playing AVI audio. 
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <AudioTypes.h>
#include <AudioPriv.h>
#include <AudioTypesPriv.h>
#include <AVIPlayer.h>
#include <string.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Global variables
//==============================================================================
static U32 numAviPlayers = 0;
static U32 maxNumAviPlayers = kAudioMaxRawStreams;

//==============================================================================
// Local Functions
//==============================================================================
int GetNextFrame(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, 
    int iAudioStream, int16_t *pFrame, int size, int bytesExpected)
{
    static AVPacket packet;
    static int      bytesRemaining = 0;
    static uint8_t  *pRawData = NULL;
    static bool     bFirstTime = true;
    int             bytesDecoded;
    int				bytesTotal = 0;
    int             frameSize = size;

    // First time we're called, set packet.data to NULL to indicate it
    // doesn't have to be freed
    if (bFirstTime)
    {
        bFirstTime = false;
        packet.data = NULL;
    }

    // Decode packets until we have decoded a complete frame
    while (true)
    {
        // Work on the current packet until we have decoded all of it
        while (bytesRemaining > 0)
        {
            // Decode the next chunk of data
        	frameSize = size; 
            bytesDecoded = avcodec_decode_audio2(pCodecCtx, pFrame,
                &frameSize, pRawData, bytesRemaining);

            // Was there an error?
            if (bytesDecoded <= 0)
            {
            	bytesRemaining = 0;
                break;
            }

            bytesRemaining 	-= bytesDecoded;
            pRawData 		+= bytesDecoded;
            bytesTotal		+= bytesDecoded;
            
            if (frameSize <= 0)
                continue; // ffplay.c

            // Audio frame buffer may contain more bytes than requested
          	return bytesTotal;
        }

        // Read the next packet, skipping all packets that aren't for this stream
        do
        {
            // Free old packet
            if (packet.data != NULL)
                av_free_packet(&packet);

            // Read new packet
            if (av_read_frame(pFormatCtx, &packet) < 0)
                goto loop_exit;

        } while (packet.stream_index != iAudioStream);

        bytesRemaining = packet.size;
        pRawData = packet.data;
    }

loop_exit:

    // Decode the rest of the last frame
	frameSize = size;
    bytesDecoded = avcodec_decode_audio2(pCodecCtx, pFrame, &frameSize, 
        pRawData, bytesRemaining);

    // Free last packet
    if (packet.data != NULL)
        av_free_packet(&packet);

    return (bytesDecoded >= 0) ? bytesDecoded : 0;
}

//==============================================================================
// CAVIPlayer implementation
//==============================================================================
CAVIPlayer::CAVIPlayer( tAudioStartAudioInfo* pInfo, tAudioID id  ) :
	CAudioPlayer( pInfo, id  )
{
	// Init player-specific vars
	id_ = kNoAudioID;
	pFormatCtx = NULL;
	pCodecCtx = NULL;
	pCodec = NULL;
	pFrame = NULL;
    iAudioStream = -1;
    bytesRead = 0;
    bytesCached = 0;
    pCachedData = NULL;
	
    // Register all formats and codecs
    av_register_all();

    // Open audio file
    if (av_open_input_file(&pFormatCtx, pInfo->path->c_str(), NULL, 0, NULL) != 0)
        return; // Couldn't open file

    // Retrieve stream information
    if (av_find_stream_info(pFormatCtx) < 0)
        return; // Couldn't find stream information

    // Dump information about file onto standard error
    dump_format(pFormatCtx, 0, pInfo->path->c_str(), 0);

    // Find the first audio stream
    iAudioStream = -1;
    for (int i=0; i < pFormatCtx->nb_streams; i++) {
        pCodecCtx = pFormatCtx->streams[i]->codec;
        if (pCodecCtx->codec_type == CODEC_TYPE_AUDIO)
        {
            iAudioStream = i;
            break;
        }
        pCodecCtx = NULL;
    }
    if (iAudioStream == -1)
        return; // Didn't find a audio stream

    // Get a pointer to the codec context for the audio stream
    pCodecCtx = pFormatCtx->streams[iAudioStream]->codec;

    // Find the decoder for the audio stream
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL)
        return; // Codec not found
    
    // Inform the codec that we can handle truncated bitstreams -- i.e.,
    // bitstreams where frame boundaries can fall in the middle of packets
    if (pCodec->capabilities & CODEC_CAP_TRUNCATED)
        pCodecCtx->flags |= CODEC_FLAG_TRUNCATED;

    // Open codec
    if (avcodec_open(pCodecCtx, pCodec) < 0)
        return; // Could not open codec

    // Allocate audio frame (size specific)
    pFrame = new int16_t[AVCODEC_MAX_AUDIO_FRAME_SIZE];
    
    // Assign audio player info
	samplingFrequency_	= pCodecCtx->sample_rate;
	channels_			= pCodecCtx->channels;	 
	totalBytesRead_ 	= 0;
	totalFramesRead		= 0;
	id_					= id;
}

// =============================================================================
// ~CAVIPlayer
// =============================================================================
CAVIPlayer::~CAVIPlayer()
{
    // Free the audio buffer
	if (pFrame)
		delete[] pFrame;
    pFrame = NULL;

    // Close the codec
    if (pCodecCtx)
    	avcodec_close(pCodecCtx);
    pCodecCtx = NULL;

    // Close the audio file
    if (pFormatCtx)
    	av_close_input_file(pFormatCtx);
    pFormatCtx = NULL;
    
    // Cleanup base vars (no base destructor)
	if (pReadBuf_)
		delete[] pReadBuf_;

	if (pDebugMPI_)
		delete (pDebugMPI_);
}

// =============================================================================
U32 CAVIPlayer::GetNumPlayers(void)
{
	// TODO: link with raw players
	return numAviPlayers;
}

// =============================================================================
U32 CAVIPlayer::GetMaxPlayers(void)
{
	// TODO: link with raw players
	return maxNumAviPlayers;
}

// =============================================================================
Boolean CAVIPlayer::IsAVIPlayer(CAudioPlayer *pPlayer)
{
	CAVIPlayer *p;
	if (p = dynamic_cast<CAVIPlayer *>(pPlayer))
		return true;
	else
		return false;
}

// ==============================================================================
// ReadBytesFromFile :	 Return specified bytes read.
// ==============================================================================
U32 CAVIPlayer::ReadBytesFromFile( void *d, U32 bytesToRead)
{
//	static U32 bytesRead = 0;
//	static U32 bytesCached = 0;
//	static U8* pCachedData = NULL;
	
	// Account for audio frame buffer caching more data than requested
	if (bytesCached > 0) {
		bytesRead = std::min(bytesToRead, bytesCached);
		memcpy(d, pCachedData, bytesRead);
		bytesCached -= bytesRead;
		pCachedData += bytesRead;
		if (bytesCached < 0)
			bytesCached = 0;
		return bytesRead;
	}

	// Decode audio frames from file packet(s)
	bytesCached = GetNextFrame(pFormatCtx, pCodecCtx, iAudioStream, pFrame, AVCODEC_MAX_AUDIO_FRAME_SIZE, bytesToRead);
	pCachedData = (U8*)pFrame;
	bytesRead = std::min(bytesToRead, bytesCached);
	memcpy(d, pFrame, bytesRead);
	if (bytesCached > bytesRead) {
		bytesCached -= bytesRead;
		pCachedData += bytesRead;
	}
	else
		bytesCached = 0;
	
	return bytesRead;
}

// =============================================================================
// Render:		  Return framesRead
// =============================================================================
U32 CAVIPlayer::Render( S16 *pOut, U32 numStereoFrames)
{	
	tErrType result;
	U32		index;
	U32		framesToProcess = 0;
	U32		framesRead = 0;
	U32		bytesRead = 0;
	U32		bytesReadThisRender = 0;
	U32		bytesToRead = numStereoFrames * sizeof(S16) * channels_;
	char*	bufPtr = (char *)pReadBuf_;

	if (bIsDone_)
		return (0);

	// Read data from file to output buffer
	while ( bytesToRead > 0) 
	{
		bytesRead = ReadBytesFromFile(bufPtr, bytesToRead);
		
		if (bytesRead == 0)
			break;

		bytesToRead			-= bytesRead;
		bufPtr				+= bytesRead;
		totalBytesRead_		+= bytesRead;
		bytesReadThisRender += bytesRead;
	}
		
	framesRead		= bytesReadThisRender / (sizeof(S16) * channels_);
	framesToProcess = framesRead;
	totalFramesRead	+= framesRead;
	
	// Track elapsed playback intervals for time events
	if (bIsTimeEvent_) {
		timeLapsed_		+= framesRead / kAudioFramesPerMS;
		bIsTimeElapsed_	= (timeLapsed_ % timeDelta_ == 0) ? true : false;
	}
	
	// Copy Stereo data to stereo output buffer
	if (2 == channels_) 
	{
		U32 samplesToProcess = channels_*framesToProcess;
		for (index = 0; index < samplesToProcess; index++)			
			pOut[index] = pReadBuf_[index];
	} 
	else 
	{
		// Fan out mono data to stereo output buffer
		for (index = 0; index < framesToProcess; index++, pOut += 2) 
		{	
			S16 x = pReadBuf_[index];
			pOut[0] = x;
			pOut[1] = x;
		}
	}

	bIsDone_ = (numStereoFrames > framesRead);

	return (framesRead);
}

// =============================================================================
// RewindFile:	  Set file ptr to start of file
// =============================================================================
void CAVIPlayer::RewindFile()
{
	int flags = AVSEEK_FLAG_BYTE | AVSEEK_FLAG_BACKWARD;
	av_seek_frame(pFormatCtx, iAudioStream, 0, flags);

	bIsDone_ = false;
	totalBytesRead_ = 0;
	totalFramesRead = 0;
	bytesCached = 0;
}

// =============================================================================
// GetAudioTime_mSec :	 Return current position in audio file in milliSeconds
// =============================================================================
U32 CAVIPlayer::GetAudioTime_mSec( void ) 
{
	U64 milliSeconds = (1000 * totalFramesRead) / samplingFrequency_;
	return (U32)(milliSeconds);
}

// =============================================================================
// Seek :	
// =============================================================================
Boolean CAVIPlayer::SeekAudioTime(U32 timeMilliSeconds)
{
	int flags = AVSEEK_FLAG_ANY;
	int64_t timestamp = (int64_t)timeMilliSeconds * samplingFrequency_ / 1000;
	int r = av_seek_frame(pFormatCtx, iAudioStream, timestamp, flags);
	if (r < 0)
		return false;
	totalFramesRead = timestamp;
	bytesCached = 0;
	bSeeked = true;
	return true;
}

LF_END_BRIO_NAMESPACE()
