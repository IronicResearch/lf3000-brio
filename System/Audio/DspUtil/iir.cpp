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
// poles (real,imag):  7.33488541E-001, 0.00000000E+000 
// zeros (real,imag): -1.00000000E+000, 0.00000000E+000 
//
// Cascade of biquadratic sections
float iir1_HalfBand_g = {
1.33255729E-001
};

float iir1_HalfBand_a[ ] = {
9.999694824218750E-001,
1.000000000000000E+000,
0.000000000000000E+000
};

float iir1_HalfBand_b[ ] = {
9.999694824218750E-001,
-7.334899902343750E-001,
0.000000000000000E+000
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

SetIIR_Hzf_v2(d, iir1_HalfBand_a, iir1_HalfBand_b, iir1_HalfBand_g, 3);

d->inGainf = DecibelToLinear(d->inGainDB);
d->inGaini = FloatToQ15(d->inGainf);

d->outGainf = DecibelToLinear(d->outGainDB);
d->outGaini = FloatToQ15(d->outGainf);
printf("UpdateIIR:  inGainDB %g -> %g (%X)\n", d->inGainDB,  d->inGainf,  d->inGaini);
printf("UpdateIIR: outGainDB %g -> %g (%X)\n", d->outGainDB, d->outGainf, d->outGaini);

d->rippleGainCompensationf  = DecibelToLinear(d->rippleGainCompensationDB);
//d->inGaini = FloatToQ15(d->gainRippleCompensationf);
printf("UpdateIIR: rippleGainCompensationDB %g -> %g\n", d->rippleGainCompensationDB, d->rippleGainCompensationf);


// Scale coefficients by output level
//for (i = 0; i < d->order; i++)
//	d->h[i] *= d->outLevel;

// Convert 32-bit floating point coefficients to signed 16-bit fixed point
for (i = 0; i < d->coeffCount; i++)
	{
	d->hq15[i] = (Q15)(k2To15m1f * d->hf32[i]);
printf("UpdateIIR  hq15[%2d] = %g -> %X \n", i, d->hf32[i], d->hq15[i]);
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
float *h = d->hf32;	// Filter transfer function: {b0, b1, a1}
float *z = d->zf32;

//{static long c=0; printf("RunIIR1_Shortsf: %d \n", c++);}

// Topology: DirectForm: 
for (long i = 0; i < length; i++)
	{
	float sum  = (h[0]*((float)inP[i]) + h[1]*z[kIIR1_x1] - h[2]*z[kIIR1_y1]);

// Update unit delays
	z[kIIR1_y1] = sum;
	z[kIIR1_x1] = inP[i];
	outP[i] = (S16) sum;	// For in-place operation, output must follow delay updates 
	}
}	// ---- end RunIIR1_Shortsf() ---- 

// *************************************************************** 
// RunIIR_Shorts:	Main calculation routine
// ***************************************************************
	void
RunIIR_Shorts(short *inP, short *outP, long length, IIR *d)
{
//if (d->useFixedPoint)
//	RunIIR1_Shortsi(inP, outP, length, d);
//else
	RunIIR1_Shortsf(inP, outP, length, d);
}	// ---- end RunIIR_Shorts() ---- 


