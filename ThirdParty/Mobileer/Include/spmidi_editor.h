#ifndef _SPMIDI_EDITOR_H
#define _SPMIDI_EDITOR_H

/* $Id: spmidi_editor.h,v 1.10 2005/05/03 22:04:00 philjmsl Exp $ */
/**
 *
 * Used internally by Instrument Editor
 * This is not normally called by an application.
 * API is subject to change without notice.
 *
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "spmidi_config.h"
#include "wave_manager.h"
#include "spmidi.h"
#include "dls_parser_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif


	/** Download an instrument definition as a byte stream.
	 * The contents of the definition are specific to the synthesizer in use.
	 * @param insIndex index within internal presets array
	 */
	int SPMIDI_SetInstrumentDefinition( SPMIDI_Context *spmidiContext, int insIndex, unsigned char *data, int numBytes );

	int SPMIDI_SetInstrumentPreset( SPMIDI_Context *spmidiContext, int insIndex, void *inputPreset );

	/** Map a MIDI program number to an instrument index.
	 * This allows multiple programs to be mapped to a single instrument.
	 */
	int SPMIDI_SetInstrumentMap( SPMIDI_Context *spmidiContext, int programIndex, int insIndex );

	/** Map a MIDI drum pitch to an instrument index.
	 * This allows multiple drums to be mapped to a single instrument.
	 * @param noteIndex MIDI pitch of note on rhythm channel that triggers this drum
	 * @param insIndex index used when defining instrument with SPMIDI_SetInstrumentDefinition()
	 * @param pitch pitch of instrument when playing this drum sound
	 */
	int SPMIDI_SetDrumMap( SPMIDI_Context *spmidiContext, int noteIndex, int insIndex, int pitch );

	/** Identify beginning of data stream. */
#define SPMIDI_BEGIN_STREAM    (0x00FF)
	/** Identify end of data stream. */
#define SPMIDI_END_STREAM    (0x00FE)

	typedef enum SPMIDI_StreamID_e
	{
	    SPMIDI_INSTRUMENT_STREAM_ID = 1,
	    SPMIDI_WAVETABLE_STREAM_ID,
	    SPMIDI_WAVESET_STREAM_ID
	} SPMIDI_StreamID;


#if SPMIDI_ME2000

	/** Download a WaveTable for internal storage and use.
	 * The contents of the definition are specific to the synthesizer in use.
	 * Returns negative error or positive waveTable token.
	 */
	int SPMIDI_LoadWaveTable( SPMIDI_Context *spmidiContext, unsigned char *data, int numBytes );

	/* Delete WaveTable if WaveSet reference count is zero. */
	int SPMIDI_UnloadWaveTable( SPMIDI_Context *spmidiContext, spmSInt32 token );

	/** Download a WaveSet for internal storage and use.
	 * The contents of the definition are specific to the synthesizer in use.
	 * Returns negative error or positive waveSet token.
	 */
	int SPMIDI_LoadWaveSet( SPMIDI_Context *spmidiContext, unsigned char *data, int numBytes );

	/* Delete WaveSet if instrument reference count is zero. */
	int SPMIDI_UnloadWaveSet( SPMIDI_Context *spmidiContext, spmSInt32 token );

	int SPMIDI_UnloadAllWaveData( SPMIDI_Context *spmidiContext );

	/** Add a WaveTable for internal storage and use.
	 * The contents of the definition are specific to the synthesizer in use.
	 * Returns negative error or positive waveTable token.
	 */
	int SPMIDI_AddWaveTable( SPMIDI_Context *spmidiContext, WaveTable_t *waveTable );

	/** Add a WaveSet for internal storage and use.
	 * The contents of the definition are specific to the synthesizer in use.
	 * Returns negative error or positive waveSet token.
	 */
	int SPMIDI_AddWaveSet( SPMIDI_Context *spmidiContext, WaveSet_t *waveSet );

#endif /* SPMIDI_ME2000 */

#ifdef __cplusplus
}
#endif

#endif /* _SPMIDI_EDITOR_H */

