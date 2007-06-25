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
#include "memtools.h"
#include "spmidi.h"
#include "spmidi_host.h"
#include "spmidi_synth.h"
#include "spmidi_hybrid.h"
#include "instrument_mgr.h"

#define SPMIDI_USE_DRUMBIT  (1)


/* Use to create default instrument with reasonable values. */
#define DEFAULT_RISEFALL_TIME   (1)
static const HybridVoice_Preset_t sInstrumentTemplate =
    { /* Pure Sawtooth */
        { SINE, 0, 0x0 },
        { SAWTOOTH, 0, 0x0 },
        { SINE, 0, 0x0 },
        { DEFAULT_RISEFALL_TIME, 1, 1023, DEFAULT_RISEFALL_TIME, 0, 0 | 0 | 0 },
        { DEFAULT_RISEFALL_TIME, 1, 1023, DEFAULT_RISEFALL_TIME, 0, 0 | 0 | 0 },
        { DEFAULT_RISEFALL_TIME, 1, 1023, DEFAULT_RISEFALL_TIME, 0, 0 | 0 | 0 },
        { 0x0, 0x0, 0 },
        0x00000000, /* phaseModDepth */
        0x0, /* lfoPitchModDepth */
        0x0, /* lfoCutoffModDepth */
        0x0, /* envPitchModDepth */
        0x0, /* envCutoffModDepth */
        0 | 0 | 0, /* flags */
        1, /* boostLog2 */
        60, /* keyCenter */
        128, /* keyScalar */
    };


/* Initialize linked lists and prepare for storing instruments. */
void InsManager_InitializePreset( HybridVoice_Preset_t *preset )
{
	MemTools_Copy( preset, &sInstrumentTemplate, sizeof( sInstrumentTemplate ) );
}

#if SPMIDI_SUPPORT_EDITING


/* Combine bank and program into unique token for quick search.
 * Add one so that token for piano in bank zero in non-zero.
 */
static spmResourceToken MakeInsToken( int bankIndex, int programIndex )

{
	spmResourceToken token = (bankIndex << 8) | (programIndex + 1);
#if SPMIDI_USE_DRUMBIT
	/* DLS may pass a drum indicator in the high bit. */
	if( (bankIndex & SPMIDI_DRUMBANK_FLAG) != 0 )
	{
		token |= SPMIDI_DRUMBANK_FLAG;
	}
#endif
	return token;
}

/* Initialize linked lists and prepare for storing instruments. */
SPMIDI_Error InsManager_Init( InsManager_t *insManager )
{
	DLL_InitList( &insManager->insList );
	return SPMIDI_Error_None;
}

/* Free all lingering instruments. */
SPMIDI_Error InsManager_Term( InsManager_t *insManager )
{
	return InsManager_Clear( insManager );
}

/* Allocate an instrument and set up its default values. */
CustomIns_t *InsManager_CreateInstrument( InsManager_t *insManager )
{
	/* Make an instrument preset. */
	CustomIns_t *customInstr = SPMIDI_ALLOC_MEM( sizeof( CustomIns_t ), "CustomIns_t" );
	if( customInstr == NULL )
		return NULL;

	(void) insManager;

	ResourceMgr_InitResource( &customInstr->tracker );
	/* Set reasonable default values for instrument. */
	InsManager_InitializePreset( &customInstr->preset );
	return customInstr;
}

/* Delete an instrument. */
SPMIDI_Error InsManager_DeleteInstrument( InsManager_t *insManager, CustomIns_t *instrument )
{
	if( instrument->tracker.token != RESOURCE_UNDEFINED_ID )
	{
		ResourceMgr_Remove( &instrument->tracker );
	}
	SPMIDI_FreeMemory( instrument );
	(void) insManager;
	return SPMIDI_Error_None;
}

/** Download an Instrument for internal storage and use.
 * The contents of the definition are specific to the synthesizer in use.
 */
SPMIDI_Error InsManager_Add( InsManager_t *insManager, CustomIns_t *instrument, int bankIndex, int programIndex )
{
	spmResourceToken token = MakeInsToken( bankIndex, programIndex );
	ResourceMgr_Add( &insManager->insList, &instrument->tracker, token );
	return (SPMIDI_Error) token;
}

/* Unload all instruments and delete them. */
SPMIDI_Error InsManager_Clear( InsManager_t *insManager )
{
	while( !DLL_IsEmpty( &insManager->insList ) )
	{
		CustomIns_t *instrument = (CustomIns_t *) DLL_First( &insManager->insList );
		InsManager_DeleteInstrument( insManager, instrument );
	}
	return SPMIDI_Error_None;
}

CustomIns_t *InsManager_Find( InsManager_t *insManager, int bankIndex, int programIndex )
{
	spmResourceToken token= MakeInsToken( bankIndex, programIndex );
	return (CustomIns_t *) ResourceMgr_Find( &insManager->insList, token );
}

#endif /* SPMIDI_SUPPORT_EDITING */

