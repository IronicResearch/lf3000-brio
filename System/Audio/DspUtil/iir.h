// *************************************************************** 
// iir.h:	Header file for Infinite Impulse Response (IIR) filters
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#ifndef __IIR_H__
#define	__IIR_H__

#include <math.h>

#include "Dsputil.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

#define kIIR_Mode_ByPass	    5
#define kIIR_Mode_LowPass	    0
#define kIIR_Mode_HighPass	    1
#define kIIR_Mode_BandPass	    2
#define kIIR_Mode_BandStop	    3
#define kIIR_Mode_Parametric	4
#define kIIR_Mode_Bypass	    kIIR_Mode_ByPass

#define kIIR1_b0 0
#define kIIR1_b1 1
#define kIIR1_a1 2

#define kIIR1_x1 0
#define kIIR1_y1 1

#define kIIR2_b0 0
#define kIIR2_b1 1
#define kIIR2_b2 2
#define kIIR2_a1 3
#define kIIR2_a2 4

#define kIIR2_x1 0
#define kIIR2_x2 1
#define kIIR2_y1 2
#define kIIR2_y2 3

// IIR Order 1 Filters
#define kIIR1_HalfBand_20_Length 3
extern float iir1_HalfBand_20_RippleGainCompensationDB;
extern float iir1_HalfBand_20_Hz[kIIR1_HalfBand_20_Length];

typedef struct iir {
// High level data
    float frequency;
	float inGainDB;
	float outGainDB;
    float rippleGainCompensationDB;
	long  useFixedPoint; 

    long coeffCount;
    long order;
    long type;

// Low level data
#define kIIR_MaxCoeffs 		    31
#define kIIR_MaxDelayElements 	31
	float hf32[kIIR_MaxCoeffs];
	Q31   hq31[kIIR_MaxCoeffs];
	Q15   hq15[kIIR_MaxCoeffs];

	float zf32[kIIR_MaxDelayElements];
	Q15   zq15[kIIR_MaxDelayElements];
	Q31   zq31[kIIR_MaxDelayElements];

    float inGainf, outGainf;
    float rippleGainCompensationf;
    short inGaini, outGaini;

	float samplingFrequency;
} IIR;

void DefaultIIR(IIR *d);
void UpdateIIR (IIR *d);
void ResetIIR  (IIR *d);
void PrepareIIR(IIR *d);

void SetIIR_Hzf(   IIR *d, float *h, long count);
void SetIIR_Hzf_v2(IIR *d, float *a, float *b, float g, long count);
void SetIIR_HzQ15( IIR *d, Q15 *h, long count);

void RunIIR1_Shortsf(short *in, short *out, long length, IIR *d);
void RunIIR2_Shortsf(short *in, short *out, long length, IIR *d);
void RunIIRN_Shortsf(short *in, short *out, long length, IIR *d);

void RunIIR1_Shortsi(short *in, short *out, long length, IIR *d);
void RunIIR2_Shortsi(short *in, short *out, long length, IIR *d);
void RunIIRN_Shortsi(short *in, short *out, long length, IIR *d);

//void RunIIR_Shortsi(short *in, short *out, long length, IIR *d);
void RunIIR1_Shorts(short *in, short *out, long length, IIR *d);
void RunIIR2_Shorts(short *in, short *out, long length, IIR *d);
void RunIIR_Shorts(short *in, short *out, long length, IIR *d);

#endif  //	end __IIR_H__
