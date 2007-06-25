/* $Id: spmidi_orchestra.c,v 1.8 2007/06/18 18:03:49 philjmsl Exp $ */
/**
 *
 * Orchestra data.
 *
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#if defined(WIN32) || defined(MACOSX)
#include <math.h>
#endif

#include "dbl_list.h"
#include "fxpmath.h"
#include "midi.h"
#include "spmidi.h"
#include "spmidi_synth_util.h"
#include "spmidi_host.h"
#include "spmidi_synth.h"
#include "spmidi_hybrid.h"
#include "spmidi_print.h"
#include "spmidi_dls.h"
#include "compressor.h"
#include "adsr_envelope.h"
#include "oscillator.h"
#include "wave_manager.h"

#include "spmidi_orchestra.h"

/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
/* Define these here becase they are needed by the following
 * include of spmidi_presets_*.h */
/* These are specific to a melodic bank or drum program. */
typedef unsigned char ProgramMapIndex;

#define SS_PROGRAM_MAP_ALLOCATED   (1<<0)

/** Use same structure for both MelodicMaps and DrumPrograms
 * Note that this structure is initialized from the MObileer Editor so do NOT change its structure.
 * Adding to it is OK.
 */
typedef struct ProgramBankMap_s
{
	DoubleNode node;
	short bankIndex;
	unsigned char programIndex;
	unsigned char flags;
	ProgramMapIndex *presetMap;
	ProgramMapIndex *pitches;
} ProgramBankMap_t;

void SS_AddMelodicBankToList( ProgramBankMap_t *melodicBank );
void SS_AddDrumProgramToList( ProgramBankMap_t *drumProgram );

#define MELODIC_STUB_INDEX  (0)
#define DRUM_STUB_INDEX     (1)

/*********************************************************************/
/******* Load Preset Orchestra ***************************************/
/*********************************************************************/
/* Include Preset data instead of linking. */
#if SPMIDI_ME2000
	#if SPMIDI_SUPPORT_EDITING
		#define SPMIDI_PRESET_FILENAME_H "spmidi_presets_editor_me2000.h"

	#elif defined(SPMIDI_PRESETS_CUSTOM)
		#define SPMIDI_PRESET_FILENAME_H "spmidi_presets_custom.h"

	#elif defined(SPMIDI_PRESETS_CUSTOM_1)
		#define SPMIDI_PRESET_FILENAME_H "spmidi_presets_custom_1.h"

	#elif defined(SPMIDI_PRESETS_CUSTOM_2)
		#define SPMIDI_PRESET_FILENAME_H "spmidi_presets_custom_2.h"

	#elif defined(SPMIDI_PRESETS_CUSTOM_3)
		#define SPMIDI_PRESET_FILENAME_H "spmidi_presets_custom_3.h"

	#elif defined(SPMIDI_PRESETS_CUSTOM_4)
		#define SPMIDI_PRESET_FILENAME_H "spmidi_presets_custom_4.h"

	#elif defined(SPMIDI_PRESETS_TEST)
		#define SPMIDI_PRESET_FILENAME_H "spmidi_presets_test.h"

	#else
		#define SPMIDI_PRESET_FILENAME_H "spmidi_hybrid_presets_me2000.h"

	#endif
#else /* SPMIDI_ME2000 */
	#define SPMIDI_PRESET_FILENAME_H "spmidi_hybrid_presets_me1000.h"
#endif /* SPMIDI_ME2000 */

#include SPMIDI_PRESET_FILENAME_H

/* Verify that the included preset file is at the necessary revision level. */
#ifndef SPMIDI_EDITOR_VERSION
	#define SPMIDI_EDITOR_VERSION (0)
#endif

#define SPMIDI_EARLIEST_COMPATIBLE_VERSION (225)

#if  SPMIDI_EDITOR_VERSION < SPMIDI_EARLIEST_COMPATIBLE_VERSION
	#error Included preset file is obsolete. Please use newly exported instrument set.
#endif

static DoubleList sMelodicBankList;
static DoubleList sDrumProgramList;

/** @return Number of entries in DrumMap */
static int SS_GetSynthDrumMapSize( void );


/********************************************************************************/
static void SS_FreeMelodicBank( ProgramBankMap_t *melodicBank )
{
	if( (melodicBank->flags & SS_PROGRAM_MAP_ALLOCATED) != 0 )
	{
		SPMIDI_FREE_MEM( melodicBank );
	}
}

/********************************************************************************/
static void SS_FreeDrumProgram( ProgramBankMap_t *drumProgram )
{
	if( (drumProgram->flags & SS_PROGRAM_MAP_ALLOCATED) != 0 )
	{
		SPMIDI_FREE_MEM( drumProgram->pitches );
		SS_FreeMelodicBank( drumProgram );
	}
}

/********************************************************************************/
/** Used by editor.
 * Allocate presets array right after structure.
 * Do not add to list because it may be a drum.
 */
static ProgramBankMap_t *SS_AllocateMelodicBankNode( int bankIndex )
{
	int i;
	int numBytes = sizeof(ProgramBankMap_t) + 128;
	ProgramBankMap_t *melodicBank = SPMIDI_ALLOC_MEM( numBytes, "ProgramBankMap_t" );
	if( melodicBank == NULL )
	{
		return NULL;
	}

	melodicBank->presetMap = (unsigned char *) &melodicBank[1]; // point to memory after structure
	melodicBank->bankIndex = (short) bankIndex;
	melodicBank->programIndex = (short) 0;
	melodicBank->flags |= SS_PROGRAM_MAP_ALLOCATED;

	/* Fill map tables. */
	for( i=0; i<GMIDI_NUM_PROGRAMS; i++ )
	{
		melodicBank->presetMap[i] = MELODIC_STUB_INDEX;
	}

	return melodicBank;
}

/********************************************************************************/
void SS_AddMelodicBankToList( ProgramBankMap_t *melodicBank )
{
	DLL_InitNode( &melodicBank->node );
	DLL_AddTail( &sMelodicBankList, &melodicBank->node );
}

/********************************************************************************/
void SS_AddDrumProgramToList( ProgramBankMap_t *drumProgram )
{
	DLL_InitNode( &drumProgram->node );
	DLL_AddTail( &sDrumProgramList, &drumProgram->node );
}

/********************************************************************************/
static ProgramBankMap_t *SS_AllocateMelodicBank( int bankIndex )
{
	ProgramBankMap_t *melodicBank = SS_AllocateMelodicBankNode( bankIndex );
	if( melodicBank != NULL )
	{
		SS_AddMelodicBankToList( melodicBank );
	}
	return melodicBank;
}

/********************************************************************************/
static ProgramBankMap_t *SS_AllocateDrumProgram( int bankIndex, int programIndex )
{
	int i;
	ProgramBankMap_t *drumProgram = SS_AllocateMelodicBankNode( bankIndex );
	if( drumProgram == NULL )
	{
		return NULL;
	}

	// Add pitch array.
	drumProgram->pitches = SPMIDI_ALLOC_MEM( SS_GetSynthDrumMapSize(), "DrumPitches" );
	if( drumProgram->pitches == NULL )
	{
		SS_FreeMelodicBank( drumProgram );
		return NULL;
	}

	drumProgram->programIndex = (unsigned char) programIndex;

	for( i=0; i<GMIDI_NUM_DRUMS; i++ )
	{
		drumProgram->presetMap[i] = DRUM_STUB_INDEX;
		drumProgram->pitches[i] = 60; /* Middle C */
	}
	
	SS_AddDrumProgramToList( drumProgram );
	
	return drumProgram;
}


/********************************************************************************/
static ProgramBankMap_t *SS_FindMelodicBank( int bankIndex )
{
	ProgramBankMap_t *melodicBank = NULL;
	ProgramBankMap_t *candidate;
	/* Find voice that is on and playing the pitch. */
	DLL_FOR_ALL( ProgramBankMap_t, candidate, &sMelodicBankList )
	{
		if( candidate->bankIndex == bankIndex )
		{
			melodicBank = candidate;
			break;
		}
	}
	return melodicBank;
}


/********************************************************************************/
/* Always return some bank so that we always have an instrument to play. */
static ProgramBankMap_t *SS_FindMelodicBankSafe( int bankIndex )
{
	ProgramBankMap_t *melodicBank = SS_FindMelodicBank( bankIndex );
	//PRTMSGNUMD("SS_FindMelodicBankSafe: bankIndex = ", bankIndex );
	//PRTMSGNUMD("SS_FindMelodicBankSafe: melodicBank = ", (int)melodicBank );
	if( melodicBank == NULL )
	{
		if( DLL_IsEmpty( &sMelodicBankList ) )
		{
			melodicBank = SS_AllocateMelodicBank( 0 );
		}
		else
		{
			melodicBank = (ProgramBankMap_t *) DLL_First( &sMelodicBankList );
		}
	}
	//PRTMSGNUMD("SS_FindMelodicBankSafe: melodicBank.bankIndex = ", melodicBank->bankIndex );
	return melodicBank;
}

/********************************************************************************/
static ProgramBankMap_t * SS_FindDrumProgram( int bankIndex, int programIndex )
{
	ProgramBankMap_t *drumProgram = NULL;
	ProgramBankMap_t *candidate;
	/* Find voice that is on and playing the pitch. */
	DLL_FOR_ALL( ProgramBankMap_t, candidate, &sDrumProgramList )
	{
		if( (candidate->bankIndex == bankIndex) && (candidate->programIndex == programIndex))
		{
			drumProgram = candidate;
			break;
		}
	}
	return drumProgram;
}

/********************************************************************************/
/* Always return some bank so that we always have an instrument to play. */
static ProgramBankMap_t *SS_FindDrumProgramSafe( int bankIndex, int programIndex )
{
	ProgramBankMap_t *drumProgram = SS_FindDrumProgram( bankIndex, programIndex );
	//PRTMSGNUMD("SS_FindDrumProgramSafe: bankIndex = ", bankIndex );
	//PRTMSGNUMD("SS_FindDrumProgramSafe: programIndex = ", programIndex );
	//PRTMSGNUMD("SS_FindDrumProgramSafe: drumProgram = ", (int)drumProgram );
	if( drumProgram == NULL )
	{
		if( DLL_IsEmpty( &sDrumProgramList ) )
		{
			drumProgram = SS_AllocateDrumProgram( 0, 0 );
		}
		else
		{
			drumProgram = (ProgramBankMap_t *) DLL_First( &sDrumProgramList );
		}
	}
	//PRTMSGNUMD("SS_FindDrumProgramSafe: drumProgram->bankIndex = ", drumProgram->bankIndex );
	//PRTMSGNUMD("SS_FindDrumProgramSafe: drumProgram->programIndex = ", drumProgram->programIndex );
	return drumProgram;
}

/********************************************************************************/
static int SS_GetSynthDrumMapSize( void )
{
	return GMIDI_NUM_DRUMS;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
EDITABLE HybridVoice_Preset_t *SS_GetSynthPreset( int presetIndex )
{
	return &gHybridSynthPresets[ presetIndex ];
}

/********************************************************************************/
EDITABLE HybridVoice_Preset_t *SS_GetSynthMelodicPreset( int bankIndex, int programIndex )
{
	ProgramBankMap_t *melodicBank = SS_FindMelodicBankSafe( bankIndex );
	int presetIndex = melodicBank->presetMap[programIndex];
	return SS_GetSynthPreset( presetIndex );
}

/********************************************************************************/
EDITABLE HybridVoice_Preset_t *SS_GetSynthDrumPreset( int bankIndex, int programIndex, int pitch )
{
	int presetIndex;
	ProgramBankMap_t *drumProgram;
	int drumIndex = pitch - GMIDI_FIRST_DRUM;
	if( (drumIndex < 0) ||
		(drumIndex >= SS_GetSynthDrumMapSize()) )
	{
		drumIndex = 0;
	}
	drumProgram = SS_FindDrumProgramSafe( bankIndex, programIndex );
	presetIndex = drumProgram->presetMap[drumIndex];
	return SS_GetSynthPreset( presetIndex );
}

/********************************************************************************/
int SS_GetSynthPresetCount( void )
{
	return SS_NUM_VALID_PRESETS;
}

/********************************************************************************/
int SS_GetSynthDrumPitch( int bankIndex, int programIndex, int noteIndex )
{
	ProgramBankMap_t *drumProgram = SS_FindDrumProgramSafe( bankIndex, programIndex );
	int drumIndex = noteIndex - GMIDI_FIRST_DRUM;
	if( (drumIndex < 0) ||
			(drumIndex >= SS_GetSynthDrumMapSize()) )
	{
		drumIndex = 0;
	}
	return drumProgram->pitches[drumIndex];
}

/********************************************************************************/
int SS_SetInstrumentMap( int bankIndex, int programIndex, int insIndex )
{
	ProgramBankMap_t *melodicBank;

	if( (programIndex < 0) || (programIndex > 127) )
		return SPMIDI_Error_OutOfRange;
	if( (insIndex >= SS_MAX_PRESETS) || (insIndex < 0) )
		return SPMIDI_Error_OutOfRange;
	
	melodicBank = SS_FindMelodicBank( bankIndex );
	if( melodicBank == NULL )
	{
		// Make sure we have a fallback bank of zero.
		if( DLL_IsEmpty( &sMelodicBankList ) && (bankIndex != 0) )
		{
			SS_AllocateMelodicBank( 0 );
		}
		melodicBank = SS_AllocateMelodicBank( bankIndex );
	}

	melodicBank->presetMap[programIndex] = (unsigned char) insIndex;
	return 0;
}

/********************************************************************************/
int SS_SetDrumMap( int bankIndex, int programIndex, int noteIndex, int insIndex, int pitch )
{
	ProgramBankMap_t *drumProgram;
	int drumIndex = noteIndex - GMIDI_FIRST_DRUM;
	
	if( (drumIndex >= GMIDI_NUM_DRUMS) || (drumIndex < 0) )
		return SPMIDI_Error_OutOfRange;
	if( (insIndex >= SS_MAX_PRESETS) || (insIndex < 0) )
		return SPMIDI_Error_OutOfRange;
	if( (pitch > 127) || (pitch < 0) )
		return SPMIDI_Error_OutOfRange;

	drumProgram = SS_FindDrumProgram( bankIndex, programIndex );
	if( drumProgram == NULL )
	{
		// Make sure we have a fallback bank of zero.
		if( DLL_IsEmpty( &sDrumProgramList ) && (bankIndex != 0) && (programIndex != 0) )
		{
			SS_AllocateDrumProgram( 0, 0 );
		}
		drumProgram = SS_AllocateDrumProgram( bankIndex, programIndex );
	}

	drumProgram->presetMap[drumIndex] = (unsigned char) insIndex;
	drumProgram->pitches[drumIndex] = (unsigned char) pitch;
	
	return 0;
}

/********************************************************************************/
void SS_ChangeMelodicMap( int oldBankIndex, int newBankIndex )
{
	ProgramBankMap_t *melodicMap = SS_FindMelodicBank( oldBankIndex );
	//PRTMSGNUMD("SS_ChangeMelodicMap: oldBankIndex = ", oldBankIndex );
	//PRTMSGNUMD("SS_ChangeMelodicMap: newBankIndex = ", newBankIndex );
	//PRTMSGNUMD("SS_ChangeMelodicMap: melodicMap = ", (int)melodicMap );
	if( melodicMap != NULL )
	{
		melodicMap->bankIndex = (short) newBankIndex;
	}
}

/********************************************************************************/
void SS_ChangeDrumMap( int oldBankIndex, int oldProgramIndex, int newBankIndex, int newProgramIndex )
{
	ProgramBankMap_t *drumProgram = SS_FindDrumProgram( oldBankIndex, oldProgramIndex );
	
	//PRTMSGNUMD("SS_ChangeDrumMap: oldBankIndex = ", oldBankIndex );
	//PRTMSGNUMD("SS_ChangeDrumMap: oldProgramIndex = ", oldProgramIndex );
	//PRTMSGNUMD("SS_ChangeDrumMap: newBankIndex = ", newBankIndex );
	//PRTMSGNUMD("SS_ChangeDrumMap: newProgramIndex = ", newProgramIndex );
	//PRTMSGNUMD("SS_ChangeDrumMap: drumProgram = ", (int)drumProgram );
	if( drumProgram != NULL )
	{
		drumProgram->bankIndex = (short) newBankIndex;
		drumProgram->programIndex = (unsigned char) newProgramIndex;
	}
}

/********************************************************************************/
void SS_RemoveMelodicMap( int bankIndex )
{
	ProgramBankMap_t *melodicBank;

	melodicBank = SS_FindMelodicBank( bankIndex );
	//PRTMSGNUMD("SS_RemoveMelodicMap: bankIndex = ", bankIndex );
	//PRTMSGNUMD("SS_RemoveMelodicMap: melodicBank = ", (int)melodicBank );
	if( melodicBank != NULL )
	{
		DLL_Remove( &melodicBank->node );
		SS_FreeMelodicBank( melodicBank );
	}
}
/********************************************************************************/
void SS_RemoveDrumMap( int bankIndex, int programIndex )
{
	ProgramBankMap_t *drumProgram;

	drumProgram = SS_FindDrumProgram( bankIndex, programIndex );
	//PRTMSGNUMD("SS_RemoveDrumMap: bankIndex = ", bankIndex );
	//PRTMSGNUMD("SS_RemoveDrumMap: programIndex = ", programIndex );
	//PRTMSGNUMD("SS_RemoveDrumMap: drumProgram = ", (int)drumProgram );
	if( drumProgram != NULL )
	{
		DLL_Remove( &drumProgram->node );
		SS_FreeDrumProgram( drumProgram );
	}
}

/********************************************************************************/
int SS_Orchestra_Init( void )
{
	DLL_InitList( &sMelodicBankList );
	DLL_InitList( &sDrumProgramList );
	return 0;
}

/********************************************************************************/
void SS_Orchestra_Term( void )
{
	while( !DLL_IsEmpty( &sMelodicBankList ) )
	{
		ProgramBankMap_t *melodicBank = (ProgramBankMap_t *) DLL_First( &sMelodicBankList );
		DLL_Remove( &melodicBank->node );
		SS_FreeMelodicBank( melodicBank );
	}
	while( !DLL_IsEmpty( &sDrumProgramList ) )
	{
		ProgramBankMap_t *drumProgram = (ProgramBankMap_t *) DLL_First( &sDrumProgramList );
		DLL_Remove( &drumProgram->node );
		SS_FreeDrumProgram( drumProgram );
	}
}
