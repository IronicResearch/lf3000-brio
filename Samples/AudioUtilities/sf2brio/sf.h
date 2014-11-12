// *************************************************************** 
// sf.h:	Header file for libsndfile functions
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#ifndef __SF_H__
#define	__SF_H__

#//include "sfconfig.h"
#include "sndfile.h"

// Brio Header stuff
//#include <CoreTypes.h>
//#include <SystemTypes.h>
//#include <AudioTypes.h>
//#include <AudioTypesPriv.h>

#ifdef __cplusplus
extern "C" {
#endif
/* Add whatever here */
#ifdef __cplusplus
}
#endif

// Some types defined for Brio
typedef unsigned long  U32;
typedef unsigned short U16;
typedef                U32 tRsrcType;

// Brio audio file header
// 
//#define kAudioRsrcType 0x10001C05
struct tAudioHeader {
	U32	  offsetToData;		// Offset from the start of the header to
					        // the start of the data (std is 16)
//	tRsrcType type;			// AudioRsrcType 0x10001C05;	// Brio raw type
	U16	  flags;		    // Bit mask of audio flags
					        // (Bit0: 0=mono, 1=stereo)
	U16	 sampleRate;		// Sample rate in Hz			
	U32	 dataSize;		    // Data size in bytes
};

void PrintBrioAudioHeader( tAudioHeader *d );
int CheckForBrioAudioHeader(char *filePath, tAudioHeader *d );

#endif  //	end __SF_H__


