/*
 * Benchmark a single instrument with many voices.
 * Copyright 2002 Mobileer
 *
 */
#include "stdio.h"
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

//#define PROGRAM             (0x00) /* Acoustic Grand. */
#define PROGRAM             (0x09) /* Glockenspiel. */
//#define PROGRAM             (0x0A) /* Music Box. */
//#define PROGRAM             (0x12) /* RockOrgan. */
//#define PROGRAM             (35) /* Fretless Bass. */
//#define PROGRAM             (40) /* Violin. */
//#define PROGRAM             (0x15) /* Accordian. */
//#define PROGRAM             (0x16) /* Harmonica. */
//#define PROGRAM             (0x37) /* Orchestra Hit */
//#define PROGRAM             (0x38) /* Trumpet */
//#define PROGRAM             (0x41) /* Alto Sax */
//#define PROGRAM             (0x47) /* Clarinet */
//#define PROGRAM             (0x49) /* Flute */
//#define PROGRAM             (0x50) /* Lead Square */
//#define PROGRAM             (0x0B) /* Vibraphone. */
//#define PROGRAM             (0x0E) /* Tubular Bells. */
//#define PROGRAM             (0x46) /* Bassoon */

#define FRAME_RATE              (22050)
#define SAMPLES_PER_FRAME       (1)
#define MAX_SIMULTANEOUS_NOTES  (32)
#define NUM_CHANNELS            (8)

#define NUM_LOOPS               (32)
#define FRAMES_PER_NOTE         (1024)

#define BUFFERS_PER_NOTE        (FRAMES_PER_NOTE / SPMIDI_MAX_FRAMES_PER_BUFFER)
#define PITCH_BASE              (40)
#define PITCH_MASK              (31)
#define SAMPLES_PER_BUFFER      (SPMIDI_MAX_FRAMES_PER_BUFFER * SAMPLES_PER_FRAME)

static short samples[ SAMPLES_PER_BUFFER ];
SPMIDI_Context *spmidiContext = NULL;


void Osc_WaveTableS16( void );

/*******************************************************************/
int Benchmark_Init( void )
{
	int  err;
	err = SPMIDI_CreateContext( &spmidiContext, FRAME_RATE );
	if( err < 0 )
		return err;
	/* Print address so we can tell which memory it is in. */
	PRTMSGNUMH("  Osc_WaveTable = ", Osc_WaveTable );

	err = SPMIDI_SetMaxVoices( spmidiContext, MAX_SIMULTANEOUS_NOTES );

	PRTMSGNUMD("  MAX_SIMULTANEOUS_NOTES = ", MAX_SIMULTANEOUS_NOTES );
	PRTMSGNUMD("  SPMIDI_MAX_VOICES = ", SPMIDI_MAX_VOICES );
	PRTMSGNUMD("  FRAME_RATE = ", FRAME_RATE );
	PRTMSGNUMD("  PROGRAM = ", PROGRAM );

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
		SPMUtil_ProgramChange( spmidiContext, channel, PROGRAM );
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
	printf("checkSum = 0x%08lX\n", checkSum);
	printf("numFrames = %ld\n", numFrames);
	printf("frameRate = %ld\n", frameRate);
	return 0;
}
#endif

