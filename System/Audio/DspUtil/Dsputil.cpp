// **********************************************************************
//
// DspUtil.cpp		
//
//			Written by Gints Klimanis
// **********************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <util.h>
#include <Dsputil.h>
#include <Dsputil2.h>

// *************************************************************** 
// BoundS8:   Force value to specified range
//              8-bit signed integer version
//          Modify pointer address as well as return the bounded value.
// ***************************************************************
	S8 
BoundS8(S8 *x, S8 lo, S8 hi)
{
//printf("BoundS8: x=%d  [%d .. %d] \n", *x, lo, hi);
if      (*x < lo)
    *x = lo;
else if (*x > hi)
    *x = hi;
return (*x);
}	// ---- end BoundS8() ---- 

// *************************************************************** 
// BoundU8:   Force value to specified range
//              8-bit unsigned integer version.
//          Modify pointer address as well as return the bounded value.
// ***************************************************************
	U8 
BoundU8(U8 *x, U8 lo, U8 hi)
{
//printf("BoundU8: x=%d  [%d .. %d] \n", *x, lo, hi);
if      (*x < lo)
    *x = lo;
else if (*x > hi)
    *x = hi;
return (*x);
}	// ---- end BoundU8() ---- 

// *************************************************************** 
// BoundS16:   Force value to specified range
//              16-bit signed integer version.
//          Modify pointer address as well as return the bounded value.
// ***************************************************************
	S16 
BoundS16(S16 *x, S16 lo, S16 hi)
{
//printf("BoundS16: x=%d  [%d .. %d] \n", *x, lo, hi);
if      (*x < lo)
    *x = lo;
else if (*x > hi)
    *x = hi;
return (*x);
}	// ---- end BoundS16() ---- 

// *************************************************************** 
// BoundS32:   Force value to specified range
//              32-bit signed integer version.
//          Modify pointer address as well as return the bounded value.
// ***************************************************************
	S32 
BoundS32(S32 *x, S32 lo, S32 hi)
{
//printf("BoundS32: x=%d  [%d .. %d] \n", *x, lo, hi);
if      (*x < lo)
    *x = lo;
else if (*x > hi)
    *x = hi;
return (*x);
}	// ---- end BoundS32() ---- 

// *************************************************************** 
// Boundf:   Force value to specified range
//              32-bit float version
// ***************************************************************
	float 
Boundf(float *x, float lo, float hi)
{
//printf("Boundf: x=%g  [%g .. %g] \n", *x, lo, hi);
if      (*x < lo)
    *x = lo;
else if (*x > hi)
    *x = hi;
return (*x);
}	// ---- end Boundf() ---- 

// *************************************************************** 
// VolumeToGain:   Convert volume slider value to gain, where
//						Dumb function.  NEEDED for anything ?
// ***************************************************************
//    float
//VolumeToGain(float x, float *xRange)
//{
//float y;
//
// Convert from range [0 to 10]  to [0.0 to 10/7],
//	where 7 is unity gain
//y = x*(1.0f/7.0f);
//y = y*y;
//printf("VolumeToGain: %g -> %g (Temporary: extra 10)\n", x, y);
//return (y);
//}	// ---- end VolumeToGain() ---- 

// *************************************************************** 
// PanValues:   Convert x position in range[-1 .. 1] to constant
//						power pan values
// ***************************************************************
	void 
PanValues(float x, float *outs)
{
// Convert from range [-1 to 1]  to [0 to Pi/2],
x += 1.0;
x *= (float) (kPi/4.0);

// This is constant power
outs[Left ] = (float) cos(x);
outs[Right] = (float) sin(x);
//CosSinf(x, outs);

//printf("PanValues: x=%g -> <%g, %g>\n", x, outs[Left], outs[Right]);
}	// ---- end PanValues() ---- 

// *************************************************************** 
// ConstantPowerValues:   Convert x position in range [0 .. 1] to constant
//						power "pan" values
// ***************************************************************
	void 
ConstantPowerValues(float x, float *outLeft, float *outRight)
{
// Convert from range [0 to 1]  to [0 to Pi/2],
float xP =  x*(float) (kPi/2.0);

// This is constant power  sin(x)^2 + cos(x)^2 = 1
*outLeft  = (float) cos(xP);
*outRight = (float) sin(xP);

//printf("ConstantPowerValues: x=%g -> <%g, %g>\n", x, *outLeft, *outRight);
}	// ---- end ConstantPowerValues() ---- 


// ****************************************************************
// RandRangeF:	 Return random float value in given Range
//				FIXXXX: coded to 16-bit precision, yuck
// **************************************************************** 
	float 
RandRangeF(float lo, float hi)
{
// Generate random value in range [-1.0f .. 1.0f] 
static unsigned long longX = 1;
float floatX;
/* 1103515245 = 0x41C64E6D
        12345 = 0x00003039 */
longX  = 1103515245*longX + 12345;
floatX = NORMAL32768f((short)(longX>>16));
	
// Shift to new range    
floatX = ChangeRangef(floatX, -1.0f, 1.0f, lo, hi);
//printf("RandF: %g\n", floatX);
return (floatX);
}	// ---- end RandRangeF() ---- 

// ****************************************************************
// SegmentChangeRangef:	 Return value stretched to new tri-point range
// **************************************************************** 
	float 
SegmentChangeRangef(float x, float inLo , float inMid , float inHi,
							 float outLo, float outMid, float outHi)
{
if (x >= inMid)
	x = ChangeRangef(x, inMid, inHi , outMid, outHi); 
else
	x = ChangeRangef(x, inLo,  inMid, outLo, outMid); 

return (x);
}	// ---- end SegmentChangeRangef() ---- 

// ****************************************************************
// SegmentChangeRanged:	 Return value stretched to new tri-point range
// **************************************************************** 
	double 
SegmentChangeRanged(double x, double inLo , double inMid , double inHi,
							  double outLo, double outMid, double outHi)
{
if (x >= inMid)
	x = ChangeRangef(x, inMid, inHi , outMid, outHi); 
else
	x = ChangeRangef(x, inLo,  inMid, outLo, outMid); 
return (x);
}	// ---- end SegmentChangeRanged() ---- 

// *************************************************************** 
// SetDoubles:   Fill DOUBLE buffer w/'value'
// ***************************************************************
    void 
SetDoubles(double *d, long length, double value)
{
for (long i = 0; i < length; i++) 
    d[i] = value;
}	// ---- end SetDoubles() ---- 

// *************************************************************** 
// ClearFloats:   Fill FLOAT buffer w/0
// ***************************************************************
    void 
ClearFloats(float *d, long length)
{
for (long i = 0; i < length; i++) 
    d[i] = 0.0f;
//memset((void *) d, 0, length*sizeof(float) );
}	// ---- end ClearFloats() ---- 

// *************************************************************** 
// SetFloats:   Fill FLOAT buffer w/'value'
// ***************************************************************
    void 
SetFloats(float *d, long length, float value)
{
for (long i = 0; i < length; i++) 
    d[i] = value;
//memset((void *) i, 0, length*sizeof(float) );
}	// ---- end SetFloats() ---- 

// *************************************************************** 
// SetShorts:   Fill SHORT buffer w/'value'
// ***************************************************************
    void 
SetShorts(short *d, long length, short value)
{
for (long i = 0; i < length; i++) 
    d[i] = value;
}	// ---- end SetShorts() ---- 

// *************************************************************** 
// ClearShorts:   Fill SHORT buffer w/0
// ***************************************************************
    void 
ClearShorts(short *d, long length)
{
//for (long i = 0; i < length; i++) 
//    d[i] = 0;
bzero( d, length*sizeof(short));
}	// ---- end ClearShorts() ---- 

// *************************************************************** 
// ClearLongs:   Fill LONG buffer w/0
// ***************************************************************
    void 
ClearLongs(long *d, long length)
{
//for (long i = 0; i < length; i++) 
//    d[i] = 0;
bzero( d, length*sizeof(long));
}	// ---- end ClearLongs() ---- 
// *************************************************************** 
// CopyShorts:    Copy SHORTs from in to out buffer
// ***************************************************************
    void 
CopyShorts(short *in, short *out, long length)
{
//for (long i = 0; i < length; i++) 
//    out[i] = in[i];
bcopy(in, out, length*sizeof(long));
}	// ---- end CopyShorts() ---- 

// *************************************************************** 
// CopyLongs:    Copy LONGs from in to out buffer
// ***************************************************************
    void 
CopyLongs(long *in, long *out, long length)
{
//for (long i = 0; i < length; i++) 
//    out[i] = in[i];
bcopy(in, out, length*sizeof(long));
}	// ---- end CopyLongs() ---- 

// *************************************************************** 
// CopyFloats:    Copy 'float's from in to out buffer
// ***************************************************************
    void 
CopyFloats(long *in, long *out, long length)
{
//for (long i = 0; i < length; i++) 
//    out[i] = in[i];
bcopy(in, out, length*sizeof(long));
}	// ---- end CopyFloats() ---- 

// *************************************************************** 
// FanOutFloats:    Copy FLOATs from in to N out buffers
// ***************************************************************
    void 
FanOutFloats(float *in, float *outs[], long length, long N)
{
for (long i = 0; i < N; i++) 
    CopyFloats(in, outs[i], length);
}	// ---- end FanOutFloats() ---- 

// *************************************************************** 
// FanOutShorts:    Copy 'shorts' from in to N out buffers
// ***************************************************************
    void 
FanOutShorts(short *in, short *outs[], long length, long N)
{
for (long i = 0; i < N; i++) 
    CopyShorts(in, outs[i], length);
}	// ---- end FanOutShorts() ---- 

// *************************************************************** 
// CompareFloats:    Compare two buffers and return #differences
// ***************************************************************
    long 
CompareFloats(float *a, float *b, long length)
{
long diff = 0;			
for (long i = 0; i < length; i++) 
	diff += (a[i] != b[i]);
return (diff);
}	// ---- end CompareFloats() ---- 

// *************************************************************** 
// CompareShorts:    Compare two buffers and return #differences
// ***************************************************************
    long 
CompareShorts(short *a, short *b, long length)
{
long diff = 0;			
for (long i = 0; i < length; i++) 
	diff += (a[i] != b[i]);
return (diff);
}	// ---- end CompareShorts() ---- 

// *************************************************************** 
// CompareLongs:    Compare two buffers and return #differences
// ***************************************************************
    long 
CompareLongs(long *a, long *b, long length)
{
long diff = 0;			
for (long i = 0; i < length; i++) 
	diff += (a[i] != b[i]);
return (diff);
}	// ---- end CompareLongs() ---- 

// *************************************************************** 
// PrintFloats:    
// ***************************************************************
    void 
PrintFloats(float *d, long length)
{
for (long i = 0; i < length; i++) 
    printf("%d, %g\n", i, d[i]);
}	// ---- end PrintFloats() ---- 

// **********************************************************************
// DoubleToFractionalInteger: Convert 64-bit double to 16-bit 1.15 signed integer
// **********************************************************************
	short 
DoubleToFractionalInteger(double x)
//	x	Range [-1 .. 1]  well, up to one LSB less than 1.0
{
short y = (short)(0.5 + x*k2To15);

return (y);
}	// ---- end DoubleToFractionalInteger() ----

// **********************************************************************
// FractionalIntegerToDouble: Convert 64-bit double to 16-bit 1.15 signed integer
// **********************************************************************
	double 
FractionalIntegerToDouble(short x)
{
double y = (1.0/k2To15)*(double)x;

return (y);
}	// ---- end FractionalIntegerToDouble() ----

// *************************************************************** 
// PrintFloatLine:    Print floats with no NewLine characters
// ***************************************************************
    void 
PrintFloatLine(float *d, long length)
{
long 	i;			
for (i = 0; i < length-1; i++) 
    printf("%f,", i, d[i]);
if (i == length-1)
    printf("%f", i, d[i]);
}	// ---- end PrintFloatLine() ---- 

// *************************************************************** 
// PrintDualAxisFloatsToFile:    
// ***************************************************************
    char 
PrintDualAxisFloatsToFile(float *x, float *y, long length, char *path)
// path			file path string.  If NULL, printed to stderr
{
FILE *h = NULL;

// "" Empty string prints to DSP standard out
if (NULL == path || '\0' == path[0])
	h = stderr;
else
	{
	if (!(h = CreateTextFile(path)))
		return (False);
	}

for (long i = 0; i < length; i++) 
    fprintf(h, "%g, %g\n", x[i], y[i]);
if (h != stderr)
	fclose(h);

return (True);
}	// ---- end PrintDualAxisFloatsToFile() ---- 

// *************************************************************** 
// PrintFloatsToFile:    
// ***************************************************************
    char 
PrintFloatsToFile(float *d, long length, char *path)
{
FILE *h = NULL;

// "" Empty string prints to DSP standard out
if (NULL == path || '\0' == path[0])
	h = stderr;
else
	{
	if (!(h = CreateTextFile(path)))
		return (False);
	}

for (long i = 0; i < length; i++) 
    fprintf(h, "%d, %g\n", i, d[i]);
if (h != stderr)
	fclose(h);

return (True);
}	// ---- end PrintFloatsToFile() ---- 

// *************************************************************** 
// PrintAxisFloatsToFile:    
// ***************************************************************
    char 
PrintAxisFloatsToFile(float *d, long length, float loRange, float hiRange, char *path)
{
FILE *h = NULL;

// "" Empty string prints to DSP standard out
if (NULL == path || '\0' == path[0])
	h = stderr;
else
	{
	if (!(h = CreateTextFile(path)))
		return (False);
	}

float x = 0.0f;
float delta = ( hiRange - loRange)/(float)length;
for (long i = 0; i < length; i++, x += delta) 
	{
	x = ChangeRangef((float)i, 0.0f, (float)(length), loRange, hiRange);
    fprintf(h, "%g, %g\n", x, d[i]);
	}
if (h != stderr)
	fclose(h);

return (True);
}	// ---- end PrintAxisFloatsToFile() ---- 

// *************************************************************** 
// PrintFloatsToFile_ZeroClamp:    Clamp all values in range 
//								[lo .. hi] to zero 
// ***************************************************************
    char 
PrintFloatsToFile_ZeroClamp(float *d, long length, char *path, float lo, float hi)
{
long 	i;			

for (i = 0; i < length; i++)
	{
	if (d[i] < hi && d[i] > lo)
		d[i] = 0.0f;
	}

return (PrintFloatsToFile(d, length, path));
}	// ---- end PrintFloatsToFile_ZeroClamp() ---- 

// *************************************************************** 
// PrintFloatsDB:    
//			Negative values and zero "floored" to -120 dB
// ***************************************************************
    void 
PrintFloatsDB(float *d, long length)
{
long 	i;			
for (i = 0; i < length; i++)
	{	
	float v = d[i];
	if (v <= 0.0f)
		v = -120.0f;
	else
		v = (float) LinearToDecibel(v);
    printf("%d, %g\n", i, v);
	}
}	// ---- end PrintFloatsDB() ---- 

// *************************************************************** 
// PrintAxisFloats:    
// ***************************************************************
    void 
PrintAxisFloats(float *d, double x0, double x1, long length)
{
long 	i;	
double x     = x0;
double delta = (x1 - x0)/(double)(length-1);		
for (i = 0; i < length; i++, x += delta) 
    printf("%d, %g\n", (long) x, d[i]);
}	// ---- end PrintAxisFloats() ---- 

// *************************************************************** 
// PrintAxisFloatsDB:    
//			Negative values and zero "floored" to -120 dB
// ***************************************************************
    void 
PrintAxisFloatsDB(float *d, double x0, double x1, long length)
{
long 	i;	
double x     = x0;
double delta = (x1 - x0)/(double)(length-1);		
for (i = 0; i < length; i++, x += delta) 
	{
 	float v = d[i];
	if (v <= 0.0f)
		v = -120.0f;
	else
		v = (float) LinearToDecibel(v);

	printf("%d, %g\n", (long) x, v);
	}
}	// ---- end PrintAxisFloatsDB() ---- 

// *************************************************************** 
// DecibelFloats:    
//			Negative values and zero "floored" to -120 dB
// ***************************************************************
    void 
DecibelFloats(float *in, float *out, long length)
{
long 	i;	
for (i = 0; i < length; i++) 
	{
	if (in[i] <= 0.0f)
		out[i] = -120.0f;
	else
		out[i] = (float) LinearToDecibel(in[i]);
	}
}	// ---- end DecibelFloats() ---- 

// *************************************************************** 
// DecibelDoubles:    
// ***************************************************************
    void 
DecibelDoubles(double *in, double *out, long length)
{
long 	i;			

for (i = 0; i < length; i++) 
	{
	if (in[i] <= 0.0)
		out[i] = -120.0;
	else
		out[i] = (double) LinearToDecibel(in[i]);
	}
}	// ---- end DecibelDoubles() ---- 

// *************************************************************** 
// ShortsToFloats:	Convert shorts to floats. Saturate option 
//			(in and out ptrs must be different)
// ***************************************************************
    void 
ShortsToFloats(short *in, float *out, long length)
{
long 	i;			
for (i = 0; i < length; i++) 
    out[i] = (float) in[i];
}	// ---- end ShortsToFloats() ---- 

// *************************************************************** 
// FloatsToShorts:	Convert floats to shorts.  
//			Saturate option to 16-bits. 
// ***************************************************************
    void 
FloatsToShorts(float *in, short *out, long length, int saturate)
// saturate		if TRUE, perform saturation 
{
long 	i;			
float	value;
float	ceiling, floor;

ceiling =  k2To15m1i;
floor   = -k2To15i;

// Convert buffer without saturation 
if (!saturate)
    {
    for (i = 0; i < length; i++) 
		out[i] = (short) in[i];
    }
// Convert buffer w/saturation to 16-bit integer range 
else
    {
    for (i = 0; i < length; i++) 
	{
	value = in[i];
	if	(value > ceiling)
	    out[i] =  k2To15m1i;
	else if (value < floor)
	    out[i] = -k2To15i;
	else 
	    out[i] = (short) value;
	}
    }
}	//---- end FloatsToShorts() ---- 

// *************************************************************** 
// Scale:	Scale FLOATs from in buffer to out buffer
//				(ok for "in place" operation )
// ***************************************************************
    void 
Scale(float *in, float *out, long length, float scale)
{
long 	i;			

//#define SCALE_UNROLL
#ifdef SCALE_UNROLL
// Scale buffer 
for (i = (length/8); --i >= 0;)
    { 
    out[0] = in[0]*scale;
    out[1] = in[1]*scale;
    out[2] = in[2]*scale;
    out[3] = in[3]*scale;
    out[4] = in[4]*scale;
    out[5] = in[5]*scale;
    out[6] = in[6]*scale;
    out[7] = in[7]*scale;
    out   += 8;
    in    += 8;
    }
// Scale remnants 
for (i = (length&0x7); --i >= 0;)
    *out++ = (*in++)*scale;
#else
for (i = 0; i < length; i++)
    out[i] = in[i]*scale;
#endif
}	// ---- end Scale() ---- 

// *************************************************************** 
// ScaleShortsf:	Scale 'short' from in buffer to out buffer
//			32-bit floating-point implementation
//				( ok for "in place" operation )
// ***************************************************************
    void 
ScaleShortsf(short *in, short *out, long length, float k)
{
for (long i = 0; i < length; i++)
    {
    long y = (long)(k*(float)in[i]);
    if      (y < kS16_Min)
        y = kS16_Min;
    else if (y > kS16_Max)
        y = kS16_Max;

    out[i] = (short)y;
    }
}	// ---- end ScaleShortsf() ---- 

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

// *************************************************************** 
// ScaleShortsi:	Scale 'short' from in buffer to out buffer
//				Fixed point implementation.  
//				(ok for "in place" operation )
// ***************************************************************
    void 
ScaleShortsi(short *in, short *out, long length, float k)
{
for (long i = 0; i < length; i++)
    out[i] = (short)(k*(float)in[i]);
}	// ---- end ScaleShortsi() ---- 

// *************************************************************** 
// ScaleAddShortsf:	Scale and Add in buffer to out buffer
//			32-bit floating-point implementation
//				( ok for "in place" operation )
//			No Saturation
// ***************************************************************
    void 
ScaleAddShortsf(short *in, short *out, long length, float k)
{
for (long i = 0; i < length; i++)
    out[i] += (short)(k*(float)in[i]);
}	// ---- end ScaleAddShortsf() ---- 

// *************************************************************** 
// ScaleAddShorts:	Add in buffer to out buffer
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
		if 	(acc > k2To15m1i)
			out[i] = k2To15m1i;
		else if (acc < -k2To15i)
			out[i] = k2To15i;
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
// Add2_Shortsi:	Add two buffers of 'short' with saturation
//
//			16-bit fixed-point implementation
// ***************************************************************
    void 
Add2_Shortsi(short *inA, short *inB, short *outY, long length)
{
//{static long c=0; printf("Add2_Shorts%d : start\n", c++);}

for (long i = 0; i < length; i++) 
	{
	long lAcc = ((long) inA[i]) + (long) inB[i];
	SATURATE_16BIT(lAcc);	// Macro doesn't return a value

	outY[i] = (short) lAcc;
	}
}	// ---- end Add2_Shortsi() ---- 

// *************************************************************** 
// Mix2_Shortsf:	Mix two buffers of 'short' with saturation
//
//			32-bit fixed-point implementation
// ***************************************************************
    void 
Mix2_Shortsf(short *inA, short *inB, short *outY, long length, float kA, float kB)
{
{static long c=0; printf("Mix2_Shortsf %d : start\n", c++);}

for (long i = 0; i < length; i++) 
	{
	float acc = kA*((float)inA[i]) + kB*((float) inB[i]);
	long lAcc = (long) acc;
	SATURATE_16BIT(lAcc);	// Macro doesn't return a value

	outY[i] = (short) lAcc;
	}
}	// ---- end Mix2_Shortsf() ---- 

// *************************************************************** 
// Mix2_Shortsi:	Mix two buffers of 'short' with saturation
//
//			16x16=32-bit fixed-point implementation
// ***************************************************************
    void 
Mix2_Shortsi(short *inA, short *inB, short *outY, long length, short kA, short kB)
{
//{static long c=0; printf("Mix2_Shortsi %d : start\n", c++);}

for (long i = 0; i < length; i++) 
	{
	long lAcc = (kA*inA[i]); 
        lAcc     += (kB*inB[i]);
	lAcc = (lAcc>>16);
	SATURATE_16BIT(lAcc);	// Macro doesn't return a value

	outY[i] = (short) (lAcc>>17);
	}
}	// ---- end Mix2_Shortsi() ---- 

// *************************************************************** 
// AccS16toS16:	 Add/Copy 16-bit buffer to 16-bit buffer
//
//			16+16=16-bit fixed-point implementation
// ***************************************************************
    void 
AccS16toS16(S16 *sumY, S16 *inX, long length, long addToOutput)
{
//{static long c=0; printf("AccS16toS16: %d : start addToOutput=%d\n", c++, addToOutput);}

if (addToOutput)
    {
    for (long i = 0; i < length; i++) 
        {
        S32 y = (S32) inX[i];
        y    += (S32) sumY[i];
    	if      (y > kS16_Max) y = kS16_Max;
    	else if (y < kS16_Min) y = kS16_Min;

     	sumY[i] = (S16) y;
        }
    }
else
    {
    for (long i = 0; i < length; i++) 
     	sumY[i] = (S16) inX[i];
    }
}	// ---- end AccS16toS16() ---- 

// *************************************************************** 
// AccS16toS32:	 Add/Copy 16-bit buffer to 32-bit buffer
//
//			16+32=32-bit fixed-point implementation
// ***************************************************************
    void 
AccS16toS32(S32 *sumY, S16 *inX, long length, long addToOutput)
{
//{static long c=0; printf("AccS16toS32: %d : start addToOutput=%d\n", c++, addToOutput);}

if (addToOutput)
    {
    for (long i = 0; i < length; i++) 
     	sumY[i] += (S32) inX[i];
    }
else
    {
    for (long i = 0; i < length; i++) 
     	sumY[i] = (S32) inX[i];
    }
}	// ---- end AccS16toS32() ---- 

// *************************************************************** 
// Pan_Shortsf:	"Pan"  buffer of 'short'
//		yRight  = 
//		32-bit floating-point implementation
// ***************************************************************
    void 
Pan_Shortsf(short *x, short *yLeft, short *yRight, long length, float gainLeft, float gainRight)
{
//{static long c=0; printf("Pan_Shortsf %d : start\n", c++);}

for (long i = 0; i < length; i++) 
	{
// NOTE:  May need saturation to 16-bits
	yLeft [i] = (short)(gainLeft *(float)x[i]);
	yRight[i] = (short)(gainRight*(float)x[i]);
	}
}	// ---- end Pan_Shortsf() ---- 

// *************************************************************** 
// Pan_Shortsi:	"Pan"  buffer of 'short'
//		yRight  = 
//		Fixed-point implementation
// ***************************************************************
    void 
Pan_Shortsi(short *x, short *yLeft, short *yRight, long length, short gainLeft, short gainRight)
{
//{static long c=0; printf("Pan_Shortsi %d : start\n", c++);}

for (long i = 0; i < length; i++) 
	{
	yLeft [i] = (gainLeft *x[i])>>16;
	yRight[i] = (gainRight*x[i])>>16;
	}
}	// ---- end Pan_Shortsi() ---- 
// *************************************************************** 
// Ramp:	Generate buffer that linearly ramps from 'start' to 'end'
// ***************************************************************
    void 
Ramp(float *out, long length, float start, float end)
{
long 	i;			
float delta = (end - start)/(float)length;

for (i = 0; i < length; i++, start += delta)
    out[i] = start;
}	// ---- end Ramp() ---- 

// **********************************************************************
// Envelope:    Compute Envelope of signal
// **********************************************************************
    void 
Envelope(float *in, float *out, long length, 
		 float attack, float release, float loThreshold, float hiThreshold, float *lastX)
{
long  i;
float peakZ = *lastX;
float localRelease;

for (i = 0; i < length; i++)
	{
// --- Measure Peak level: Pass |x| only if >=  peak storage 'peakZ'
	float x = in[i];
	if (x < 0.0f)
		x = 0.0f - x;

	if (x < loThreshold)
		x = 0.0f;
	if (x > hiThreshold)
		{
		localRelease = 0.0f; 
		//x = hiThreshold;
		}
	else
		localRelease = release;

// Update peak storage with attack and release characteristics
	if (x > peakZ)
		peakZ = peakZ - peakZ*localRelease + x*attack;
	else
		peakZ = peakZ - peakZ*localRelease;
	if (peakZ < 0.0f)
		peakZ = 0.0f;
	out[i] = peakZ;
	}
*lastX = peakZ;
}	// ---- end Envelope() ----

// *************************************************************** 
// ScaleFloatsToShorts:	Convert floats to shorts.  
//			Saturate option to 16-bits. 
// Assume input float range is dynamic range of signed 16-bit	
// ***************************************************************
    void 
ScaleFloatsToShorts(float *input, short *output, long length, float g, int saturate)
// saturate		if True, perform saturation 
{
long 	i;			

// Convert buffer without saturation 
if (!saturate)
    {
    for (i = 0; i < length; i++) 
		output[i] = (short) (g*input[i]);
    }
// Convert buffer w/saturation to 16-bit integer range 
else
    {
	float	value;
	float	ceiling =  k2To15m1;
	float 	floor   = -k2To15;
    for (i = 0; i < length; i++) 
		{
		value = g*input[i];
		if		(value > ceiling)
		    output[i] =  k2To15m1i;
		else if (value < floor)
		    output[i] = -k2To15i;
		else 
		    output[i] = (short) value;
		}
    }
}	//---- end ScaleFloatsToShorts() ---- 

// *************************************************************** 
// ScaleFloatsToLongs:	Convert floats to longs.  
//			Saturate option to 32-bits. 
// Assume input float range is that of signed 16-bit integer
// ***************************************************************
    void 
ScaleFloatsToLongs(float *input, long *output, long len, float g, int saturate)
// saturate		if True, perform saturation 
{
long 	i;			

// Convert buffer without saturation 
if (!saturate)
    {
    for (i = 0; i < len; i++) 
		output[i] = (long) (g*input[i]);
   }
// Convert buffer w/saturation to signed 16-bit integer range  
else
    {
	float	ceiling =  k2To15m1;
	float 	floor   = -k2To15;
    for (i = 0; i < len; i++) 
		{
		float value = g*input[i];
		if		(value > ceiling)
		    output[i] =  k2To15m1i;
		else if (value < floor)
		    output[i] = -k2To15i;
		else 
		    output[i] = (long) value;
		}
    }
}	//---- end ScaleFloatsToLongs() ---- 

// *************************************************************** 
// CopyFloats:		Copy buffer of 32-bit floating point numbers
// ***************************************************************
	void 
CopyFloats(float *inP, float *outP, long length)
{
for (long i = 0; i < length; i++)
	outP[i] = inP[i];
}	// ---- end CopyFloats() ---- 

// *************************************************************** 
// ScaleFloats:		scale floating point buffer
// ***************************************************************
	void 
ScaleFloats(float *inP, float *outP, long length, float k)
{
for (long i = 0; i < length; i++)
	outP[i] = inP[i]*k;
}	// ---- end ScaleFloats() ---- 

// *************************************************************** 
// ScaleShorts:		scale buffer of 'short'
// ***************************************************************
	void 
ScaleShorts(short *inP, short *outP, long length, float k)
{
for (long i = 0; i < length; i++)
	outP[i] = (short)(k*(float)inP[i]);
}	// ---- end ScaleShorts() ---- 


// *************************************************************** 
// ScaleAdd:		Scale and add input to output
//			OK for "in place" operation
// ***************************************************************
    void 
ScaleAdd(float *in, float *out, long length, float gain)
{
for (long i = 0; i < length; i++) 
	out[i] += in[i]*gain;
}	// ---- end ScaleAdd() ---- 

// *************************************************************** 
// ScaleRamp:		Scale with Ramp
//			ISO9000 certified "in place" operation
// ***************************************************************
    void 
ScaleRamp(float *in, float *out, long length, float gain, float delta)
{
if (delta != 0.0f)
    {
    for (long i = 0; i < length; i++) 
		{
		float tmp = in[i]*gain;
		gain     += delta;
		out[i]    = tmp;
		}    
    }
else
    Scale(in, out, length, gain);
}	// ---- end ScaleRamp() ---- 

// *************************************************************** 
// Mask:	Mask FLOATs from in buffer to out buffer
//			ISO9000 certified "in place" operation
// ***************************************************************
    void 
Mask(float *in, float *out, long length, long mask)
{
long 	i;			

for (i = 0; i < length; i++)
	out[i] = (float)(mask & (long) in[i]);
}	// ---- end Mask() ---- 

// *************************************************************** 
// Gate:		Return # samples gated
// ***************************************************************
    long 
Gate(float *in, float *out, long length, float floor, float ceiling)
{
long    i;
long	k = 0;
for (i = 0; i < length; i++) 
	{
	if (in[i] >= floor && in[i] <= ceiling)
		{
		out[i] = 0.0f;
		k++;
		}
	}
return (k);
}	// ---- end Gate() ---- 

// *************************************************************** 
// Clamp:		Return # samples clamped to specified value
// ***************************************************************
    long 
Clamp(float *in, float *out, long length, float floor, float ceiling, float x)
{
long    i;
long	k = 0;
for (i = 0; i < length; i++) 
	{
	if (in[i] >= floor && in[i] <= ceiling)
		{
		out[i] = x;
		k++;
		}
	else 
		out[i] = in[i];
	}
return (k);
}	// ---- end Clamp() ---- 

// *************************************************************** 
// GateDB:		Return # samples clamped to specified value
// ***************************************************************
    long 
GateDB(float *in, float *out, long length, float levelDB, float kDB)
{
long    i;
long	count = 0;
float	level = (float) DecibelToLinear(levelDB);
float	k     = (float) DecibelToLinear(kDB);

for (i = 0; i < length; i++) 
	{
	float x = (float) fabs(in[i]);

	if (x < level)
		{
		out[i] = k;
		count++;
		}
	else 
		out[i] = in[i];
	}
return (count);
}	// ---- end GateDB() ---- 

// *************************************************************** 
// Bound:		Return # samples bound to range [floor .. ceiling]
// ***************************************************************
    long 
Bound(float *in, float *out, long length, float floor, float ceiling)
{
long    i;
long	k = 0;
for (i = 0; i < length; i++) 
	{
	if (in[i] >= floor && in[i] <= ceiling)
		{
		out[i] = 0.0f;
		k++;
		}
	}
return (k);
}	// ---- end Bound() ---- 

// *************************************************************** 
// Add:		Add two buffers:  out = a + b
// ***************************************************************
    void 
Add(float *a, float *b, float *out, long length)
{
for (long i = 0; i < length; i++) 
	out[i] = a[i] + b[i];
}	// ---- end Add() ---- 

// *************************************************************** 
// Subtract:		Subtract two buffers:	out = a - b
// ***************************************************************
    void 
Subtract(float *a, float *b, float *out, long length)
{
for (long i = 0; i < length; i++) 
	out[i] = a[i] - b[i];
}	// ---- end Subtract() ---- 

// *************************************************************** 
// Mix2Floats:		Mix two buffers:  out = a*kA + b*kB
// ***************************************************************
    void 
Mix2Floats(float *inA, float *inB, float *out, long length, float gainA, float gainB)
{
for (long i = 0; i < length; i++) 
	out[i] = inA[i]*gainA + inB[i]*gainB;
}	// ---- end Mix2Floats() ---- 

// *************************************************************** 
// MixNFloats:		Mix N buffers
// ***************************************************************
    void 
MixNFloats(float **ins, float *out, long length, int count, float *gains)
{
long i, j;
for (i = 0; i < length; i++) 
	{
	float sum = 0.0f;
	for (j = 0; j < count; j++)
		sum += ins[j][i]*gains[j];
	out[i] = sum;
	}
}	// ---- end MixNFloats() ---- 

// **************************************************************
// DeinterleaveShortsToFloats: Deinterleave 2 channel buffer 
//				
//			First word of input buffer at 'input' will 
//			be first word in output buffer 'outputLeft.'
// **************************************************************
    void 
DeinterleaveShortsToFloats(short *in, float *outL, float *outR, long outLength)
{
long	i;

// Handle deinterleaves of paired samples from input buffer  
for (i = (outLength/4); --i >= 0;) 
    {
// NOTE w/type conversion (thus most likely good instruction
//	scheduling, dunno if memory optimization pays off 
      outL[0] = (float) in[0];
      outR[0] = (float) in[1];
      outL[1] = (float) in[2];
      outR[1] = (float) in[3];
      outL[2] = (float) in[4];
      outR[2] = (float) in[5];
      outL[3] = (float) in[6];
      outR[3] = (float) in[7];
      outL   += 4;
      outR   += 4;
      in     += 8;
    }

// Could be 1-3 sample pairs left over. Not pretty.  
for (i = (outLength&0x3); --i >= 0; in += 2)
    {
    *outL++ = (float) in[0];
    *outR++ = (float) in[1];  
    }
}	// ---- end DeinterleaveShortsToFloats() ---- 

// **************************************************************
// DeinterleaveShorts: Deinterleave stereo buffer
//				
//			First word of input buffer at 'input' will 
//			be first word in output buffer 'outputLeft.'
// **************************************************************
    void 
DeinterleaveShorts(short *in, short *outL, short *outR, long outLength)
// in	    ptr to input buffer
// outL	    ptr to left  output buffer 
// outR	    ptr to right output buffer
// outLength length of output buffer 
{
long	i;
// Handle deinterleaves of paired samples from input buffer  
for (i = (outLength/4); --i >= 0;) 
    {
      outL[0] = in[0];
      outR[0] = in[1];
      outL[1] = in[2];
      outR[1] = in[3];
      outL[2] = in[4];
      outR[2] = in[5];
      outL[3] = in[6];
      outR[3] = in[7];
      outL   += 4;
      outR   += 4;
      in     += 8;
    } 

// Could be 1-3 sample pairs left over. Not pretty.  
for (i = (outLength&0x3); --i >= 0; in += 2)
    {
    *outL++ = in[0];
    *outR++ = in[1];  
    }
}	// ---- end DeinterleaveShorts() ---- 
// **************************************************************
// DeinterleaveNShortsToFloats:    Deinterleave to N buffers
//				
//			The first word of input buffer at 'input' 
//			will be first word in output buffer 'outputLeft.'
// ***************************************************************
    void 
DeinterleaveNShortsToFloats(short *in, float *outs[], long outLength,
				int interleave)
// input		ptr to input buffer
//  outs		ptr to array of output buffers
// outLength		length of output buffer 
//  interleave	# output buffers (interleave factor)
{
long	i, j;

// Deinterleave of N-sample frames  
for (i = 0; i < outLength; i++) 
    {
    for (j = 0; j < interleave; j++) 
	outs[j][i] = (float) in[j];
    in += interleave;
    }
}	// ---- end DeinterleaveNShortsToFloats() ---- 

// **************************************************************
// DeinterleaveNShorts:    Deinterleave to N buffers
//				
//			The first word of input buffer at 'input' 
//			will be first word in output buffer 'outputLeft.'
// ***************************************************************
    void 
DeinterleaveNShorts(short *in, short *outs[], long outLength, int interleave)
// input		ptr to input buffer
//  outs		ptr to array of output buffers
// outLength		length of output buffer 
//  interleave	# output buffers (interleave factor)
{
long	i, j;

// Deinterleave of N-sample frames  
for (i = 0; i < outLength; i++) 
    {
    for (j = 0; j < interleave; j++) 
		outs[j][i] = in[j];
    in += interleave;
    }
}	// ---- end DeinterleaveNShorts() ---- 

// ***************************************************************
// InterleaveFloatsToShorts:	
//		Interleave data word streams of buffers
//			'inputLeft' and 'inputRight' to output buffer
//			'output'. Option to saturate to 16-bits.
//				
//			Input buffers are assumed to be of equal length
//			and data type.
//			Output buffer is assumed to be twice the length
//			of input buffer.  The first word of input buffer
//			at 'inputLeft' will be first word in output buffer
//			'output.'
// ****************************************************************
    void 
InterleaveFloatsToShorts(float *inL, float *inR, short *out, 
			    long inLength, int saturate, int stride)
/* inL		ptr to left input buffer
   inR		ptr to right input buffer
   outs		ptr to output buffer
   inLength	length of input buffer 
   saturate	if TRUE, perform saturation 
*/
{
long i;
int step = 2*stride;

// Rounding = truncation + 1/2 bit DC offset -->> just truncate 
// Convert buffer w/saturation to integer range [iFloor..iCeiling] 
if (saturate)
    {
    float value;
    float ceiling =  k2To15m1;
    float floor   = -k2To15;

    for (i = 0; i < inLength; i++, out += step) 
	{
    // Left channel 
	value = inL[i];
	if	(value > ceiling)
	    out[0] =  k2To15m1i;
	else if (value < floor)
	    out[0] = -k2To15i;
	else 
	    out[0] = (short) value;

    // Right channel 
	value = inR[i];
	if	(value > ceiling)
	    out[1] =  k2To15m1i;
	else if (value < floor)
	    out[1] = -k2To15i;
	else 
	    out[1] = (short) value;
	}
    }
// Convert buffer without saturation 
else 
    {
    for (i = 0; i < inLength; i++, out += step) 
	{
	out[0] = (short) inL[i];
	out[1] = (short) inR[i];
	}
    }
}	// ---- end InterleaveFloatsToShorts() ----

// ***************************************************************
// InterleaveNFloatsToShorts: Interleave N buffers
//
//			Option to saturate to 16-bits.
//				
//			Input buffers are assumed to be of equal length
//			and data type.
//			Output buffer is N*length of input buffer.  First 
//			word of input buffer[0] will be first word of 
//			output buffer
// ***************************************************************
    void 
InterleaveNFloatsToShorts(float *ins[], short *out, long inLength, 
				int interleave, int saturate)
// ins		ptr to array of input buffers
// out		ptr to output buffer
// inLength	length of input buffer 
// interleave	# input buffers 
// saturate	if TRUE, do saturation 
{
long	    i, j;

// Rounding = truncation + 1/2 bit DC offset -->> just truncate 

// Convert buffer, no saturation 
if (!saturate)
    {
    for (i = 0; i < inLength; i++, out += interleave) 
	{
	for (j = 0; j < interleave; j++) 
	    out[j] = (short) ins[j][i];
	}
    }
// Convert buffer w/saturation to integer range [iFloor..iCeiling] 
else 
    {
    float fCeiling =  k2To15m1;
    float fFloor   = -k2To15;
    short iCeiling =  k2To15m1i;	//  2^15 - 1 
    short iFloor   = -k2To15i;	// -2^15 

    for (i = 0; i < inLength; i++, out += interleave) 
	{
	for (j = 0; j < interleave; j++) 
	    {
	    float fValue = ins[j][i];
	    if	    (fValue > fCeiling)
		out[j] = iCeiling;
	    else if (fValue < fFloor)
		out[j] = iFloor;
	    else 
		out[j] = (short) fValue;
	    }
	}
    }
}	// ---- end InterleaveNFloatsToShorts() ---- 

// ***************************************************************
// InterleaveNShorts: Interleave N buffers
//				
//			Input buffers are assumed to be of equal length
//			and data type.
//			Output buffer is N*length of input buffer.  First 
//			word of input buffer[0] will be first word of 
//			output buffer
// ***************************************************************
    void 
InterleaveNShorts(short *ins[], short *out, long inLength, int interleave)
// ins		ptr to array of input buffers
// out		ptr to output buffer
// inLength	length of input buffer 
// interleave	# input buffers 
{
long	 i, j;

for (i = 0; i < inLength; i++, out += interleave) 
	{
	for (j = 0; j < interleave; j++) 
	    out[j] = ins[j][i];
	}
}	// ---- end InterleaveNShorts() ---- 

// ***************************************************************
// InterleaveShorts:	
// ***************************************************************
	void 
InterleaveShorts(short *inL, short *inR, short *out, long inLength)
{
long i;
for (i = 0; i < inLength; i++, out += 2) 
	{
	out[0] = inL[i];
	out[1] = inR[i];
	}
}	// ---- end InterleaveShorts() ----

// **********************************************************************************
// DotFloats:		 Take dot product of two arrays of floats
// ********************************************************************************** 
	void 
DotFloats(float *a, float *b, float *out, long length)
{
long	i;
for (i = 0; i < length; i++)	
	out[i] = a[i] * b[i];
}	// ---- end DotFloats() ---- 

// **********************************************************************************
// LogN:		Return LogN(x)
// ********************************************************************************** 
	double 
LogN(double n, double x)
{
// logB (a) = logX(a)/logX(b)
// logN (x) = log10(x)/log10(N)
// log2 (x) = log10(x)/log10(2) = log10(x)*log2(10)
// log10(x) = log2(x)/log2(10)  = log2(x)*log10(2)
	
return (log10(x)/log10(n));
}			    // ---- end LogN() ---- 

// *************************************************************************
// PrepareSineOscillator:  Compute coefficients for sinusoid generator
// ************************************************************************* 
	void 
PrepareSineOscillator(double *h,
					double *z,
					double  amplitude,
					double  frequency,
					double  phase)
// h				coefficient array
// z 				delay element array
// amplitude
// frequency		frequency normalized to sampling rate, 0 <= f < 0.5
// phase			in Radians
{
// amplitude  = 1 for definitive sinusoid
// 
// For sine    wave: phase  =  90*(Pi/180) = Pi/2
// For cosine  wave: phase  = 180*(Pi/180) = Pi
// For -cosine wave: phase  = 180*( 0/180) = 0
double tuning = cos(k2Pi*frequency);
h[0] = tuning;

// Initialize delay elements, if supplied
if (z)
    {
    double amplitudeH = sqrt((1.0 - tuning)/(1.0 + tuning));

    z[0] =  amplitude*sin(phase)*amplitudeH;
    z[1] = -amplitude*cos(phase);
    }   
}	// ---- end PrepareSineOscillator() ---- 

// **********************************************************************
// ComputeSineOscillator:   Compute oscillating IIR filter
//
//  Use ComputeWaveGuideHz() to setup
// ********************************************************************** 
    void 
ComputeSineOscillator(float *out, long length, double *z, double *h)
// z		ptr to delay elements
// h		ptr to coefficients
{
long	i;
double	sum1, sum2;

double z1   = z[0];
double z2   = z[1];
double tune = h[0];

// Compute transformer normalized waveguide filter 
//  Use ComputeWaveGuideHz() to setup
for (i = 0; i < length; i++)
	{
	out[i] = (float) z2;
	sum2   = (z1  + z2)*tune;
	sum1   = sum2 - z2;
	z2     = z1   + sum2;
	z1     = sum1;
	}

// Save delay state for next iteration
z[0] = z1;
z[1] = z2;
}	// ---- end ComputeSineOscillator() ---- 

// **********************************************************************
// SetUpSampleNHoldOscillator:    
// **********************************************************************
	void  
SetUpSampleNHoldOscillator(unsigned long *z, unsigned long *counter, 
						   unsigned long *delta, float normalPeriod, float phase)
// z		ptr to last state
// delta	counter increment
// normalPeriod	samplingFrequency/frequency
{
*z       = mDoubleToULong(phase);
*delta   = (unsigned long) normalPeriod;
*counter = *delta;
}	// ---- end SetUpSampleNHoldOscillator() ----

// **********************************************************************
// SetUpSawtoothOscillator:    
// **********************************************************************
	void 
SetUpSawtoothOscillator(unsigned long *z, unsigned long *delta, float normalFrequency, float phase)
// z		ptr to last state
// delta	counter increment
// normalFrequency	hertz*samplingPeriod
{
*delta = mFloatToULong(normalFrequency);
*z     = mFloatToULong(phase);
}	// ---- end SetUpSawtoothOscillator() ----

// **********************************************************************
// SampleNHoldOscillator:    Generate band-unlimited sample-n-hold
//
//					Return value in range:	[-1 ..1]
// **********************************************************************
    void 
SampleNHoldOscillator(float *out, long length, unsigned long *z, 
			unsigned long *counter, unsigned long delta)
// z		ptr to last state
// delta	counter increment
{
long  i;
unsigned long z0;

for (i = 0; i < length; i++)
	{
	if (*counter == 0)
		{
		z0 = 1103515245*z0 + 12345;
		*counter = delta;
		}
	else
		{
		z0 = *z;
		*counter += 1;
		}

// Convert range [0..2^32 - 1] to range range [-1..1] 
	out[i] = NormalTwoTo31m1f((long)z0);
	}
*z = z0;
}	// ---- end SampleNHoldOscillator() ----

// **********************************************************************
// SawtoothOscillator:    Generate band-unlimited sawtooth wave
//
//					Return value in range:	[-1 ..1]
// **********************************************************************
    void 
SawtoothOscillator(float *out, long length, unsigned long *z, unsigned long delta)
// z		ptr to last state
// delta	counter increment
{
long  i;
unsigned long z0;

for (i = 0; i < length; i++)
	{
	z0 = *z;
	*z = z0 + delta;

// Convert to range [-1..1] 
	out[i] = NORMALTwoTo31m1f((long)z0);
	}
}	// ---- end SawtoothOscillator() ----

// **********************************************************************
// SawtoothOscillator_S16:    Generate band-unlimited sawtooth wave
//
//					Return value in range:	[-32768 .. 32767]
// **********************************************************************
    void 
SawtoothOscillator_S16(short *outP, long length, unsigned long *z, unsigned long delta)
// z		ptr to last state
// delta	counter increment
{
for (long i = 0; i < length; i++)
	{
	unsigned long z0 = *z;
	*z = z0 + delta;

// Convert to range [-32768..32767] 
	outP[i] = (short) (((long)z0)>>1);
	}
}	// ---- end SawtoothOscillator_S16() ----

// **********************************************************************
// SetUpSquareOscillator:    
// **********************************************************************
	void 
SetUpSquareOscillator(unsigned long *z, unsigned long *delta, float normalFrequency, float phase)
// z		ptr to last state
// delta	counter increment
// normalFrequency	hertz*d->samplingPeriod
{
*delta = mDoubleToULong(normalFrequency);
*z     = mDoubleToULong(phase);
}	// ---- end SetUpSquareOscillator() ----

// **********************************************************************
// SquareOscillator:    Generate band-unlimited square wave
//
//					Return value in range:	[-1 ..1]
// **********************************************************************
    void 
SquareOscillator(float *out, long length, unsigned long *z, unsigned long delta)
// z		ptr to last state
// delta	counter increment
{
long  i;

for (i = 0; i < length; i++)
	{
	unsigned long z0 = *z;
	*z = z0 + delta;

// If > 2^31, output 0
	if (z0 < k2To31m1i)
		out[i] =  1.0f;
	else
		out[i] = -1.0f;
	}
}	// ---- end SquareOscillator() ----

// **********************************************************************
// SetUpTriangleOscillator    
// **********************************************************************
	void 
SetUpTriangleOscillator(unsigned long *z, unsigned long *delta, float normalFrequency, float phase)
// z		ptr to last state
// delta	counter increment
// normalFrequency	hertz*d->samplingPeriod
{
*delta = mDoubleToULong(normalFrequency);
*z     = mDoubleToULong(phase);
}	// ---- end SetUpTriangleOscillator() ----

// **********************************************************************
// TriangleOscillator    Generate band-unlimited triangle cycle
//
//					This routine optimized for low frequency operation.
//					Due to 30-bit arithmetic problems with
//					triangle waveform extrema is not suitable for high 
//					frequency operation
//
//					Return value in range:	[-1 ..1]
// **********************************************************************
    void 
TriangleOscillator(float *out, long length, unsigned long *z, unsigned long delta)
// z		ptr to last state
// delta	counter increment
{
long  i;
unsigned long z0;
long  tmp;

for (i = 0; i < length; i++)
	{
	z0 = *z;
	*z = z0 + delta;

// Mirror down to 2^31 range
	if (z0 > k2To31m1i)
	 	z0 = k2To31m1i - z0 + k2To31m1i;
	tmp = z0 - (unsigned long) k2To30m1i;

// Convert to to range [-1..1] 
	out[i] = NORMALTwoTo30m1f(tmp);
	}
}	// ---- end TriangleOscillator() ----

// **********************************************************************
// BlastSineOscillator:   most concise setup routine to init and compute 
//							440 Hz sine wave sine
// ********************************************************************** 
    void 
BlastSineOscillator(float *out, long length, double *z, double *h, float gain, int *init)
{
if (!*init)
	{	
	PrepareSineOscillator(h, z, k2To15m1/4.0, 440.0/44100.0, kPi/2.0);
	*init=True;
	}
ComputeSineOscillator(out, length, z, h);
}	// ---- end PrepareSineOscillator() ---- 

// **********************************************************************
// Sawtooth    Generate band-unlimited sawtooth wave
//		    	Generates one cycle
// ********************************************************************** 
    void 
Sawtooth(float *out, long length, float floor, float ceiling)
{
long	    i;
float	arg, argInc;

arg    = ceiling;
argInc = (ceiling - floor)/((float) length);
for (i = 0; i < length; i++, arg -= argInc)
    out[i] = arg;
}	// ---- end Sawtooth() ----

// **********************************************************************
// Sine    Generate sine wave
//		    	Generates one cycle
// ********************************************************************** 
    void 
Sine(float *out, long length, double amplitude, double phase)
{
long   i;
double arg    = phase;
double argInc = kTwoPi/(double) length;

for (i = 0; i < length; i++, arg += argInc)
    out[i] = (float)(amplitude*sin(arg));
}	// ---- end Sine() ----

// **********************************************************************
// Triangle    Generate band-unlimited triangle cycle
//		    	Generates one cycle
// **********************************************************************
    void 
Triangle(float *out, long length, float floor, float ceiling)
{
long		i;
float	arg, argInc;

// Do positive slope portion 
arg    = floor;
argInc = 2*(ceiling-floor)/((float) length);
for (i = 0; i < length && arg <= ceiling; i++)
    {
    out[i] = arg;
    arg   += argInc;
    }

// Do negative slope portion 
if (arg > ceiling)
    arg = ceiling - (arg - ceiling);
for (; i < length; i++)
    {
    out[i] = arg;
    arg   -= argInc;
    }
}	// ---- end Triangle() ----

// **********************************************************************
// Pulse:	Generate band-unlimited pulse cycle
//		    Currently generates one cycle
// **********************************************************************
    void 
Pulse(float *out, long length, float floor, float ceiling, float duty)
{
long	i = 0;
long	dutyEnd = (long)(duty*(float)(length-1));

// Duty cycle is percentage of wavelength signal positive. 
// Duty cycle controls partial content.  A pulse function in the
// time domain transforms into a sin(x)/x or sinc(x) function in the
// frequency domain.  Value of sinc function over the frequency axis
// specifies the amplitude of each partial.  
//  
// A square wave (duty cycle 50%) is member of the pulse wave family.  Just 
// so happens that the frequency domain sinc function for the square wave 
// is zero at every even partial.  Thus, a square wave contains odd only
// partials whose amplitudes are 1/partial #.  
//  
// A pulse wave w/a 33% duty cycle is missing every third harmonic.
// A pulse wave w/a 25% duty cycle is missing every fourth harmonic.
// A pulse wave w/a 20% duty cycle is missing every fifth harmonic.
// And so on, and so on.   However, harmonics don't have a 
// simple 1/harmonic # amplitude relationship.  Need to examine the sinc 
// function.  Very small and very large duty cycles have a nasal quality.
// A large duty cycle D is a inverted wave of small duty cycle 100-D
//  
// A traditional method for producing ballsy sounds from a single oscillator
// is to modulate the duty cycle.  This can be done by computing many
// pulse waves w/increasing duty cycles and cycling through this list of
// waves in a forward&backward manner.  THIS IS TERMED PULSE WIDTH MODULATION.
// The more samples are in your wave, the thinner your thinnest pulse can be.
//   
// The modulation rate is determined by the rate at which the pulse widths change.
// This is determined by the rate at which the pulse wave set is stepped 
// thru and # of different pulse waves in set.
//

// Do on portion 
for (i = 0; i <= dutyEnd; i++)
    out[i] = ceiling;	

// Do off portion 
for (; i < length; i++)
    out[i] = floor;
}	// ---- end Pulse() ---- 

// **********************************************************************
// OtherNoise    Generate a pseudo-random number sequence 
//					Period = 2^32 values
// ********************************************************************** 
    void 
OtherNoise(float *out, long length)
{
long	    i;

unsigned long z0 = 59;

// Run linear congruator 
// Convert range [0..2^32 - 1] to range range [-1..1] 
// Numerical Recipes in C:   x = x0*1664525 + 1013904223
for (i = 0; i < length; i++)
	{
	z0     = 5609937*z0 + 1;
	out[i] = NORMALTwoTo31m1f((long)z0);
	}
}	// ---- end OtherNoise() ----

// **********************************************************************
// NoiseWhite    Generate a pseudo-random number sequence with uniform pdf
//					Period = 2^32 values
// ********************************************************************** 
    void 
NoiseWhite(float *out, long length, unsigned long *z, float gain, char additive)
{
long	    i;
unsigned long z0 = *z;

// Run linear congruator 
// Convert range [0..2^32 - 1] to range range [-1..1] 
if (additive)
	{
	for (i = 0; i < length; i++)
		{
		z0      = 1103515245*z0 + 12345;
		out[i] += gain*NORMALTwoTo31m1f((long)z0);
		}
	}
else
	{
	for (i = 0; i < length; i++)
		{
		z0     = 1103515245*z0 + 12345;
		out[i] = gain*NORMALTwoTo31m1f((long)z0);
		}
	}
*z = z0;
}	// ---- end NoiseWhite() ----

// **********************************************************************
// NoiseRed    Generate a pseudo-random number sequence with 1/f^2 pdf
//
//				Note: a glitch is generated unless 'length' is even
// ********************************************************************** 
    void 
NoiseRed(float *out, long length, unsigned long *z, float gain, char additive)
{
long i;
long  sum;
unsigned long z0 = *z;

// Red Noise = low pass-filtered white noise 
// y[n] = (x[n] + x[n-1])/2   
if (additive)
	{
	for (i = 0; i < length; i++)
		{
		sum = ((long) z0)>>1;
	// Run linear congruator. 
		z0  = 1103515245*z0 + 12345;

	// Note: avoid 32-bit overflow: *must* pre-shift
		sum += ((long) z0)>>1;

	// Convert range [-2^31..+2^31] to range [-1..1] 
		out[i] += gain*NORMALTwoTo31m1f(sum);
		}
	}
else
	{
	for (i = 0; i < length; i++)
		{
		sum    = ((long) z0)>>1;
		z0     = 1103515245*z0 + 12345;
		sum   += ((long) z0)>>1;
		out[i] = gain*NORMALTwoTo31m1f(sum);
		}
	}
*z = z0;
}	// ---- end NoiseRed() ----

// **********************************************************************
// NoiseViolet    Generate a pseudo-random number sequence with f^2 pdf
//
//				Note: a glitch is generated unless 'length' is even
// ********************************************************************** 
    void 
NoiseViolet(float *out, long length, unsigned long *z, float gain, char additive)
{
long i;
long  sum;
unsigned long z0 = *z;

// Violet Noise = high pass-filtered white noise 
// y[n] = (x[n] - x[n-1])/2   
if (additive)
	{
	for (i = 0; i < length; i++)
		{
		sum = ((long) z0)>>1;
	// Run linear congruator.   
		z0  = 1103515245*z0 + 12345;

	// Note: avoid 32-bit overflow: *must* pre-shift
		sum -= ((long) z0)>>1;

	// Convert range [-2^31..+2^31] to range [-1..1] 
		out[i] += gain*NORMALTwoTo31m1f(sum);
		}
	}
else
	{
	for (i = 0; i < length; i++)
		{
		sum = ((long) z0)>>1;
	// Run linear congruator.   
		z0  = 1103515245*z0 + 12345;

	// Note: avoid 32-bit overflow: *must* pre-shift
		sum -= ((long) z0)>>1;

	// Convert range [-2^31..+2^31] to range [-1..1] 
		out[i] = gain*NORMALTwoTo31m1f(sum);
		}
	}
*z = z0;
}	// ---- end NoiseViolet() ----

// ******************************************************************
// DurationInSeconds:	Convert to format hours:minutes:seconds
// ****************************************************************** 
    void 
DurationInSeconds(double seconds, char *out, char printFormat)
/*
   seconds - time value
   out - assumed long enough
   printFormat - SF_SHORT or !SF_SHORT.  Short format uses time labels such
    as hrs, min, sec
*/
{
int	hours, minutes, wholeSeconds;
double	subSeconds, milliSeconds, microSeconds, nanoSeconds;
char	s2[50];
char    isShort = (printFormat== kPrintFormat_Short);

if (seconds < 0)
    {
    strcpy(out, isShort ? "?sec" : "? time");
    return;
    }
if (0 == seconds)
    {
    strcpy(out, isShort ? "0 sec" : "0 seconds");
    return;
    }

// compute hours, minutes and seconds 
wholeSeconds  = (int) seconds;
minutes       = (wholeSeconds/60)%60;
hours         = wholeSeconds/3600;
subSeconds    = seconds - ((double) wholeSeconds);
wholeSeconds %= 60;

// duration less than 1 hour  
if (hours < 1)
	{
// duration less than 1 minute  
    if (minutes < 1)
	{
// duration less than 0.1 second
//This way, 900 milliseconds are printed as .900 seconds 
// subsecond units not printed for SF_SHORT format 
	if (seconds < 0.1 && !isShort)
	    {
	      milliSeconds = subSeconds*1000;
		if (milliSeconds < 0.1)
		    {
		    microSeconds = milliSeconds*1000;

	    // print in nano seconds 
		    if (microSeconds < 0.1)
			{
			nanoSeconds = microSeconds*1000;

			if (printFormat == kPrintFormat_Normal)
			    sprintf(out, "%.4g nSec", nanoSeconds);
			else
			    {
			    sprintf(out, "%.4g nSecond", nanoSeconds);
		    // pluralize units *rounded* to integer 
			    if ((int)(nanoSeconds+0.001) != 1)
				strcat(out, "s");
			    }
			}
	     // print in micro seconds 
		    else
			{
			if (printFormat == kPrintFormat_Normal)
			    sprintf(out, "%.4g uSec", microSeconds);
			else
			    {
			    sprintf(out, "%.4g uSecond", microSeconds);
			/* pluralize units *rounded* to integer */
			    if ((int)(microSeconds+0.001) != 1)
				strcat(out, "s");
			    }
			}
		    }
	     // print in milli seconds 
		else
		    {
		    if (printFormat == kPrintFormat_Normal)
			sprintf(out, "%.4g mSec", milliSeconds);
		    else
			{
			sprintf(out, "%.4g mSecond", milliSeconds);
		// pluralize units *rounded* to integer 
			if ((int) (milliSeconds+0.001) != 1)
			    strcat(out, "s");
			}
		    }
	    }
// SF_SHORT=printFormat duration greater than 1 second or , Use Minute:Second format 
	else
	    {
	    if (isShort)
		sprintf(out, "%g sec", seconds);
	    else
		{
		if (printFormat == kPrintFormat_Normal)
		    sprintf(out, "%.4g Sec", seconds);
		else
		    {
		    sprintf(out, "%.4g Second", seconds);
	// pluralize units *rounded* to integer 
		    if ((int)(seconds+0.001) != 1)
			strcat(out, "s");
		    }
		}
	    }
	}
// duration >= 1 minute.  Use Minute:Second format 
    else
	{
	sprintf(out, "%d:%.2d", minutes, wholeSeconds);
	if (subSeconds != 0)
	    {
	    sprintf(s2, ".%.3g", subSeconds);
	    strcat(out, s2+2);	// offset avoids leading "0."  
	    }
	if		(printFormat == kPrintFormat_Short)
	    strcat(out, " min");
	else if (printFormat == kPrintFormat_Normal)
	    strcat(out, " Min");
	else
	    {
	    strcat(out, " Minute");
	// pluralize units to integer 
	    if (minutes == 1 && wholeSeconds == 0 && subSeconds == 0)
		strcat(out, "s");
	    }
	}
    }

// duration >= 1 hour.  Use Hour:Minute:Second format 
else
    {
    sprintf(out, "%d:%.2d:%.2d", hours, minutes, wholeSeconds);
    if (subSeconds != 0)
	{
	sprintf(s2, ".%.3g", subSeconds);
	strcat(out, s2+2);	// offset avoids leading "0."  
	}
    strcat(out, isShort ? " hrs" : " Hour");

// pluralize units *
    if (!isShort && hours == 1 && minutes == 0 && wholeSeconds == 0) 
	strcat(out, "s");
    }
} // ---- end DurationInSeconds() ---- 

// ******************************************************************
// SizeInBytes:	    Convert to bytes with metric system prefixes
// ****************************************************************** 
    void 
SizeInBytes(long totalBytes, char *out, char printFormat)
{
char	s[50];
double	bytes;

if (totalBytes < 0)
    {
    sprintf(out, "? Bytes");
    return;
    }

// print size with 3 significant digits in range 
bytes = (double) totalBytes;
// units in Giga bytes 
if	(totalBytes > 1024*1024*1024)
    {
    bytes /= 1024*1024*1024;
    s[0]   = 'G'; s[1] = '\0';
    }
// units in Mega bytes 
else if (totalBytes > 1024*1024)
    {
    bytes /= 1024*1024;
    s[0]   = 'M'; s[1] = '\0';
    }
// units in Kilo bytes 
else if (totalBytes > 1024)
    {
    bytes /= 1024;
    s[0]   = 'K'; s[1] = '\0';
    }
// units in bytes 
else 
    s[0] = '\0';

if	(printFormat == kPrintFormat_Short)
    sprintf(out, "%.2f %sByte", bytes, s);
else 
    sprintf(out, "%.3g %sByte", bytes, s);

/* pluralize bytes */
if (bytes != 1)
    strcat(out, "s");
} // ---- end SizeInBytes() ---- 

// ******************************************************************
// FrequencyInHertz:	Convert to Hertz with metric system prefixes
// ****************************************************************** 
    void 
FrequencyInHertz(double frequency, char *out, char printFormat)
{
char	s[50];

if (frequency < 0)
    {
    sprintf(out, "? Hertz");
    return;
    }

// print size with 3 significant digits in range 

// units in Giga Hertz 
if	(frequency >= 1000000000)
    {
    frequency *= 0.000000001;
//   s[0] = 'G'; s[1] = '\0';
    strcpy(s, "GHz");
    }
// units in Mega Hertz 
else if (frequency >= 1000000)
    {
    frequency *= 0.000001;
//    s[0] = 'M'; s[1] = '\0';
    strcpy(s, "MHz");
    }
// units in Kilo Hertz 
else if (frequency >= 1000)
    {
    frequency *= 0.001;
//    s[0] = 'K'; s[1] = '\0';
    strcpy(s, "KHz");
    }
// units in Hertz 
else 
    {
//    s[0] = '\0';
    strcpy(s, "Hertz");
    }

if	(printFormat == kPrintFormat_Short)
    sprintf(out, "%8gHz",    frequency, s);
else
    sprintf(out, "%.15g %s", frequency, s);
} // ---- end FrequencyInHertz() ---- 

// **********************************************************************
// MIDINoteToNotation:	Convert MIDI Note to string
//					w/musical notation 
//
//				Range [0..127] -> [C-1..G9], where 
//				middle C is C4=60. 
// ********************************************************************** */
    int 
MIDINoteToNotation(int noteNumber, char *note, int useFlats)
// noteNumber	note# to convert
//    note		ptr to space for output 
{
char	*ptr;
int	octave;

if (noteNumber > 127)
    return (0);

// Determine note letter 
ptr = note;
switch (((int) noteNumber)%12)
    {
    case 0:
	*ptr++ = 'C';
    break;
    case 1:
		if (useFlats)
		{
		*ptr++ = 'D';
		*ptr++ = 'b';
		}
		else
		{
		*ptr++ = 'C';
		*ptr++ = '#';
		}
    break;
    case 2:
	*ptr++ = 'D';
    break;
    case 3:
    	if (useFlats)
		{
		*ptr++ = 'E';
		*ptr++ = 'b';
		}
		else
		{
		*ptr++ = 'D';
		*ptr++ = '#';
		} 
	break;
    case 4:
	*ptr++ = 'E';
    break;
    case 5:
	*ptr++ = 'F';
    break;
    case 6:
       	if (useFlats)
		{
		*ptr++ = 'G';
		*ptr++ = 'b';
		}
		else
		{
		*ptr++ = 'F';
		*ptr++ = '#';
		} 
	break;
    case 7:
	*ptr++ = 'G';
    break;
    case 8:
		if (useFlats)
		{
		*ptr++ = 'A';
		*ptr++ = 'b';
		}
		else
		{
		*ptr++ = 'G';
		*ptr++ = '#';
		} 
	break;
    case 9:
	*ptr++ = 'A';
    break;
    case 10:
		if (useFlats)
		{
		*ptr++ = 'B';
		*ptr++ = 'b';
		}
		else
		{
		*ptr++ = 'A';
		*ptr++ = '#';
		} 
	break;
    case 11:
	*ptr++ = 'B';
    break;
    }

// Determine octave# around C0 = MIDI note #12 
octave = ((int) noteNumber)/12 - 1;
if (octave < 0)
    {
    *ptr++ = '-';
    octave = -octave;
    }
*ptr++ = '0' + ((char) octave);
*ptr = '\0';

#ifdef DEBUG
printf("MIDINoteToNotation() octave=%d", octave);
printf("MIDINoteToNotation(): %d -> '%s'", noteNumber, note);
#endif
return (1);
}	// ---- end MIDINoteToNotation() ---- 

// **********************************************************************
// NotationToMIDINote:	Return MIDI Note # of string
//							w/musical notation  
//
//					Return (-1) on error
// ********************************************************************** */
    int 
NotationToMIDINote(char *note)
// note	    ptr to notation string 
{
int	MIDINoteNumber;
char	*s;
int	negative, octave, letter;

/* MIDI range is values 0..127
MIDI Note #	note
	0   = C-1
	60  = C4
	120 = C9
	127 = G9
note will be in format Letter[A..G,a..g]<#,f,b>number[-2..9] 
example:    D#4
*/

// 1st character must be letter in range ['a'..'g','A'..'G'] 
s = note;
// capitalize 
if  (*s >= 'a' && *s <= 'g')
    {
    *s -= 'a';
    *s += 'A';
    }
if  (*s >= 'A' && *s <= 'G')
    s++;
else
    return (-1);

// 2nd character may be '#'=sharp, 'b'or'f'=flat, '-', '0'..'9' *
if (*s == '#' || *s == 'b'|| *s == 'f')	    //  advance past modifier 
    s++;
negative = FALSE;
if (*s == '-')
    {
    s++;
    negative = TRUE;
    }
if (*s >= '0' && *s <= '9')
    {
    octave = (int)(*s - '0');
    if (negative)
	octave = -octave;

    if	    (note[0] == 'C')
		letter = 0;
    else if (note[0] == 'D')
		letter = 2;
    else if (note[0] == 'E')
		letter = 4;
    else if (note[0] == 'F')
		letter = 5;
    else if (note[0] == 'G')
		letter = 7;
    else if (note[0] == 'A')
		letter = 9;
    else if (note[0] == 'B')
		letter = 11;

// Deal w/"sharp" character 
    if (note[1] == '#')
		letter++;
// Deal w/"flat" character 
   else if (note[1] == 'b' || note[1] == 'f')
		letter--;

// Compute MIDI note#.  12 is value of C0 
    MIDINoteNumber = 12 + letter + octave*12;

#ifdef DEBUG_UTIL
printf("NotationToMIDINote() '%s'->%d, letter=%d, octave=%d\n", 
	note, MIDINoteNumber, letter, octave);
#endif
// Ensure result is in MIDI note range 0..127 
    if (MIDINoteNumber < 0 || MIDINoteNumber > 127)
	return (-1);
    s++;
    }
else
    return (-1);

if (*s != '\0')
    return (-1);

return ((int) MIDINoteNumber);
}	// ---- end NotationToMIDINote() ---- 

// **********************************************************************************
// HexToLong:	Convert Hexadecimal	string to long value
// ********************************************************************************** 
	long
HexToLong(char *s)
{
int end, found, longIndex;
long value = 0;
char *p = (char *) &value;

if (!s)
	return (0);

// Find end index and process backwards
end = 0;
while (s[end] != '\0')
	end++;

if (0 == end)
	return (0);

printf("HexToLong: '%s' end =%d\n", s, end);
end--;
longIndex = 3;
found = 0;
while (end >= 0 && found < 4)
	{
	char c    = s[end];
	char cTmp = (int) c;;
//printf("c='%c' %d\n", c, c);
	if		(c >= 'a' && c <= 'f')
		cTmp = cTmp  - 'a' + 10;
	else if	(c >= 'A' && c <= 'F')
		cTmp = cTmp  - 'A' + 10;
	else if	(c >= '0' && c <= '9')
		cTmp = cTmp  - '0';
	else 
		break;
	cTmp &= 0xF;
	if (found & 0x1)
		{
		*(p+longIndex) |= (char)(cTmp<<4);	// to Upper nibble
//printf("HexToLong: ODD  end=%d cTmp=%d p[%d]=%2X value=%4X\n", end, cTmp, longIndex, p[longIndex], value);
		longIndex--;
		}
	else
		{
		*(p+longIndex) |= (char) cTmp;		// to Lower nibble
//printf("HexToLong: EVEN end=%d cTmp=%d p[%d]=%2X value=%4X\n", end, cTmp, longIndex, p[longIndex], value);
		}
	found++;
	end--;
//printf("HexToLong: s[%d]='%c' %d  value=%4X\n", end, c, c, value);
	}

printf("HexToLong: END '%s'=%d %X\n\n", s, value, value);
return (value);
}	// ---- end HexToLong() ---- 

// **********************************************************************************
// HexToShort:	Convert Hexadecimal	string to Int16 value - little endian (Intel) version
// ********************************************************************************** 
	int
HexToShort(char *s)
{
int end, found, index;
short value = 0;
char *p = (char *) &value;

if (!s)
	return (0);

// Find end index and process backwards
end = 0;
while (s[end] != '\0')
	end++;

if (0 == end)
	return (0);

//printf("HexToShort: '%s' end =%d\n", s, end);
end--;
index = 0;	// NOTE:  this is little endian C-code
found = 0;
while (end >= 0 && found < 4)
	{
	char c    = s[end];
	char cTmp = (int) c;;
//printf("c='%c' %d\n", c, c);
	if		(c >= 'a' && c <= 'f')
		cTmp = cTmp  - 'a' + 10;
	else if	(c >= 'A' && c <= 'F')
		cTmp = cTmp  - 'A' + 10;
	else if	(c >= '0' && c <= '9')
		cTmp = cTmp  - '0';
	else 
		break;
	cTmp &= 0xF;
	if (found & 0x1)
		{
		*(p+index) |= (char)(cTmp<<4);	// to Upper nibble
//printf("HexToShort: ODD  end=%d cTmp=%d p[%d]=%2X value=%4X\n", end, cTmp, index, p[index], value);
		index++;
		}
	else
		{
		*(p+index) |= (char) cTmp;		// to Lower nibble
//printf("HexToShort: EVEN end=%d cTmp=%d p[%d]=%2X value=%4X\n", end, cTmp, index, p[index], value);
		}
	found++;
	end--;
//printf("HexToShort: s[%d]='%c' %d  value=%4X\n", end, c, c, value);
	}

//printf("HexToShort: END '%s'=%d %X\n\n", s, value, value);
return (value);
}	// ---- end HexToShort() ---- 

// **********************************************************************************
// Binary16ToShort:	Convert Binarystring to Int16 value - little endian (Intel) version
// ********************************************************************************** 
	short
Binary16ToShort(char *s)
// Assuming <= 16 digit string.
{
int i;
long value = 0;
long bitMask = 0x00008000;
short returnValue;

if (!s)
	return (0);

for (i = 0; s[i] != '\0'; i++)
	{
	if (s[i] != '0' && s[i] != '1')
		break;

	if (s[i] == '1')
		value |= bitMask;
	bitMask = bitMask>>1;
//printf("Binary16ToShort: s[%d]='%c' bitMask=%X\n", i, s[i], bitMask);
	}

returnValue = (short)value;
//printf("Binary16ToShort: '%s' -> %d %X\n", s, returnValue, returnValue);
return (returnValue);
}	// ---- end Binary16ToShort() ---- 

// ********************************************************************** 
// SecondsToSamples    Return time in units of samples
//			    Rounded to nearest sample.
// ********************************************************************** 
    int
SecondsToSamples(double time, double samplingFrequency)
// time		time (in seconds)  
//  samplingFrequency	sampling rate (in samples per second) 
//
{
// Compute time in samples
//	Seconds * (samples/Second) = samples 
int length = (int) (time*samplingFrequency + 0.5);

return (length);
}	// ---- end SecondsToSamples() ---- 

// ********************************************************************** 
// SamplesToSeconds    Return time in units of seconds
// ********************************************************************** 
    double
SamplesToSeconds(int samples, double samplingFrequency)
// samples		# samples
// samplingFrequency	sampling rate (in samples per second) 
{
// compute time in seconds
//	samples / (samples/Second) = seconds
//
if (samples >= 0 && samplingFrequency > 0.0)
    return (((double) samples)/samplingFrequency);

return (-1);
}	// ---- end SamplesToSeconds() ---- 

// ********************************************************************
// CreateTextFile:    Create file and return file handle
// ******************************************************************** 
    FILE * 
CreateTextFile(char *path)
{
FILE *handle = fopen(path, "w");

//printf("CreateTextFile: '%s'=%d\n", path, handle);
if (!handle)
    printf("CreateTextFile: failed to open '%s' for writing\n", path);
return (handle);
}	// ---- end CreateTextFile() ---- 

// ********************************************************************
// CreateFileOrExit:    Create file and return file handle.
//						On failure, exit application
// ******************************************************************** 
    FILE * 
CreateFileOrExit(char *path)
{
FILE *handle = CreateTextFile(path);
if (!handle)
	exit (-1);
return (handle);
}	// ---- end CreateFileOrExit() ---- 

/* **********************************************************************
 * RemoveCharacter:   Remove all target characters
 *		    Return # removed.
 * ********************************************************************** */
    int 
RemoveCharacter(char *s, char target)
{
int i, j;

if (!s)
    return (0);

/* scan for last space=' ' character at beginning of string */
for (i = 0, j = 0; s[j] != '\0'; j++)
    {    
    if (s[j] != target)
	s[i++] = s[j];
    }
s[i] = '\0';

return (j-i);
}   /* ---- end RemoveCharacter() ---- */

/* **********************************************************************
 * RemoveNonNumericals:   Remove all characters not in set 
 *			    {'0'..'9', '.', '-'}
 *		    Return # characters removed.
 * ********************************************************************** */
    int 
RemoveNonNumericals(char *s)
{
int i, j;
if (!s)
    return (0);

for (i = 0, j = 0; s[j] != '\0'; j++)
    {    
    if (s[j] == '-' || s[j] == '.' || (s[j] >= '0' && s[j] <= '9'))
	s[i++] = s[j];
    }
s[i] = '\0';

return (j-i);
}   /* ---- end RemoveNonNumericals() ---- */

/* **********************************************************************
 * IsNumerical:		Disallow non-numerical characters:  
 *			    Must be in set: 
 *				{'0'..'9', '.', '-'}
 *				Input typed from keyboard is of length 1.  
 *				Xpasted input may be any length
 * ********************************************************************** */
    int 
IsNumerical(char *s) 
{
int i;
int foundNumerical = False;

if (!s)
    return (False);

for (i = 0; s[i] != '\0'; i++)
    {    
    if	    (s[i] == ' ' || s[i] == '-' || s[i] == '+' || s[i] == '.')
		foundNumerical = True;
    else if (s[i] >= '0' && s[i] <= '9')
		foundNumerical = True;
    else
		return (False);
    }
return (foundNumerical);
}   /* ---- end IsNumerical() ---- */

/* **********************************************************************
 * IsPositiveNumerical:		Disallow non-numerical characters: 
 *			    Must be in set: 
 *				{'0'..'9', '.'}
 *				Input typed from keyboard is of length 1.  
 *				Xpasted input may be any length
 * ********************************************************************** */
    int 
IsPositiveNumerical(char *s) 
{
int foundNumerical = False;
int i;
if (!s)
    return (False);

for (i = 0; s[i] != '\0'; i++)
    {    
    if	    (s[i] == ' ' || s[i] == '.' || s[i] == '+')
		foundNumerical = True;
    else if (s[i] >= '0' && s[i] <= '9')
		foundNumerical = True;
    else
		return (False);
    }

return (foundNumerical);
}   /* ---- end IsPositiveNumerical() ---- */

// *************************************************************** 
// ScaleDB:	Scale in buffer to out buffer
//				(ok for "in place" scale )
// ***************************************************************
    void
ScaleDB(float *in, float *out, long length, float gainDB)
{
long  i;	
float k = (float)pow(10.0, 0.05*(double)gainDB);	
for (i = 0; i < length; i++)
    out[i] = in[i]*k;
//Printf("ScaleDB: gainDB=%g, k=%g\n", gainDB, k);	
}	// ---- end ScaleDB() ---- 

// *************************************************************** 
// RampDB:	Ramp in buffer to out buffer
//				(ok for "in place" scale )
// ***************************************************************
    void
RampDB(float *in, float *out, long length, float gainDB, float deltaDB)
// gainDB:	start value
// deltaDB:	change over interval
{
//Printf("RampDB: gainDB=%g, deltaDB=%g\n", gainDB, deltaDB);	

if (0.0 == deltaDB)
	ScaleDB(in, out, length, gainDB);
else
	{
	long  i;
	float delta = deltaDB/(float)length;
// FIXXX: inefficient but at least the curve is desirable, for once
	for (i = 0; i < length; i++, gainDB += delta)
		{
		float k = (float)pow(10.0, 0.05*(double)gainDB);	
		out[i] = in[i]*k;
		}
	}
}	// ---- end RampDB() ---- 

// *************************************************************** 
// Dot:	y = inA * inB
//				(ok for "in place" operation )
// ***************************************************************
    void
Dot(float *inA, float *inB, float *out, long length)
{
long 	i;			
for (i = 0; i < length; i++)
    out[i] = inA[i] * inB[i];
}	// ---- end Dot() ---- 

// *************************************************************** 
// Sum:	y = inA + inB
//				(ok for "in place" operation )
// ***************************************************************
    void
Sum(float *inA, float *inB, float *out, long length)
{
long 	i;			
for (i = 0; i < length; i++)
    out[i] = inA[i] + inB[i];
}	// ---- end Sum() ---- 

// **********************************************************************
// Clip:    Limit signal to range  [lo .. hi]
// **********************************************************************
    void
Clip(float *in, float *out, long length, float lo, float hi)
{
//{static long c=0;printf("Clip: %d lo=%g hi=%g\n", c++, lo, hi);}
for (long i = 0; i < length; i++)
	{
	if		(in[i] < lo)
		out[i] = lo;
	else if (in[i] > hi)
		out[i] = hi;
	else
		out[i] = in[i];
	}
}	// ---- end Clip() ----

// **********************************************************************
// Floor:    Limit signal to range  [lo .. N]
// **********************************************************************
    void
Floor(float *in, float *out, long length, float lo)
{
//{static long c=0;printf("Floor: %d lo=%g\n", c++, lo);}
for (long i = 0; i < length; i++)
	{
	if		(in[i] < lo)
		out[i] = lo;
	else
		out[i] = in[i];
	}
}	// ---- end Floor() ----

// **********************************************************************
// Ceiling:    Limit signal to range  [N .. hi]
// **********************************************************************
    void
Ceiling(float *in, float *out, long length, float hi)
{
//{static long c=0; printf("Ceiling: %d lo=%g\n", c++, lo);}
for (long i = 0; i < length; i++)
	{
	if		(in[i] > hi)
		out[i] = hi;
	else
		out[i] = in[i];
	}
}	// ---- end Ceiling() ----

// **********************************************************************
// GateScale:    Scale signal that falls below threshold: range  [lo .. hi]
// **********************************************************************
    void
GateScale(float *in, float *out, long length, float lo, float hi, float k)
{
long  i;

for (i = 0; i < length; i++)
	{
	if	(in[i] > lo && in[i] < hi)
		out[i] = k*in[i];
	else
		out[i] = in[i];
	}
}	// ---- end GateScale() ----

// **********************************************************************
// CompressClip:    Scale signal that exceeds threshold: range  [lo .. hi]
// **********************************************************************
    void
CompressClip(float *in, float *out, long length, float threshold, float limit, float ratio)
{
long  i;
float thresholdLo, thresholdHi;
float limitLo    , limitHi;
float tmp;

thresholdLo = -threshold;
thresholdHi =  threshold;
limitLo     = -limit;
limitHi     =  limit;

for (i = 0; i < length; i++)
	{
	if	    (in[i] < thresholdLo) 
		{
		tmp = in[i] * ratio;
		if (tmp < limitLo)
			out[i] = limitLo;
		else
			out[i] = tmp;
		}
	else if (in[i] > thresholdHi)
		{
		tmp = in[i] * ratio;
		if (tmp > limitHi)
			out[i] = limitHi;
		else
			out[i] = tmp;
		}
	else
		out[i] = in[i];
	}

//Printf("CompressClip: threshold=%g,ratio=%g\n", threshold, ratio);
}	// ---- end CompressClip() ----

// **********************************************************************
// Average2:    Average 2 neighboring samples
//					Ok for in-place operation
// **********************************************************************
    void
Average2(float *in, float *out, long length)
{
long i;
for (i = 0; i < length; i += 2)
	*out++ = 0.5f*(in[i] + in[i+1]);
}	// ---- end Floor() ----

// **********************************************************************
// AverageN:    Average N neighboring samples
//					Ok for in-place operation
// **********************************************************************
    void
AverageN(float *in, float *out, long length, int count)
{
long i, j;
float k = 1.0f/(float) count;

for (i = 0; i < length; i += count)
	{
	float sum = in[i];
	for (j = 1; j < count; j++)
		sum += in[i + j];
	*out++ = k*sum;
	}
}	// ---- end AverageN() ----

// **********************************************************************
// Average2Shorts:    Average 2 neighboring samples
//					Ok for in-place operation
// **********************************************************************
    void
Average2Shorts(short *in, short *out, long length)
{
long i;
for (i = 0; i < length; i += 2)
	{
	long sum = (long) in[i];
	sum += in[i+1];
	*out++ = (short)(sum>>1);
	}
}	// ---- end Average2Shorts() ----

// ****************************************************************
// WriteToFile:	Append text string to specified text file
//					Return Boolean success
// ****************************************************************
	int 
WriteToFile(char *text, char *path)
{
FILE *h;

if (NULL == (h = fopen(path, "a+t"))) 
	{
	printf("WriteToFile: unable to open log file '%s'\n", path);
	return (False);
	}

fwrite(text, 1, strlen(text),  h);
fclose(h);

return (True);
}	// ---- end WriteToFile() ---- 
// **********************************************************************
// ChangeRangeLog10: 
// **********************************************************************
	float
ChangeRangeLog10(float x, float inRangeLo, float inRangeHi, float outRangeLo, float outRangeHi)
{
float logX0 = (float) log10(1.0f + inRangeLo);
float logX1 = (float) log10(1.0f + inRangeHi);
float y     = (float) log10(1.0f + x);

y -= logX0;
y /= (logX1 - logX0);
y *= (outRangeHi - outRangeLo);
y +=  outRangeLo;

//		ShiftRangeArrayf(t, logRangeX, linearRange);
//printf("%d: hashValue=%f logHashValue=%f -> %f\n", i, hashValue, logHashValue, t);
//		x =  (int) (0.5f + ChangeRangeLog10(hashValue, d->rangeX[Lo], d->rangeX[Hi], x, width));

return (y);
}	// ---- end ChangeRangeLog10() ----

// **********************************************************************
// ShiftRangef:     Massage Range [LO1 .. HI1] to [LO2 .. HI2]
// **********************************************************************
	float
ShiftRangef(float x, float LO1, float HI1, float LO2, float HI2)
// x		input 
// LO1, HI1	low and high bounds of input  range 
// LO2, HI2	low and high bounds of output range 
{
float y = (x - LO1)*((HI2 - LO2)/(HI1 - LO1)) + LO2;
return (y);
}	// ---- end ShiftRangef() ----

// **********************************************************************
// ShiftRanged:     Massage Range [LO1 .. HI1] to [LO2 .. HI2]
// **********************************************************************
	double
ShiftRanged(double x, double LO1, double HI1, double LO2, double HI2)
// x		input 
// LO1, HI1	low and high bounds of input  range 
// LO2, HI2	low and high bounds of output range 
{
double y = (x - LO1)*((HI2 - LO2)/(HI1 - LO1)) + LO2;
return (y);
}	// ---- end ShiftRanged() ----

// **********************************************************************************
// ByteToHex:		Convert char to two ASCII hex characters 
//
// ********************************************************************************** 
	int   
ByteToHex(char c, int capitalize, char *outS)
// outS: ptr to two bytes 
{
char a = (c & 0xF0)>>4;
char b = (c & 0x0F);
char hexBase = 'a';

//printf("ByteToHex c=%X a=%X b=%X\n", c, a, b);

if (capitalize)
	hexBase = 'A';

if (a < 10)
	outS[0] = '0' + a;
else
	outS[0] = hexBase + a - 10;

if (b < 10)
	outS[1] = '0' + b;
else
	outS[1] = hexBase + b - 10;

//printf("ByteToHex s[0]=%c s[1]=%c\n", outS[0], outS[1]);

return (True);
}	// ---- end ByteToHex() ---- 

#ifdef NEEDED
// **********************************************************************************
// FloatToHexFrac24:		Convert 32-bit floating point number to 6 char hex ASCII string 
//
//					Valid for fractional range -1 < x < 1
//
//							Return Boolean success
// ********************************************************************************** 
	int   
FloatToHexFrac24(float x, int capitalize, char *outS)
{
long y, mask;
int i, shift = 16;
char hexBase = 'a';
int returnValue = True;

if (x <= -1.0 || x >= 1.0)
	{
	printf("FloatToHexFrac24: %g out of 2.22 fractional format\n", x);
	returnValue = False;
	BOUND(x, -1.0, 1.0);
	}

if (capitalize)
	hexBase = 'A';

// Squeeze out 6 nibbles (4-bit) as ASCII characters
//y    = 0x00FFFFFF & (long)(0.5 + x*k2To23);	// five +1 diffs
y    = 0x00FFFFFF & (long)(x*k2To23);	// two -1, two +1 diffs
//y    = 0x00FFFFFF & (long)(0.5 + x*k2To23m1); // four +1, one -1, one +2
//y    = 0x00FFFFFF & (long)(x*k2To23m1); // four +1, one -2, one +2, one -2

mask = 0x00FF0000;

for (i = 0; i < 3; i++, mask >>=8, shift -= 8, outS += 2)
	{
	unsigned long z = (unsigned long) (y & mask);
	z = z>>shift;
	{
	char a = (char)((z & 0xF0)>>4);
	char b = (char) (z & 0x0F);
	if (a < 10)
		outS[0] = '0' + a;
	else
		outS[0] = hexBase + a - 10;

	if (b < 10)
		outS[1] = '0' + b;
	else
		outS[1] = hexBase + b - 10;
	}
	ByteToHex((char)z, True, outS);
//printf("y & %6X = %X '%s'\n\n", mask, z, outS);
	}

outS[6] = '\0';

//printf("FloatToHexFrac24 s[0]=%c s[1]=%c\n", outS[0], outS[1]);

return (returnValue);
}	// ---- end FloatToHexFrac24() ---- 
#endif // NEEDED

// **********************************************************************
// FloatToQ31: Convert 32-bit float to 32-bit 1.31 signed integer
// **********************************************************************
	Q31 
FloatToQ31(float x)
//	x	Range [-1 .. 1]  well, up to one LSB less than 1.0
{
float y = (Q31)(0.5f + x*k2To31m1);
//Boundf(&y, kS32_Min, kS32_Max);
return ((Q31) y);
}	// ---- end FloatToQ31() ----

// **********************************************************************
// Q31ToFloat: Convert 32-bit 1.31 signed integer to 32-bit float
//                 NOTE: Precision lost when 32-bit value is stored in 24-bit mantissa
// **********************************************************************
	float 
Q31ToFloat(Q31 x)
//	x	Range [-1 .. 1]  well, up to one LSB less than 1.0
{
return ((1.0/k2To31m1)*(float)x);       // FIXXX: Never tested
}	// ---- end Q31ToFloat() ----

// **********************************************************************
// FloatToQ15: Convert 32-bit float to 16-bit 1.15 signed integer
//                      Return 
// **********************************************************************
	Q15 
FloatToQ15(float x)
//	x	Range [-1 .. 1]  well, up to one LSB less than 1.0
{
long y = (long)(0.5f + k2To15m1*x);
return ((Q15) BoundS32(&y, kS16_Min, kS16_Max));
}	// ---- end FloatToQ15() ----

// **********************************************************************
// FloatToQ15v2: Convert 32-bit float to 16-bit 1.15 signed integer range
//                          But in lower 16-bits of 32-bit container
// **********************************************************************
	Q31
FloatToQ15v2(float x)
//	x	Range [-1 .. 1]  well, up to one LSB less than 1.0
{
long y = (long)(k2To15m1*x);
//BoundS32(&y, kS16_Min, kS16_Max);
return ((Q31) y);
}	// ---- end FloatToQ15v2() ----

// **********************************************************************
// Q15ToFloat: Convert 16-bit 1.15 signed integer to 32-bit float
// **********************************************************************
	float 
Q15ToFloat(Q15 x)
//	x	Range [-1 .. 1]  well, up to one LSB less than 1.0
{
return ((1.0f/k2To15m1)*(float)x);
}	// ---- end Q15ToFloat() ----

// **********************************************************************
// PrintQ15asFloat: Print 16-bit 1.15 signed integer as a 32-bit float
// **********************************************************************
	void 
PrintQ15asFloat(Q15 x)
{
printf("PrintQ15asFloat: %04X -> %g \n", x, Q15ToFloat(x));
}	// ---- end PrintQ15asFloat() ----

// **********************************************************************
// PrintQ31asFloat: Print 32-bit 1.31 signed integer as a 32-bit float
// **********************************************************************
	void 
PrintQ31asFloat(Q15 x)
{
printf("PrintQ15asFloat: %08X -> %g \n", x, Q31ToFloat(x));
}	// ---- end PrintQ31asFloat() ----

// **********************************************************************************
// AddQ15:		Add two Q15 (1.15) integers with saturation arithmetic
// ********************************************************************************** 
__inline Q15 AddQ15( Q15 x, Q15 y )
{
Q15 z;
#ifdef USE_ARM946_DSPEXT
asm volatile (
	"qadd   %0, %1, %2\n\t"
	: "=r" (z) 
	: "r" (x), "r" (y)
	);
#else
z = x + y;  // FIXX: no saturation here
#endif
return (z);
}	// ---- end AddQ15() ---- 

// **********************************************************************************
// SubQ15:	Subtract two Q15 (1.15) integers with saturation arithmetic
//			z = x - y;
// ********************************************************************************** 
__inline Q15 SubQ15( Q15 x, Q15 y )
{
Q15 z;
#ifdef USE_ARM946_DSPEXT
asm volatile (
	"qsub   %0, %1, %2\n\t"
	: "=r" (z) 
	: "r" (x), "r" (y)
	);
#else
z = x - y;  // FIXX: no saturation here
#endif
return (z);
}	// ---- end SubQ15() ---- 

// **********************************************************************************
// AddQ31:		Add two Q31 (1.31) integers with saturation arithmetic
// ********************************************************************************** 
__inline Q31 AddQ31( Q31 x, Q31 y )
{
Q31 z;
#ifdef USE_ARM946_DSPEXT
asm volatile (
	"qadd   %0, %1, %2\n\t"
	: "=r" (z) 
	: "r" (x), "r" (y)
	);
#else
z = x + y;  // FIXX: no saturation here
#endif
return (z);
}	// ---- end AddQ31() ---- 

// **********************************************************************************
// SubQ31:	Subtract two Q31 (1.31) integers with saturation arithmetic
//			z = x - y;
// ********************************************************************************** 
__inline Q31 SubQ31( Q31 x, Q31 y )
{
Q31 z;
#ifdef USE_ARM946_DSPEXT
asm volatile (
	"qsub   %0, %1, %2\n\t"
	: "=r" (z) 
	: "r" (x), "r" (y)
	);
#else
z = x - y;  // FIXX: no saturation here
#endif
return (z);
}	// ---- end SubQ31() ---- 

// *************************************************************** 
// MultQ31:	Integer 
//
//			16x16=32-bit fixed-point implementation
// Use ARM DSP Extensions. 
// ***************************************************************
__inline Q31 MultQ31( Q31 a, Q31 b )
{
Q31 yl;

#ifdef USE_ARM946_DSPEXT
asm volatile (
//	"smulwt %2, %0, %1\n\t"
//	"qadd   %2, %2, %2\n\t"
	"smulwt %0, %1, %2\n\t"
	"qadd   %0, %0, %0\n\t"
	: "=r" (yl) 
	: "r" (a), "r" (b)
	);
#else
// 32x32=64 multiply doesn't work unless arguments are promoted to 64-bit type
yl = (Q31) (((((long long)a) * (long long)b))>>31);
//printf("MultQ31 :  yll= %llu (%16llX) \n", yll, yll);
//printf("MultQ31 :  yl= %d (%08X) \n", yl, yl);
#endif

return (yl);
}	// ---- end MultQ31() ---- 

// *************************************************************** 
// MacQ31:	Integer Multiply-accumulate
//
//			16x16=32-bit fixed-point implementation
// Use ARM DSP Extensions. 
// ***************************************************************
__inline Q31 MacQ31(Q31 x, Q31 a, Q31 b )
{
Q31 yl;

#ifdef USE_ARM946_DSPEXT
asm volatile (
//	"smulwt %2, %0, %1\n\t"
//	"qadd   %2, %2, %2\n\t"
	"smulwt %0, %2, %3\n\t"
	"qadd   %0, %0, %1\n\t"
	"qadd   %0, %0, %0\n\t"
	: "=r" (yl) 
	: "r" (x), "r" (a), "r" (b)
	);
#else
// 32x32=64 multiply doesn't work unless arguments are promoted to 64-bit type
yl = x + (Q31) (((((long long)a) * (long long)b))>>31);
//printf("MultQ31 :  yll= %llu (%16llX) \n", yll, yll);
//printf("MultQ31 :  yl= %d (%08X) \n", yl, yl);
#endif

return (yl);
}	// ---- end MacQ31() ---- 

// **********************************************************************************
// htonl:		test routine: swap bytes (big to little endian)
// ********************************************************************************** 
unsigned long htonl(unsigned long inVal)
{
unsigned long rVal = 0;

#ifdef USE_ARM946_DSPEXT
    asm volatile ("eor r3, %1, %1, ror #16\n\t"
                  "bic r3, r3, #0x00FF0000\n\t"
                  "mov %0, %1, ror #8\n\t"
                  "eor %0, %0, r3, lsr #8"
                  : "=r" (rVal)
                  : "r"(inVal)
                  : "r3" 
        );
#else
#endif
    return (rVal);
}	// ---- end htonl() ---- 

// *************************************************************** 
// FXP31_MULT:	Integer 
//			Used in Mobileer MIDI engine
//
// Use ARM DSP Extensions. 
// ***************************************************************
__inline FXP31 testFXP31_MULT( FXP31 x_1, FXP31 y_2 )
{
FXP31 yl_0;

#ifdef USE_ARM946_DSPEXT
{static int c=0; if (!c) printf("testFXP31_MULT %d: using ARM946\n", c++);}
asm volatile (
//	"smulwt %2, %0, %1\n\t"
//	"qadd   %2, %2, %2\n\t"
	"smulwt %0, %1, %2\n\t"
	"qadd   %0, %0, %0\n\t"
	: "=r" (yl_0) 
	: "r" (x_1), "r" (y_2)
	);
#else
/* Portable 'C' macro to multiply two 1.31 fixed-point values. */
{static int c=0; if (!c) printf("testFXP31_MULT %d: portable C version\n", c++);}
yl_0 = (x_1>>15) * (y_2>>16);
#endif

return (yl_0);
}	// ---- end testFXP31_MULT() ---- 

/* This macro is a code mixing operation.
 * It scales the input signal by the gain and adds it to an accumulator.
 */
/* Inline function for optimized ARM version.
 * Note this macro is missing the QADD so we have to adjust shiftby accordingly.
 */
__inline FXP31 testMIX_SCALE_SHIFT_ADD( FXP31 accum_1, FXP31 signal_2, FXP31 gain_3, int shiftby_4 )
{
FXP31 temp_0;  // R4

#ifdef USE_ARM946_DSPEXT
{static int c=0; if (!c) printf("testMIX_SCALE_SHIFT_ADD %d: using ARM946 \n", c++);}
// ORIGINAL code from Phil Burk
//	__asm
//	{
//		SMULWT    temp, signal, gain
//		ADD       temp, accum, temp, asr shiftby
//	}
asm volatile (
	"smulwt	%0, %2, %3\n\t"
//	"add    %0, %1, %0, asr %4\n\t"
	"asr    %0, %4\n\t"
	"add    %0, %1, %0\n\t"
	: "=r" (temp_0) 
        : "r" (accum_1), "r" (signal_2), "r" (gain_3), "r" (shiftby_4)
        );
#else
{static int c=0; if (!c) printf("testMIX_SCALE_SHIFT_ADD %d: portable C version\n", c++);}
/* Portable 'C' version of core mixing element. */
temp_0 = accum_1 + (testFXP31_MULT( signal_2, gain_3 ) >> shiftby_4);
#endif

return (temp_0);
}	// ---- end testMIX_SCALE_SHIFT_ADD()

// **********************************************************************************
// Test_Dsputil:		Routine just to call tests
// ********************************************************************************** 
	void   
Test_Dsputil()
{
long i = 0, j = 0, k = 0;
Q15   x15 = 0, y15 = 0, z15 = 0;
Q15   a15 = 0, b15 = 0, c15 = 0;
Q31   x31 = 0, y31 = 0, z31 = 0;
Q31   a31 = 0, b31 = 0, c31 = 0;
float af = 0.0f, bf = 0.0f, cf = 0.0f;
float xf = 0.0f, yf = 0.0f, zf = 0.0f;
float df = 0.0f, gf = 0.0f, kf = 0.0f;
float xfout = 0.0f, yfout = 0.0f, zfout = 0.0f;
short xi, yi, zi;
unsigned long aul, bul, cul;

printf("Test_Dsputil: start \n");

// 
// ---- Tests for scaling routines
//
//#define TEST_SCALING_ROUTINES
#ifdef TEST_SCALING_ROUTINES
{
long length = 1;

printf("Test_Dsputil: start  TEST_SCALING_ROUTINES \n");

xf = 10000.0;
kf = 0.25;
Scale(&xf, &yf, length, kf);
printf("Scale: %g * %g = %g \n", xf, kf, yf);

xi = 10000;
kf = 0.25;
ScaleShortsf(&xi, &yi, length, k);
printf("ScaleShortsf: %d * %g = %d \n", xi, kf, yi);

xi = 10000;
kf = 0.25;
//ScaleShortsi(&xi, &yi, length, k);
//printf("ScaleShortsi: %d * %g = %d \n", xi, kf, yi);
//void ScaleShortsf(short *in, short *out, long length, float k);
//void ScaleShortsi(short *in, short *out, long length, float k);
//void ScaleShortsi_Fractional(short *in, short *out, long length, short k);
}
#endif // end TEST_SCALING_ROUTINES

//#define TEST_Q15_ROUTINES
#ifdef TEST_Q15_ROUTINES
{
printf("---- Q15 routines ---- \n");

xf = 0.125f;
yf = 0.125f;
x15 = FloatToQ15(xf);
y15 = FloatToQ15(yf);
z15 = AddQ15(x15, y15);
zf    = xf + yf;
zfout = Q15ToFloat(z15);
printf("AddQ15: REF %g + %g = %g \n", xf, yf, zf);
printf("AddQ15: OUT %g + %g = %g \n", xf, yf, zfout);
printf("AddQ15: %d + %d = %d \n", x15, y15, z15);
printf("AddQ15: $%04X + $%04X = $%04X \n\n", x15, y15, z15);

xf = 0.250f;
yf = 0.125f;
x15 = FloatToQ15(xf);
y15 = FloatToQ15(yf);
z15 = SubQ15(x15, y15);
zf    = xf - yf;
zfout = Q15ToFloat(z15);
printf("SubQ15: REF %g - %g = %g \n", xf, yf, zf);
printf("SubQ15: OUT %g - %g = %g \n", xf, yf, zfout);
printf("SubQ15: %d - %d = %d \n", x15, y15, z15);
printf("SubQ15: $%04X - $%04X = $%04X \n\n", x15, y15, z15);

xf  = 0.25f;
yf  = 0.25f;
x15 = FloatToQ15(xf);
y15 = FloatToQ15(yf);
z15 = MultQ15(x15, y15);
zf     = xf * yf;
zfout  = Q15ToFloat(z15);
printf("MultQ15: REF %g * %g = %g \n", xf, yf, zf);
printf("MultQ15: OUT %g * %g = %g \n", xf, yf, zfout);
printf("MultQ15: %d * %d = %d \n", x15, y15, z15);
printf("MultQ15: $%04X * $%04X = $%04X \n\n", x15, y15, z15);

xf  =  0.25f;
yf  = -0.75f;
x15 = FloatToQ15(xf);
y15 = FloatToQ15(yf);
z15 = MultQ15(x15, y15);
zf    = xf * yf;
zfout = Q15ToFloat(z15);
printf("MultQ15: REF %g * %g = %g \n", xf, yf, zf);
printf("MultQ15: OUT %g * %g = %g \n", xf, yf, zfout);
printf("MultQ15: %d * %d = %d \n", x15, y15, z15);
printf("MultQ15: $%04X * $%04X = $%04X \n\n", 0xffff&x15, 0xffff&y15, 0xffff&z15);

af = 0.10f;
xf = 0.25f;
yf = 0.25f;
a15 = FloatToQ15(af);
x15 = FloatToQ15(xf);
y15 = FloatToQ15(yf);
z15 = MacQ15(a15, x15, y15);
zf    = af + xf*yf;
zfout = Q15ToFloat(z15);
printf("MacQ15: REF %g + %g * %g = %g \n", af, xf, yf, zf);
printf("MacQ15: OUT %g + %g * %g = %g \n", af, xf, yf, zfout);
printf("MacQ15: %d + %d * %d = %d \n", a15, x15, y15, z15);
printf("MacQ15: $%04X + $%04X * $%04X = $%04X \n\n", 0xffff&a15, 0xffff&x15, 0xffff&y15, 0xffff&z15);
}
#endif // end TEST_Q15_ROUTINES

//aul = 0x12345678;
//bul = htonl(aul);
//printf("htonl(): $%08X -> $%08X \n", aul, bul);
//bul = htonl(aul);
//printf("htonl(): $%08X -> $%08X \n", bul, aul);

//#define TEST_Q31_ROUTINES
#ifdef TEST_Q31_ROUTINES
{
printf("---- Q31 routines ---- \n");

xf = 0.125f;
yf = 0.125f;
x31 = FloatToQ31(xf);
y31 = FloatToQ31(yf);
z31 = AddQ31(x31, y31);
zf    = xf + yf;
zfout = Q31ToFloat(z31);
printf("AddQ31: REF %g + %g * %g = %g \n", af, xf, yf, zf);
printf("AddQ31: OUT %g + %g * %g = %g \n", af, xf, yf, zfout);
printf("AddQ31: %d + %d = %d \n", x31, y31, z31);
printf("AddQ31: $%08X + $%08X = $%08X \n\n", x31, y31, z31);

xf = 0.250f;
yf = 0.125f;
x31 = FloatToQ31(xf);
y31 = FloatToQ31(yf);
z31 = SubQ31(x31, y31);
zf    = xf - yf;
zfout = Q31ToFloat(z31);
printf("SubQ31: REF %g + %g * %g = %g \n", af, xf, yf, zf);
printf("SubQ31: OUT %g + %g * %g = %g \n", af, xf, yf, zfout);
printf("SubQ31: %d - %d = %d \n", x31, y31, z31);
printf("SubQ31: $%08X - $%08X = $%08X \n\n", x31, y31, z31);

xf = 0.125f;
yf = 0.125f;
x31 = FloatToQ31(xf);
y31 = FloatToQ31(yf);
z31 = MultQ31(x31, y31);
zf    = xf * yf;
zfout = Q31ToFloat(z31);
printf("MultQ31: REF %g + %g * %g = %g \n", af, xf, yf, zf);
printf("MultQ31: OUT %g + %g * %g = %g \n", af, xf, yf, zfout);
printf("MultQ31: %d * %d = %d \n", x31, y31, z31);
printf("MultQ31: $%08X * $%08X = $%08X \n\n", x31, y31, z31);

af = 0.10f;
xf = 0.25f;
yf = 0.25f;
a31 = FloatToQ31(af);
x31 = FloatToQ31(xf);
y31 = FloatToQ31(yf);
z31 = MacQ31(a31, x31, y31);
zf    = af + xf * yf;
zfout = Q31ToFloat(z31);
printf("MacQ31: REF %g + %g * %g = %g \n", af, xf, yf, zf);
printf("MacQ31: OUT %g + %g * %g = %g \n", af, xf, yf, zfout);
printf("MacQ31: %d + %d * %d = %d \n", a31, x31, y31, z31);
printf("MacQ31: $%08X + $%08X * $%08X = $%08X \n\n", a31, x31, y31, z31);
}
#endif // end TEST_Q31_ROUTINES

#define TEST_FXP31_ROUTINES
#ifdef TEST_FXP31_ROUTINES
{
FXP31 afx, bfx, cfx, dfx, efx;
long shiftValue = 3;
printf("---- FXP31 routines ---- \n");

afx = 0x2fffffff;
bfx = 0x1fffffff;
cfx = testFXP31_MULT(afx, bfx);
af = Q31ToFloat(afx);
bf = Q31ToFloat(bfx);
cf = Q31ToFloat(cfx);

printf("FXP31_MULT: afx=$%08X * bfx=$%08X  = cfx=$%08X\n", afx, bfx, cfx);
printf("FXP31_MULT: OUT %g * %g = %g \n", af, bf, cf);

afx = 0x2fffffff;
bfx = 0x1fffffff;
dfx = 0x3fffffff;
// FXP31 testMIX_SCALE_SHIFT_ADD( afx, bfx, dfx, -1 );
cfx = testMIX_SCALE_SHIFT_ADD(dfx, afx, bfx, shiftValue);
af = Q31ToFloat(afx);
bf = Q31ToFloat(bfx);
cf = Q31ToFloat(cfx);
df = Q31ToFloat(dfx);

efx = testFXP31_MULT(afx, bfx);
efx >>= shiftValue;
//printf("MIX_SCALE_SHIFT_ADD: afx=$%08X * bfx=$%08X +dfx=$%08X  =  cfx=$%08X\n", afx, bfx, dfx, cfx);
printf("MIX_SCALE_SHIFT_ADD: (%g * %g)>>%d) =%g\n", af, bf, shiftValue, Q31ToFloat(efx));
printf("MIX_SCALE_SHIFT_ADD: OUT %g + ((%g * %g)>>%d) = %g \n", df, af, bf, shiftValue, cf);

}
#endif // end TEST_FXP31_ROUTINES

}	// ---- end Test_Dsputil() ---- 

