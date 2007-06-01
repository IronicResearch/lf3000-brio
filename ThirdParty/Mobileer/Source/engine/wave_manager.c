/* $Id: wave_manager.c,v 1.17 2006/03/24 19:59:35 philjmsl Exp $ */
/**
 *
 * WaveTable manager.
 *
 * Copyright 2004 Mobileer, Phil Burk, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "fxpmath.h"
#include "spmidi.h"
#include "spmidi_synth_util.h"
#include "spmidi_synth.h"
#include "spmidi_host.h"
#include "spmidi_print.h"
#include "spmidi_editor.h"
#include "oscillator.h"
#include "wave_manager.h"
#include "memtools.h"

#if SPMIDI_ME2000

#if SPMIDI_SUPPORT_EDITING
static WaveSet_t *WaveManager_UnreferenceWaveSet( WaveSet_t *waveSet );
static WaveTable_t *WaveManager_UnreferenceWaveTable( WaveTable_t *waveTable );
#endif /* SPMIDI_SUPPORT_EDITING */

static void WaveManager_FreeWaveTable( WaveTable_t *waveTable );
static void WaveManager_FreeWaveSet( WaveSet_t *waveSet );


#define INCREMENT_REFERENCE(trkr) { if( trkr.referenceCount > 0 ) trkr.referenceCount += 1; }
#define DECREMENT_REFERENCE(trkr) { if( trkr.referenceCount > 1 ) trkr.referenceCount -= 1; }
#define CLEAR_REFERENCE(trkr) { if( trkr.referenceCount > 1 ) trkr.referenceCount = 1; }

#define GET_NEXT_ID   (waveManager->nextID++)

/****************************************************************/
/* Initialize linked lists and prepare for storing tables. */
spmSInt32  WaveManager_Init( WaveManager_t *waveManager )
{
	DLL_InitList( &waveManager->waveTableList );
	DLL_InitList( &waveManager->waveSetList );
	/* Start at 1 so we know zero means no ID. */
	waveManager->nextID = 1;
	waveManager->totalWaveBytes = 0;
	return 0;
}

/****************************************************************/
/* Free all lingering data. */
spmSInt32 WaveManager_Term( WaveManager_t *waveManager )
{
	while( !DLL_IsEmpty( &waveManager->waveSetList ) )
	{
		WaveSet_t *waveSet = (WaveSet_t *) DLL_First( &waveManager->waveSetList );
		DLL_Remove( &waveSet->tracker.node );
		if( waveSet->tracker.referenceCount > 0 )
		{
			WaveManager_FreeWaveSet(waveSet);
		}
	}
	while( !DLL_IsEmpty( &waveManager->waveTableList ) )
	{
		WaveTable_t *waveTable = (WaveTable_t *) DLL_First( &waveManager->waveTableList );
		DLL_Remove( &waveTable->tracker.node );
		if( waveTable->tracker.referenceCount > 0 )
			WaveManager_FreeWaveTable(waveTable);
	}
	return SPMIDI_Error_None;
}

/****************************************************************/
/** Add WaveSet to managed storage.
 * There is potential conflict if a caller passes a low ID.
 * So this should only be called internally when wavetables are loaded.
 */
spmSInt32 WaveManager_AddWaveSet( WaveManager_t *waveManager, WaveSet_t *waveSet, spmResourceToken id )
{

	ResourceMgr_InitResource( &waveSet->tracker );
	if( id == RESOURCE_UNDEFINED_ID )
	{
		id = GET_NEXT_ID; /* TODO review calls to this function and nextID */
	}
	if( waveManager->nextID <= id )
	{
		waveManager->nextID = id + 1;
	}
	ResourceMgr_Add( &waveManager->waveSetList, &waveSet->tracker, id );

	return id;
}

WaveSet_t * WaveManager_FindWaveSet( WaveManager_t *waveManager, spmResourceToken token )
{
	return (WaveSet_t *) ResourceMgr_Find( &waveManager->waveSetList, token );
}


#if SPMIDI_SUPPORT_EDITING

static void WaveManager_FreeWaveTable( WaveTable_t *waveTable )
{
	SPMIDI_FreeMemory( waveTable->samples );    /* MemoryTracking - Free #2 */
	SPMIDI_FreeMemory( waveTable );     /* MemoryTracking -  Free #1 */
}
static void WaveManager_FreeWaveSet( WaveSet_t *waveSet )
{
	SPMIDI_FreeMemory( waveSet );     /* MemoryTracking - Free #3 */
}

/****************************************************************/
/** Download a ready WaveTable for internal storage and use.
 * Return negative error or positive token.
 */
spmSInt32 WaveManager_AddWaveTable( WaveManager_t *waveManager, WaveTable_t *waveTable )
{
	ResourceMgr_InitResource( &waveTable->tracker );
	ResourceMgr_Add( &waveManager->waveTableList, &waveTable->tracker, GET_NEXT_ID );

	waveManager->totalWaveBytes += waveTable->numSamples * sizeof(spmSInt16);

	PRTMSGNUMD("totalWaveBytes = ", waveManager->totalWaveBytes );
	return waveTable->tracker.token;
}

/****************************************************************/
/* If reference count goes down to 1 then delete waveTable and return NULL
 * Return waveTable if not deleted.
 */
static WaveTable_t *WaveManager_UnreferenceWaveTable( WaveTable_t *waveTable )
{
	if( waveTable == NULL )
		return NULL;
	DECREMENT_REFERENCE(waveTable->tracker);
	/* Is everyone done using this? */
	if( waveTable->tracker.referenceCount == 1 )
	{
		if( waveTable->samples != NULL )
			SPMIDI_FreeMemory( waveTable->samples );
		DLL_Remove( &waveTable->tracker.node );
		WaveManager_FreeWaveTable( waveTable );
		waveTable = NULL;
	}
	return waveTable;
}

/****************************************************************/
/* Delete WaveTable if WaveSet reference count is zero. */
spmSInt32 WaveManager_UnloadWaveTable( WaveManager_t *waveManager, spmResourceToken token )
{
	WaveTable_t *waveTable = (WaveTable_t *) ResourceMgr_Find( &waveManager->waveTableList, token );
	if( waveTable == NULL )
	{
		return SPMIDI_Error_BadToken;
	}

	/* Mark as being invalid for lookup by this routine. */
	waveTable->tracker.token = 0;
	WaveManager_UnreferenceWaveTable( waveTable );

	return SPMIDI_Error_None;
}


/****************************************************************/
/** Download a WaveTable for internal storage and use.
 * The contents of the definition are specific to the synthesizer in use.
 * Data is in the form of a byte stream so it can come from Java.
 * Return negative error or positive token.
 */
spmSInt32 WaveManager_LoadWaveTable( WaveManager_t *waveManager, unsigned char *data,
                                     int numBytes )
{
	WaveTable_t *waveTable;
	int err = SPMIDI_Error_BadFormat;
	unsigned char *p = data;
	int numSampleBytes;
	int bytesRead;
	int i;
	FXP16 midiPitch;

	/* Parse stream header */
	/*
		Must match stream builder in Java editor and test routines.
		WaveTable byte stream
		1	SPMIDI_BEGIN_STREAM
		1	WaveTableStreamID
		4 	PitchOctave      basePitch;
		4	PitchOctave      sampleRateOffset;
		4	int              loopBegin;
		4	int              loopEnd;
		4 	int              numSamples
		1   byte             velocity
		1   byte             type; 0 for signed 16, 1 for ALaw

		finally
		  numSamples*2 short[]	samples; if S16
		OR
		  numSamples byte[]	samples; if ALaw
		1	SPMIDI_END_STREAM
	*/
	if( *p++ != SPMIDI_BEGIN_STREAM )
		goto error;
	if( *p++ != SPMIDI_WAVETABLE_STREAM_ID )
		goto error;

	/* Allocate WaveTable */
	waveTable = SPMIDI_AllocateMemory(sizeof(WaveTable_t));  /* MemoryTracking - Allocation #1 */
	if( waveTable == NULL )
	{
		err = SPMIDI_Error_OutOfMemory;
		goto error;
	}
	MemTools_Clear( waveTable, sizeof(WaveTable_t) );
	DLL_InitNode( &waveTable->tracker.node );
	INCREMENT_REFERENCE(waveTable->tracker);

	p = SS_ParseLong( &midiPitch, p );
	waveTable->basePitch = SPMUtil_MIDIPitchToOctave( midiPitch );
	p = SS_ParseLong( &waveTable->sampleRateOffset, p );
	p = SS_ParseLong( &waveTable->loopBegin, p );
	p = SS_ParseLong( &waveTable->loopEnd, p );

	p = SS_ParseLong( &waveTable->numSamples, p );

	waveTable->lowVelocity = *p++;
	waveTable->highVelocity = *p++;

	if( *p++ == SPMIDI_WAVE_TYPE_ALAW )
	{
		unsigned char *samples8;

		numSampleBytes = waveTable->numSamples; /* ALaw is one byte per sample. */
		PRTMSGNUMD("Unpacking ALAW, numSampleBytes = ", numSampleBytes );
		samples8 = SPMIDI_AllocateMemory(numSampleBytes);     /* MemoryTracking - Allocation #2 */
		if( samples8 == NULL )
		{
			err = SPMIDI_Error_OutOfMemory;
			goto cleanup;
		}

		/* Move bytes from stream to waveTable. */
		waveTable->samples = samples8;
		for( i=0; i<numSampleBytes; i++ )
		{
			*samples8++ = *p++;
		}
		waveTable->type = SPMIDI_WAVE_TYPE_ALAW;
		PRTMSGNUMD("Finished ALAW, numSampleBytes = ", numSampleBytes );
	}
	else
	{
		spmSample *samples;
		numSampleBytes = waveTable->numSamples * sizeof(spmSample);
		samples = SPMIDI_AllocateMemory(numSampleBytes);     /* MemoryTracking - Allocation #2 */
		if( samples == NULL )
		{
			err = SPMIDI_Error_OutOfMemory;
			goto cleanup;
		}

		/* Parse from BigEndian data format to CPU native format. */
		waveTable->samples = samples;
		for( i=0; i<waveTable->numSamples; i++ )
		{
			p = SS_ParseShort( samples++, p );
		}
		waveTable->type = SPMIDI_WAVE_TYPE_S16;
	}

	if( *p++ != SPMIDI_END_STREAM )
		goto cleanup;
	bytesRead = p - data;
	if( bytesRead != numBytes )
	{
		err = SPMIDI_Error_IllegalSize;
		goto cleanup;
	}

	return WaveManager_AddWaveTable( waveManager, waveTable );

cleanup:
	WaveManager_UnreferenceWaveTable( waveTable );
error:
	return err;
}

/****************************************************************/
/** Download a WaveSet for internal storage and use.
 * The contents of the definition are specific to the synthesizer in use.
 */
spmSInt32 WaveManager_LoadWaveSet( WaveManager_t *waveManager, unsigned char *data, int numBytes )
{
	WaveTable_t **tableArray;
	WaveSet_t *waveSet = NULL;
	int err = SPMIDI_Error_BadFormat;
	int numTables;
	unsigned char *p = data;
	int bytesRead;
	int i;
	int waveSetSize;

	/* Parse stream header */
	if( *p++ != SPMIDI_BEGIN_STREAM )
		goto error;
	if( *p++ != SPMIDI_WAVESET_STREAM_ID )
		goto error;

	/* Parse table tokens from stream, find them and fill array. */
	numTables = *p++;
	if( numTables == 0 )
	{
		err = SPMIDI_Error_OutOfRange;
		goto error;
	}

	/* Allocate WaveSet with table pointer array at end. */
	waveSetSize = sizeof(WaveSet_t) + (numTables * sizeof(WaveTable_t *));
	waveSet = SPMIDI_AllocateMemory(waveSetSize);   /* MemoryTracking - Allocation #3 */
	if( waveSet == NULL )
	{
		err = SPMIDI_Error_OutOfMemory;
		goto error;
	}
	MemTools_Clear( waveSet, sizeof(WaveSet_t) );
	DLL_InitNode( &waveSet->tracker.node );
	INCREMENT_REFERENCE(waveSet->tracker);

	/* Table array is immediately after the waveset. */
	tableArray = (WaveTable_t **)(waveSet + 1);
	waveSet->tables = tableArray;
	for( i=0; i<numTables; i++ )
	{
		WaveTable_t *waveTable;
		spmResourceToken token;

		p = SS_ParseLong( (long *) &token, p );
		waveTable = (WaveTable_t *) ResourceMgr_Find( &waveManager->waveTableList, token );
		if( waveTable == NULL )
		{
			err = SPMIDI_Error_BadToken;
			goto cleanup;
		}

		INCREMENT_REFERENCE(waveTable->tracker);
		waveSet->numTables += 1;
		tableArray[i] = waveTable;
	}

	if( *p++ != SPMIDI_END_STREAM )
		goto cleanup;
	bytesRead = p - data;
	if( bytesRead != numBytes )
	{
		err = SPMIDI_Error_IllegalSize;
		goto cleanup;
	}

	/* Assign ID and add WaveWrapper to manager. */
	WaveManager_AddWaveSet( waveManager, waveSet, GET_NEXT_ID );

	return waveSet->tracker.token;

cleanup:
	WaveManager_UnreferenceWaveSet( waveSet );
error:
	return err;
}


/****************************************************************/
/* If reference count == 1 then delete waveSet and return NULL
 * Return waveSet if not deleted.
 */
static WaveSet_t * WaveManager_UnreferenceWaveSet( WaveSet_t *waveSet )
{
	int i;
	if( waveSet == NULL )
		return NULL;
	DECREMENT_REFERENCE(waveSet->tracker);
	/* Is everyone done using this? */
	if( waveSet->tracker.referenceCount == 1 )
	{
		for( i=0; i<waveSet->numTables; i++ )
		{
			WaveTable_t *waveTable = waveSet->tables[i];
			WaveManager_UnreferenceWaveTable( waveTable );
		}
		ResourceMgr_Remove( &waveSet->tracker );
		SPMIDI_FreeMemory(waveSet);
		waveSet = NULL;
	}
	return waveSet;
}

/****************************************************************/
/* Delete WaveSet if instrument reference count is zero. */
spmSInt32 WaveManager_UnloadWaveSet( WaveManager_t *waveManager, spmResourceToken token )
{
	WaveSet_t *waveSet = (WaveSet_t *) ResourceMgr_Find( &waveManager->waveSetList, token );
	if( waveSet == NULL )
	{
		return SPMIDI_Error_BadToken;
	}
	/* Mark as being invalid for lookup by this routine. */
	waveSet->tracker.token = 0;
	WaveManager_UnreferenceWaveSet( waveSet );

	return SPMIDI_Error_None;
}
#else /* SPMIDI_SUPPORT_EDITING */
/* Stubs for runtime.
* These are required because simpler stubs result in compiler warnings.
*/
static void WaveManager_FreeWaveTable( WaveTable_t *waveTable )
{
	(void) waveTable;
}
static void WaveManager_FreeWaveSet( WaveSet_t *waveSet )
{
	(void) waveSet;
}
#endif /* SPMIDI_SUPPORT_EDITING */


#endif  /* SPMIDI_ME2000 */
