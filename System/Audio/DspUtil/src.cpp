// *************************************************************** 
// src.cpp:		Sampling rate conversion routines
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "Dsputil.h"
#include "src.h"

#define kSRC_FIR_Triangle9_Length 9
float srcHz_Triangle9[kSRC_FIR_Triangle9_Length] = {
1.0f/4.0f,
1.0f/3.0f,
1.0f/2.0f,
1.0f/1.0f,
1.0f/2.0f,
1.0f/3.0f,
1.0f/4.0f
};

// FIR low pass pseudo-Half Band filter
// Edges (normalized)       : 0...0.1904761905 , 0.275...0.5 
// Specified deviations (dB): 0.4237859814, -26.02059991
// Worst deviations     (dB): 0.3934868464, -26.6782971
#define kSRC_FIR_HalfBandV1_Length 15
float srcHz_FIR_HalfBandV1_GainCompensationDB = -0.43f;
float srcHz_FIR_HalfBandV1[kSRC_FIR_HalfBandV1_Length] = {
-1.966319704923826E-002,     
 3.222681179230342E-002,     
 4.623194395383842E-002,     
-2.482473304234042E-002,     
-8.977879949891251E-002,     
 3.260044908950705E-002,     
 3.132100525943124E-001,     
 4.663313867843080E-001,     
 3.132100525943124E-001,     
 3.260044908950705E-002,     
-8.977879949891251E-002,     
-2.482473304234042E-002,     
 4.623194395383842E-002,     
 3.222681179230342E-002,     
-1.966319704923826E-002      
};

// FIR low pass pseudo-Third Band filter
// Edges (normalized)       : 0...0.05476190476 , 0.1660714286...0.5 
// Specified deviations (dB): 0.4237859814, -26.02059991
// Worst deviations     (dB): 0.1664067777, -34.27288912
#define kSRC_FIR_ThirdBandV1_Length 15
float srcHz_FIR_ThirdBandV1_GainCompensationDB = -0.43f;
float srcHz_FIR_ThirdBandV1[kSRC_FIR_ThirdBandV1_Length] = { 
-2.06583462E-002,     
-2.27924394E-002,     
-1.31107674E-002,     
 2.16633934E-002,     
 8.01341135E-002,     
 1.47992286E-001,     
 2.02693761E-001,     
 2.23676473E-001,     
 2.02693761E-001,     
 1.47992286E-001,     
 8.01341135E-002,    
 2.16633934E-002,     
-1.31107674E-002,     
-2.27924394E-002,     
-2.06583462E-002    
};


// FIR bandstop filter 
// Edges (normalized)       : 0...0.05476190476, 0.09702380952...0.2446428571, 0.2738095238...0.5 
// Specified deviations (dB): 0.4237859814, -26.02059991, 0.4237859814
// Worst deviations     (dB): 1.868578976, -12.38890846, 1.871478139
#define kSRC_FIR_NotchTest_Length 15
float srcHz_FIR_NotchTest_GainCompensationDB = -1.87f;
float srcHz_FIR_NotchTest[kSRC_FIR_NotchTest_Length] = {
 1.64909722E-002,     
 6.49425240E-002,    
-7.91175331E-002,     
 8.01686616E-002,     
 2.36217196E-001,    
 1.62159823E-001,    
-1.73590635E-001,     
 6.25478722E-001,    
-1.73590635E-001,    
 1.62159823E-001,     
 2.36217196E-001,     
 8.01686616E-002,    
-7.91175331E-002,     
 6.49425240E-002,     
 1.64909722E-002      
};

// Low Pass FIR Half Band filter
// Stop Band Ripple               : -32.8 dB
// Pass band as fraction of band  : 0.8
// Weight on transition band error: 1.0E-6
#define kSRC_LowPass_HalfBandHz_Length	15
static float srcLowPass_HalfBand_GainCompensationDB = 0.0f;
float srcLowPass_HalfBandHz[kSRC_LowPass_HalfBandHz_Length] = {
     -0.025915889069,
      0.000000000000,
      0.043797262013,
      0.000000000000,
     -0.093198247254,
      0.000000000000,
      0.313836872578,
      0.500000000000,
      0.313836872578,
      0.000000000000,
     -0.093198247254,
      0.000000000000,
      0.043797262013,
      0.000000000000,
     -0.025915889069
};

// *************************************************************** 
// DefaultSRC:		Set Default high-level parameter values
// ***************************************************************
    void 
DefaultSRC(SRC *d)
{
long i;
// AddDrop, Linear, FIR1	

d->type = kSRC_Interpolation_Type_AddDrop;
d->useFixedPoint = True;

d->inSamplingFrequency  = 1.0f;
d->outSamplingFrequency = 1.0f;
d->inScale = 1.0f;

for (i = 0; i < kSRC_Filter_MaxCoeffs; i++)
	d->h[i] = 0.0f;

d->samplingFrequency = 1.0f;
}	// ---- end DefaultSRC() ---- 

// *************************************************************** 
// UpdateSRC:	Convert high-level parameter values to low level data
// ***************************************************************
    void 
UpdateSRC(SRC *d)
{
long i;
d->inOutRateRatio = d->inSamplingFrequency/d->outSamplingFrequency;
d->outInRateRatio = 1.0f/d->inOutRateRatio;	

long iInOutRateRatio = (long) d->inOutRateRatio;
long iOutInRateRatio = (long) d->outInRateRatio;	

// by2 interpolation or decimation
if 	(2 == iInOutRateRatio || 2 == iOutInRateRatio)
	{
//	d->firLength = kSRC_FIR_HalfBandV1_Length;
//	CopyFloats(srcHz_FIR_HalfBandV1, d->h, d->firLength);
//	d->inScaleDB = srcHz_FIR_HalfBandV1_GainCompensationDB;

//#define kSRC_LowPass_HalfBandHz_Length	15
//static float srcLowPass_HalfBand_GainCompensationDB = 0.0f;
//float srcLowPass_HalfBandHz[kFIR_LowPass_HalfBandHz_Length] = {

	d->firLength = kSRC_FIR_HalfBandV1_Length;
	CopyFloats(srcLowPass_HalfBandHz, d->h, d->firLength);
	d->inScaleDB = srcLowPass_HalfBand_GainCompensationDB;
	}
// by3 interpolation or decimation
else if (3 == iInOutRateRatio || 3 == iOutInRateRatio)
	{
	d->firLength = kSRC_FIR_ThirdBandV1_Length;
	CopyFloats(srcHz_FIR_ThirdBandV1, d->h, d->firLength);
	d->inScaleDB = srcHz_FIR_ThirdBandV1_GainCompensationDB;
	}
// whatever, junk data
else
	{
	d->firLength = kSRC_FIR_NotchTest_Length;
	CopyFloats(srcHz_FIR_NotchTest, d->h, d->firLength);
	d->inScaleDB = srcHz_FIR_NotchTest_GainCompensationDB;
	}

d->inScale = DecibelToLinear(d->inScaleDB);
printf("UpdateSRC: inScale %g dB -> %g \n", d->inScaleDB, d->inScale);
printf("UpdateSRC: inOutRateRatio = %g , outInRateRatio = %g \n", d->inOutRateRatio, d->outInRateRatio);

// Scale coefficients by output level
for (i = 0; i < d->firLength; i++)
	d->h[i] *= d->inScale;

// Convert 32-bit floating point coefficients to signed 16-bit fixed point
for (i = 0; i < d->firLength; i++)
	{
	d->hI[i] = (short)(32767.0f * d->h[i]);
//	printf("UpdateSRC  hI[%2d] = %d <- %g \n", i, d->hI[i], d->h[i]);
	}

}	// ---- end UpdateSRC() ---- 

// *************************************************************** 
// ResetSRC:	Reset unit to initial state
// ***************************************************************
    void 
ResetSRC(SRC *d)
{
long i;
d->x = 0.0f;

for (i = 0; i < kSRC_Filter_MaxDelayElements; i++)
	d->z[i] = 0;
}	// ---- end ResetSRC() ---- 

// *************************************************************** 
// PrepareSRC:	Update() + Reset()
// ***************************************************************
    void 
PrepareSRC(SRC *d)
{
UpdateSRC(d);
ResetSRC(d);
}	// ---- end PrepareSRC() ---- 

// *************************************************************** 
// InterpolateShorts_AddDrop:		Resample by fractional counter
// ***************************************************************
    static void 
InterpolateShorts_AddDrop(short *in, short *out, long inLength, long outLength, double delta)
// outLength:		# of samples to produce
// delta:		inLength/outLength
{
long    i;
double x = 0.0;
//{static long c=0; printf("InterpolateShorts_AddDrop%d delta=%g\n", c++, delta);}

// General interpolation/decimation with zero filtering
for (i = 0; i < outLength; i++) 
	{
	out[i] = in[(long) x];
	x     += delta;
	}
x -= outLength;
}	// ---- end InterpolateShorts_AddDrop() ---- 

// *************************************************************** 
// InterpolateShorts_Linear:		Resample by linear interpolation
//
//		In-place operation: can   decimate in-place but
//							can't interpolate in-place
// ***************************************************************
    static void 
InterpolateShorts_Linear(short *in, short *out, long inLength, long outLength, float *x, float xInc)
// outLength:		# of samples to produce
// delta:			inLength/outLength
{
//{static long c=0; printf("InterpolateShorts_Linear%d\n", c++);}

// General interpolation/decimation
for (long i = 0; i < outLength; i++) 
	{
	long  x0       = (long ) *x;
	float fraction = (float)(*x - (float)x0);
	short *p      = &in[x0-1];
	*x    += xInc;

	out[i] = (short)(((float)p[0]) + ((float)(p[1]-p[0]))*fraction);
	}
// Save fractional part of counter
*x = (float)(*x - (float)((long ) *x));

// Save end of buffer to start of buffer for next iteration
in[-1] = in[inLength-1];
}	// ---- end InterpolateShorts_Linear() ---- 

// *************************************************************** 
// SRCShorts_Triangle2		Resample by Avg2 interpolation
//
//		In-place operation: can   decimate in-place but
//							can't interpolate in-place
// ***************************************************************
    static void 
SRCShorts_Triangle2(short *in, short *out, long inLength, long outLength)
{
{static long c=0; printf("SRCShorts_Triangle%d\n", c++);}

// ---- Downsampling by 2
if (outLength < inLength)
	{
	for (long i = 0; i < outLength; i++) 
		{
		long y = ((long) in[i]) + (long)in[i+1];
		out[i] = (short)(y>>1);
		}
	}
// ---- Upsampling   by 2
else
	{
	for (long i = 0, j = 0; i < inLength; i++, j += 2) 
		{
	// Output 1 of 2
		long y = ((long) in[i-1]) + (long)in[i];
		out[j] = (short)(y>>1);
	// Output 2 of 2
		out[j+1] = in[i];
		}
	}

// Save end of buffer to start of buffer for next iteration
in[-1] = in[inLength-1];
}	// ---- end SRCShorts_Triangle2() ---- 

// *************************************************************** 
// ComputeSRC_TestFIR:	FIR filter test, no rate change
//
// ***************************************************************
    void 
ComputeSRC_TestFIR(short *in, short *out, long length, SRC *d)
// length:		# of samples to produce
{
long   i = 0, j = 0, k = 0, lAcc;
short *zP = in;
float *h = d->h;
//{static long c=0; printf("ComputeSRC_TestFIR %d length=%d\n", c++, length);}

for (i = 0; i < length; i++) 
	{
        float acc = 0.0f;
// Compute FIR filter
        zP = &in[i];
        for (j = 0; j < d->firLength; j++)
		acc += h[j]*(float)zP[j-(d->firLength-1)];
	acc *= d->inScale;

// Saturate accumulator and write output
	lAcc = (long) acc;
	if      (lAcc >  32767)
		lAcc = 32767;
	else if (lAcc < -32768)
		lAcc = -32768;
	out[i] = (short) lAcc;
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->firLength; i++)
	in[-1-i] = in[length-1-i];
}	// ---- end ComputeSRC_TestFIR() ---- 

// *************************************************************** 
// ComputeSRC_DownBy2FIR:	Sampling rate downconversion by factor 2
//				with FIR filtering
// ***************************************************************
    void 
ComputeSRC_DownBy2FIR(short *in, short *out, long inLength, long outLength, SRC *d)
// length:		# of samples to produce
{
long   i = 0, j = 0, k = 0, lAcc;
short *zP = in;
float *h = d->h;
{static long c=0; printf("ComputeSRC_DownBy2FIR %d outLength=%d\n", c++, outLength);}

for (i = 0, k = 0; i < outLength; i++, k +=2) 
	{
        float acc = 0.0f;
// Compute FIR filter.  Since half the outputs are thrown away, there is no need 
//                      to compute them. 
        zP = &in[k];
        for (j = 0; j < d->firLength; j++)
		acc += h[j]*(float)zP[j-(d->firLength-1)];
	acc *= d->inScale;  // FIXXX tuck into coefficients

// Saturate accumulator and write output
	lAcc = (long) acc;
	if      (lAcc >  32767)
		lAcc = 32767;
	else if (lAcc < -32768)
		lAcc = -32768;
	out[i] = (short) lAcc;
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->firLength; i++)
	in[-1-i] = in[inLength-1-i];
}	// ---- end ComputeSRC_DownBy2FIR() ---- 

// *************************************************************** 
// ComputeSRC_DownBy3FIR:	Sampling rate downconversion by factor 3
//				with FIR filtering
// ***************************************************************
    void 
ComputeSRC_DownBy3FIR(short *in, short *out, long inLength, long outLength, SRC *d)
// length:		# of samples to produce
{
long   i = 0, j = 0, k = 0, lAcc;
short *zP = in;
float *h = d->h;
{static long c=0; printf("ComputeSRC_DownBy3FIR %d outLength=%d\n", c++, outLength);}

for (i = 0, k = 0; i < outLength; i++, k +=3) 
	{
        float acc = 0.0f;
// Compute FIR filter.  Since half the outputs are thrown away, there is no need 
//                      to compute them. 
        zP = &in[k];
        for (j = 0; j < d->firLength; j++)
		acc += h[j]*(float)zP[j-(d->firLength-1)];
	acc *= d->inScale;  // FIXXX tuck into coefficients
// Saturate accumulator and write output
	lAcc = (long) acc;
	if      (lAcc >  32767)
		lAcc = 32767;
	else if (lAcc < -32768)
		lAcc = -32768;
	out[i] = (short) lAcc;
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->firLength; i++)
	in[-1-i] = in[inLength-1-i];
}	// ---- end ComputeSRC_DownBy3FIR() ---- 

#ifdef NEEDED
// *************************************************************** 
// InterpolateShortsBy2_FIR15:	Resample by 2 interpolation with FIR filter
//
// ***************************************************************
    static void 
InterpolateShortsBy2_FIR15(short *in, short *out, long inLength, long outLength, SRC *d)
{
long    i, j;
long hI, zI, lAcc;
short *zP = in;
float *h = d->h;
float k = 2.0f*d->inScale;

{static long c=0; printf("InterpolateShortsBy2_FIR15 %d\n", c++);}

// Produce two output values for each input value
for (i = 0, j = 0; i < inLength; i++, j +=2) 
	{
	float acc = 0.0f;
        zP = &in[i];
// Compute FIR filter (even coeffs)
        acc = 0.0f;
//hI = 0;
//zI = -14;
	acc += h[ 0]*(float)zP[ 0];
	acc += h[ 2]*(float)zP[-1];
	acc += h[ 4]*(float)zP[-2];
	acc += h[ 6]*(float)zP[-3];
	acc += h[ 8]*(float)zP[-4];
	acc += h[10]*(float)zP[-5];
	acc += h[12]*(float)zP[-6];
	acc += h[14]*(float)zP[-7];
	acc *= k;  // FIXXX tuck into coefficients in Update() for efficiency

// Saturate accumulator and output
	lAcc = (long) acc;
	if      (lAcc >  32767)
		lAcc = 32767;
	else if (lAcc < -32768)
		lAcc = -32768;
	out[j] = (short) lAcc;

// Compute FIR filter (odd coeffs)
        acc = 0.0f;
	acc += h[ 1]*(float)zP[ 0];
	acc += h[ 3]*(float)zP[-1];
	acc += h[ 5]*(float)zP[-2];
	acc += h[ 7]*(float)zP[-3];
	acc += h[ 9]*(float)zP[-4];
	acc += h[11]*(float)zP[-5];
	acc += h[13]*(float)zP[-6];	acc *= k;  // FIXXX tuck into coefficients in Update() for efficiency

// Saturate accumulator and output
	lAcc = (long) acc;
	if      (lAcc >  32767)
		lAcc = 32767;
	else if (lAcc < -32768)
		lAcc = -32768;
	out[j+1] = (short) lAcc;
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->firLength/2; i++)
	in[-1-i] = in[inLength-1-i];
}	// ---- end InterpolateShortsBy2_FIR15() ---- 
#endif

// *************************************************************** 
// ComputeSRC_UpBy2FIRf:	Resample by 2 interpolation with FIR filter
//
//			32-bit floating point implementation
// ***************************************************************
    static void 
ComputeSRC_UpBy2FIRf(short *in, short *out, long inLength, long outLength, SRC *d)
{
long    i, j;
long hI, zI, lAcc;
short *zP = in;
float *h = d->h;
float k = 2.0f*d->inScale;

//{static long c=0; printf("ComputeSRC_UpBy2FIRf %d\n", c++);}

// Produce two output values for each input value
for (i = 0, j = 0; i < inLength; i++, j +=2) 
	{
	float acc = 0.0f;
        zP = &in[i];
// Compute FIR filter (even coeffs)
        acc = 0.0f;
        for (hI = 0, zI = 0; hI < d->firLength; hI += 2, zI--)
		acc += h[hI]*(float)zP[zI];
	acc *= k;  // FIXXX tuck into coefficients in Update() for efficiency

// Saturate accumulator and output
	lAcc = (long) acc;
	if      (lAcc >  32767)
		lAcc = 32767;
	else if (lAcc < -32768)
		lAcc = -32768;
	out[j] = (short) lAcc;

// Compute FIR filter (odd coeffs)
        acc = 0.0f;
        for (hI = 1, zI = 0; hI < d->firLength; hI += 2, zI--)
		acc += h[hI]*(float)zP[zI];
	acc *= k;  // FIXXX tuck into coefficients in Update() for efficiency

// Saturate accumulator and output
	lAcc = (long) acc;
	if      (lAcc >  32767)
		lAcc = 32767;
	else if (lAcc < -32768)
		lAcc = -32768;
	out[j+1] = (short) lAcc;
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->firLength/2; i++)
	in[-1-i] = in[inLength-1-i];
}	// ---- end ComputeSRC_UpBy2FIRf() ---- 

// *************************************************************** 
// ComputeSRC_UpBy2FIRi:	Resample by 2 interpolation with FIR filter
//
//			16-bit fixed point implementation
// ***************************************************************
    static void 
ComputeSRC_UpBy2FIRi(short *in, short *out, long inLength, long outLength, SRC *d)
{
long    i, j;
long hI, zI, lAcc;
short *zP = in;
short *hi = d->hI;
//float k = 2.0f*d->inScale;

//{static long c=0; printf("ComputeSRC_UpBy2FIRi %d\n", c++);}

// Produce two output values for each input value
for (i = 0, j = 0; i < inLength; i++, j +=2) 
	{
        zP = &in[i];
// Compute FIR filter (even coeffs)
        long lAcc = 0;
        for (hI = 0, zI = 0; hI < d->firLength; hI += 2, zI--)
		lAcc += hi[hI]*zP[zI];

// Saturate and output:  FIXXX add guard bits to accumulator, then add saturation code
	lAcc = lAcc>>14;  // 15 instead of 16 for guard bits, TEMPORARY
			  // 14 to compensate for by2 interpolation
	if      (lAcc >  32767)
		lAcc = 32767;
	else if (lAcc < -32768)
		lAcc = -32768;
	out[j] = (short) lAcc;

// Compute FIR filter (odd coeffs)
        lAcc = 0;
        for (hI = 1, zI = 0; hI < d->firLength; hI += 2, zI--)
		lAcc += hi[hI]*zP[zI];

// Saturate and output:  FIXXX add guard bits to accumulator, then add saturation code
	lAcc = lAcc>>14;  // 15 instead of 16 for guard bits, TEMPORARY
			  // 14 to compensate for by2 interpolation
//	if      (lAcc >  32767)
//		lAcc = 32767;
//	else if (lAcc < -32768)
//		lAcc = -32768;
	out[j+1] = (short) lAcc;
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->firLength/2; i++)
	in[-1-i] = in[inLength-1-i];
}	// ---- end ComputeSRC_UpBy2FIRi() ---- 

// *************************************************************** 
// ComputeSRC_UpBy3FIR:	Resample by 3 interpolation with FIR filter
//
// ***************************************************************
    static void 
ComputeSRC_UpBy3FIR(short *in, short *out, long inLength, long outLength, SRC *d)
{
long    i, j;
long hI, zI, lAcc;
short *zP = in;
float *h = d->h;
float k = 3.0f*d->inScale;

//{static long c=0; printf("ComputeSRC_UpBy3FIR %d\n", c++);}

// Produce three output values for each input value
for (i = 0, j = 0; i < inLength; i++, j += 3) 
	{
	float acc = 0.0f;
        zP = &in[i];
// Compute FIR filter output #1
        acc = 0.0f;
        for (hI = 0, zI = 0; hI < d->firLength; hI += 3, zI--)
		acc += h[hI]*(float)zP[zI];
	acc *= k;  // FIXXX tuck into coefficients in Update() for efficiency

// Saturate and output
	lAcc = (long) acc;
	if      (lAcc >  32767)
		lAcc = 32767;
	else if (lAcc < -32768)
		lAcc = -32768;
	out[j] = (short) acc;

// Compute FIR filter output #2
        acc = 0.0f;
        for (hI = 1, zI = 0; hI < d->firLength; hI += 3, zI--)
		acc += h[hI]*(float)zP[zI];
	acc *= k;  // FIXXX tuck into coefficients in Update() for efficiency

// Saturate and output
	lAcc = (long) acc;
	if      (lAcc >  32767)
		lAcc = 32767;
	else if (lAcc < -32768)
		lAcc = -32768;
	out[j+1] = (short) lAcc;

// Compute FIR filter output #3
        acc = 0.0f;
        for (hI = 2, zI = 0; hI < d->firLength; hI += 3, zI--)
		acc += h[hI]*(float)zP[zI];
	acc *= k;  // FIXXX tuck into coefficients in Update() for efficiency

// Saturate and output
	lAcc = (long) acc;
	if      (lAcc >  32767)
		lAcc = 32767;
	else if (lAcc < -32768)
		lAcc = -32768;
	out[j+2] = (short) lAcc;
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->firLength/2; i++)
	in[-1-i] = in[inLength-1-i];
}	// ---- end ComputeSRC_UpBy3FIR() ---- 

// *************************************************************** 
// RunSRC:		Resample, big-mama routine
// ***************************************************************
	void
RunSRC(short *in, short *out, long inLength, long outLength, SRC *d)
{
switch (d->type)
	{
	case kSRC_Interpolation_Type_AddDrop:
		InterpolateShorts_AddDrop(in, out, inLength, outLength, d->inOutRateRatio);
	break;
	default:
	case kSRC_Interpolation_Type_Linear:
		InterpolateShorts_Linear(in, out, inLength, outLength, &d->x, d->inOutRateRatio);
	break;
	case kSRC_Interpolation_Type_Triangle:
		SRCShorts_Triangle2(in, out, inLength, outLength);
	break;
	case kSRC_Interpolation_Type_FIR1:
		{
		if (inLength > outLength)
			{
			long iRatio = inLength/outLength;
			switch (iRatio)
				{	
				case 2:
				ComputeSRC_DownBy2FIR(in, out, inLength, outLength, d);
				break;
				case 3:
				ComputeSRC_DownBy3FIR(in, out, inLength, outLength, d);
				break;
				default:
				printf("Unsupported decimation ratio = %d \n", iRatio);
				break;
				}
			}
		else
			{
			long iRatio = outLength/inLength;
			switch (iRatio)
				{	
				case 2:
				if (d->useFixedPoint)
				ComputeSRC_UpBy2FIRi(in, out, inLength, outLength, d);
				else
				ComputeSRC_UpBy2FIRf(in, out, inLength, outLength, d);
				break;
				case 3:
				ComputeSRC_UpBy3FIR(in, out, inLength, outLength, d);
				break;
				default:
				printf("Unsupported interpolation ratio = %d \n", iRatio);
				break;
				}
			}
		}
	break;
	case kSRC_Interpolation_Type_TestFIR:
		ComputeSRC_TestFIR(in, out, inLength, d);
	break;
	}
}	// ---- end RunSRC() ---- 

