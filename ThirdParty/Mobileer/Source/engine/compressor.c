/* $Id: compressor.c,v 1.16 2006/05/16 00:00:36 philjmsl Exp $ */
/**
 *
 * Dynamic Range Compressor
 * This compressor uses a unique formula to calculate the gain from the peak amplitude:
 *
 *  if( peakAmplitude < threshold ) peakAmplitude = threshold;
 *
 *	gain = (target + curve) / (peakAmplitude + curve)
 *
 * The effect of this curve value is to soften the gain versus peak function
 * that normally has a sharp corner at the threshold value.
 * The result is that softer passage are boosted, but are still
 * softer then loud passages resulting in a more natural sound.
 *
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "fxpmath.h"
#include "memtools.h"
#include "spmidi.h"
#include "compressor.h"
#include "spmidi_host.h"
#include "spmidi_synth_util.h"
#include "spmidi_synth.h"
#include "spmidi_hybrid.h"
#include "spmidi_print.h"

#if 0
#define DBUGMSG(x)   PRTMSG(x)
#define DBUGNUMD(x)  PRTNUMD(x)
#define DBUGNUMH(x)  PRTNUMH(x)
#else
#define DBUGMSG(x)
#define DBUGNUMD(x)
#define DBUGNUMH(x)
#endif

/* Set resolution for gain so that mixer bus resolution is preserved. */
#define COMPRESSOR_SHIFT   (12)

/* Default compressor parameters. */
#define THRESHOLD    ( 2)
#define CURVATURE    (20)
#define TARGET       (88)

#define PER_10000(n)  (((n) << COMPRESSOR_SHIFT) / 10000) /* DIVIDE - init */

/** Precalculated attack and decay scalers based on this equation.
 * attackScaler = 10000 * (2.0 ^ (blockSize / (sampleRate * timeInSeconds)))
 * decayScaler  = 10000 * (0.5 ^ (blockSize / (sampleRate * timeInSeconds)))
 *
 * Default values are based on 25 msec attack and 260 msec decay times.
 * Times are "half lifes", the time it takes to double or half the value.
 */
#define ATTACK_16000 PER_10000(10070)
#define DECAY_16000  PER_10000( 9993)
#define BLOCK_16000   (4)

#define ATTACK_22050 PER_10000(10101)
#define DECAY_22050  PER_10000( 9990)
#define BLOCK_22050   (8)

#define ATTACK_44100 PER_10000(10050)
#define DECAY_44100  PER_10000( 9995)
#define BLOCK_44100   (8)

/* Sufficient delay to give gain time to drop before big peak is heard. */
#define MSEC_DELAY   (22)
#define MSEC_PER_SECOND   (1000)

/********************************************************************/
void Compressor_Term( Compressor_t *compressor )
{
#if SPMIDI_SUPPORT_MALLOC
	if( compressor->delayLine != NULL )
	{
		SPMIDI_FreeMemory( compressor->delayLine );
		compressor->delayLine = NULL;
	}
#else
	(void) compressor;
#endif
}

/********************************************************************/
int Compressor_Init( Compressor_t *compressor, int sampleRate )
{
	int delayLineNumBytes = COMP_DELAY_LINE_SIZE * sizeof(FXP31);

#if SPMIDI_SUPPORT_MALLOC
	compressor->delayLine = SPMIDI_ALLOC_MEM( delayLineNumBytes, "compressor" );
	if( compressor->delayLine == NULL )
	{
		return SPMIDI_Error_OutOfMemory;
	}
#endif

	MemTools_Clear( compressor->delayLine, delayLineNumBytes );

	/* Sample rate dependant parameters. */
	if( sampleRate <= 20000 )
	{
		compressor->attack = ATTACK_16000;
		compressor->decay = DECAY_16000;
		compressor->blockSize = BLOCK_16000;
	}
	else if( sampleRate <= 30000 )
	{
		compressor->attack = ATTACK_22050;
		compressor->decay = DECAY_22050;
		compressor->blockSize = BLOCK_22050;
	}
	else
	{
		compressor->attack = ATTACK_44100;
		compressor->decay = DECAY_44100;
		compressor->blockSize = BLOCK_44100;
	}

	Compressor_SetParameter( compressor, SPMIDI_PARAM_COMPRESSOR_CURVE, CURVATURE );
	Compressor_SetParameter( compressor, SPMIDI_PARAM_COMPRESSOR_TARGET, TARGET );
	Compressor_SetParameter( compressor, SPMIDI_PARAM_COMPRESSOR_THRESHOLD, THRESHOLD );

	compressor->sampleCounter = 0;
	compressor->writeIndex = 0;
	/* Start peak follower at a reasonable value so beginning of song does not swell or drop. */
	compressor->fastPeak = ( compressor->threshold > (FXP31_MAX_VALUE >> 5) )
	                       ? (FXP31_MAX_VALUE >> 5)
	                       : (compressor->threshold << 5);
	compressor->slowPeak = compressor->fastPeak;

	compressor->frameDelay = (MSEC_DELAY * sampleRate) / MSEC_PER_SECOND; /* DIVIDE - init */

	return SPMIDI_Error_None;
}

/********************************************************************/
void Compressor_CompressBuffer( Compressor_t *compressor, FXP31 *samples, int samplesPerFrame, FXP7 gain )
{
	FXP31  scaledSample;
	FXP31  absSample;
	FXP31  sample;
	long   shiftedPeak;
	long   numerator;
	int    is;
	int    numSamples = SS_FRAMES_PER_BUFFER * samplesPerFrame;
	int    blockMask = (compressor->blockSize * samplesPerFrame) - 1;
	int    sampleDelay = compressor->frameDelay * samplesPerFrame;
	int    readIndex;

	/* Make sure delay is not bigger than array size. */
	if( sampleDelay > (COMP_DELAY_LINE_SIZE - samplesPerFrame) )
	{
		sampleDelay = (COMP_DELAY_LINE_SIZE - samplesPerFrame);
	}

	/* Assumes always called with same samplesPerFrame. */
	readIndex = (compressor->writeIndex - sampleDelay) & COMP_DELAY_LINE_MASK;

	numerator = (((compressor->target + compressor->curvature) * gain) << (COMPRESSOR_SHIFT - 7));

	for( is=0; is<numSamples; is++ )
	{
		/* Occasionally update gain.
		 * The fastPeak only decays in this section. So 
		 * Input value of fastPeak is max of samples in previous sections.
		 */
		if( (compressor->sampleCounter++ & blockMask) == 0 )
		{
			/* Calculate slow peak follower. */
			if( compressor->fastPeak > compressor->slowPeak )
			{
				compressor->slowPeak = (compressor->slowPeak >> COMPRESSOR_SHIFT) * compressor->attack;
			}
			else
			{
				/* Decay for fast and slow follower is the same. */
				compressor->slowPeak = compressor->fastPeak;
			}

			/* Exponentially decay the fast peak by scaling. */
			compressor->fastPeak = (compressor->fastPeak >> COMPRESSOR_SHIFT)  * compressor->decay;

			/* Don't let peak go below threshold. */
			if( compressor->fastPeak < compressor->threshold )
			{
				compressor->fastPeak = compressor->threshold;
			}

			/* Use slowPeak for calculating gain. */
			shiftedPeak = compressor->slowPeak >> ((SS_MIXER_BUS_RESOLUTION - 1) - COMPRESSOR_SHIFT);

			compressor->gain = numerator / (shiftedPeak + compressor->curvature); /* DIVIDE - control rate */
		}

		/* Get next sample from input. Advance pointer when we write to it at end of loop. */
		sample = samples[is];

		/* Write incoming samples to delay line. */
		compressor->delayLine[ compressor->writeIndex + is ] = sample;

		/* Absolute value of current sample. */
		absSample = (sample < 0) ? (0 - sample) : sample;

		/* Traditional peak follower that rises directly with signal. */
		if( absSample > compressor->fastPeak )
		{
			compressor->fastPeak = absSample;
		}

		/* Apply latest gain to delayed signal. */
		scaledSample = (compressor->delayLine[readIndex] >> COMPRESSOR_SHIFT) * compressor->gain;
		readIndex = (readIndex + 1) & COMP_DELAY_LINE_MASK;

		/* Clip to bus rails. */
#if SPMIDI_USE_SOFTCLIP
		/* Soft clip.. */
		scaledSample = SS_MixerSoftClip( scaledSample );
#else
		/* Hard clip. */
		if( scaledSample > SS_MIXER_BUS_MAX )
		{
			scaledSample = SS_MIXER_BUS_MAX;
		}
		else if( scaledSample < SS_MIXER_BUS_MIN )
		{
			scaledSample = SS_MIXER_BUS_MIN;
		}
#endif

		/* Write back to original array. */
#if 1
		samples[is] = scaledSample;
#else
		/* Write debug data to right channel. */
		if( (is & 1) == 0)
		{
			samples[is] = scaledSample;
		}
		/* else samples[is] = compressor->slowPeak >> 2; */
		else
		{
			//samples[is] = compressor->gain << 11;
			samples[is] = compressor->slowPeak;
			//samples[is] = compressor->fastPeak;
		}
#endif

	}
	/* Be careful. This assumes that the writeIndex only wraps at the end of the FOR loop.
	 * So the delay line size must be a multiple of the loop count.
	 */
	compressor->writeIndex = (compressor->writeIndex + numSamples) & COMP_DELAY_LINE_MASK;
}


/********************************************************************/
FXP31 Compressor_EstimateBuffer( Compressor_t *compressor, FXP31 sample, FXP7 gain )
{
	FXP31  compressedSample;
	FXP31  shiftedPeak;

	shiftedPeak = sample >> ((SS_MIXER_BUS_RESOLUTION - 1) - COMPRESSOR_SHIFT);

	compressor->gain = (((compressor->target + compressor->curvature) * gain) << (COMPRESSOR_SHIFT - 7)) /
	                   (shiftedPeak + compressor->curvature);

	compressedSample = (sample >> COMPRESSOR_SHIFT) * compressor->gain;

	return compressedSample;

}

/********************************************************************/
int Compressor_SetParameter( Compressor_t *compressor, SPMIDI_Parameter parameterIndex, int value )
{
	int result = 0;
	switch( parameterIndex )
	{

	case SPMIDI_PARAM_COMPRESSOR_CURVE:
		compressor->curvature = ((value << COMPRESSOR_SHIFT) / 100); /* DIVIDE - init */
		/* Avoid divide-by-zero error if shiftedPeak is also zero. */
		if( compressor->curvature < 16 )
		{
			compressor->curvature = 16;
		}
		break;

	case SPMIDI_PARAM_COMPRESSOR_TARGET:
		compressor->target = ((value << COMPRESSOR_SHIFT) / 100); /* DIVIDE - init */
		break;

	case SPMIDI_PARAM_COMPRESSOR_THRESHOLD:
		compressor->threshold = ((value << (SS_MIXER_BUS_RESOLUTION - 1)) / 100); /* DIVIDE - init */
		break;

	default:
		result = SPMIDI_Error_UnrecognizedParameter;
		break;
	}
	return result;
}

/********************************************************************/
int Compressor_GetParameter( Compressor_t *compressor, SPMIDI_Parameter parameterIndex, int *valuePtr )
{
	int result = 0;
	switch( parameterIndex )
	{

	case SPMIDI_PARAM_COMPRESSOR_CURVE:
		*valuePtr = (compressor->curvature * 100) >> COMPRESSOR_SHIFT;
		break;

	case SPMIDI_PARAM_COMPRESSOR_TARGET:
		*valuePtr = (compressor->target * 100) >> COMPRESSOR_SHIFT;
		break;

	case SPMIDI_PARAM_COMPRESSOR_THRESHOLD:
		*valuePtr = (compressor->threshold * 100) >> (SS_MIXER_BUS_RESOLUTION - 1);
		break;

	default:
		result = SPMIDI_Error_UnrecognizedParameter;
		break;
	}
	return result;
}
