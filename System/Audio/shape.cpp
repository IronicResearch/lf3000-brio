// **********************************************************************
//
// shape.cpp		Routines for waveshaper/soft clipper
//
//					Written by Gints Klimanis, 2007
// **********************************************************************
#include <string.h>

#include "shape.h"

// ************************************************************************
// DefaultWaveShaper:	 Default parameter set
// ************************************************************************ 
    void
DefaultWaveShaper(WAVESHAPER *d)
{
d->type = kWaveShaper_Type_V1;

d->inGainDB    = kWaveShaper_InGainDB_Default;
d->outGainDB   = kWaveShaper_OutGainDB_Default;
d->thresholdDB = kWaveShaper_ThresholdDB_Default;

d->headroomBits = 0;

d->oneThf = 1.0f/3.0f;           
d->twoThf = 2.0f/3.0f;           
d->oneThi = FloatToQ15(d->oneThf);           
d->twoThi = FloatToQ15(d->twoThf);  

d->q31_2     = FloatToQ15v2(2.0f);
d->q31_3     = FloatToQ15v2(3.0f);
d->q31_3rd   = FloatToQ15v2(1.0f/3.0f);
d->q15_2_3rd = FloatToQ15v2(2.0f/3.0f);

//printf("DefaultWaveShaper: oneThf %g -> %X\n", d->oneThf, d->oneThi);
// Initialize some low level data

d->lowLevelDataBogus = true;
d->samplingFrequency = kBogusSamplingFrequency;
}	// ---- end DefaultWaveShaper() ---- 

// ************************************************************************
// UpdateWaveShaper:	Convert high-level to low-level parameters
//				NOTE: does not reset state
// ************************************************************************ 
    void
UpdateWaveShaper(WAVESHAPER *d)
{
//{static long c=0; printf("UpdateWaveShaper%ld: inGainDB %g -> %g\n", c++, d->inGainDB, d->inGainf);}

d->inGainf  = DecibelToLinearf(d->inGainDB);
d->inWholeI = (long)d->inGainf;
d->inFracF  = d->inGainf - (float)d->inWholeI;
d->inFracI  = FloatToQ15(d->inFracF);

//printf("UpdateWaveShaper: inGainDB %g -> %g\n", d->inGainDB, d->inGainf);
//printf("UpdateWaveShaper: inWholeI=%d inFracF=%g inFracI=$%X\n", 
//        d->inWholeI, d->inFracF, (unsigned int) d->inFracI);

d->outGainf = DecibelToLinearf(d->outGainDB);
d->outGaini = FloatToQ15(d->outGainf);
//printf("UpdateWaveShaper: outGainDB %g -> %g\n", d->outGainDB, d->outGainf);

d->thresholdf = DecibelToLinearf(d->thresholdDB);
d->thresholdi = FloatToQ15(d->thresholdf);
//printf("UpdateWaveShaper: thresholdDB %g -> %g\n", d->thresholdDB, d->thresholdf);

// Currently doesn't use fs setting
//if (kBogusSamplingFrequency == d->samplingFrequency)
//	printf("UpdateWaveShaper: Hey.  SamplingFrequency=%g not set !\n", d->samplingFrequency);

d->lowLevelDataBogus = false;
}	// ---- end UpdateWaveShaper() ---- 

// ************************************************************************
// ResetWaveShaper:	 Reset state and delta parameters
// ************************************************************************ 
    void
ResetWaveShaper(WAVESHAPER *d)
{
}	// ---- end ResetWaveShaper() ---- 

// ************************************************************************
// PrepareWaveShaper:	Update() + Reset() + one-time low level initializations
// ************************************************************************ 
    void
PrepareWaveShaper(WAVESHAPER *d)
{
d->lowLevelDataBogus = true;

UpdateWaveShaper(d);
ResetWaveShaper (d);
}	// ---- end PrepareWaveShaper() ---- 

// ************************************************************************
// SetWaveShaper_Parameters:	
// ************************************************************************ 
    void
SetWaveShaper_Parameters(WAVESHAPER *d, int type, float inGainDB, float outGainDB)
{
//{static long c=0; printf("SetWaveShaper_Parameters%ld: inGainDB=%g outGainDB=%g\n", c++, inGainDB, outGainDB);}

d->type = type;

Boundf(&inGainDB, kWaveShaper_InGainDB_Min, kWaveShaper_InGainDB_Max);
d->inGainDB = inGainDB;

Boundf(&outGainDB, kWaveShaper_OutGainDB_Min, kWaveShaper_OutGainDB_Max);
d->outGainDB = outGainDB;

//d->lowLevelDataBogus = true;
//d->samplingFrequency = kBogusSamplingFrequency;
}	// ---- end SetWaveShaper_Parameters() ---- 

// ************************************************************************
// SetWaveShaper_SamplingFrequency:	Set module sampling frequency 
//                          Does not update parameters.  Call Update()
// ************************************************************************ 
	void 
SetWaveShaper_SamplingFrequency(WAVESHAPER *d, float samplingFrequency)
{
d->lowLevelDataBogus = (d->samplingFrequency != samplingFrequency);
d->samplingFrequency      = samplingFrequency;
// FIXXX: add bounding code
}	// ---- end SetWaveShaper_SamplingFrequency() ---- 

// ************************************************************************
// ComputeWaveShaper_V1f:   Soft clipping function (S-curve)
// ************************************************************************ 
    void
ComputeWaveShaper_V1f(S16 *x, S16 *y, long length, WAVESHAPER *d)
{
//{static long c=0; printf("ComputeWaveShaper_V1f: %d START\n", c++);}
float inK  = d->inGainf  * (1.0f/32768.0f);
float outK = d->outGainf * (     32768.0f);

// Classic S-curve saturation
//   f(x) = x / (1 + |x|)
//2.0*x/(1.0 + fabs(x)) );

for (long i = 0; i < length; i++)
	{
    float xf = inK*(float) x[i];
// Take absolute value
    S16 absXi = y[i];
    if (absXi < 0)
        absXi = 0 - absXi;
    float absXf = inK*(float) absXi;
// Apply range warping
    float yf    = 2.0f*xf/(1.0 + absXf);

// Convert and bound to S16 range
    long yl = (long) (outK*yf);
    if      (yl > kS16_Max)
        y[i] = kS16_Max;
    else if (yl < kS16_Min)
        y[i] = kS16_Min;
    else
	    y[i] = (S16) yl;	 
	}
}   // ---- end ComputeWaveShaper_V1f() ---- 

// ************************************************************************
// ComputeWaveShaper_V2f:   Soft clipping function
// ************************************************************************ 
    void
ComputeWaveShaper_V2f(S16 *x, S16 *y, long length, WAVESHAPER *d)
{
float th    = 0.75f;
float invTh = 1.0f/(1.0f-th);
float yf;
float inK  = d->inGainf  * (1.0f/32768.0f);
float outK = d->outGainf * (     32768.0f);

// x < th : f(x) = x
// x > th : f(x) = th + (x-th)/(1+((x-th)/(1-th))^2)
//ScaleShortsf(x, y, length, d->inGainf);
for (long i = 0; i < length; i++)
	{
    float xf = inK*(float) y[i];
	if		(xf < th)
		yf = xf;
	else //if (x > th)
		{
		float b = (xf-th)*invTh;
		yf = th + (xf-th)/(1.0f + b*b);
		}

// Bound to S16 range
    long yl = (long) (outK*yf);
    if      (yl > kS16_Max)
        y[i] = kS16_Max;
    else if (yl < kS16_Min)
        y[i] = kS16_Min;
    else
	    y[i] = (S16) yl;	 
	}
}   // ---- end ComputeWaveShaper_V2f() ---- 

// ************************************************************************
// ComputeWaveShaper_V3f:   Soft clipping function  (polynomial, divide-free)
// ************************************************************************ 
    void
ComputeWaveShaper_V3f(S16 *x, S16 *y, long length, WAVESHAPER *d)
{
float th    = 0.75f;
float invTh = 1.0f/(1.0f-th);
float inK  = d->inGainf  * (1.0f/32768.0f);
float outK = d->outGainf * (     32768.0f);

// f(x) = 3x/2 - (x^3)/2
//ScaleShortsf(x, y, length, d->inGainf);
for (long i = 0; i < length; i++)
	{
    float xf = inK*(float) y[i];
// x <= 1: f(x) = 1.5x - (x*x*x)/2
	float yf = 1.5f*xf - 0.5f*xf*xf*xf;

// Convert and bound to S16 range
    long yl = (long) (outK*yf);
    if      (yl > kS16_Max)
        y[i] = kS16_Max;
    else if (yl < kS16_Min)
        y[i] = kS16_Min;
    else
	    y[i] = (S16) yl;	 
	}
}   // ---- end ComputeWaveShaper_V3f() ---- 

// ************************************************************************
// ComputeWaveShaper_V4f:   Soft clipping function  (polynomial, divide-free)
//                          Innate +6 dB gain
// ************************************************************************ 
    void
ComputeWaveShaper_V4f(S16 *inX, S16 *y, long length, WAVESHAPER *d)
{
float inK  = d->inGainf  * (1.0f/32768.0f);
float outK = d->outGainf * (     32768.0f);
float oneTh = 1.0f/3.0f;
float twoTh = 2.0f*oneTh;

float t;
//{static long c=0; printf("ComputeWaveShaper_V4f: START %d\n", c++);}

// f(x)                Range
// ----            ---------------
//  2x             0   <= x <= th
//  3-(2-3x)^2     th <= x <= 2*th
//  ----------
//      3    
//  1              2*th <= x <= 1
// NOTE:  work on this some more to reduce 
for (long i = 0; i < length; i++)
	{
    float xf = inK*(float) inX[i];
    float yf;
    float absXf = xf; //fabsf(xf);
    if (inX[i] < 0)
        absXf = -xf;

    if      (absXf < oneTh)
        yf = 2.0f*xf;
    else if (absXf < twoTh)
        {
#define SHAPE_V4f_MESSEDUP
#ifdef SHAPE_V4f_MESSEDUP
        t  = 2.0f - 3.0f*absXf;
        if (xf > 0.0f)
            yf =  (3.0f - t*t)/3.0f;
        else
            yf = -(3.0f - t*t)/3.0f;
#else
        t  = (2.0f - 3.0f*xf);
        yf = 1.0f - (1.0f/3.0f)*t*t;
#endif
		}
    else
        {
        if (inX[i] > 0)
            yf =  1.0f;
        else
            yf = -1.0f;
        }

// Convert and bound to S16 range
    long yl = (long) (outK*yf);
    if      (yl > kS16_Max)
        y[i] = kS16_Max;
    else if (yl < kS16_Min)
        y[i] = kS16_Min;
    else
	    y[i] = (S16) yl;	 
	}
}   // ---- end ComputeWaveShaper_V4f() ---- 

// ************************************************************************
// ComputeWaveShaper_V4i:   Soft clipping function  (polynomial, divide-free)
//                          Innate ~ +6 dB gain
// ************************************************************************ 
    void
ComputeWaveShaper_V4i(S16 *inX, S16 *outY, long length, WAVESHAPER *d)
{
//Q15 inK  = d->inGaini  ;
//Q15 outK = d->outGaini ;
{static long c=0; printf("ComputeWaveShaper_V4i: START %ld\n", c++);}

// f(x)                Range
// ----            ---------------
//  2x             0  <= x <=   th
//  3-(2-3x)^2     th <= x <= 2*th
//  ----------
//      3    
//  1              2*th <= x <= 1
// NOTE:  work on this some more to reduce 
for (long i = 0; i < length; i++)
	{
    Q15 xi    = inX[i];
    Q15 absXi = xi; 
    short yi;

	if (xi < 0)
		absXi = -xi;

	if		(absXi <= d->oneThi)
		yi = 2*xi;
	else if (absXi <= d->twoThi)
		{
//		int shift = 15;
		long ti = d->q31_2 - ((d->q31_3*absXi)>>15);
	    long yl = d->q31_3 - ((ti*ti)>>15);
		if (xi < 0)
			yl = -yl;
		yl *= d->q31_3rd;
		yi = (Q15) (yl>>15);
		}	
	else
		{
		if (xi < 0)
			yi = kS16_Min;
		else
			yi = kS16_Max;
		}

// Convert and bound to S16 range
//    long yl = (long) (acc);
//    if      (yl > kS16_Max)
//        outY[i] = kS16_Max;
//    else if (yl < kS16_Min)
//        outY[i] = kS16_Min;
//    else
	    outY[i] = (S16) yi;	 
	}

}   // ---- end ComputeWaveShaper_V4i() ---- 

// ************************************************************************
// ComputeWaveShaper_V4d1i:   Soft clipping function  (polynomial, divide-free)
//                          Innate ~ +6 dB gain in algorithm
// ************************************************************************ 
    void
ComputeWaveShaper_V4d1i(S16 *inX, S16 *outY, long length, WAVESHAPER *d)
{
//Q15 inK  = d->inGaini  ;
//Q15 outK = d->outGaini ;
int bits = d->headroomBits; //2;
long s16_Min = kS16_Min;//<<bits;
long s16_Max = kS16_Max;//<<bits;

//{static long c=0; printf("ComputeWaveShaper_V4d1i: START %d headroomBits=%d\n", c++, d->headroomBits);}
//{static long c=0; if (!c) printf("ComputeWaveShaper_V4d1i: START %d bits=%d inKf=%g\n", c++, bits, inKf);}
//ScaleShortsf(inX, inX, length, 2.0f/3.0f);
ScaleShortsi_Q15(inX, inX, length, d->q15_2_3rd);

// f(x)                Range
// ----            ---------------
//  2x             0  <= x <=   th
//  3-(2-3x)^2     th <= x <= 2*th
//  ----------
//      3    
//  1              2*th <= x <= 1
// NOTE:  work on this some more to reduce 
for (long i = 0; i < length; i++)
	{
    long yl, absX;
    long x = (inX[i]<<bits);

// Scale by soft clipper preGain
    x = x*d->inWholeI + ((x*d->inFracI)>>15);

    absX = x; 
	if (x < 0)
		absX = -x;
//printf("IN : inX=%5d -> x=%5d\n", inX[i], x);
	if		(absX <= d->oneThi)
        {
		yl = x+x;  // 2*x
//printf("1/3 : x=%5d -> yl=%5d (%g)\n", x, yl, ((float)yl)/(float)x);
        }
	else if (absX <= d->twoThi)
		{
		long ti = d->q31_2 - ((d->q31_3*absX)>>15);
	    yl = d->q31_3 - ((ti*ti)>>15);
		if (x < 0)
			yl = -yl;
		yl *= d->q31_3rd;
		yl = (yl>>15);
//printf("2/3 : x=%5d -> yl=%5d (%g)\n", x, yl, ((float)yl)/(float)x);
		}	
	else
		{
//printf("absXi=%5d -> %5d ::: oneThi=%5d twoThi=%5d\n", absX, yl, d->oneThi, d->twoThi);
		if (inX[i] < 0)
			yl = s16_Min;
		else
			yl = s16_Max;
//printf("3/3 : x=%5d -> yl=%5d  (%g)\n", x, yl, ((float)yl)/(float)x);
		}
 
//printf("END : %5d -> %5d  >> bits \n", x, yl, yl>>bits);
    yl >>= bits;
//printf("--- : %5d -> %5d (OUT) (%g) \n", inX[i], yl, ((float)yl)/(float)inX[i]);
	outY[i] = (S16) yl;	 
	}
}   // ---- end ComputeWaveShaper_V4d1i() ---- 

// ************************************************************************
// ComputeWaveShaper:   Master call (wrapper of all routines)
// ************************************************************************ 
    void
ComputeWaveShaper(S16 *x, S16 *y, long length, WAVESHAPER *d)
{
//{static long c=0; printf("ComputeWaveShaper%d: fixpt=%d type=%d headroomBits=%d\n", c++, d->useFixedPoint, d->type, d->headroomBits);}

if (d->useFixedPoint)
    {
//    ScaleShortsf(x, x, length, d->inGainf);
//{static long c=0; printf("ComputeWaveShaper%d: useFixedPoint  inGainf=%g outGainf=%g \n", c++, d->inGainf, d->outGainf);}
//ShiftLeft_S16(x, x, length, d->headroomBits);  
//ShiftLeft_S16(x, x, length, 1);  // GK FIXX: 1 is for single channel testing

    switch (d->type)
        {
        default:
        case kWaveShaper_Type_ByPass:
//            CopyShorts(x, y, length);
//            ScaleShortsf(x, y, length, d->inGainf*d->outGainf);
//printf("ComputeWaveShaper: inGainf=%g\n", d->inGainf);
        break;
        case kWaveShaper_Type_V1:
        case kWaveShaper_Type_V2:
        case kWaveShaper_Type_V3:
printf("ComputeWaveShaper: useFixedPoint: type=%d not implemented\n", d->type);
        break;
        case kWaveShaper_Type_V4:
//            ComputeWaveShaper_V4i(x, y, length, d);
            ComputeWaveShaper_V4d1i(x, y, length, d);
        break;
        }
// ScaleShortsf(y, y, length, d->outGainf);
// ShiftRight_S16(y, y, length, d->headroomBits);
//ShiftRight_S16(y, y, length, d->headroomBits);
    }
else
    {
    switch (d->type)
        {
        default:
        case kWaveShaper_Type_ByPass:
//            CopyShorts(x, y, length);
            ScaleShortsf(x, y, length, d->inGainf*d->outGainf);
//printf("ComputeWaveShaper: inGainf=%g\n", d->inGainf);
        break;
        case kWaveShaper_Type_V1:
            ComputeWaveShaper_V1f(x, y, length, d);
        break;
        case kWaveShaper_Type_V2:
            ComputeWaveShaper_V2f(x, y, length, d);
        break;
        case kWaveShaper_Type_V3:
            ComputeWaveShaper_V3f(x, y, length, d);
        break;
        case kWaveShaper_Type_V4:
            ComputeWaveShaper_V4f(x, y, length, d);
        break;
        }
    }
}   // ---- end ComputeWaveShaper() ---- 

