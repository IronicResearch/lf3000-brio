//============================================================================
//  sf.cpp : some libsndfile routines
//
//============================================================================
#include <math.h>
#include <stdlib.h>
#include <stdio.h>#include <string.h>

#include "sf.h"

//==============================================================================
// PrintBrioAudioHeader  :   print description of tAudioHeader structure to stdio
//==============================================================================
    void 
PrintBrioAudioHeader( tAudioHeader *d )
{	     
//struct tAudioHeader {
//	U32	  offsetToData;		// Offset from the start of the header to
//					// the start of the data (std is 16)
//	tRsrcType type;			// AudioRsrcType
//	U16	  flags;		// Bit mask of audio flags
//					// (Bit0: 0=mono, 1=stereo)
//	U16	 sampleRate;		// Sample rate in Hz			
//	U32	 dataSize;		// Data size in bytes
//};

printf("PrintBrioAudioHeader $%X \n", d);
printf("    offsetToData = %d flags=$X dataSize=%d Bytes\n", 
        d->offsetToData, d->flags, d->dataSize);
printf("    AudioRsrcType = %d \n", d->type);
printf("    channels      = %d \n", (0x1 & d->flags) + 1);
printf("    sampleRate    = %d Hz\n", d->sampleRate);

}		// ---- end PrintBrioAudioHeader() ----



