#ifndef _OSCILLATOR_H
#define _OSCILLATOR_H

/* $Id: oscillator.h,v 1.24 2007/05/30 16:52:47 philjmsl Exp $ */
/**
 *
 * Oscillator - waveform generator
 *
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "fxpmath.h"
#include "spmidi.h"
#include "wave_manager.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef M_PI
#define M_PI   (3.14159256)
#endif
#define TWO_PI   (2.0 * M_PI)

#define OSC_PHASE_INC_NYQUIST (FXP31_MAX_VALUE)

	/*****************************************************************/
	/********** Oscillator *******************************************/
	/*****************************************************************/


	/** Note: these must match Osc_Init() in oscillator.c
	 * and also order in java HybridEditor.
	 * Also must keep all noise waveforms after periodic waveforms. DEPENDENCY001
	 */
	typedef enum {
	    SINE_PM,
	    TRIANGLE_PM,
	    SAWTOOTH_PM,
	    SQUARE_PM,
	    SINE,
	    TRIANGLE,
	    SAWTOOTH,
	    SQUARE,
	    WHITENOISE,
	    REDNOISE,
	    RANDOMHOLD,
	    PARTICLE,
#if SPMIDI_ME2000
	    WAVETABLE,
#endif
	    SQUARE_HARSH,
	    SQUARE_HARSH_PM,
	    NUM_WAVEFORMS
	} WaveForms;

	#define OSC_FIRST_NOISE_WAVEFORM (WHITENOISE)

	typedef short SPMIDI_SAMPLE; /* TODO - replace every occurence with spmSample in editor */


	typedef void (SS_OscillatorNextProc)( void *osc, FXP31 amplitude, FXP31 *output,
	                                      FXP31 *modulator );

	typedef struct Oscillator_s
	{
		FXP31 phase;
		FXP31 phaseInc;
		/** Function that calculates waveform: saw, square, etc. */
		SS_OscillatorNextProc *nextProc;
		union
		{
			struct
			{ /* Used for smoothing square and saw waveforms. */
				FXP31 level;
				FXP31 scale;
			}
			square;
			struct
			{ /* Used for interpolation by RedNoise and RandomHold. */
				FXP31 previous;
				FXP31 delta;
			}
			interp;
#if SPMIDI_ME2000

			struct
			{
				spmSInt32           sampleIndex;
				/** Set to loopEnd on noteOn, numSamples on noteOff. */
				spmSInt32           endAt;
				spmSInt16           previous;
				spmSInt16           next;
			}
			wave;
#endif

		} shared;
	}
	Oscillator_t;

#define OSC_FLAG_ABSOLUTE_PITCH      (0x01)
#define OSC_FLAG_UNRESOLVED_WAVESET  (0x02)

	/* Do not modify. Must match structure generated by instrument editor. */
	typedef struct Oscillator_Preset_s
	{
		spmUInt8   waveform;
		spmUInt8   flags;
		FXP16      pitchControl; /* relative or absolute pitch in octave pitch fraction*/
	}
	Oscillator_Preset_t;

	void Osc_Init( void );
	void Osc_Term( void );

	unsigned char *Osc_Define( Oscillator_Preset_t *preset, unsigned char *p );

	/*
	 * Called when oscillator is started for a note.
	 * Calculates parameters that are an approximate function of pitch.
	 */
	void Osc_Start(const Oscillator_Preset_t *preset,  Oscillator_t *osc, FXP16 octavePitch, FXP16 srateOffset, WaveSetRegion_t *waveSetRegion );

	void Osc_SetPitch(const Oscillator_Preset_t *preset,  Oscillator_t *osc,
		FXP16 octavePitch, FXP16 srateOffset );

	void Osc_Next_Sine( Oscillator_t *osc, FXP31 amplitude, FXP31 *output, FXP31 *modulator );


#if SPMIDI_ME2000

	void Osc_SetWavePitch( const Oscillator_Preset_t *preset, Oscillator_t *osc,
                   FXP16 octavePitch, FXP16 srateOffset, WaveSetRegion_t *waveSetRegion );

	void Osc_WaveTableS16( Oscillator_t *osc, const WaveTable_t *waveTable, FXP31 *output );
	void Osc_WaveTableU8( Oscillator_t *osc, const WaveTable_t *waveTable, FXP31 *output );
	void Osc_WaveTableALaw( Oscillator_t *osc, const WaveTable_t *waveTable, FXP31 *output );

	void Osc_WaveTable_Release( Oscillator_t *osc, const WaveTable_t *waveTable );

#if SPMIDI_USE_REGIONS
	WaveSetRegion_t *Osc_FindMatchingRegion( const WaveSet_t *waveSet, int pitch, int velocity );
#else
	WaveTable_t *Osc_SelectNearestWaveTable( const WaveSet_t *waveSet, FXP16 octavePitch, int velocity );
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif /* _OSCILLATOR_H */

