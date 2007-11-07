// *************************************************************** 
// iir.cpp:		Code for Infinite Impulse Response (IIR) filters
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "util.h"
#include "iir.h"

// IIR low pass Half Band filter
// Edges (normalized)       : 0...0.1904761905 , 0.275...0.5 
// Specified deviations (dB): 0.4237859814, -26.02059991
// Worst deviations     (dB): 0.3934868464, -26.6782971
float iir1_HalfBand_20_GainCompensationDB = -0.4404795255;
float iir1_HalfBand_20_Hz[kIIR1_HalfBand_20_Length] = {
};

// IIR Elliptical Half Band lowpass filter Order: 1
// Edges (normalized)       : 0. . 0.01607142857  0.275...0.5   
// Specified deviations (dB): 0.4455278942  -26.02059991   
// Worst deviations     (dB): 0.4404795255  -17.70966446   
//  
// Target precision: fixed point 16 bits 
// Cascade of biquadratic sections
float iir1_HalfBand_g = {
1.33255729E-001
};

float iir1_HalfBand_b[ ] = {
9.999694824218750E-001,
-7.334899902343750E-001,
0.000000000000000E+000
};

float iir1_HalfBand_a[ ] = {
9.999694824218750E-001,
1.000000000000000E+000,
0.000000000000000E+000
};

// IIR Elliptical Half Band lowpass filter Order: 2
// Edges (normalized): 0...0.07380952381  0.275...0.5   
// Specified deviations (dB): 0.4455278942  -26.02059991   
// Worst deviations (dB): 0.4449990497  -30.01629988   
// 
// Target precision: fixed point 16 bits 
// Cascade of biquadratic sections
float iir2_HalfBand_gain = {
8.17084464E-002
};

float iir2_HalfBand_a[ ] = {
9.999694824218750E-001,
9.228820800781250E-001,
9.999694824218750E-001
};

float iir2_HalfBand_b[ ] = {
9.999694824218750E-001,
-1.268402099609375E+000,
5.198059082031250E-001
};

// IIR Butterworth highpass filter  Order=2
// Edges (normalized)       : 0...0.225  0.275...0.5    
// Specified deviations (dB): -26.02059991  0.4455278942    
// Worst deviations (dB)    : -6.596087323  2.988440769   
// 
// Target precision: fixed point 16 bits 
// cascade form, L1 scaling
float iir2_ButterHPF_gain = {
2.48341079E-001
};

float iir2_ButterHPF_a[ ] = {
9.99969482E-001,
-2.00000000E+000,
9.99969482E-001
};

float iir2_ButterHPF_b[ ] = {
9.99969482E-001,
1.84204102E-001,
1.77581787E-001
};

// IIR Butterworth lowpass filter Order: 2
// Edges (normalized): 0...0.225  0.275...0.5   
// Specified deviations (dB): 0.4455278942  -26.02059991   
// Worst deviations (dB): 2.988440769  -6.596087323   
//
// Target precision: fixed point 16 bits 
// cascade form, L1 scaling
float iir2_ButterLPF_gain = {
2.48341079E-001
};

float iir2_ButterLPF_a[ ] = {
9.99969482E-001,
2.00000000E+000,
9.99969482E-001
};

float iir2_ButterLPF_b[ ] = {
9.99969482E-001,
-1.84204102E-001,
1.77581787E-001
};

// IIR Elliptical lowpass filter  Order: 2
// Edges (normalized)       : 0...0.07380952381  0.275...0.5   
// Specified deviations (dB): 0.4455278942  -26.02059991   
// Worst deviations (dB)    : 0.4449990497  -30.01629988   
// cascade form
float iir2_HalfBand30_gain = {
8.17084464E-002
};
float iir2_HalfBand30_a[] = {
9.999694824218750E-001,
9.228820800781250E-001,
9.999694824218750E-001
};

float iir2_HalfBand30_b[] = {
9.999694824218750E-001,
-1.268402099609375E+000,
5.198059082031250E-001
};


// *************************************************************** 
// DefaultIIR:		Set Default high-level parameter values
// ***************************************************************
    void 
DefaultIIR(IIR *d)
{
long i;
d->inGainDB  = 0.0f;
d->outGainDB = 0.0f;
d->useFixedPoint = False;
d->coeffCount = 0;

d->frequency = 0.25f;

for (i = 0; i < kIIR_MaxCoeffs; i++)
    {
	d->hf32[i] = 0.0f;
	d->hq15[i] = 0;
    }
d->hf32[0] = 1.0f;
d->hq15[0] = FloatToQ15(d->hf32[0]);

for (i = 0; i < kIIR_MaxDelayElements; i++)
    {
	d->zf32[i] = 0.0f;
	d->zq15[i] = 0;
    }

d->samplingFrequency = 1.0f;
}	// ---- end DefaultIIR() ---- 

// **********************************************************************
// ComputeIIR1Hz:   
// ********************************************************************** 
    void
ComputeIIR1Hz(float *h, float frequency, float gainDB, int mode)
// frequency	fc/fs	normalized to sampling frequency
{
float wc = kTwoPi * frequency;
float radius;
float b0, a1;

//	IIR1Type All Pole
// Uh oh: HighPass 3dB points *do not* line up with frequency
if (kIIR_Mode_HighPass == mode)
	{
	radius  = exp(-wc);
	radius -= 1.0;
	b0      = 1.0 + radius;
	a1      =      -radius; 
	}
// LowPass 3dB points do line up with frequency
else // if (kIIR_Mode_LowPass == mode)
	{
	radius = exp(-wc);
	b0     = 1.0 - radius;
	a1     =      -radius; 
	}

h[kIIR1_b0] = b0 * DecibelToLinear(gainDB);
h[kIIR1_b1] = 0.0f;
h[kIIR1_a1] = a1;
printf("ComputeIIR1Hz: b0=%g, b1=%g, a1=%g\n", h[0], h[1], h[2]);
}	// ---- end ComputeIIR1Hz() ---- 

// *************************************************************** 
// SetIIR_Hzf:	Copy 32-bit 'float' coefficients to structure
// ***************************************************************
    void 
SetIIR_Hzf(IIR *d, float *h, long count)
{
d->coeffCount = count;
for (long i = 0; i < count; i++)
	d->hf32[i] = h[i];
}	// ---- end SetIIR_Hzf() ---- 

// *************************************************************** 
// SetIIR_Hzf_v2:	Copy 32-bit 'float' coefficients to structure
// ***************************************************************
    void 
SetIIR_Hzf_v2(IIR *d, float *a, float *b, float g, long count)
{
long i = 0;
float *h = d->hf32;

d->coeffCount = count;
//for (long i = 0; i < count; i++)
//	d->hf32[i] = h[i];

h[kIIR1_b0] = b[0];
h[kIIR1_b1] = b[1];
h[kIIR1_a1] = a[0];

h[kIIR1_b0] *= g;
h[kIIR1_b1] *= g;
}	// ---- end SetIIR_Hzf_v2() ---- 

// *************************************************************** 
// SetIIR_HzQ15:	Copy 16-bit 'Q15' coefficients to structure
// ***************************************************************
    void 
SetIIR_HzQ15(IIR *d, Q15 *h, long count)
{
d->coeffCount = count;
for (long i = 0; i < count; i++)
	d->hq15[i] = h[i];
}	// ---- end SetIIR_HzQ15() ---- 

// *************************************************************** 
// UpdateIIR:	Convert high-level parameter values to low level data
// ***************************************************************
    void 
UpdateIIR(IIR *d)
{
long i = 0;
//SetIIR_Hzf_v2(d, iir1_HalfBand_a, iir1_HalfBand_b, iir1_HalfBand_g, 3);

//#define TEST_IIR1
#ifdef TEST_IIR1
switch (d->mode
{
printf("UpdateIIR: TEST_IIR1 order=%d f=%g \n", d->order, d->frequency);
d->coeffCount = 3;

ComputeIIR1Hz(d->hf32, d->frequency/d->samplingFrequency, 0.0f, kIIR_Mode_LowPass);
//float *h = d->hf32;
//h[kIIR1_b0] = (1.0f - f);
//h[kIIR1_b1] = 0.0f;
//h[kIIR1_a1] = f;
}
#endif // TEST_IIR1
//printf("UpdateIIR: samplingFrequency=%g \n", d->samplingFrequency);

//#define TEST_IIR_ELLIPTICAL_O1_HALFBAND
#ifdef TEST_IIR_ELLIPTICAL_O1_HALFBAND
{
printf("UpdateIIR: TEST_IIR_ELLIPTICAL_O1_HALFBAND \n");
d->coeffCount = 3;

float *h = d->hf32;
h[kIIR1_b0] = iir1_HalfBand_b[0] * iir1_HalfBand_g;
h[kIIR1_b1] = iir1_HalfBand_b[1] * iir1_HalfBand_g;
h[kIIR1_a1] = -iir1_HalfBand_a[0];
}
#endif // TEST_IIR_ELLIPTICAL_O1_HALFBAND

//#define TEST_IIR_ELLIPTICAL_O2_HALFBAND
#ifdef TEST_IIR_ELLIPTICAL_O2_HALFBAND
{
printf("UpdateIIR: TEST_IIR_ELLIPTICAL_O2_HALFBAND \n");
d->coeffCount = 5;

float *h = d->hf32;
h[kIIR2_b0] =  iir2_HalfBand_b[0] * iir1_HalfBand_g;
h[kIIR2_b1] =  iir2_HalfBand_b[1] * iir1_HalfBand_g;
h[kIIR2_b2] =  iir2_HalfBand_b[2] * iir1_HalfBand_g;
h[kIIR2_a1] = -iir2_HalfBand_a[0];
h[kIIR2_a2] = -iir2_HalfBand_a[1];
}
#endif // TEST_IIR_ELLIPTICAL_O2_HALFBAND

//#define TEST_IIR_BUTTER_O2_HP
#ifdef TEST_IIR_BUTTER_O2_HP
{
printf("UpdateIIR: TEST_IIR_BUTTER_O2_HP \n");
d->coeffCount = 5;

float *h = d->hf32;
h[kIIR2_b0] =  iir2_ButterHPF_a[0] * iir2_ButterHPF_gain;
h[kIIR2_b1] =  iir2_ButterHPF_a[1] * iir2_ButterHPF_gain;
h[kIIR2_b2] =  iir2_ButterHPF_a[2] * iir2_ButterHPF_gain;
h[kIIR2_a1] = -iir2_ButterHPF_b[1];
h[kIIR2_a2] = -iir2_ButterHPF_b[2];
}
#endif // TEST_IIR_BUTTER_O2_HP

//#define TEST_IIR_BUTTER_O2_LP
#ifdef TEST_IIR_BUTTER_O2_LP
{
printf("UpdateIIR: TEST_IIR_BUTTER_O2_LP \n");
d->coeffCount = 5;

float *h = d->hf32;
h[kIIR2_b0] =  iir2_ButterLPF_a[0] * iir2_ButterLPF_gain;
h[kIIR2_b1] =  iir2_ButterLPF_a[1] * iir2_ButterLPF_gain;
h[kIIR2_b2] =  iir2_ButterLPF_a[2] * iir2_ButterLPF_gain;
h[kIIR2_a1] = -iir2_ButterLPF_b[1];
h[kIIR2_a2] = -iir2_ButterLPF_b[2];
}
#endif // TEST_IIR_BUTTER_O2_LP

#define TEST_IIR_HalfBand30_O2
#ifdef TEST_IIR_HalfBand30_O2
{
printf("UpdateIIR: TEST_IIR_HalfBand30_O2 \n");
d->coeffCount = 5;

float *h = d->hf32;
h[kIIR2_b0] =  iir2_HalfBand30_a[0] * iir2_HalfBand30_gain;
h[kIIR2_b1] =  iir2_HalfBand30_a[1] * iir2_HalfBand30_gain;
h[kIIR2_b2] =  iir2_HalfBand30_a[2] * iir2_HalfBand30_gain;
h[kIIR2_a1] = -iir2_HalfBand30_b[1];
h[kIIR2_a2] = -iir2_HalfBand30_b[2];
}
#endif // TEST_IIR_HalfBand30_O2



d->inGainf = DecibelToLinear(d->inGainDB);
d->inGaini = FloatToQ15(d->inGainf);

d->outGainf = DecibelToLinear(d->outGainDB);
d->outGaini = FloatToQ15(d->outGainf);
printf("UpdateIIR:  inGainDB %g -> %g (%X)\n", d->inGainDB,  d->inGainf,  d->inGaini);
printf("UpdateIIR: outGainDB %g -> %g (%X)\n", d->outGainDB, d->outGainf, d->outGaini);

d->rippleGainCompensationf  = DecibelToLinear(d->rippleGainCompensationDB);
//d->inGaini = FloatToQ15(d->gainRippleCompensationf);
//printf("UpdateIIR: rippleGainCompensationDB %g -> %g\n", d->rippleGainCompensationDB, d->rippleGainCompensationf);

// Scale coefficients by output level
//for (i = 0; i < d->order; i++)
//	d->h[i] *= d->outLevel;

// Convert 32-bit floating point coefficients to signed 16-bit fixed point
for (i = 0; i < d->coeffCount; i++)
	{
    float k = 0.5f;
//	d->hq15[i] = (Q15)(k2To15m1f * d->hf32[i]);
	d->hq31[i] = FloatToQ31(k*d->hf32[i]);   // Really, just 24 bits from 32-bit float
	d->hq15[i] = (d->hq31[i])>>16;

printf("UpdateIIR  hq15[%2d] = %g -> %X \n", i, k*d->hf32[i], 0x0000ffff & d->hq15[i]);
	}
}	// ---- end UpdateIIR() ---- 

// *************************************************************** 
// ResetIIR:	Reset unit to initial state
// ***************************************************************
    void 
ResetIIR(IIR *d)
{
// Clear delay line
for (long i = 0; i < kIIR_MaxDelayElements; i++)
    {
	d->zf32[i] = 0.0f;
	d->zq15[i] = 0;
	d->zq31[i] = 0;
    }
}	// ---- end ResetIIR() ---- 

// *************************************************************** 
// PrepareIIR:	Update() + Reset()
// ***************************************************************
    void 
PrepareIIR(IIR *d)
{
UpdateIIR(d);
ResetIIR(d);
}	// ---- end PrepareIIR() ---- 

// *************************************************************** 
// RunIIR1_Shortsf:	    IIR Order 1 calculation routine, 32-bit floating point
// ***************************************************************
	void
RunIIR1_Shortsf(short *inP, short *outP, long length, IIR *d)
{
//{static long c=0; printf("RunIIR1_Shortsf: %d \n", c++);}
float *h = d->hf32;	// Filter transfer function: {b0, b1, a1}
float *z = d->zf32;

// Topology: DirectForm: 
for (long i = 0; i < length; i++)
	{
	float sum  = (h[0]*((float)inP[i]) + h[1]*z[kIIR1_x1] - h[2]*z[kIIR1_y1]);
    sum *= 0.5f;

// Update unit delays
	z[kIIR1_y1] = sum;
	z[kIIR1_x1] = inP[i];
	outP[i] = (S16) sum;	// For in-place operation, output must follow delay updates 
	}
}	// ---- end RunIIR1_Shortsf() ---- 

// ************************************************************************
// RunIIR1_Shortsi: 1st Order IIR bi-quadratic
//                  Topology: Direct form implementation
// ************************************************************************ 
    void
RunIIR1_Shortsi(Q15 *x, Q15 *y, long length, IIR *d)
{
//{static long c=0; printf("RunIIR1_Shortsi %d: start \n", c++);}
Q15  *h = d->hq15;	 // Filter transfer function: {b0, b1, a1}
Q15  *z = d->zq15;

for (long i = 0; i < length; i++)
	{
	Q31 sum  = (h[0]*x[i] + h[1]*z[0] - h[2]*z[1])>>14;
// Shift right 14 =  (right 15 + left 1)

// Update unit delays
	z[1] = (Q15) sum;	z[0] = x[i];
	y[i] = (Q15) sum;	// For in-place operation, output must follow delay updates 
	}
}   // ---- end RunIIR1_Shortsi() ---- 

// *************************************************************** 
// RunIIR1_Shorts:	Main calculation routine
// ***************************************************************
	void
RunIIR1_Shorts(short *inP, short *outP, long length, IIR *d)
{
if (d->useFixedPoint)
	RunIIR1_Shortsi(inP, outP, length, d);
else
	RunIIR1_Shortsf(inP, outP, length, d);
}	// ---- end RunIIR1_Shorts() ---- 

// ************************************************************************
// RunIIR2_Shortsf: 2nd Order IIR bi-quadratic
// ************************************************************************ 
    void
RunIIR2_Shortsf(S16 *x, S16 *y, long length, IIR *d)
{
float *h = d->hf32;	// Filter transfer function: {b0, b1, b2, a1, a2}
float *z = d->zf32;

//{static long c=0; printf("RunIIR2_Shortsf %d: start \n", c++);}
for (long i = 0; i < length; i++)
	{
	float sum  = (h[0]*((float)x[i]) + h[1]*z[0] + h[2]*z[1] - h[3]*z[2] - h[4]*z[3]);

// Update unit delays
	z[3] = z[2];
	z[2] = sum;
	z[1] = z[0];
	z[0] = x[i];
	y[i] = (S16) sum;	// For in-place operation, output must follow delay updates 
	}
}   // ---- end RunIIR2_Shortsf() ---- 

// ************************************************************************
// RunIIR2_Shortsi: 2nd Order IIR bi-quadratic
// ************************************************************************ 
    void
RunIIR2_Shortsi(Q15 *x, Q15 *y, long length, IIR *d)
{
Q15  *h = d->hq15;	// Filter transfer function: {b0, b1, b2, a1, a2}
Q15  *z = d->zq15;

//{static long c=0; printf("ComputeEQi %d: start \n", c++);}for (long i = 0; i < length; i++)
	{
	Q31 sum  = (h[0]*x[i] + h[1]*z[0] + h[2]*z[1] - h[3]*z[2] - h[4]*z[3])>>14;
// Shift right 14 =  (right 15 + left 1)

// Update unit delays
    z[3] = z[2];
	z[2] = sum;    z[1] = z[0];
	z[0] = x[i];
	y[i] = (Q15) sum;	// For in-place operation, output must follow delay updates 
	}
}   // ---- end RunIIR2_Shortsi() ---- 

// ************************************************************************
// RunIIR2_Shorts: 
// ************************************************************************ 
    void
RunIIR2_Shorts(Q15 *x, Q15 *y, long length, IIR *d)
{
if (d->useFixedPoint)

    RunIIR2_Shortsi(x, y, length, d);
else
    RunIIR2_Shortsf(x, y, length, d);
}   // ---- end RunIIR2_Shorts() ---- 



