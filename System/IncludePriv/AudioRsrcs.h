#ifndef AUDIORSRCS_H
#define AUDIORSRCS_H

//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		AudioRsrcs.h
//
// Description:
//		Audio resource configuration structures.
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>

//==============================================================================
//==============================================================================
#define kAudioRsrcConfigFmtVersion	MakeVersion(1, 0)

struct tAudioRsrcConfig {
//	tVersion		fmtVersion;		// format version of this struct
									// (kAudioRsrcConfigFmtVersion)
//	U16				reserved1;		// reserved/alignment (must be 0)

	U8				numChannels;	// Number of audio Channels
									// Number of samples in audio channel is 
									// sampleRateInKHz*tickInMs
									// Each sample is 4 bytes (16-bit stereo)  
	U8				sampleRateInKHz;// Output sample rate in KHz (32)
	U8				tickInMs;		// Tick in ms (4)
	U8				bufSizeRatio;	// Ratio of input buffer to channel buffer size
									// Ratio >= 2  
};

#endif		// AUDIORSRCS_H

// EOF	
