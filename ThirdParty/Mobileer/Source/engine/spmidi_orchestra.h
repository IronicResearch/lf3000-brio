#ifndef _SPMIDI_ORCHESTRA_H
#define _SPMIDI_ORCHESTRA_H

/* $Id: spmidi_orchestra.h,v 1.7 2007/06/12 21:09:08 philjmsl Exp $ */
/**
 *
 * Isolate Orchestra data so it can be linked separately from the code.
 *
 * @author Phil Burk, Copyright 2005 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include "spmidi_config.h"
#include "spmidi.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Determine whether instrument is read-only. */
#if SPMIDI_SUPPORT_EDITING
#define EDITABLE /* */
#define SPMIDI_MAX_PROGRAM_MAP_BANKS (256)
#else
#define EDITABLE const
#endif

/** @return Indexed preset structure from instrument library. */
EDITABLE HybridVoice_Preset_t *SS_GetSynthPreset( int presetIndex );

/** @return number of valid preset structures in instrument library */
int SS_GetSynthPresetCount( void );

/** @return preset corresponding to drum bank, program and pitch. */
EDITABLE HybridVoice_Preset_t *SS_GetSynthDrumPreset( int bank, int program, int pitch );

/** @return preset corresponding to melodic bank and program. */
EDITABLE HybridVoice_Preset_t *SS_GetSynthMelodicPreset( int bank, int program );

/** Map a MIDI program number to an instrument index.
 * This allows multiple programs to be mapped to a single instrument.
 */
int SS_SetInstrumentMap( int bankIndex, int programIndex, int insIndex );

/** Map a MIDI drum pitch to an instrument index.
 * This allows multiple drums to be mapped to a single instrument.
 */
int SS_SetDrumMap( int bankIndex, int programIndex, int noteIndex, int insIndex, int pitch );

/**
 * MIDI drums are indexed by noteIndex or "pitch".
 * But their actual played pitch is independant of the pitch used to 
 * select the drum.
 * This maps MIDI note to the acoustic pitch that will be heard.
 * This allows, for example, one Tom preset to be used for several
 * drums by playing it at different tuned pitches.
 * @return playback pitch.
 */
int SS_GetSynthDrumPitch( int bank, int program, int noteIndex );

int SS_SetSynthDrumPitch( int bank, int program, int noteIndex, int pitch );

#if SPMIDI_ME2000
/** Defined by Editor to load waves at runtime. */
void WaveManager_LoadWaves( WaveManager_t *waveManager );
#endif

int SS_Orchestra_Init( void );
void SS_Orchestra_Term( void );

/** This function is defined in the Preset file exported by the
 * Mobileer Instrument Editor. */
void SS_LoadPresetOrchestra( void );

#ifdef __cplusplus
}
#endif

#endif /* _SPMIDI_ORCHESTRA_H */
