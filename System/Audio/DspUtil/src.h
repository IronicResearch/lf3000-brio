// *************************************************************** 
// src.h:	Header file for sampling rate conversion (SRC) routines
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#ifndef __SRC_H__
#define	__SRC_H__

#include <math.h>
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

#define kSRC_Interpolation_Type_AddDrop	   0
#define kSRC_Interpolation_Type_Linear	   1
#define kSRC_Interpolation_Type_FIR	       2
#define kSRC_Interpolation_Type_IIR	       3
#define kSRC_Interpolation_Type_Unfiltered 4
#define kSRC_Interpolation_Type_Triangle   5
#define kSRC_Interpolation_Type_Box	       6

#define kSRC_Interpolation_Type_TestFIR	   10

#define kSRC_FilterVersion_0 0
#define kSRC_FilterVersion_1 1
#define kSRC_FilterVersion_2 2
#define kSRC_FilterVersion_3 3
#define kSRC_FilterVersion_4 4
#define kSRC_FilterVersion_5 5
#define kSRC_FilterVersion_6 6
#define kSRC_FilterVersion_7 7
#define kSRC_FilterVersion_8 8
#define kSRC_FilterVersion_9 9
#define kSRC_FilterVersion_10 10
#define kSRC_FilterVersion_Default kSRC_FilterVersion_0

#define kSRC_Linear_MSBits 13
#define kSRC_Linear_LSBits (32-kSRC_Linear_MSBits)
#define kSRC_Linear_LSmask (0xFFFFFFFF>>kSRC_Linear_MSBits)
#define kSRC_Linear_MSmask (0xFFFFFFFF ^ kSRC_Linear_LSmask)
#define kSRC_Linear_Divisor (1<<kSRC_Linear_LSBits)

typedef struct src {
	long type; 
	long filterVersion; 
	long useFixedPoint;
	long useImpulse;

	float inSamplingFrequency;
	float outSamplingFrequency;

	unsigned long xI, xIncI;
	float         xF, xIncF;
	float inOutRateRatio;
	float outInRateRatio;	

	float inScale, inScaleDB;

#define kSRC_Filter_MaxCoeffs 		    64
#define kSRC_Filter_MaxDelayElements 	64
	float *hP;
	float h [kSRC_Filter_MaxCoeffs];
	short hI[kSRC_Filter_MaxCoeffs];
	short z [kSRC_Filter_MaxDelayElements];
	long firLength;
	long firCoeffCount;

	long  computingDone;
	float samplingFrequency;
} SRC;

void DefaultSRC(SRC *d);
void UpdateSRC (SRC *d);
void ResetSRC  (SRC *d);
void PrepareSRC(SRC *d);
void SRC_SetInSamplingFrequency (SRC *d, float x);
void SRC_SetOutSamplingFrequency(SRC *d, float x);

char *TranslateSRC_ModeID(int id);

void RunSRC(short *in, short *out, long inLength, long outLength, SRC *d);

#endif  //	__SRC_H__
