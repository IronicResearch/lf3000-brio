/*
 * $Id: me_demo_stub.c,v 1.2 2005/05/03 22:04:00 philjmsl Exp $
 * Stub for Mobileer Demo API.
 *
 * Author: Phil Burk
 * (C) 2004 Mobileer, All Rights Reserved
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


static long sSampleRate;
static short sLeftPhase;
static short sRightPhase;
static short sLeftPhaseIncrement;
static short sRightPhaseIncrement;

#define CalcPhaseIncrement( frequency, sampleRate ) ((2 * 0x00007FFF * frequency ) / sampleRate)

/**
 * Initialize everything needed.
 */
int MobileerDemo_Init( long sampleRate )
{
	sSampleRate = sampleRate;

	sLeftPhaseIncrement = (short) CalcPhaseIncrement( 440, sampleRate );
	/* On the second channel, generate a sawtooth wave a fifth higher. */
	sRightPhaseIncrement = (short) CalcPhaseIncrement( 660, sampleRate );

	return 0;
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

	for( i=0; i<numFrames; i++ )
	{
		long input = inputBuffer[ inIndex ];

		/* Generate a sawtooth wave by incrementing phase
		* and letting it wrap.
		* Use long form to prevent compiler warnings about casts.
		*/
		sLeftPhase = (short) (sLeftPhase + sLeftPhaseIncrement);
		/* Mix input and sawtooth 50:50 */
		output = (sLeftPhase >> 1) + (input >> 1);
		mixedBuffer[outIndex++] = (short) output;

		if( numChannels > 1 )
		{
			sRightPhase = (short) (sRightPhase + sRightPhaseIncrement);
			output = (sRightPhase >> 1) + (input >> 1);
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
	return;
}
