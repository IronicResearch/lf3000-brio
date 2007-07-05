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

#define kFIR_HalfBandV1_Length 15
extern float firHalfBand_V1_GainCompensationDB;
extern float firHalfBandHz_V1[kFIR_HalfBandV1_Length];

#define kFIR_HalfBand31_Length 31
extern float firHalfBand_31_GainCompensationDB;
extern float firHalfBand31Hz[kFIR_HalfBand31_Length];

#define kFIR_BandPassHz_Length	15
extern float firBandPass_GainCompensationDB;
extern float firBandPassHz[kFIR_BandPassHz_Length];

#define kFIR_BandStopHz_Length	17
extern float firBandStop_GainCompensationDB;
extern float firBandStopHz[kFIR_BandStopHz_Length];

#define kFIR_Notch_Hz_Length 15
extern float fir_Notch_GainCompensationDB;
extern float fir_Notch_Hz[kFIR_Notch_Hz_Length];

#define kFIR_HighPassHz_Length	15
extern float firHighPass_GainCompensationDB;
extern float firHighPassHz[kFIR_HighPassHz_Length];

#define kFIR_LowPassHz_Length	15
extern float firLowPass_GainCompensationDB;
extern float firLowPassHz[kFIR_LowPassHz_Length];

#define kFIR_LowPass_HalfBandHz_Length	15
extern float firLowPass_HalfBand_GainCompensationDB;
extern float firLowPass_HalfBandHz[kFIR_LowPass_HalfBandHz_Length];

#define kFIR_LowPass58dB_HalfBandHz_Length	31
extern float firLowPass58dB_HalfBand_GainCompensationDB;
extern float firLowPass58dB_HalfBandHz[kFIR_LowPass58dB_HalfBandHz_Length];

#define kFIR_ThirdBandV1_Hz_Length 15
extern float fir_ThirdBandV1_GainCompensationDB ;
extern float fir_ThirdBandV1_Hz[kFIR_ThirdBandV1_Hz_Length];

#endif  //	end __FIR_H__
