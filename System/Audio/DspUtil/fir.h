// *************************************************************** 
// fir.h:	Header file for Finite Impulse Response (FIR) filters
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#ifndef __FIR_H__
#define	__FIR_H__

#include <math.h>

#include "Dsputil.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

#define kFIR_Type_LowPass	0
#define kFIR_Type_HighPass	1
#define kFIR_Type_BandPass	2
#define kFIR_Type_BandStop	3

// 1/2 Band filters  (fs/4)
#define kFIR_Type_HalfBand_15	10
#define kFIR_Type_HalfBand_31	11
#define kFIR_Type_HalfBand_32dB	12
#define kFIR_Type_HalfBand_58dB	13
#define kFIR_Type_HalfBand	kFIR_Type_HalfBand_15

// 1/3 band filters (fs/6)
#define kFIR_Type_ThirdBand_15	20
#define kFIR_Type_ThirdBand_31	21
#define kFIR_Type_ThirdBand	kFIR_Type_ThirdBand_15

// Triangle filters 
#define kFIR_Type_Triangle_3	30
#define kFIR_Type_Triangle_9	31

typedef struct fir {
// High level data
	long type; // AddDrop, Linear, FIR1
	float outLevelDB;
	long order;
	long coeffCount;
	long useFixedPoint; 

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

//void RunFIR_Shortsf(short *in, short *out, long length, FIR *d);
//void RunFIR_Shortsi(short *in, short *out, long length, FIR *d);
void RunFIR_Shorts(short *in, short *out, long length, FIR *d);


#define kFIR_Triangle_3_Hz_Length 3
extern float fir_Triangle_3_GainCompensationDB;
extern float fir_Triangle_3_Hz[kFIR_Triangle_3_Hz_Length];

#define kFIR_Triangle_9_Hz_Length 9
extern float fir_Triangle_9_GainCompensationDB;
extern float fir_Triangle_9_Hz[kFIR_Triangle_9_Hz_Length];

#define kFIR_BandPass_Hz_Length	15
extern float fir_BandPass_GainCompensationDB;
extern float fir_BandPassHz[kFIR_BandPass_Hz_Length];

#define kFIR_BandStop_Hz_Length	17
extern float fir_BandStop_GainCompensationDB;
extern float fir_BandStopHz[kFIR_BandStop_Hz_Length];

#define kFIR_Notch_Hz_Length 15
extern float fir_Notch_GainCompensationDB;
extern float fir_Notch_Hz[kFIR_Notch_Hz_Length];

#define kFIR_HighPass_Hz_Length	15
extern float fir_HighPass_GainCompensationDB;
extern float fir_HighPass_Hz[kFIR_HighPass_Hz_Length];

#define kFIR_LowPass_Hz_Length	15
extern float fir_LowPass_GainCompensationDB;
extern float fir_LowPass_Hz[kFIR_LowPass_Hz_Length];

// Half band filters
#define kFIR_HalfBand_32dB_Hz_Length	15
extern float fir_HalfBand_32dB_GainCompensationDB;
extern float fir_HalfBand_32dB_Hz[kFIR_HalfBand_32dB_Hz_Length];

#define kFIR_HalfBand_58dB_Hz_Length	31
extern float fir_HalfBand_58dB_GainCompensationDB;
extern float fir_HalfBand_58dB_Hz[kFIR_HalfBand_58dB_Hz_Length];

#define kFIR_HalfBand_15_Hz_Length 15
extern float fir_HalfBand_15_GainCompensationDB;
extern float fir_HalfBand_15_Hz[kFIR_HalfBand_15_Hz_Length];

#define kFIR_HalfBand_31_Hz_Length 31
extern float fir_HalfBand_31_GainCompensationDB;
extern float fir_HalfBand_31_Hz[kFIR_HalfBand_31_Hz_Length];

// Third band filters
#define kFIR_ThirdBand_15_Hz_Length 15
extern float fir_ThirdBand_15_GainCompensationDB ;
extern float fir_ThirdBand_15_Hz[kFIR_ThirdBand_15_Hz_Length];

#define kFIR_ThirdBand_31_Hz_Length 31
extern float fir_ThirdBand_31_GainCompensationDB;
extern float fir_ThirdBand_31_Hz[kFIR_ThirdBand_31_Hz_Length]; 

#endif  //	end __FIR_H__
