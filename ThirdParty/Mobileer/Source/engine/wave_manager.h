#ifndef _WAVE_MANAGER_H
#define _WAVE_MANAGER_H

/* $Id: wave_manager.h,v 1.10 2006/03/24 19:59:35 philjmsl Exp $ */
/**
 *
 * WaveTable and WaveSet manager.
 * Maintain lists of wave resources.
 *
 * @author Phil Burk, Copyright 2004 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */

#include "fxpmath.h"
#include "dbl_list.h"
#include "spmidi.h"
#include "spmidi_synth_util.h"
#include "resource_mgr.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum WaveType_e
{
	SPMIDI_WAVE_TYPE_S16 = 0, /* Default, in case type not initialized. */
	SPMIDI_WAVE_TYPE_ALAW
} WaveType;


	typedef struct WaveTable_s
	{
		ResourceTracker_t tracker;
		void             *samples; /**< May be ALaw bytes or signed 16 bit PCM. */
		spmSInt32         numSamples;
		PitchOctave       basePitch; /**< Pitch of recorded note. */
		PitchOctave       sampleRateOffset; /**< Pitch offset to account for custom sample rate. 22050 Hz => 0x010000 */
		/** Sample index of beginning of loop, or -1 if no loop. */
		spmSInt32         loopBegin;
		/** Sample index of end of loop, or -1 if no loop. */
		spmSInt32         loopEnd;
		/** SPMIDI_WAVE_TYPE_S16 or SPMIDI_WAVE_TYPE_ALAW */
		spmSInt8          type;
		/** Low Velocity for note matching. */
		spmSInt8          lowVelocity; 
		/** High Velocity for note matching. If zero then assume 127 */
		spmSInt8          highVelocity; 
	}
	WaveTable_t;

	typedef struct WaveSet_s
	{
		ResourceTracker_t tracker;
		int               numTables;
		WaveTable_t	    **tables;
	}
	WaveSet_t;

	typedef struct WaveManager_s
	{
		DoubleList       waveTableList;
		DoubleList       waveSetList;
		spmResourceToken nextID;
		spmSInt32        totalWaveBytes;
	}
	WaveManager_t;


	/* Initialize linked lists and prepare for storing tables. */
	spmSInt32 WaveManager_Init( WaveManager_t *waveManager );

	/* Free all lingering data. */
	spmSInt32 WaveManager_Term( WaveManager_t *waveManager );

	/** Download a WaveTable for internal storage and use.
	 * The contents of the definition are specific to the synthesizer in use.
	 */
	spmSInt32 WaveManager_LoadWaveTable( WaveManager_t *waveManager, unsigned char *data, int numBytes );

	/* Delete WaveTable if WaveSet reference count is zero. */
	spmSInt32 WaveManager_UnloadWaveTable( WaveManager_t *waveManager, spmResourceToken token );

	/** Download a WaveSet for internal storage and use.
	 * The contents of the definition are specific to the synthesizer in use.
	 */
	spmSInt32 WaveManager_LoadWaveSet( WaveManager_t *waveManager, unsigned char *data, int numBytes );

	/* Delete WaveSet if instrument reference count is zero. */
	spmSInt32 WaveManager_UnloadWaveSet( WaveManager_t *waveManager, spmResourceToken token );

	WaveSet_t *WaveManager_FindWaveSet( WaveManager_t *waveManager, spmResourceToken token );

	/**
	 * @param id should be preallocated by editor or WAVEMGR_UNDEFINED_ID
	 */
	spmSInt32 WaveManager_AddWaveSet( WaveManager_t *waveManager, WaveSet_t *waveSet, spmResourceToken id );

	/****************************************************************/
	/** Download a ready WaveTable for internal storage and use.
	 * Return negative error or positive token.
	 */
	spmSInt32 WaveManager_AddWaveTable( WaveManager_t *waveManager, WaveTable_t *waveTable );

#ifdef __cplusplus
}
#endif

#endif /* _WAVE_MANAGER_H */

