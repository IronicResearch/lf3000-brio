/* $Id: spmidi_orchestra.c,v 1.2 2005/11/28 19:04:09 philjmsl Exp $ */
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


/* Include Preset data instead of linking. */
#if SPMIDI_ME2000
	#if SPMIDI_SUPPORT_EDITING
		#include "spmidi_presets_editor_me2000.h"

	#elif defined(SPMIDI_PRESETS_CUSTOM)
		#include "spmidi_presets_custom.h"

	#elif defined(SPMIDI_PRESETS_CUSTOM_1)
		#include "spmidi_presets_custom_1.h"

	#elif defined(SPMIDI_PRESETS_CUSTOM_2)
		#include "spmidi_presets_custom_2.h"

	#elif defined(SPMIDI_PRESETS_CUSTOM_3)
		#include "spmidi_presets_custom_3.h"

	#elif defined(SPMIDI_PRESETS_CUSTOM_4)
		#include "spmidi_presets_custom_4.h"
	#else
		#include "spmidi_hybrid_presets_me2000.h"

	#endif
#else /* SPMIDI_ME2000 */
	#include "spmidi_hybrid_presets_me1000.h"
#endif /* SPMIDI_ME2000 */

EDITABLE HybridVoice_Preset_t *SS_GetSynthPreset( int presetIndex )
{
	return &gHybridSynthPresets[ presetIndex ];
}

EDITABLE spmUInt8 *SS_GetSynthDrumMap( void )
{
	return gHybridSynthDrumMap;
}

int SS_GetSynthDrumMapSize( void )
{
	return sizeof( gHybridSynthDrumMap );
}

EDITABLE spmUInt8 *SS_GetSynthProgramMap( void )
{
	return gHybridSynthProgramMap;
}

EDITABLE spmUInt8 *SS_GetSynthDrumPitchArray( void )
{
	return gHybridSynthDrumPitches;
}

