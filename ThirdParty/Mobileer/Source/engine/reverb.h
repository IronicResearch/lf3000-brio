#ifndef _REVERB_H
#define _REVERB_H
/* $Id: reverb.h,v 1.8 2005/10/26 17:18:11 philjmsl Exp $ */
/**
 *
 * Reverberation - delay based effect that adds ambience to a sound
 *
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "fxpmath.h"
#include "spmidi.h"
#include "spmidi_synth.h"

/** Delay line size. Must be power of two so we can mask indices. */
#define REVERB_MAX_DELAY    (4096)
#define REVERB_INDEX_MASK   (REVERB_MAX_DELAY - 1)

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct Reverb_s
	{
		int     writeIndex;
		/** Use an IIR lowpass filter to attenuate high frequencies faster than low frequencies. */
		int     filterOutput;
		int     framesSinceTapsMoved;
		short   delayLine[REVERB_MAX_DELAY];
	}
	Reverb_t;

	void Reverb_Next( Reverb_t *reverb, FXP31 *input, FXP31 *output, int numSamples, int isStereoInput );

#ifdef __cplusplus
}
#endif

#endif /* _REVERB_H */
