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

#include "util.h"

//ORIG #define SAMPLE_RATE         (22050)
//#define SAMPLES_PER_FRAME   (2)
#define kMAX_SAMPLES_PER_FRAME   (2)
extern long samplingFrequency;
extern long samplingFrequencySpecified;
extern long channels;
extern long voiceLimit;

#define SAMPLES_PER_BUFFER  (kMAX_SAMPLES_PER_FRAME * SPMIDI_MAX_FRAMES_PER_BUFFER)
static short sSampleBuffer[SAMPLES_PER_BUFFER];

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
}

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
long framesPerBuffer;

	SPMIDI_Initialize();

//printf("MIDIFile_Play:  samplingFrequency=%d, channels=%d\n", samplingFrequency, channels);

	/* Create a player, parse MIDIFile image and setup tracks. */
	result = MIDIFilePlayer_Create( &player, (int) samplingFrequency, image, numBytes );
	if( result < 0 )
		goto error;

	msec = MIDIFilePlayer_GetDurationInMilliseconds( player );
	seconds = msec / 1000;
	rem_msec = msec - (seconds * 1000);
	*fileTime = 0.001 * (double) msec;
printf("MIDIFile_Play fileTime = %g Seconds\n", *fileTime);
	// Set function to be called when a text MetaEvent is
	// encountered while playing. Includes Lyric events.
//	MIDIFilePlayer_SetTextCallback( player, MyTextCallback, NULL );

	// Initialize SPMIDI Synthesizer. Output to audio or a file.	 
	
	/* Start synthesis engine with default number of voices. */
	result = SPMIDI_CreateContext( &spmidiContext, samplingFrequency );
	if( result < 0 )
		goto error;
	SPMIDI_SetMaxVoices( spmidiContext, voiceLimit );

	if (outFilePath)
		{
	printf("Writing to file '%s' \n", outFilePath);
		result = SPMUtil_StartVirtualAudio( samplingFrequency, outFilePath, channels );
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
	if (execTime)
		{
		*execTime = 0.0;
		gettimeofday(&startTv, NULL);
		}
// FIXXX: experiment with various buffer sizes
framesPerBuffer = SPMIDI_GetFramesPerBuffer();
//printf("framesPerBuffer=%d \n", framesPerBuffer);
	while( go )
	{
	int mfperr = MIDIFilePlayer_PlayFrames( player, spmidiContext, framesPerBuffer );
		if( mfperr < 0 )
		{
			result = mfperr;
			goto error;
		}
		else if( mfperr > 0 )
			go = 0;

	/* Synthesize samples and fill buffer */
	SPMIDI_ReadFrames( spmidiContext, sSampleBuffer, framesPerBuffer, channels, 16 );
	if (outFilePath)
		SPMUtil_WriteVirtualAudio( sSampleBuffer, channels, framesPerBuffer );
	}

	/* Wait for sound to die out. */
	timeout = (samplingFrequency * 4) / SPMIDI_MAX_FRAMES_PER_BUFFER;
	while( (SPMIDI_GetActiveNoteCount(spmidiContext) > 0) && (timeout-- > 0) )
	{
		SPMIDI_ReadFrames( spmidiContext, sSampleBuffer, framesPerBuffer, channels, 16 );
		SPMUtil_WriteVirtualAudio( sSampleBuffer, channels, framesPerBuffer );
	}
// end timer
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
}

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
} // 

