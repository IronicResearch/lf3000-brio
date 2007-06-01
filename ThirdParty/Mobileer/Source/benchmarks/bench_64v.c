/* $Id: bench_64v.c,v 1.5 2005/05/03 22:04:00 philjmsl Exp $ */
/**
 *
 * Benchmark 64 voices print the checksum result.
 *
 * Purpose: to benchmark synthesis.
 *
 * Author: Phil Burk
 * Copyright 2002 Mobileer
 */

#include "midi.h"
#include "spmidi.h"
#include "spmidi_print.h"
#include "spmidi_util.h"
#include "spmidi_audio.h"

/*
 * Adjust these for your system.
 */
#define SAMPLE_RATE         (22050)
#define SAMPLES_PER_FRAME   (1)
#define FRAMES_PER_BUFFER   (SPMIDI_MAX_FRAMES_PER_BUFFER)
#define MAX_VOICES   		(16)

typedef struct TestData_s
{
	int numChords;
	int notesPerChord;
	int numChannels;
	int buffersPerChord;
	int checksum;
	int frameCount;
	int maxVoiceCount;
}
TestData_t;

/****************************************************************/
/**
 * Use SP-MIDI to synthesize a buffer full of audio.
 * Then checksum that audio for verification.
 */
static int ChecksumNextBuffer( SPMIDI_Context *spmidiContext, int checkSum )
{
	int i;
	/* You may wish to move this buffer from the stack to another location. */
#define SAMPLES_PER_BUFFER  (SAMPLES_PER_FRAME * FRAMES_PER_BUFFER)

	short samples[SAMPLES_PER_BUFFER];

	/* Generate a buffer full of audio data as 16 bit samples. */
	SPMIDI_ReadFrames( spmidiContext, samples, FRAMES_PER_BUFFER,
	                   SAMPLES_PER_FRAME, 16 );

	for( i=0; i<SAMPLES_PER_BUFFER; i++ )
	{
		checkSum += samples[i];
	}

	return checkSum;
}


/****************************************************************/
/**
 * Play lots of voices simultaneously.
 * Turn lots of voices on and off each channel. 
 */
static int PlayManyVoices( TestData_t *data )
{
	SPMIDI_Context *spmidiContext = NULL;
	int result = 0;
	int checkSum = 0;
	int chordIndex;
	int noteIndex;
	int bufferIndex;
	int channelIndex;

	data->frameCount = 0;

	/* Start SP-MIDI synthesis engine using the desired sample rate. */
	result = SPMIDI_CreateContext( &spmidiContext, SAMPLE_RATE );
	if( result < 0 )
		goto error1;
	/* Use scattered collectionof pitches and programs. */
#define NOTE_PITCH (30 + ((channelIndex + (chordIndex*5) + (noteIndex * 7)) & 63))
#define DRUM_PITCH (GMIDI_FIRST_DRUM + ((channelIndex + (chordIndex*5) + (noteIndex * 7)) & 31))
#define CALC_PITCH ((channelIndex == MIDI_RHYTHM_CHANNEL_INDEX) ? DRUM_PITCH : NOTE_PITCH)

	for( chordIndex=0; chordIndex < data->numChords; chordIndex++ )
	{
		/* On every channel. */
		for( channelIndex=0; channelIndex < data->numChannels ; channelIndex++ )
		{
			if( (chordIndex & 3) == 0 )
			{
				int programIndex = (chordIndex + (channelIndex * 7)) & 127;
				SPMUtil_ProgramChange( spmidiContext, channelIndex, programIndex );
			}

			/* Turn ON notes. */
			for( noteIndex=0; noteIndex < data->notesPerChord; noteIndex++ )
			{
				int pitch = CALC_PITCH;
				SPMUtil_NoteOn( spmidiContext, channelIndex, pitch, 64 );
			}
		}

		/* Let notes play. */
		for( bufferIndex=0; bufferIndex < (data->buffersPerChord/2); bufferIndex++ )
		{
			checkSum  += ChecksumNextBuffer( spmidiContext, checkSum );
			data->frameCount += FRAMES_PER_BUFFER;
		}

		/* On every channel. */
		for( channelIndex=0; channelIndex < data->numChannels; channelIndex++ )
		{
			/* Turn OFF notes. */
			for( noteIndex=0; noteIndex < data->notesPerChord; noteIndex++ )
			{
				int pitch = CALC_PITCH;
				SPMUtil_NoteOff( spmidiContext, channelIndex, pitch, 64 );
			}
		}
		/* Let notes die down. */
		for( bufferIndex=0; bufferIndex < (data->buffersPerChord/2); bufferIndex++ )
		{
			checkSum  += ChecksumNextBuffer( spmidiContext, checkSum );
			data->frameCount += FRAMES_PER_BUFFER;
		}
	}

	data->checksum = checkSum & 0x7FFFFFFF;
	data->maxVoiceCount = SPMIDI_GetMaxNoteCount(spmidiContext);

	/* Terminate SP-MIDI synthesizer. */
	SPMIDI_DeleteContext(spmidiContext);

error1:
	return result;
}


/****************************************************************/
int main( int argc, char ** argv )
{
	int result;
	TestData_t DATA = {0};
	(void) argc;
	(void) argv;

	PRTMSG("Benchmark Mobileer Synthesizer\n");
	PRTMSG("(C) 2003 Mobileer\n");

	DATA.buffersPerChord = 20;
	DATA.numChannels = (MAX_VOICES < MIDI_NUM_CHANNELS) ? MAX_VOICES : MIDI_NUM_CHANNELS;
	DATA.notesPerChord = MAX_VOICES / DATA.numChannels;
	DATA.numChords = 32;

	result = PlayManyVoices( &DATA );
	if( result < 0 )
	{
		PRTMSG("Error: result = ");
		PRTNUMD( result );
		PRTMSG( SPMUtil_GetErrorText( result ) );
		PRTMSG("\n");
	}
	else
	{
		PRTMSG("Checksum = ");
		PRTNUMH( DATA.checksum );
		PRTMSG("\n");

		PRTMSG("NumFrames = ");
		PRTNUMD( DATA.frameCount );
		PRTMSG("\n");

		PRTMSG("MaxVoices = ");
		PRTNUMD( DATA.maxVoiceCount );
		PRTMSG("\n");
	}

	PRTMSG("Benchmark complete.\n");

	return (result < 0);
}

