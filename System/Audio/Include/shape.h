// *************************************************************************
//
// shape.h Header file for signal non-linear shaping functions
//
//				Written by Gints Klimanis, 2007
// *************************************************************************

#ifndef __SHAPE_H__
#define	__SHAPE_H__

#include "Dsputil.h"

#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------
// ----  Data and Functions ----
// -------------------------------------------
#define kWaveShaper_Type_ByPass	 0
#define kWaveShaper_Type_V1      1
#define kWaveShaper_Type_V2      2
#define kWaveShaper_Type_V3      3
#define kWaveShaper_Type_V4      4

#define kWaveShaper_InGainDB_Default  ( 0.0f)
#define kWaveShaper_InGainDB_Min      (-96.0f)
#define kWaveShaper_InGainDB_Max      ( 24.0f)

#define kWaveShaper_OutGainDB_Default ( -kWaveShaper_InGainDB_Default)
#define kWaveShaper_OutGainDB_Min     (-96.0f)
#define kWaveShaper_OutGainDB_Max     ( 24.0f)

#define kWaveShaper_ThresholdDB_Default ( -3.0f)
#define kWaveShaper_ThresholdDB_Min     (-96.0f)
#define kWaveShaper_ThresholdDB_Max     (  0.0f)

typedef struct waveshaper {
// High Level parameters
    float   inGainDB;
    float   outGainDB;
    float   thresholdDB;
	int		type;
	int     useFixedPoint;
    int     headroomBits;

// Low Level data
    float   inGainf;           // linear
    float   outGainf;          // linear
    float   thresholdf;        // linear
    Q15     inGaini;
    Q15     outGaini;
    Q15     thresholdi;

    Q15   inWholeI;
    float inFracF ;
    Q15   inFracI ;

    float   oneThf, twoThf;           
    long    oneThi, twoThi;           

    long q31_2    ; // = FloatToQ15v2(2.0f);
    long q31_3    ; // = FloatToQ15v2(3.0f);
    long q31_3rd  ; // = FloatToQ15v2(1.0f/3.0f);
    Q15  q15_2_3rd; // = FloatToQ15v2(2.0f/3.0f);

	int		lowLevelDataBogus;
	float	samplingFrequency;
} WAVESHAPER;

#define WaveShaperInvalidate(d)	((d)->lowLevelDataBogus = True)

void DefaultWaveShaper(WAVESHAPER *d);
void UpdateWaveShaper (WAVESHAPER *d); 
void ResetWaveShaper  (WAVESHAPER *d);
void PrepareWaveShaper(WAVESHAPER *d);

void SetWaveShaper_Parameters(WAVESHAPER *d, int type, float inGainDB, float outGainDB);
void SetWaveShaper_SamplingFrequency(WAVESHAPER *d, float x);

void ComputeWaveShaper(short *x, short *y, long length, WAVESHAPER *d); 
void ComputeWaveShaper_V1f(short *x, short *y, long length, WAVESHAPER *d); 
void ComputeWaveShaper_V2f(short *x, short *y, long length, WAVESHAPER *d); 
void ComputeWaveShaper_V3f(short *x, short *y, long length, WAVESHAPER *d); 
void ComputeWaveShaper_V4f(short *x, short *y, long length, WAVESHAPER *d); 

//void ComputeWaveShaper_V1i(short *x, short *y, long length, WAVESHAPER *d); 
//void ComputeWaveShaper_V2i(short *x, short *y, long length, WAVESHAPER *d); 
//void ComputeWaveShaper_V3i(short *x, short *y, long length, WAVESHAPER *d); 
void ComputeWaveShaper_V4i(short *x, short *y, long length, WAVESHAPER *d); 
void ComputeWaveShaper_V4d1i(short *x, short *y, long length, WAVESHAPER *d); 

#ifdef __cplusplus
}
#endif

#endif  //	__SHAPE_H__
