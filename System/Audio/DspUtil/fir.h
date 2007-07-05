// *************************************************************** 
// fir.h:	Header file for Finite Impulse Response (FIR) filters
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#ifndef __FIR_H__
#define	__FIR_H__

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

#define DecibelToLinear(d)		(pow(10.0, ((double)(d))*(1.0/20.0)))
#define LinearToDecibel(x)		(log10((x))*20.0)

#define kFIR_Type_LowPass	0
#define kFIR_Type_HighPass	1
#define kFIR_Type_BandPass	2
#define kFIR_Type_BandStop	3

#define kFIR_Type_HalfBand	10
#define kFIR_Type_ThirdBand	11

typedef struct fir {
// High level data
	long type; // AddDrop, Linear, FIR1
	float outLevelDB;
	long order;
	long coeffCount;

	float samplingFrequency;
// Low level data
#define kFIR_MaxCoeffs 		31
#define kFIR_MaxDelayElements 	31
	float h [kFIR_MaxCoeffs];
	short hI[kFIR_MaxCoeffs];
	short z [kFIR_MaxDelayElements];
	float outLevel;  // Linear scale 
} FIR;

void DefaultFIR(FIR *d);
void UpdateFIR (FIR *d);
void ResetFIR  (FIR *d);
void PrepareFIR(FIR *d);

void RunFIR_Shorts(short *in, short *out, long length, FIR *d);
void RunFIRFixedPoint_Shorts(short *in, short *out, long length, FIR *d);


#endif  //	end __FIR_H__
