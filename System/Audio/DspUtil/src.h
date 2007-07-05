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
#define kSRC_Interpolation_Type_FIR	   2
#define kSRC_Interpolation_Type_IIR	   3
#define kSRC_Interpolation_Type_Unfiltered 4
#define kSRC_Interpolation_Type_Triangle   5

#define kSRC_Interpolation_Type_TestFIR	10

#define kSRC_Linear_MSBits 13
#define kSRC_Linear_LSBits (32-kSRC_Linear_MSBits)
#define kSRC_Linear_LSmask (0xFFFFFFFF>>kSRC_Linear_MSBits)
#define kSRC_Linear_MSmask (0xFFFFFFFF ^ kSRC_Linear_LSmask)
#define kSRC_Linear_Divisor (1<<kSRC_Linear_LSBits)

typedef struct src {
	long type; // AddDrop, Linear, FIR1
	long useFixedPoint;
	long useImpulse;

	float inSamplingFrequency;
	float outSamplingFrequency;

	unsigned long arg, argInc;
	float x, delta;

	float inScale, inScaleDB;

	float inOutRateRatio;
	float outInRateRatio;	

#define kSRC_Filter_MaxCoeffs 		63
#define kSRC_Filter_MaxDelayElements 	63
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

void RunSRC(short *in, short *out, long inLength, long outLength, SRC *d);

#endif  //	__SRC_H__
