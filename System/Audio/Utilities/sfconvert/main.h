// *************************************************************** 
// main.h:	Header file for this project
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#ifndef __MAIN_H__
#define	__MAIN_H__

#include <math.h>

#include "src.h"
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

#define kOutFileFormat_Unspecified	(-1)
#define kOutFileFormat_Brio	0
#define kOutFileFormat_AIFF	1
#define kOutFileFormat_WAV	2
#define kOutFileFormat_Raw	3

#define kMaxSamplingFrequency_UpsamplingRatio	3
#define kMaxChannels	2

#define kSamplingFrequency_Unspecified (-1)
extern long inSamplingFrequency;
extern long outSamplingFrequency;
extern long outSamplingFrequencySpecified;

extern SRC srcData[kMaxChannels];
extern long inBlockLength, outBlockLength;
extern long inSamplingRate, outSamplingRate;

extern long channels;

// MIDI variables
extern long midiSamplingFrequency;
extern long midiVoiceCount;
extern long midiVoiceLimit;

int PlayMIDIFile(char *inFileName, char *outFileName, double *fileTime, double *execTime);

#endif  //	__MAIN_H__
