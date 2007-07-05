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

#define kSRC_Interpolation_Type_AddDrop	0
#define kSRC_Interpolation_Type_Linear	1
#define kSRC_Interpolation_Type_FIR1	2
#define kSRC_Interpolation_Type_Triangle 3

#define kSRC_Interpolation_Type_TestFIR	10

typedef struct src {
	long type; // AddDrop, Linear, FIR1
	long useFixedPoint;

	float inSamplingFrequency;
	float outSamplingFrequency;

	float inScale, inScaleDB;
	float x;

	float inOutRateRatio;
	float outInRateRatio;	

#define kSRC_Filter_MaxCoeffs 		31
#define kSRC_Filter_MaxDelayElements 	31
	float h[kSRC_Filter_MaxCoeffs];
	short hI[kSRC_Filter_MaxCoeffs];
	short z[kSRC_Filter_MaxDelayElements];
	long firLength;
	long firCoeffCount;

	float samplingFrequency;
} SRC;

void DefaultSRC(SRC *d);
void UpdateSRC (SRC *d);
void ResetSRC  (SRC *d);
void PrepareSRC(SRC *d);

void RunSRC(short *in, short *out, long inLength, long outLength, SRC *d);

#endif  //	__SRC_H__
