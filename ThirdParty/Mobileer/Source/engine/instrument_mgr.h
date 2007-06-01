#ifndef _INSTRUMENT_MGR_H
#define _INSTRUMENT_MGR_H

/* $Id$ */
/**
 *
 * Manager for Externally Loaded Instruments.
 *
 * @author Phil Burk, Copyright 2004 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "fxpmath.h"
#include "dbl_list.h"
#include "spmidi.h"
#include "resource_mgr.h"
#include "spmidi_synth.h"
#include "spmidi_voice.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SPMIDI_DRUMBANK_FLAG   (0x80000000)

	typedef struct CustomIns_s
	{
		ResourceTracker_t   tracker;
		HybridVoice_Preset_t      preset;
	}
	CustomIns_t;

	typedef struct InsManager_s
	{
		DoubleList      insList;
	}
	InsManager_t;

	void InsManager_InitializePreset( HybridVoice_Preset_t *preset );

#if SPMIDI_SUPPORT_EDITING


	/* Initialize linked lists and prepare for storing instruments. */
	SPMIDI_Error InsManager_Init( InsManager_t *insManager );

	/* Free all lingering data. */
	SPMIDI_Error InsManager_Term( InsManager_t *insManager );

	/* Allocate an instrument and set up its default values. */
	CustomIns_t *InsManager_Create( InsManager_t *insManager );

	/* Delete an instrument. */
	SPMIDI_Error InsManager_Delete( InsManager_t *insManager, CustomIns_t *instrument );

	/** Download an Instrument for internal storage and use.
	 * The contents of the definition are specific to the synthesizer in use.
	 */
	SPMIDI_Error InsManager_Add( InsManager_t *insManager, CustomIns_t *instrument, int bankIndex, int programIndex );

	/* Unload all instruments and delete them. */
	SPMIDI_Error InsManager_Clear( InsManager_t *insManager );

	CustomIns_t *InsManager_Find( InsManager_t *insManager, int bankIndex, int programIndex );


	CustomIns_t *SPMIDI_CreateCustomInstrument( SPMIDI_Context *spmidiContext );

	int SPMIDI_AddCustomInstrument( SPMIDI_Context *spmidiContext, CustomIns_t * instrument, int bankIndex, int programIndex );

	int SPMIDI_UnloadAllCustomInstruments( SPMIDI_Context *spmidiContext );

#endif

#ifdef __cplusplus
}
#endif

#endif /* _INSTRUMENT_MGR_H */

