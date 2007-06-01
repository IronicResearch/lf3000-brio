/*
 * Benchmark some instruments.
 * Copyright 2002 Mobileer
 *
 */
#include "midi.h"
#include "spmidi.h"
#include "spmidi_util.h"
#include "spmidi_print.h"
#include "bench_tools.h"


#if 0
#define DBUGMSG(x)   PRTMSG(x)
#define DBUGNUMD(x)  PRTNUMD(x)
#define DBUGNUMH(x)  PRTNUMH(x)
#else
#define DBUGMSG(x)
#define DBUGNUMD(x)
#define DBUGNUMH(x)
#endif

#define FRAME_RATE              (22050)

#define NUM_NOTES               (128)
#define FRAMES_PER_NOTE         (4096)

/* 75 Percent duty cycle. On:Off */
#define FRAMES_ON               ((3 * FRAMES_PER_NOTE) / 4)
#define FRAMES_OFF              ((1 * FRAMES_PER_NOTE) / 4)

#define PITCH_BASE              (40)
#define PITCH_MASK              (31)
#define SAMPLES_PER_FRAME       (1)
#define SAMPLES_PER_BUFFER      (SPMIDI_MAX_FRAMES_PER_BUFFER * SAMPLES_PER_FRAME)
#define MAX_SIMULTANEOUS_NOTES  (16)
#define NUM_CHANNELS            (8)
#define NOTES_PER_CHANNEL       (MAX_SIMULTANEOUS_NOTES / NUM_CHANNELS)

static short samples[ SAMPLES_PER_BUFFER ];
SPMIDI_Context *spmidiContext = NULL;

int Benchmark_Init( void );
int Benchmark_Run( long *checkSumPtr, long *numFramesPtr, long *frameRatePtr );
int Benchmark_Term( void );


/*******************************************************************/
int Benchmark_Init( void )
{
	int  err;
	err = SPMIDI_CreateContext( &spmidiContext, FRAME_RATE );
	if( err < 0 )
		return err;

	err = SPMIDI_SetMaxVoices( MAX_SIMULTANEOUS_NOTES );

	return err;
}

/*******************************************************************/
int Benchmark_Run( long *checkSumPtr, long *numFramesPtr, long *frameRatePtr )
{
	int  channel, inote, i;
	long numFrames;
	int  pitch;
	int  programIndex = 0;
	int  pitchSeed;
	long checksum = 0;

	DBUGNUMD( FRAME_RATE );
	DBUGMSG(" = FRAME_RATE\n");
	DBUGNUMD( BUFFERS_PER_NOTE );
	DBUGMSG(" = BUFFERS_PER_NOTE\n");

	pitchSeed = 0;
	numFrames = 0;
	for( inote=0; inote<NUM_NOTES; )
	{
		int iframe;

		for( channel=0; channel<NUM_CHANNELS; channel++ )
		{
			SPMUtil_ProgramChange( spmidiContext, channel, programIndex );
			programIndex = ((programIndex + 17) & 0x7F);
		}

		pitch = pitchSeed;
		DBUGMSG("Note Ons.\n");
		for( i=0; i<MAX_SIMULTANEOUS_NOTES; i++ )
		{
			channel = i / NOTES_PER_CHANNEL;
			SPMUtil_NoteOn( spmidiContext, channel, PITCH_BASE + pitch, 64 );
			pitch = ((pitch + 3) & PITCH_MASK);
		}
		inote += MAX_SIMULTANEOUS_NOTES;

		for( iframe=0; iframe<FRAMES_ON; iframe += SPMIDI_MAX_FRAMES_PER_BUFFER )
		{
			int k;
			numFrames += SPMIDI_ReadFrames( spmidiContext, samples, SPMIDI_MAX_FRAMES_PER_BUFFER, SAMPLES_PER_FRAME, sizeof(short)*8 );
			/* Calculate checksum */
			for( k=0; k<SAMPLES_PER_BUFFER; k++ )
			{
				checksum += samples[k];
			}
		}

		pitch = pitchSeed;
		for( i=0; i<MAX_SIMULTANEOUS_NOTES; i++ )
		{
			channel = i / NOTES_PER_CHANNEL;
			SPMUtil_NoteOff( channel, PITCH_BASE + pitch, 0 );
			pitch = ((pitch + 3) & PITCH_MASK);
		}

		for( iframe=0; iframe<FRAMES_OFF; iframe += SPMIDI_MAX_FRAMES_PER_BUFFER )
		{
			int k;
			numFrames += SPMIDI_ReadFrames( spmidiContext, samples, SPMIDI_MAX_FRAMES_PER_BUFFER, SAMPLES_PER_FRAME, sizeof(short)*8 );
			/* Calculate checksum */
			for( k=0; k<SAMPLES_PER_BUFFER; k++ )
			{
				checksum += samples[k];
			}
		}

		pitchSeed = (pitchSeed + 5) & PITCH_MASK;
	}

	*checkSumPtr = checksum;
	*numFramesPtr = numFrames;
	*frameRatePtr = FRAME_RATE;

	return 0;
}

/*******************************************************************/
int Benchmark_Term( void )
{
	return SPMIDI_DeleteContext(spmidiContext);
}
