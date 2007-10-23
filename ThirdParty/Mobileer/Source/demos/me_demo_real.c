/* $Id: me_demo_real.c,v 1.13 2007/10/02 16:13:35 philjmsl Exp $ */
/**
 *
 * Mobileer Demo with very simple API for integrating with
 * other technology.
 *
 * Author: Phil Burk
 * Copyright 2004 Mobileer, All Rights Reserved
 */

#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_load.h"
#include "spmidi/include/spmidi_print.h"
#include "spmidi/include/midifile_player.h"


#define DEMO_MAX_VOICES   (8)


/****************************************************************/
/* Load MIDI file from initialized 'C' source arrays.
 *
 * Each file has two variables called midiFileImage and midiFileImage_size.
 * Using these defines we can declare uniquely named variables.
 */
#include "songs/song_FunToy8.h"
#include "songs/song_UpAndDown_rt.h"
#include "songs/song_ChaCha01Ring.h"

typedef struct PlayListEntry_s
{
	unsigned char *image;
	int size;
	int numLoops;
}
PlayListEntry_t;


/* List of songs to play. '*' means sounds OK with 8 maxVoices. */
PlayListEntry_t playList[] =
    {
        { song_ChaCha01Ring, sizeof(song_ChaCha01Ring), 1 },
        { song_UpAndDown_rt, sizeof(song_UpAndDown_rt), 2 },
        { song_FunToy8, sizeof(song_FunToy8), 1 },
    };
#define NUM_SONGS (sizeof(playList)/sizeof(PlayListEntry_t))


/****************************************************************/

/* Context for storing all persistent demo data.
 */
typedef struct DemoContext_s
{
	SPMIDI_Context *spmidiContext;
	MIDIFilePlayer *currentPlayer;
	int numLoopsLeft;
	int songIndex;
	long sampleRate;
	int gapCountDown;
}
DemoContext_t;

static DemoContext_t  sDemoContext = { 0 };

/**
 * Start playing a new song by deleting the previous
 * player and creating a new one.
 */
int StartNextSong( DemoContext_t *demoContext )
{
	int result;
	PlayListEntry_t *song = &playList[demoContext->songIndex++];

	if( demoContext->currentPlayer != NULL )
	{
		MIDIFilePlayer_Delete( demoContext->currentPlayer );
		demoContext->currentPlayer = NULL;
		PRTMSGNUMD("Max voices used = ", SPMIDI_GetMaxNoteCount(demoContext->spmidiContext) );
	}

	SPMIDI_ResetMaxNoteCount( demoContext->spmidiContext );

	/* Create a player, parse MIDIFile image and setup tracks. */
	result = MIDIFilePlayer_Create( &demoContext->currentPlayer, demoContext->sampleRate, song->image, song->size );
	if( result < 0 )
	{
		PRTMSG( "ERROR: MIDIFilePlayer_Create - " );
		PRTMSG( (char *) SPMUtil_GetErrorText(result) );
		PRTMSG( "\n" );
		goto error1;
	}

	demoContext->numLoopsLeft = song->numLoops;
	PRTMSGNUMD("Start song #", demoContext->songIndex );

	/* Start over again. */
	if( demoContext->songIndex >= NUM_SONGS )
		demoContext->songIndex = 0;

error1:
	return result;
}

int RenderRingtoneBuffer( DemoContext_t *demoContext, short *buffer, int numFrames, int samplesPerFrame )
{
	int result = 0;
	int framesLeft = numFrames;
	int framesPerLoop = SPMIDI_GetFramesPerBuffer();
	int samplesPerLoop = framesPerLoop * samplesPerFrame;

	while( framesLeft > 0 )
	{
		if( demoContext->numLoopsLeft <= 0 )
		{
			if( demoContext->gapCountDown-- < 0 )
			{
				result = StartNextSong( demoContext );
				if( result < 0 )
					return result;
			}
		}

		if( (demoContext->numLoopsLeft > 0)  && (demoContext->currentPlayer != NULL))
		{
			MIDIFilePlayer * currentPlayer = (MIDIFilePlayer *) demoContext->currentPlayer;

			int songComplete = MIDIFilePlayer_PlayFrames( currentPlayer, demoContext->spmidiContext, framesPerLoop );
			if( songComplete )
			{
				demoContext->numLoopsLeft--;
				if( demoContext->numLoopsLeft > 0 )
				{
					/* Rewind song. */
					MIDIFilePlayer_GoToFrame( currentPlayer, demoContext->spmidiContext, 0 );
				}
				else
				{
					demoContext->gapCountDown = 25;
				}
			}
		}

		/* Synthesize samples and fill buffer.
		 * Always do this even if no MIDIFile so we catch note tails.
		 */
		result = SPMIDI_ReadFrames( demoContext->spmidiContext, buffer, framesPerLoop, samplesPerFrame, 16 );
		if( result < 0 )
			return result;

		framesLeft -= framesPerLoop;
		buffer += samplesPerLoop;
	}
	return result;
}

/**
 * Initialize everything needed.
 */
int MobileerDemo_Init( long sampleRate )
{
	int result;
	DemoContext_t  *demoContext = &sDemoContext;

	demoContext->sampleRate = sampleRate;
	result = SPMIDI_CreateContext( &demoContext->spmidiContext, sampleRate );
	if( result < 0 )
		return result;

	PRTMSGNUMD("Set max voice in MobileerDemo_Init() to ", DEMO_MAX_VOICES );
	SPMIDI_SetMaxVoices( demoContext->spmidiContext, DEMO_MAX_VOICES );

	return result;
}


/**
 * Synthesizes the next buffer of audio samples.
 * A stereo frame is two samples. Recommend multiple of 256
 * If stereo then it will be interleaved.
 * Number of samples in sampleBuffer is numFrames*numChannels.
 * This will play from an internal series of MIDI files in a loop.
 *
 * @param inputBuffer array of mixed signed 16 bit audio data
 * @param outputBuffer array of mixed signed 16 bit audio data
 * @param numFrames is number of audio frames in buffer.
 * @param numChannels is 1 for mono, 2 for stereo
 */
int MobileerDemo_Synthesize( short *inputBuffer,
                             short *mixedBuffer,
                             int numFrames, int numChannels )
{
	int i;
	int inIndex = 0;
	int outIndex = 0;
	long output;
	DemoContext_t  *demoContext = &sDemoContext;

	RenderRingtoneBuffer( demoContext, mixedBuffer, numFrames, numChannels );

	/* Mix results of synthesizer with input audio. */
	for( i=0; i<numFrames; i++ )
	{
		long input = inputBuffer[ inIndex ];

		output = (mixedBuffer[outIndex] >> 1) + (input >> 1);
		mixedBuffer[outIndex++] = (short) output;

		if( numChannels > 1 )
		{
			output = (mixedBuffer[outIndex] >> 1) + (input >> 1);
			mixedBuffer[outIndex++] = (short) output;
		}
	}

	return 0;
}


/**
 * Cleanup.
 */
void MobileerDemo_Term( void )
{
	DemoContext_t  *demoContext = &sDemoContext;
	SPMIDI_DeleteContext(demoContext->spmidiContext);
	return;
}
