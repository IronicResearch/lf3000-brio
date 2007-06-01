#ifndef _SPMIDI_ORCHESTRA_H
#define _SPMIDI_ORCHESTRA_H

/* $Id: spmidi_orchestra.h,v 1.2 2005/11/28 19:04:25 philjmsl Exp $ */
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
#else
#define EDITABLE const
#endif

/** @return Indexed preset structure from instrument library. */
EDITABLE HybridVoice_Preset_t *SS_GetSynthPreset( int presetIndex );

/** @return Address of array of that maps pitch to preset. */
EDITABLE spmUInt8 *SS_GetSynthDrumMap( void );

/** @return Address of array of that maps program to preset. */
EDITABLE spmUInt8 *SS_GetSynthProgramMap( void );

/**
 * MIDI drums are indexed by pitch.
 * But their actual played pitch is independant of the pitch used to 
 * select the drum.
 * This array maps drumIndex to the pitch that will be heard.
 * This allows, for example, one Tom preset to be used for several
 * drums by playing it at different tuned pitches.
 * @return Get array of audio pitches to play instrument for drum.
 */
EDITABLE spmUInt8 *SS_GetSynthDrumPitchArray( void );

/** @return Number of entries in DrumMap */
int SS_GetSynthDrumMapSize( void );

#if SPMIDI_ME2000
/** Defined by Editor to load waves at runtime. */
void WaveManager_LoadWaves( WaveManager_t *waveManager );
#endif

#ifdef __cplusplus
}
#endif

#endif /* _SPMIDI_ORCHESTRA_H */
