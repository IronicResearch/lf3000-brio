// *************************************************************************
//
// eq.h Header file for Equalizer
//
//				Written by Gints Klimanis
// *************************************************************************

#ifndef __EQ_H__
#define	__EQ_H__

#include "Dsputil.h"

#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------
// ---- EQ Data and Functions ----
// -------------------------------------------
#define kEQ_Coeffs	5
#define kEQ_Zs	    4

#define kEQ_g	0
#define kEQ_b0	0
#define kEQ_b1	1
#define kEQ_b2	2
#define kEQ_a1	3	
#define kEQ_a2	4

#define kEQ_x1	0
#define kEQ_x2	1
#define kEQ_y1	2
#define kEQ_y2	3

#define kEQ_DefaultQ		(kSqrt2/2.0f)	// Maximally flat amplitude

#define kEQ_Mode_LowPass	0
#define kEQ_Mode_HighPass	1
#define kEQ_Mode_LowShelf	2
#define kEQ_Mode_HighShelf	3
#define kEQ_Mode_Parametric	4
#define kEQ_Mode_Bypass		5
#define kEQ_Mode_ByPass		kEQ_Mode_Bypass

typedef struct eq {
// High Level parameters
	float	frequency;
	float	q;		// Also, holds slope for Shelving filters
	float	gainDB;

	float   damping;

	int		mode;
	int     useFixedPoint;

// Low Level data
	int		lowLevelDataBogus;
	float	samplingFrequency;
    float   gain;           // linear

	float	hf[kEQ_Coeffs];	// {b0, b1, b2, a1, a2}
	float	zf[kEQ_Zs    ];	// {x1, x2, y1, y2}
	Q15	    h16[kEQ_Coeffs];	
	Q15  	z16[kEQ_Zs    ];								
	Q31	    h32[kEQ_Coeffs];	
	Q31  	z32[kEQ_Zs    ];								

} EQ;

// Coefficient calculation functions
void ComputeEQ_LowPassHz   (float *h, float frequency);
void ComputeEQ_HighPassHz  (float *h, float frequency);
void ComputeEQ_LowShelfHz  (float *h, float frequency, float slope, float gainDB);
void ComputeEQ_HighShelfHz (float *h, float frequency, float slope, float gainDB);
void ComputeEQ_ParametricHz(float *h, float frequency, float q,     float gainDB);
void ComputeEQ_Hz          (float *h, float frequency, float q,     float gainDB, int mode);

#define EQInvalidate(d)	((d)->lowLevelDataBogus = True)
void SetEQ_SamplingFrequency(EQ *d, float x);
void SetEQ_Parameters(EQ *d, float frequency, float q, float gainDB, int mode);
void SetEQ_Frequency (EQ *d, float x);
void SetEQ_Q         (EQ *d, float x);
void SetEQ_GainDB    (EQ *d, float x);

//void SetEQ_GainAdjustment(EQ *d, float k);

double DampingFromQ   (double q);
double DampingFromQ_v2(double frequency, double q);
double QFromDamping   (double damping);
double QFromDamping_v2(double frequency, double damping);

//void AdjustGainEQHz(double *h, double gainDB);

void DefaultEQ(EQ *d);
void UpdateEQ (EQ *d); 
void ResetEQ  (EQ *d);
void PrepareEQ(EQ *d);

void ComputeEQf(short *x, short *y, long length, EQ *d); 
void ComputeEQi(short *x, short *y, long length, EQ *d); 
void ComputeEQ (short *x, short *y, long length, EQ *d); 
void ComputeEQf2(short *x, short *y, long length, EQ *d); 

char *TranslateEQ_ModeID(int id);

#ifdef __cplusplus
}
#endif

#endif  //	__EQ_H__
