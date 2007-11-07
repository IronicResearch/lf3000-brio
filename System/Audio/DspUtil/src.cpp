// *************************************************************** 
// src.cpp:		Sampling rate conversion routines,
//			currently 32-bit floating point,
//			16-bit or 32-bit fixed point or a development
//			fixed-point/floating-point hybrid
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "Dsputil.h"
#include "fir.h"
#include "src.h"

// ============================================================================
// DefaultSRC:		Set high-level parameter values
// ============================================================================
    void 
DefaultSRC(SRC *d)
{
long i;
// AddDrop, Linear, FIR1	

d->type = kSRC_Interpolation_Type_FIR;
d->useFixedPoint = True;
d->useImpulse    = False;
d->filterVersion = kSRC_FilterVersion_0;

d->inSamplingFrequency  = 1.0f;
d->outSamplingFrequency = 1.0f;
d->inScale = 1.0f;

for (i = 0; i < kSRC_Filter_MaxCoeffs; i++)
	d->h[i] = 0.0f;

//printf("kSRC_Linear_MSBits =%d \n", kSRC_Linear_MSBits);
//printf("kSRC_Linear_LSBits =%d \n", kSRC_Linear_LSBits);
//printf("kSRC_Linear_MSmask =%08X \n", kSRC_Linear_MSmask);
//printf("kSRC_Linear_LSmask =%08X \n", kSRC_Linear_LSmask);
//printf("kSRC_Linear_Divisor = %d \n", kSRC_Linear_Divisor);

d->samplingFrequency = 1.0f;
}	// ---- end DefaultSRC() ---- 

// ============================================================================
// UpdateSRC:	Convert high-level parameter values to low level data
// ============================================================================
    void 
UpdateSRC(SRC *d)
{
long i;
d->inOutRateRatio = d->inSamplingFrequency/d->outSamplingFrequency;
d->outInRateRatio = 1.0f/d->inOutRateRatio;	

long iInOutRateRatio = (long) d->inOutRateRatio;
long iOutInRateRatio = (long) d->outInRateRatio;	
char *firName = "Dunno";


// Set default filters values to cover initialization, even if they are unused.
d->hP        = fir_HalfBand_58dB_Hz;
d->firLength = kFIR_HalfBand_58dB_Hz_Length;
d->inScaleDB = fir_HalfBand_58dB_GainCompensationDB;
firName      = "HalfBand_58dB";

// by2 interpolation or decimation
if 	(2 == iInOutRateRatio || 2 == iOutInRateRatio)
	{
//	d->firLength = kFIR_HalfBandV1_Length;
//	d->hP        = firHz_FIR_HalfBandV1
//	d->inScaleDB = firHz_FIR_HalfBandV1_GainCompensationDB;

	if (kSRC_Interpolation_Type_FIR == d->type)
		{
		switch (d->filterVersion)
			{
			default:
			case kSRC_FilterVersion_0:
				d->hP        = fir_HalfBand_58dB_Hz;
				d->firLength = kFIR_HalfBand_58dB_Hz_Length;
				d->inScaleDB = fir_HalfBand_58dB_GainCompensationDB;
				firName      = "HalfBand_58dB";
			break;
			case kSRC_FilterVersion_1:
				d->hP        = fir_HalfBand_32dB_Hz;
				d->firLength = kFIR_HalfBand_32dB_Hz_Length;
				d->inScaleDB = fir_HalfBand_32dB_GainCompensationDB;
				firName      = "HalfBand_32dB";
			break;

			case kSRC_FilterVersion_2:
				d->hP        = fir_Triangle_3_Hz;
				d->firLength = kFIR_Triangle_3_Hz_Length;
				d->inScaleDB = fir_Triangle_3_GainCompensationDB;
				firName      = "Triangle_3";
			break;
			case kSRC_FilterVersion_3:
				d->hP        = fir_Triangle_9_Hz;
				d->firLength = kFIR_Triangle_9_Hz_Length;
				d->inScaleDB = fir_Triangle_9_GainCompensationDB;
				firName      = "Triangle_9";
			break;

			case kSRC_FilterVersion_4:
				d->hP        = fir_HalfBand_15_Hz;
				d->firLength = kFIR_HalfBand_15_Hz_Length;
				d->inScaleDB = fir_HalfBand_15_GainCompensationDB ;
				firName      = "HalfBand_15";
			break;
			case kSRC_FilterVersion_5:
				d->hP        = fir_HalfBand_31_Hz;
				d->firLength = kFIR_HalfBand_31_Hz_Length;
				d->inScaleDB = fir_HalfBand_31_GainCompensationDB ;
				firName      = "HalfBand_31";
			break;

			case kSRC_FilterVersion_6:
				d->hP        = fir_HalfBand_30dB_15_Hz;
				d->firLength = kFIR_HalfBand_30dB_15_Hz_Length;
				d->inScaleDB = fir_HalfBand_30dB_15_GainCompensationDB ;
				firName      = "HalfBand_30dB_15";
			break;
			case kSRC_FilterVersion_7:
				d->hP        = fir_HalfBand_50dB_31_Hz;
				d->firLength = kFIR_HalfBand_50dB_31_Hz_Length;
				d->inScaleDB = fir_HalfBand_50dB_31_GainCompensationDB ;
				firName      = "HalfBand_50dB_31";
			break;
			case kSRC_FilterVersion_8:
				d->hP        = fir_HalfBand_50dB_41_Hz;
				d->firLength = kFIR_HalfBand_50dB_41_Hz_Length;
				d->inScaleDB = fir_HalfBand_50dB_41_GainCompensationDB ;
				firName      = "HalfBand_50dB_41";
			break;
			}

		}
	}
// by3 interpolation or decimation
else if (3 == iInOutRateRatio || 3 == iOutInRateRatio)
	{
	d->firLength = kFIR_ThirdBand_31_Hz_Length;
	d->hP        = fir_ThirdBand_31_Hz;
	d->inScaleDB = fir_ThirdBand_31_GainCompensationDB;
	firName      = "ThirdBand_31";
	}
// Whatever, junk data
else
	{
	d->firLength = kFIR_Notch_Hz_Length;
	d->hP        = fir_Notch_Hz;
	d->inScaleDB = fir_Notch_GainCompensationDB;
	firName      = "fir_Notch";
	}

d->xIncF = d->inOutRateRatio;
d->xIncI = (unsigned long)(d->inOutRateRatio*(double)(kSRC_Linear_Divisor));
//printf("inOutRateRatio=%g argInc= %d (%X) \n", d->inOutRateRatio, d->argInc, d->argInc);
//printf("UpdateSRC: xIncF=%g xIncI=%d \n", d->xIncF, d->xIncI);

d->inScale = DecibelToLinear(d->inScaleDB);
//printf("UpdateSRC: inScale %g dB -> %g \n", d->inScaleDB, d->inScale);
//printf("UpdateSRC: inOutRateRatio = %g , outInRateRatio = %g \n", d->inOutRateRatio, d->outInRateRatio);

if (kSRC_Interpolation_Type_FIR == d->type)
	{
// Scale coefficients by output level
	for (i = 0; i < d->firLength; i++)
		d->h[i] = d->inScale*d->hP[i];

// Convert 32-bit floating point coefficients to signed 16-bit fixed point
	for (i = 0; i < d->firLength; i++)
		d->hI[i] = (short)(k2To15m1f * d->h[i]);
//	printf("UpdateSRC  hI[%2d] = %d <- %g \n", i, d->hI[i], d->h[i]);
	}

//printf("UpdateSRC: JOEJOE '%s': Length = %d inScaleDB=%g\n", firName, d->firLength, d->inScaleDB);
}	// ---- end UpdateSRC() ---- 

// ============================================================================
// ResetSRC:	Reset unit to initial state
// ============================================================================
    void 
ResetSRC(SRC *d)
{
long i;
d->xF = 0.0;
d->xI = 0;
d->computingDone = False;

for (i = 0; i < kSRC_Filter_MaxDelayElements; i++)
	d->z[i] = 0;
}	// ---- end ResetSRC() ---- 

// ============================================================================
// PrepareSRC:	Update() + Reset()
// ============================================================================
    void 
PrepareSRC(SRC *d)
{
UpdateSRC(d);
ResetSRC(d);
}	// ---- end PrepareSRC() ---- 

// ============================================================================
// ImpulseShortsf:		Resample by fractional counter
// ============================================================================
    static void 
InterpolateShortsf(short *in, short *out, long inLength, long outLength, SRC *d)
// outLength:		# of samples to produce
{
//{static long c=0; printf("InterpolateShortsf %d : delta=%g x=%g \n", c++, d->delta, d->x);}

// General interpolation/decimation with zero filtering
for (long i = 0; i < outLength; i++) 
	{
	out[i] = in[(long) d->xF];
	d->xF  += d->xIncF;
	}
// Save fractional part of counter
d->xF -= d->xIncF*(double)outLength;
}	// ---- end InterpolateShortsf() ---- 

// ============================================================================
// InterpolateShorts_AddDropf:		Resample by fractional counter
// ============================================================================
    static void 
InterpolateShorts_AddDropf(short *in, short *out, long inLength, long outLength, SRC *d)
// outLength:		# of samples to produce
{
//{static long c=0; printf("InterpolateShorts_AddDropf %d : delta=%g x=%g \n", c++, d->delta, d->x);}

// General interpolation/decimation with zero filtering
for (long i = 0; i < outLength; i++) 
	{
	out[i] = in[(long) d->xF];
	d->xF  += d->xIncF;
	}
// Save fractional part of counter
d->xF -= d->xIncF*(double)outLength;
}	// ---- end InterpolateShorts_AddDropf() ---- 

// ============================================================================
// InterpolateShorts_AddDropi:		Resample by fractional counter
// ============================================================================
    static void 
InterpolateShorts_AddDropi(short *in, short *out, long inLength, long outLength, SRC *d)
// outLength:		# of samples to produce
{
//{static long c=0; printf("InterpolateShorts_AddDropi %d : arg=%d argInc=%d\n", 
//			c++, d->arg, d->argInc);}

// General interpolation/decimation with zero filtering
for (long i = 0; i < outLength; i++) 
	{
	unsigned long xi = (d->xI)>>kSRC_Linear_LSBits;
//printf("%3d : arg=%d argInc=%d %g\n", i, d->arg, d->argInc, x);
	out[i]  = in[xi];
	d->xI += d->xIncI;
	}
// Save fractional part of counter
d->xI -= d->xIncI*outLength;
}	// ---- end InterpolateShorts_AddDropi() ---- 

// ============================================================================
// InterpolateShorts_Linearf:		Resample by linear interpolation
//
//		In-place operation: can   decimate in-place but
//							can't interpolate in-place
// ============================================================================
    static void 
InterpolateShorts_Linearf(short *in, short *out, long inLength, long outLength, SRC *d)
// outLength:		# of samples to produce
{
//{static long c=0; printf("InterpolateShorts_Linearf %d\n", c++);}

// General interpolation/decimation
short *lastP = &in[-1];	// Start with last sample of previous iteration
for (long i = 0; i < outLength; i++) 
	{
	long  x0       = (long ) d->xF;
	float fraction = (float)(d->xF - (float)x0);
	d->xF    += d->xIncF;

	short *p      = &lastP[x0];
	float diff    = (float)(p[1]-p[0]);

	out[i] = (short)(p[0] + (long)(fraction*diff));
	}

// Save fractional part of counter
d->xF -= d->xIncF*(float)outLength;

// Save end of buffer to start of buffer for next iteration
in[-1] = in[inLength-1];
}	// ---- end InterpolateShorts_Linearf() ---- 

// ============================================================================
// InterpolateShorts_Lineari_TEST:		Resample by linear interpolation
//
//		In-place operation: can   decimate in-place but
//							can't interpolate in-place
// ============================================================================
    static void 
InterpolateShorts_Lineari_TEST(short *in, short *out, long inLength, long outLength, SRC *d)
// outLength:		# of samples to produce
{
{static long c=0; printf("InterpolateShorts_Lineari_TEST %d\n", c++);}

#define kSRC_Lineari_Shift 3 // FIXXX hard-coded to line up with bit15 of signed 16-bit

for (long i = 0; i < outLength; i++) 
	{
	long  x0       = (long ) d->xI;
	float fraction = (float)(d->xI - (float)x0);
	short *p      = &in[x0-1];
	float diff    = (float)(p[1]-p[0]);
	long y        = (long) p[0];
	d->xI    += d->xIncI;
	out[i] = (short)(y + (long)(fraction*diff));
	}

// General interpolation/decimation
//unsigned long arg = d->arg;
//short *lastP = &in[1];
//for (long i = 0; i < outLength; i++) 
//	{
//	long  x0      = (arg)>>kSRC_Linear_LSBits;
//	short *p      = &lastP[x0];
//	long fraction = (arg & kSRC_Linear_LSmask)>>kSRC_Lineari_Shift;
//	long  y       = fraction*(long)(p[1]-p[0]);  
//
//	arg += d->argInc;
//	out[i] = (short)(((y>>16) + p[0]));
//	}
// Save fractional part of counter
//d->arg = arg - ((d->argInc*outLength));

// Save last sample of buffer for next iteration
in[-1] = in[inLength-1];
}	// ---- end InterpolateShorts_Lineari_TEST() ---- 

// ============================================================================
// InterpolateShorts_Lineari:		Resample by linear interpolation
//
//		In-place operation: can   decimate in-place but
//							can't interpolate in-place
// ============================================================================
    static void 
InterpolateShorts_Lineari(short *in, short *out, long inLength, long outLength, SRC *d)
// outLength:		# of samples to produce
{
//{static long c=0; printf("InterpolateShorts_Lineari %d\n", c++);}

#define kSRC_Lineari_Shift 3 // FIXXX hard-coded to line up with bit15 of signed 16-bit

// General interpolation/decimation
unsigned long xI = d->xI;
short *lastP = &in[-1];	// Start with last sample of previous iteration
for (long i = 0; i < outLength; i++) 
	{
	long  x0      = (xI)>>kSRC_Linear_LSBits;
	short *p      = &lastP[x0];
	long fraction = (xI & kSRC_Linear_LSmask)>>kSRC_Lineari_Shift;
	long  y       = fraction*(long)(p[1]-p[0]);  

	xI += d->xIncI;
	out[i] = (short)(((y>>16) + p[0]));
	}
// Save fractional part of counter
d->xI = xI - ((d->xIncI*outLength));

// Save last sample of buffer for next iteration
in[-1] = in[inLength-1];
}	// ---- end InterpolateShorts_Lineari ---- 

// ============================================================================
// SRCShorts_Triangle2		Resample by Avg2 interpolation
//
//		Unsuitable for in-place operation due to trick to save state of
//			previous iteration
// ============================================================================
    static void 
SRCShorts_Triangle2(short *in, short *out, long inLength, long outLength)
{
//{static long c=0; printf("SRCShorts_Triangle2%d\n", c++);}
short *p = &in[-1];
// Downsampling by 2
if (outLength < inLength)
	{
	for (long i = 0; i < outLength; i++) 
		{
		long y = ((long) p[i]) + (long)p[i+1];
		out[i] = (short)(y>>1);
		}
	}
// Upsampling by 2
else
	{
	for (long i = 0, j = 0; i < inLength; i++, j += 2) 
		{
	// Output 1 of 2
		long y = ((long) p[i]) + (long)p[i+1];
		out[j] = (short)(y>>1);
	// Output 2 of 2
		out[j+1] = in[i];
		}
	}

// Save end of buffer to start of buffer for next iteration
in[-1] = in[inLength-1];
}	// ---- end SRCShorts_Triangle2() ---- 

// ============================================================================
// SRCShorts_Triangle4		Resample by triangle interpolation
//
// y[n] = 1/4*x[n] + 1/2*x[n-1] + 1/4*x[n-2]
//		Unsuitable for in-place operation due to trick to save state of
//			previous iteration
// ============================================================================
    static void 
SRCShorts_Triangle4(short *in, short *out, long inLength, long outLength)
{
//{static long c=0; printf("SRCShorts_Triangle4 %d start\n", c++);}
short *p = &in[-3];

// Downsampling by 4
if (outLength < inLength)
	{
	for (long i = 0; i < outLength; i++) 
		{
		long y = ((long)p[i]) + ((long)p[i+2])>>1;
		out[i] = (short)  ((y +  (long)p[i+1])>>1);
		}
	}
// Upsampling by 4
else
	{
	for (long i = 0, j = 0; i < inLength; i++, j += 4) 
		{
		long diffd4 = (((long) p[i+1]) - (long)p[i])>>2;
		long x      = (long) p[i];
// FIXXX: someday, redo with shifting to avoid truncation of difference
	// Output 1 of 4
		out[j  ] = (short)x;
		x += diffd4;
	// Output 2 of 4
		out[j+1] = (short)x;
		x += diffd4;
	// Output 3 of 4
		out[j+2] = (short)x;
		x += diffd4;
	// Output 4 of 4
		out[j+3] = (short)x;
		}
	}

// Save end of buffer to start of buffer for next iteration
in[-1] = in[inLength-1];
in[-2] = in[inLength-2];
in[-3] = in[inLength-3];
}	// ---- end SRCShorts_Triangle4() ---- 

// ============================================================================
// ComputeSRC_TestFIR:	FIR filter test, no rate change
//
// ============================================================================
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
	SATURATE_16BIT(lAcc);  // Macro but no return value
	out[i] = (short) lAcc;
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->firLength; i++)
	in[-1-i] = in[length-1-i];
}	// ---- end ComputeSRC_TestFIR() ---- 

//============================================================================
// ComputeSRC_DownBy2FIRf:	Sampling rate downconversion by factor 2
//				with FIR filtering
//============================================================================
    void 
ComputeSRC_DownBy2FIRf(short *in, short *out, long inLength, long outLength, SRC *d)
{
long   i = 0, j = 0, k = 0, lAcc;
short *zP = in;
float *h  = d->h;
{static long c=0; printf("ComputeSRC_DownBy2FIRf %d outLength=%d\n", c++, outLength);}

for (i = 0, k = 0; i < outLength; i++, k +=2) 
	{
        float acc = 0.0f;
// Compute FIR filter.  Half the outputs are tossed, so don't compute them. 
        zP = &in[k];
        for (j = 0; j < d->firLength; j++)
		acc += h[j]*(float)zP[j-(d->firLength-1)];
	acc *= d->inScale;  // FIXXX tuck into coefficients

// Saturate accumulator and write output
	lAcc = (long) acc;
	SATURATE_16BIT(lAcc);  // Macro but no return value
	out[i] = (short) lAcc;
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->firLength; i++)
	in[-1-i] = in[inLength-1-i];
}	// ---- end ComputeSRC_DownBy2FIRf() ---- 

//============================================================================
// ComputeSRC_DownBy2FIRi:	Sampling rate downconversion by factor 2
//				with FIR filtering
//				Fixed-point implementation
//============================================================================
    void 
ComputeSRC_DownBy2FIRi(short *in, short *out, long inLength, long outLength, SRC *d)
{
long   i = 0, j = 0, k = 0;
long   hI, zI, lAcc;
short *zP, *hi = d->hI;

{static long c=0; printf("ComputeSRC_DownBy2FIRi %d inLength=%d outLength=%d\n", c++, inLength, outLength);}

for (i = 0, k = 0; i < outLength; i++, k +=2) 
	{
// Compute FIR filter.  Half the outputs are tossed, so don't compute them. 
        zP = &in[k];
        for (hI = 0, zI = 0, lAcc = 0; hI < d->firLength; hI++, zI--)
		lAcc += hi[hI]*zP[zI];
//	acc *= d->inScale;  // FIXXX tuck into coefficients

// Saturate and output:  FIXXX add guard bits to accumulator, then add saturation code
// 15 instead of 16 for guard bits, TEMPORARY
	SATURATE_16BIT(lAcc);  // Macro but no return value
	out[i] = (short) (lAcc>>15);
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->firLength; i++)
	in[-1-i] = in[inLength-1-i];
}	// ---- end ComputeSRC_DownBy2FIRi() ---- 

//============================================================================
// ComputeSRC_DownBy3FIRf:	Sampling rate downconversion by factor 3
//				with FIR filtering
//============================================================================
    void 
ComputeSRC_DownBy3FIRf(short *in, short *out, long inLength, long outLength, SRC *d)
// length:		# of samples to produce
{
long   i = 0, j = 0, k = 0, lAcc;
short *zP = in;
float *h = d->h;
{static long c=0; printf("ComputeSRC_DownBy3FIRf %d outLength=%d\n", c++, outLength);}

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
	SATURATE_16BIT(lAcc);  // Macro but no return value
	out[i] = (short) lAcc;
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->firLength; i++)
	in[-1-i] = in[inLength-1-i];
}	// ---- end ComputeSRC_DownBy3FIRf() ---- 

//============================================================================
// ComputeSRC_DownBy3FIRi:	Sampling rate downconversion by factor 3
//				with FIR filtering
//				Fixed-point implementation
//============================================================================
    void 
ComputeSRC_DownBy3FIRi(short *in, short *out, long inLength, long outLength, SRC *d)
{
long   i = 0, j = 0, k = 0;
long   hI, zI, lAcc;
short *zP, *hi = d->hI;

{static long c=0; printf("ComputeSRC_DownBy3FIRi %d inLength=%d outLength=%d\n", c++, inLength, outLength);}

for (i = 0, k = 0; i < outLength; i++, k +=3) 
	{
// Compute FIR filter.  Half the outputs are tossed, so don't compute them. 
        zP = &in[k];
        for (hI = 0, zI = 0, lAcc = 0; hI < d->firLength; hI++, zI--)
		lAcc += hi[hI]*zP[zI];
//	acc *= d->inScale;  // FIXXX tuck into coefficients

// Saturate and output:  FIXXX add guard bits to accumulator, then add saturation code
// 15 instead of 16 for guard bits, TEMPORARY
//	SATURATE_16BIT(lAcc);  // Macro but no return value
	out[i] = (short) (lAcc>>15);
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->firLength; i++)
	in[-1-i] = in[inLength-1-i];
}	// ---- end ComputeSRC_DownBy3FIRi() ---- 

#ifdef NEEDED
//============================================================================
// InterpolateShortsBy2_FIR15:	Resample by 2 interpolation with FIR filter
//
//============================================================================
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
	SATURATE_16BIT(lAcc);  // Macro but no return value
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
	SATURATE_16BIT(lAcc);  // Macro but no return value
	out[j+1] = (short) lAcc;
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->firLength; i++)
	in[-1-i] = in[inLength-1-i];
}	// ---- end InterpolateShortsBy2_FIR15() ---- 
#endif

//============================================================================
// ComputeSRC_UpBy2FIRf:	Resample by 2 interpolation with FIR filter
//
//			32-bit floating point implementation
//============================================================================
    static void 
ComputeSRC_UpBy2FIRf(short *in, short *out, long inLength, long outLength, SRC *d)
{
long    i, j;
long hI, zI, lAcc;
short *zP;
float *h = d->h, acc;
float k = 2.0f*d->inScale;

//{static long c=0; printf("ComputeSRC_UpBy2FIRf %d inLength=%d outLength=%d\n", c++, inLength, outLength);}

// Produce two output values for each input value
for (i = 0, j = 0; i < inLength; i++, j +=2) 
	{
        zP = &in[i];
// Compute FIR filter (even coeffs)
        for (hI = 0, zI = 0, acc = 0.0f; hI < d->firLength; hI += 2, zI--)
		acc += h[hI]*(float)zP[zI];		
	acc *= k;  // FIXXX tuck into coefficients in Update() for efficiency

// Saturate accumulator and output
	lAcc = (long) acc;
	SATURATE_16BIT(lAcc);  // Macro but no return value
	out[j] = (short) lAcc;

// Compute FIR filter (odd coeffs)
        for (hI = 1, zI = 0, acc = 0.0f; hI < d->firLength; hI += 2, zI--)
		acc += h[hI]*(float)zP[zI];
	acc *= k;  // FIXXX tuck into coefficients in Update() for efficiency

// Saturate accumulator and output
	lAcc = (long) acc;
	SATURATE_16BIT(lAcc);  // Macro but no return value
	out[j+1] = (short) lAcc;
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->firLength; i++)
	in[-1-i] = in[inLength-1-i];
}	// ---- end ComputeSRC_UpBy2FIRf() ---- 

//============================================================================
// ComputeSRC_UpBy2FIRf_Unfiltered:	Resample by 2 interpolation, NO FIR filter
//
//			32-bit floating point implementation
//============================================================================
    static void 
ComputeSRC_UpBy2FIRf_Unfiltered(short *in, short *out, long inLength, long outLength, SRC *d)
{
long    i, j, lY;

//{static long c=0; printf("ComputeSRC_UpBy2FIRf_Unfiltered %d\n", c++);}

// Interpolation loop (unfiltered)
for (i = 0, j = 0; i < inLength; i++, j +=2) 
	{
//        out[j  ] = in[i];
//	float y = 2.0f*(float)in[i];
// Saturate accumulator and output
//	lY = (long) y;
	SATURATE_16BIT(lY);  // Macro but no return value
//	out[j] = (short) lY;

	out[j] = in[i];
	out[j+1] = 0;
	}
}	// ---- end ComputeSRC_UpBy2FIRf_Unfiltered() ---- 

//============================================================================
// ComputeSRC_UpBy2FIRf_BREAKOUT:	Resample by 2 interpolation with FIR filter
//
//			32-bit floating point implementation
//============================================================================
    static void 
ComputeSRC_UpBy2FIRf_BREAKOUT(short *in, short *out, long inLength, long outLength, SRC *d)
{
long    i, j;
long hI, zI, lAcc;
short *zP = in;
float *h = d->h;
float k = 2.0f*d->inScale;
static short tmpBuf[10000];
short *tmpBufP = &tmpBuf[kSRC_Filter_MaxDelayElements];

//{static long c=0; printf("ComputeSRC_UpBy2FIRf_BREAKOUT %d : inLength=%d outLength=%d\n", c++, inLength, outLength);}

// Interpolation loop (unfiltered)
for (i = 0, j = 0; i < inLength+d->firLength; i++, j +=2) 
	{
        tmpBuf[j  ] = in[i-d->firLength];
	tmpBuf[j+1] = 0;
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->firLength; i++)
	in[-1-i] = in[inLength-1-i];

// Filter loop:  input already interpolated by factor 2
for (i = 0; i < outLength; i++) 
	{
	float acc = 0.0f;
        zP = &tmpBufP[i];
// Compute FIR filter (all coeffs)
        for (hI = 0, zI = 0; hI < d->firLength; hI++, zI--)
		acc += h[hI]*(float)zP[zI];
	acc *= k;  // FIXXX tuck into coefficients in Update() for efficiency

// Saturate accumulator and output
	lAcc = (long) acc;
	SATURATE_16BIT(lAcc);  // Macro but no return value
	out[i] = (short) lAcc;
	}
}	// ---- end ComputeSRC_UpBy2FIRf_BREAKOUT() ---- 

//============================================================================
// ComputeSRC_UpBy2FIRi:	Resample by 2 interpolation with FIR filter
//
//			16-bit fixed point implementation
//============================================================================
    static void 
ComputeSRC_UpBy2FIRi(short *in, short *out, long inLength, long outLength, SRC *d)
{
long   i, j;
long   hI, zI, lAcc;
short *zP, *hi = d->hI;

//{static long c=0; printf("ComputeSRC_UpBy2FIRi %d inLength=%d firLength=%d\n", c++, inLength, d->firLength);}

// Produce two output values for each input value
for (i = 0, j = 0; i < inLength ; i++, j +=2) 
	{
        zP = &in[i];
// Compute FIR filter (even coeffs)
        for (hI = 0, zI = 0, lAcc = 0; hI < d->firLength; hI += 2, zI--)
		lAcc += hi[hI]*zP[zI];

// Saturate and output:  FIXXX add guard bits to accumulator, then add saturation code
// 15 instead of 16 for guard bits, TEMPORARY
// 14 to compensate for by2 interpolation
//	SATURATE_16BIT(lAcc);  // Macro but no return value
	out[j] = (short) (lAcc>>14);

// Compute FIR filter (odd coeffs)
        for (hI = 1, zI = 0, lAcc = 0; hI < d->firLength; hI += 2, zI--)
		lAcc += hi[hI]*zP[zI];
	out[j+1] = (short) (lAcc>>14);
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->firLength; i++)
	in[-1-i] = in[inLength-1-i];
}	// ---- end ComputeSRC_UpBy2FIRi() ---- 

// ============================================================================
// ComputeSRC_UpBy3FIRf:	Resample by 3 interpolation with FIR filter
//
//			32-bit floating point implementation
// ============================================================================
    static void 
ComputeSRC_UpBy3FIRf(short *in, short *out, long inLength, long outLength, SRC *d)
{
long    i, j;
long hI, zI, lAcc;
short *zP;
float *h = d->h;
float k = 3.0f*d->inScale;

//{static long c=0; printf("ComputeSRC_UpBy3FIRf %d\n", c++);}

// Produce three output values for each input value
for (i = 0, j = 0; i < inLength; i++, j += 3) 
	{
	float acc;
        zP = &in[i];
// Compute FIR filter output #1
        for (hI = 0, zI = 0, acc = 0.0f; hI < d->firLength; hI += 3, zI--)
		acc += h[hI]*(float)zP[zI];
	acc *= k;  // FIXXX tuck into coefficients in Update() for efficiency

// Saturate and output
	lAcc = (long) acc;
	SATURATE_16BIT(lAcc);  // Macro but no return value
	out[j] = (short) acc;

// Compute FIR filter output #2
        for (hI = 1, zI = 0, acc = 0.0f; hI < d->firLength; hI += 3, zI--)
		acc += h[hI]*(float)zP[zI];
	acc *= k;  // FIXXX tuck into coefficients in Update() for efficiency

// Saturate and output
	lAcc = (long) acc;
	SATURATE_16BIT(lAcc);  // Macro but no return value
	out[j+1] = (short) lAcc;

// Compute FIR filter output #3
        for (hI = 2, zI = 0, acc = 0.0f; hI < d->firLength; hI += 3, zI--)
		acc += h[hI]*(float)zP[zI];
	acc *= k;  // FIXXX tuck into coefficients in Update() for efficiency

// Saturate and output
	lAcc = (long) acc;
	SATURATE_16BIT(lAcc);  // Macro but no return value
	out[j+2] = (short) lAcc;
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->firLength; i++)
	in[-1-i] = in[inLength-1-i];
}	// ---- end ComputeSRC_UpBy3FIRf() ---- 

// ============================================================================
// ComputeSRC_UpBy3FIRi:	Resample by 3 interpolation with FIR filter
//
//			16-bit fixed-point implementation
// ============================================================================
    static void 
ComputeSRC_UpBy3FIRi(short *in, short *out, long inLength, long outLength, SRC *d)
{
long    i, j;
long   hI, zI, lAcc;
short *zP, *hi = d->hI;
short compI = (short)(k2To15m1f * d->inScale);
// {static long c=0; printf("ComputeSRC_UpBy3FIRi %d\n", c++);}

// Produce three output values for each input value
for (i = 0, j = 0; i < inLength; i++, j += 3) 
	{
        zP = &in[i];
// Compute output #1
        for (hI = 0, zI = 0, lAcc = 0; hI < d->firLength; hI += 3, zI--)
		lAcc += hi[hI]*zP[zI];

// Saturate and output:  FIXXX add guard bits to accumulator, then add saturation code
// 15 instead of 16 for guard bits, TEMPORARY
// 14 to compensate for by2 interpolation  FIXXX: need by3
//	SATURATE_16BIT(lAcc);  // Macro but no return value
	lAcc = (lAcc + (lAcc>>1))>>14; // Scale by 3 to compensate for By3 interpolation
	out[j] = (short) ((lAcc*compI)>>15);

// Compute output #2
        for (hI = 1, zI = 0, lAcc = 0; hI < d->firLength; hI += 3, zI--)
		lAcc += hi[hI]*zP[zI];
	lAcc = (lAcc + (lAcc>>1))>>14;
	out[j+1] = (short) ((lAcc*compI)>>15);

// Compute output #3
        for (hI = 2, zI = 0, lAcc = 0; hI < d->firLength; hI += 3, zI--)
		lAcc += hi[hI]*zP[zI];
	lAcc = (lAcc + (lAcc>>1))>>14;	
	out[j+2] = (short) ((lAcc*compI)>>15);
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->firLength; i++)
	in[-1-i] = in[inLength-1-i];
}	// ---- end ComputeSRC_UpBy3FIRi() ---- 

//============================================================================
// SRC_SetInSamplingFrequency:		
//============================================================================
	void
SRC_SetInSamplingFrequency(SRC *d, float x)
{
d->inSamplingFrequency = x;
}	// ---- end SRC_SetInSamplingFrequency() ---- 

//============================================================================
// SRC_SetOutSamplingFrequency:		
//============================================================================
	void
SRC_SetOutSamplingFrequency(SRC *d, float x)
{
d->outSamplingFrequency = x;
}	// ---- end SRC_SetOutSamplingFrequency() ---- 

// ============================================================================
// TranslateSRC_ModeID:   Translate ID to English string
// ============================================================================
    char * 
TranslateSRC_ModeID(int id)
{
switch (id)
	{
	 case kSRC_Interpolation_Type_AddDrop:
		return ("AddDrop");
	 case kSRC_Interpolation_Type_Linear:
		return ("Linear");
	 case kSRC_Interpolation_Type_FIR:
		return ("FIR");
	 case kSRC_Interpolation_Type_IIR:
		return ("IIR");
	 case kSRC_Interpolation_Type_Unfiltered:
		return ("Unfiltered");
	 case kSRC_Interpolation_Type_Triangle:
		return ("Triangle");
	 case kSRC_Interpolation_Type_Box:
		return ("Box");
	}
return ("Bogus");
}	// ---- end TranslateSRC_ModeID() ---- 
//============================================================================
// RunSRC:		Resample, big-mama routine
//============================================================================
	void
RunSRC(short *in, short *out, long inLength, long outLength, SRC *d)
{
long oiRatio = outLength/inLength;
long ioRatio = inLength/outLength;

#ifdef NEEDED
if (kSRC_Interpolation_Type_FIR == d->type)
{
{static long c=0; printf("RunSRC%d: type=%d '%s' V%d inLength=%d outLength=%d fixedPoint=%d \n", c++, 
d->type, TranslateSRC_ModeID(d->type), d->filterVersion,
inLength, outLength, d->useFixedPoint);}
}
else
{
{static long c=0; printf("RunSRC%d: type=%d '%s' inLength=%d outLength=%d fixedPoint=%d \n", c++, 
d->type, TranslateSRC_ModeID(d->type), inLength, outLength, d->useFixedPoint);}
}
#endif

if (d->useImpulse)
	{
	ClearShorts(in, inLength);
	if (1 == d->useImpulse)
		{
		in[0] = 10000;
		d->useImpulse = 2;
//		printf("RunSRC: impulse generated \n");
		}
//	printf("RunSRC: clearing input buffer \n");
	}

switch (d->type)
	{
	case kSRC_Interpolation_Type_AddDrop:
		if (d->useFixedPoint)
			InterpolateShorts_AddDropi(in, out, inLength, outLength, d);
		else
			InterpolateShorts_AddDropf(in, out, inLength, outLength, d);
	break;
	default:
	case kSRC_Interpolation_Type_Linear:
		if (d->useFixedPoint)
			InterpolateShorts_Lineari(in, out, inLength, outLength, d);
		else
			InterpolateShorts_Linearf(in, out, inLength, outLength, d);
	break;
	case kSRC_Interpolation_Type_Triangle:
// FIXXX:  only implemented for a few cases of upsampling, for now
		if 	    (2 == oiRatio)
			SRCShorts_Triangle2(in, out, inLength, outLength);
		else if (4 == oiRatio)
			SRCShorts_Triangle4(in, out, inLength, outLength);
		else
			printf("RunSRC: Type_Triangle oiRatio=%d  or ioRatio=%d\n", oiRatio, ioRatio);
	break;
	case kSRC_Interpolation_Type_FIR:
		{
		if (inLength > outLength)
			{
			switch (ioRatio)
				{	
				case 2:
				if (d->useFixedPoint)
					ComputeSRC_DownBy2FIRi(in, out, inLength, outLength, d);
				else
					ComputeSRC_DownBy2FIRf(in, out, inLength, outLength, d);
				break;
				case 3:
				if (d->useFixedPoint)
					ComputeSRC_DownBy3FIRi(in, out, inLength, outLength, d);
				else
					ComputeSRC_DownBy3FIRf(in, out, inLength, outLength, d);
				break;
				default:
				printf("RunSRC: Unsupported FIR decimation ratio = %d \n", ioRatio);
				break;
				}
			}
		else
			{
			switch (oiRatio)
				{	
				case 2:
				if (d->useFixedPoint)
					ComputeSRC_UpBy2FIRi(in, out, inLength, outLength, d);
				else
					ComputeSRC_UpBy2FIRf(in, out, inLength, outLength, d);
//				ComputeSRC_UpBy2FIRf_BREAKOUT(in, out, inLength, outLength, d, True);
				break;
				case 3:
				if (d->useFixedPoint)
					ComputeSRC_UpBy3FIRi(in, out, inLength, outLength, d);
				else
					ComputeSRC_UpBy3FIRf(in, out, inLength, outLength, d);
//				ComputeSRC_UpBy3FIRf_BREAKOUT(in, out, inLength, outLength, d, True);
				break;
				default:
				printf("RunSRC: Unsupported interpolation ratio = %d \n", oiRatio);
				break;
				}
			}
		}
	break;
	case kSRC_Interpolation_Type_TestFIR:
		ComputeSRC_TestFIR(in, out, inLength, d);
	break;
	case kSRC_Interpolation_Type_Unfiltered:
		{
		if (inLength > outLength)
			{
			switch (ioRatio)
				{	
				default:
				printf("RunSRC: Unsupported decimation ratio = %d \n", ioRatio);
				break;
				}
			}
		else
			{
			switch (oiRatio)
				{	
				case 2:
					ComputeSRC_UpBy2FIRf_Unfiltered(in, out, inLength, outLength, d);
				break;
//				case 3:
//				break;
				default:
				printf("RunSRC: Unsupported interpolation ratio = %d \n", oiRatio);
				break;
				}
			}
		}
	break;
	}
}	// ---- end RunSRC() ---- 

