// *************************************************************** 
// dsputil_mix.cpp:		Support DSP for channel mixing routines
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>

#include "dsputil_mix.h"

// *************************************************************** 
// BoundS16:   Force value to specified range
//              16-bit integer version
// ***************************************************************
	void 
BoundS16(S16 *x, S16 lo, S16 hi)
{
//printf("BoundS16: x=%d  [%d .. %d] \n", *x, lo, hi);
if      (*x < lo)
    *x = lo;
else if (*x > hi)
    *x = hi;
}	// ---- end BoundS16() ---- 

// *************************************************************** 
// Boundf:   Force value to specified range
//              32-bit float version
// ***************************************************************
	void 
Boundf(float *x, float lo, float hi)
{
//printf("Boundf: x=%g  [%g .. %g] \n", *x, lo, hi);
if      (*x < lo)
    *x = lo;
else if (*x > hi)
    *x = hi;
}	// ---- end Boundf() ---- 

// *************************************************************** 
// PanValues_ConstantPower:   Convert x position in range [-1 .. 1] 
//                              to constant	power pan values
// ***************************************************************
	void 
PanValues_ConstantPower(float x, float *outs)
{
// Convert from range [-1 to 1]  to [0 to Pi/2]
x = (x + 1.0f)*(float)(kPi/4.0);

outs[kLeft ] = (float) cos(x);
outs[kRight] = (float) sin(x);
//printf("PanValues: x=%g -> <%g, %g>\n", x, outs[kLeft], outs[kRight]);
}	// ---- end PanValues_ConstantPower() ---- 

// *************************************************************** 
// AddShorts:	Add in buffer to out buffer
//			32-bit floating-point implementation
//				( ok for "in place" operation )
//			Saturation option to 16-bit range
// ***************************************************************
    void 
AddShorts(short *in, short *out, long length, long saturate)
{
if (saturate)
	{
	for (long i = 0; i < length; i++)
		{
		long acc = ((long)out[i]) + (long) in[i];
		if 	    (acc >=  k2To15m1i)
			out[i] =  k2To15m1i;
		else if (acc <= -k2To15i)
			out[i] = -k2To15i;
		else
			out[i] = (short) acc;
		}
	}
else
	{
	for (long i = 0; i < length; i++)
	    out[i] += in[i];
	}
}	// ---- end AddShorts() ---- 

// *************************************************************** 
// CopyShorts:    Copy SHORTs from in to out buffer
// ***************************************************************
    void 
CopyShorts(short *in, short *out, long length)
{
//for (long i = 0; i < length; i++) 
//    out[i] = in[i];
bcopy((void *) in, (void *) out, length*sizeof(short));
}	// ---- end CopyShorts() ---- 

// *************************************************************** 
// ClearShorts:    Set SHORTs buffer to 0
// ***************************************************************
    void 
ClearShorts(short *out, long length)
{
for (long i = 0; i < length; i++) 
    out[i] = 0;
}	// ---- end ClearShorts() ---- 

// *************************************************************** 
// ScaleShortsi_Q15:	Scale 'short' from in buffer to out buffer
//				1.15 Fixed point implementation.  
//				 0 <= k < 1
//				(ok for "in place" operation )
// ***************************************************************
    void 
ScaleShortsi_Q15(Q15 *in, Q15 *out, long length, Q15 k)
{
for (long i = 0; i < length; i++)
    out[i] = MultQ15(in[i], k);
}	// ---- end ScaleShortsi_Q15() ---- 

// *************************************************************** 
// MACShortsi_Q15:	Scale 'short' from 'in' and add to 'out'
//				1.15 Fixed point implementation.  
//				 0 <= k < 1
//				(ok for "in place" operation )
// ***************************************************************
    void 
MACShortsi_Q15(Q15 *in, Q15 *out, long length, Q15 k)
{
for (long i = 0; i < length; i++)
    out[i] = MacQ15(out[i], in[i], k);
}	// ---- end MACShortsi_Q15() ---- 

// **********************************************************************
// FloatToQ15: Convert 32-bit float to 16-bit 1.15 signed integer
//                  Truncated not rounded to one LSB
// **********************************************************************
	Q15 
FloatToQ15(float x)
//	x	Range [-1 .. 1]  well, up to one LSB less than 1.0
{
float y = x*kQ15_Maxf;
if      (y >= kQ15_Maxf)
    return (kQ15_Max);
else if (y <= kQ15_Minf)
    return (kQ15_Min);
return ((Q15)y);
}	// ---- end FloatToQ15() ----

// **********************************************************************
// Q15ToFloat: Convert 16-bit 1.15 signed integer to 32-bit float
// **********************************************************************
	float 
Q15ToFloat(Q15 x)
//	x	Range [-1 .. 1]  well, up to one LSB less than 1.0
{
return ((1.0f/kQ15_Maxf)*(float)x);
}	// ---- end Q15ToFloat() ----

// **********************************************************************
// BlastSineOscillatorS16:  Initialize and compute sine wave.  
// **********************************************************************
	void 
BlastSineOscillatorS16(short *out, long length, float fc, float gain)
//	fc = frequency/samplingFrequency
//  gain  linear gain
{
double x     = 0.0;
double delta = (kTwoPi*fc)/(double)length;
for (long i = 0; i < length; i++, x += delta)
    {
    long y = (long)(gain*sin(x));
	if 	    (y > kS16_Max)
		y = kS16_Max;
	else if (y < kS16_Min)
		y = kS16_Min;

    out[i] = (short) y;
//    printf("%d, %d\n", i, y);
    }
}	// ---- end BlastSineOscillatorS16() ----




