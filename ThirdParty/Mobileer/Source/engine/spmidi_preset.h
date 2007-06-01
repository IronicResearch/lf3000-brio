#ifndef _SPMIDI_PRESET_H
#define _SPMIDI_PRESET_H
/* $Id: spmidi_preset.h,v 1.2 2005/05/13 23:15:32 philjmsl Exp $ */
/**
 *
 * Hybrid Synthesizer for SPMIDI Engine.
 *
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "fxpmath.h"
#include "midi.h"
#include "spmidi.h"
#include "spmidi_synth_util.h"
#include "oscillator.h"
#include "adsr_envelope.h"
#include "svfilter.h"
#include "reverb.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Program description loaded from library with sample rate independant parameters. */
typedef struct HybridVoice_Preset_s
{
    Oscillator_Preset_t      modOsc;
    Oscillator_Preset_t      mainOsc;
    Oscillator_Preset_t      lfo;
    EnvelopeADSR_Preset_t    modEnv;
    EnvelopeADSR_Preset_t    mainEnv;
    EnvelopeADSR_Preset_t    ampEnv;
    SVFilter_Preset_t        filter;
    FXP31                    phaseModDepth; /* Also controls the level of the mix for modOsc when. */
/* Alias for ME3000 */
#define vibratoPitchModDepth phaseModDepth

    PitchOctave              lfoPitchModDepth;
    PitchOctave              lfoCutoffModDepth;
    PitchOctave              envPitchModDepth;
    PitchOctave              envCutoffModDepth;
    unsigned char            flags;
    signed char              boostLog2;
	unsigned char            keyCenter;
	unsigned char            keyScalar;  /* Uses 0x80 as the nominal value. */
#if SPMIDI_ME2000
	spmSInt32                waveSetID; /* Index or token that identifies a waveSet. Zero if not used. */
#endif
} HybridVoice_Preset_t;

#ifdef __cplusplus
}
#endif

#endif /* _SPMIDI_PRESET_H */
