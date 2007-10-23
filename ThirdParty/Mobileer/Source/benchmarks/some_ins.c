/*
 * Benchmark some instruments.
 * Copyright 2002 Mobileer
 *
 */
#include <stdio.h>
#include "spmidi/include/midi.h"
#include "spmidi/include/spmidi.h"
#include "spmidi/include/spmidi_util.h"
#include "spmidi/include/spmidi_print.h"
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
#define SAMPLES_PER_FRAME       (1)
#define MAX_SIMULTANEOUS_NOTES  (32)

#define USE_ONLY_WAVETABLES     (1)
#define USE_HALF_WAVETABLES     (0)

#define NUM_LOOPS               (50)
#define FRAMES_PER_NOTE         (512)

#define BUFFERS_PER_NOTE        (FRAMES_PER_NOTE / SPMIDI_MAX_FRAMES_PER_BUFFER)
#define PITCH_BASE              (40)
#define PITCH_MASK              (31)
#define SAMPLES_PER_BUFFER      (SPMIDI_MAX_FRAMES_PER_BUFFER * SAMPLES_PER_FRAME)
#define NUM_CHANNELS            (8)

static short samples[ SAMPLES_PER_BUFFER ];
SPMIDI_Context *spmidiContext = NULL;

#if USE_ONLY_WAVETABLES
static int programs[NUM_CHANNELS] =
    {
        0, /* Piano */
        25, /* GuitarSteel */
        71, /* Clarinet */
        66, /* Sax */
        30, /* FuzzGuitar */
        56, /* Trumpet */
        40, /* Violin */
        25, /* GuitarNylon */
    };
#elif USE_HALF_WAVETABLES
static int programs[NUM_CHANNELS] =
    {
        0, /* Piano */
        25, /* GuitarSteel */
        71, /* Clarinet */
        66, /* Sax */
        0x48, /* Flute */
        0x58, /* Pad */
        0x09, /* FMBell */
        7, /* Clav  */
    };
#else
static int programs[NUM_CHANNELS] =
	{
		101,  /* Echos */
		0x10, /* Organ */
		0x18, /* Pluck */
		0x38, /* Brass */
		0x48, /* Flute */
		0x58, /* Pad */
		0x09, /* FMBell */
		0x47, /* Wood */
	};
#endif

/*******************************************************************/
int Benchmark_Init( void )
{
	int  err;
	
	PRTMSGNUMD("  MAX_SIMULTANEOUS_NOTES = ", MAX_SIMULTANEOUS_NOTES );
	PRTMSGNUMD("  SPMIDI_MAX_VOICES = ", SPMIDI_MAX_VOICES );
	PRTMSGNUMD("  SPMIDI_MAX_SAMPLE_RATE = ", SPMIDI_MAX_SAMPLE_RATE );
	PRTMSGNUMD("  FRAME_RATE = ", FRAME_RATE );
	PRTMSGNUMD("  SAMPLES_PER_FRAME = ", SAMPLES_PER_FRAME );
	
	err = SPMIDI_CreateContext( &spmidiContext, FRAME_RATE );
	if( err < 0 )
		return err;

	err = SPMIDI_SetMaxVoices( spmidiContext, MAX_SIMULTANEOUS_NOTES );

	return err;
}

/*******************************************************************/
int Benchmark_Run( long *checkSumPtr, long *numFramesPtr, long *frameRatePtr )
{
	int  channel, ibuf, inote, i;
	long numFrames;
	int  pitch;
	int  pitchSeed;
	long checksum = 0;

	for( channel=0; channel<NUM_CHANNELS; channel++ )
	{
		SPMUtil_ProgramChange( spmidiContext, channel, programs[channel] );
	}

	pitchSeed = 0;
	numFrames = 0;

	for( inote=0; inote<NUM_LOOPS; inote++ )
	{
		pitch = pitchSeed;
		DBUGMSG("Note Ons.\n");
		for( i=0; i<MAX_SIMULTANEOUS_NOTES; i++ )
		{
			channel = i % NUM_CHANNELS;
			SPMUtil_NoteOn( spmidiContext, channel, PITCH_BASE + pitch, 64 );
			pitch = ((pitch + 3) & PITCH_MASK);
		}

		for( ibuf=0; ibuf<BUFFERS_PER_NOTE; ibuf++ )
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
			channel = i % NUM_CHANNELS;
			SPMUtil_NoteOff( spmidiContext, channel, PITCH_BASE + pitch, 0 );
			pitch = ((pitch + 3) & PITCH_MASK);
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
	PRTMSGNUMD( "Max active notes was ",
	SPMIDI_GetMaxNoteCount( spmidiContext ) );
	return SPMIDI_DeleteContext(spmidiContext);
}

#if 0
int main( void )
{
	long checkSum = 0;
	long frameRate;
	long numFrames;
	Benchmark_Init();
	Benchmark_Run( &checkSum, &numFrames, &frameRate );
	Benchmark_Term();
	PRTMSGNUMH("checkSum = ", checkSum);
	PRTMSGNUMD("numFrames = ", numFrames);
	PRTMSGNUMD("frameRate = ", frameRate);
	return 0;
}
#endif

