// *************************************************************** 
// main.h:	Header file for this project
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#ifndef __MAIN_H__
#define	__MAIN_H__

//#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

#ifndef False
#define False 0
#endif
#ifndef True
#define True 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif


#define kOutFileFormat_Unspecified	(-1)
#define kOutFileFormat_Brio	0
#define kOutFileFormat_BRIO	kOutFileFormat_Brio
#define kOutFileFormat_AIFF	1
#define kOutFileFormat_WAV	2
#define kOutFileFormat_Raw	3
#define kOutFileFormat_RAW	kOutFileFormat_Raw

#define kMaxSamplingFrequency_UpsamplingRatio	3
#define kMaxChannels	2

#define kSamplingFrequency_Unspecified (-1)
extern long inSamplingFrequency;
extern long outSamplingFrequency;
extern long outSamplingFrequencySpecified;

extern long inBlockLength, outBlockLength;
extern long inSamplingRate, outSamplingRate;

extern long channels;

#endif  //	__MAIN_H__
