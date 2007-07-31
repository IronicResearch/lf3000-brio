/* $Id: play_midifile.c,v 1.16 2006/05/23 00:11:06 philjmsl Exp $ */
/**
 *
 * Play a MIDI File whose name is passed on the command line.
 * Use the ME2000 Synthesizer.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "spmidi.h"
#include "spmidi_play.h"
#include "spmidi_load.h"
#include "midifile_player.h"
#include "midifile_names.h"

#include "Dsputil.h"
#include "util.h"
#include "main.h"

#define kMaxSamplePerBuffer  (kMaxChannels*(SPMIDI_MAX_FRAMES_PER_BUFFER+kSRC_Filter_MaxDelayElements))
static short inSampleBuffer [kMaxSamplePerBuffer];static short outSampleBuffer[kMaxSamplePerBuffer*kMaxSamplingFrequency_UpsamplingRatio];
static short *inSampleBufPtr  = &inSampleBuffer [kSRC_Filter_MaxDelayElements];static short *outSampleBufPtr = &outSampleBuffer[kSRC_Filter_MaxDelayElements];

#define kMaxTempBufs	4
static short tmpBuffers[kMaxTempBufs][kMaxSamplePerBuffer];static short *tmpBufPtrs[kMaxTempBufs];
/****************************************************************
 * Print a text meta event.
 * Lyrics are type 5 in a standard MIDI file.
 */
void MyTextCallback( int trackIndex, int metaEventType,
                     const char *addr, int numChars, void *userData )
{
	int i;
	(void) userData; /* Prevent compiler warnings about unused data. */
	printf("MetaEvent type %d on track %d: ", metaEventType, trackIndex );
	for( i=0; i<numChars; i++ )
		printf("%c", addr[i] );
	printf("\n");
}	// ---- end MyTextCallback() ----

/****************************************************************
 * Play a MIDI file one audio buffer at a time.
 */
int MIDIFile_Play( unsigned char *image, int numBytes, const char *outFilePath , double *fileTime, double *execTime)
{
	int result;
	int msec;
	int seconds;
	int rem_msec;
	int timeout;
	int go = 1;
	MIDIFilePlayer *player;
	SPMIDI_Context *spmidiContext = NULL;
struct timeval startTv, endTv;

	SPMIDI_Initialize();
	inSamplingFrequency = midiSamplingFrequency;

//printf("MIDIFile_Play:  START midiSamplingFrequency=%d, channels=%d\n", midiSamplingFrequency, channels);

	/* Create a player, parse MIDIFile image and setup tracks. */
	result = MIDIFilePlayer_Create( &player, (int) midiSamplingFrequency, image, numBytes );
	if( result < 0 )
		goto error;

	msec     = MIDIFilePlayer_GetDurationInMilliseconds( player );
	seconds  = msec / 1000;
	rem_msec = msec - (seconds * 1000);
	*fileTime = 0.001 * (double) msec;
printf("MIDIFile_Play fileTime = %g Seconds\n", *fileTime);

	// Set function to be called when a text MetaEvent is
	// encountered while playing. Includes Lyric events.
//	MIDIFilePlayer_SetTextCallback( player, MyTextCallback, NULL );

	// Initialize SPMIDI Synthesizer. Output to audio or a file.	 
	
	/* Start synthesis engine with default number of voices. */
	result = SPMIDI_CreateContext( &spmidiContext, midiSamplingFrequency );
	if( result < 0 )
		goto error;
	SPMIDI_SetMaxVoices( spmidiContext, midiVoiceLimit );

	if (outFilePath)
		{
//	printf("Writing to file '%s' \n", outFilePath);
		result = SPMUtil_StartVirtualAudio( midiSamplingFrequency, outFilePath, channels );
		if( result < 0 )
			goto error;
		}

	fflush(stdout); /* Needed for Mac OS X */

	/*
	 * Play until we hit the end of all tracks.
	 * Tell the MIDI player to move forward on all tracks
	 * by a time equivalent to a buffers worth of frames.
	 * Generate one buffers worth of data and write it to the output stream.
	 */

// Start performance timer
	if (execTime)
		{
		*execTime = 0.0;
		gettimeofday(&startTv, NULL);
		}

// NOTE:  buffersizes from 128..1024 had little impact on performance, 64 slightly worse
inBlockLength  = SPMIDI_GetFramesPerBuffer();
outBlockLength = (inBlockLength*outSamplingFrequency)/inSamplingFrequency;
printf("MIDI inBlockLength=%d outBlockLength=%d\n", inBlockLength, outBlockLength);
printf("MIDI inSamplingFrequency=%d outSamplingFrequency=%d\n", inSamplingFrequency, outSamplingFrequency);

	while( go )
	{
	int mfperr = MIDIFilePlayer_PlayFrames( player, spmidiContext, inBlockLength );
		if( mfperr < 0 )
		{
			result = mfperr;
			goto error;
		}
		else if( mfperr > 0 )
			go = 0;

	/* Synthesize samples and fill buffer */
	SPMIDI_ReadFrames( spmidiContext, inSampleBufPtr, inBlockLength, channels, 16 );
	if (outFilePath)
		{
	// Convert sampling rate and write to output device
		if (inBlockLength != outBlockLength)
			{
			if (1 == channels)
				{
				RunSRC(inSampleBufPtr, outSampleBufPtr, inBlockLength, outBlockLength, &srcData[0]);
				SPMUtil_WriteVirtualAudio( outSampleBufPtr, channels, outBlockLength );
				}
			else
				{
				long i, ch;
				for (i = 0; i < 4; i++)
					tmpBufPtrs[i] = &tmpBuffers[i][kSRC_Filter_MaxDelayElements];
			// Deinterleave and convert
				DeinterleaveShorts(inSampleBufPtr, tmpBufPtrs[0], tmpBufPtrs[1], inBlockLength);
			// Convert sampling rate (unsuitable for in-place operation)
				for (ch = 0; ch < channels; ch++)
					RunSRC(tmpBufPtrs[ch], tmpBufPtrs[2+ch], inBlockLength, outBlockLength, &srcData[i]);
			// Interleave and write to file
				InterleaveShorts(tmpBufPtrs[2], tmpBufPtrs[3], outSampleBufPtr, outBlockLength);
				SPMUtil_WriteVirtualAudio( outSampleBufPtr, channels, outBlockLength );
				}
			}
	// Just write to output device
		else
			SPMUtil_WriteVirtualAudio( inSampleBufPtr, channels, outBlockLength );
		}
	}

	/* Wait for sound to die out. */
	timeout = (midiSamplingFrequency * 4) / SPMIDI_MAX_FRAMES_PER_BUFFER;
	while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
	{
		SPMIDI_ReadFrames( spmidiContext, inSampleBufPtr, inBlockLength, channels, 16 );
		SPMUtil_WriteVirtualAudio( inSampleBufPtr, channels, outBlockLength );
	}

// Stop performance timer
	if (execTime)
		{
		gettimeofday(&endTv, NULL);
		*execTime = SecondsFromTimevalDiff(&startTv, &endTv);
		}

	SPMUtil_StopVirtualAudio();
	SPMIDI_DeleteContext(spmidiContext);
	MIDIFilePlayer_Delete( player );

error:
	SPMIDI_Terminate();
	return result;
}	// ---- end MIDIFile_Play() ----

// ***************************************************************
// PlayMIDIFile
// ***************************************************************
	int
PlayMIDIFile(char *inFilePath, char *outFilePath, double *fileTime, double *execTime)
{
	int   i;
	void *data = NULL;
	int  fileSize;
	int   result = 0;
//	char *outputFileName = NULL;

//printf("PlayMIDIFile: inFilePath = '%s' \n", inFilePath);
// Output to file only for valid path
	if (outFilePath && outFilePath[0] != '\0')
		{
//		printf("PlayMIDIFile: outFilePath = '%s' \n", outFilePath);
		}
	else
		outFilePath = NULL;

	/* Load MIDI File into a memory image. */
	data = SPMUtil_LoadFileImage( inFilePath, &fileSize );
	if( data == NULL )
	{
		printf("PlayMIDIFile: ERROR reading MIDI File '%'s\n", inFilePath );
		goto error;
	}

//	printf("PlayMIDIFile: File: %s\n", inFilePath );

	result = MIDIFile_Play( (unsigned char *) data, fileSize, outFilePath, fileTime, execTime );
	if( result < 0 )
	{
		printf("PlayMIDIFile: Error playing MIDI File = %d = %s\n", result,
		       SPMUtil_GetErrorText( (SPMIDI_Error) result ) );
		goto error;
	}

error:
	if( data != NULL )
		SPMUtil_FreeFileImage( data );
//	printf("PlayMIDIFile: end\n" );
	return result;
}	// ---- end PlayMIDIFile() ----
 


