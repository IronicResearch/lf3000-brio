// *************************************************************** 
// dsputil_mix.h:	Header file for reduced mixer routines
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#ifndef __DSPUTIL_MIX_H__
#define	__DSPUTIL_MIX_H__

#include <math.h>

#include "dsputil_mix.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

#ifndef False
#define False 0
#endif

#ifndef True
#define True 1
#endif

#ifndef kLeft
#define kLeft 0
#endif

#ifndef kRight
#define kRight 1
#endif

#define kPi			3.14159265358979323846 // M_PI
#define kTwoPi      (2.0*kPi)

#define	k2To15i		    (1<<15)
#define	k2To15m1i		(k2To15i - 1)
#define	k2To15m1		((double) k2To15m1i)

typedef short           S16;   
typedef long            S32;   
typedef unsigned long   U32;   

typedef short Q15;   // 1.15 format in 16 bits
typedef long  Q31;   // 1.31 format in 32 bits

#define kS16_Max      32767
#define kS16_Min    (-32768)
#define kS16_Maxf     ((float)kS16_Max)
#define kS16_Minf     ((float)kS16_Min)

#define kQ15_Max     kS16_Max
#define kQ15_Min     kS16_Min
#define kQ15_Maxf     ((float)kS16_Max)
#define kQ15_Minf     ((float)kS16_Min)

#define kPanValue_FullLeft  (-1.0)
#define kPanValue_Center      0.0
#define kPanValue_FullRight ( 1.0)

#define DecibelToLinear(d)		(pow(10.0, ((double)(d))*(1.0/20.0)))
#define LinearToDecibel(x)		(log10((x))*20.0)

#define DecibelToLinearf(d)		((float) DecibelToLinear((d)))
#define LinearToDecibelf(x)		((float) LinearToDecibel((x)))

void PanValues_ConstantPower(float x, float *outs);

void AddShorts(short *in, short *out, long length, long saturate);
void CopyShorts(short *in, short *out, long length);
void ClearShorts(short *in, long length);

void ScaleShortsi_Q15(Q15 *in, Q15 *out, long length, Q15 k);
void MACShortsi_Q15(Q15 *in, Q15 *out, long length, Q15 k);

Q15   FloatToQ15(float x);
float Q15ToFloat(Q15 x);
void BlastSineOscillatorS16(Q15 *out, long length, float fc, float gain);

void BoundS16(S16 *x, S16 lo, S16 hi);
void Boundf(float *x, float lo, float hi);

// *************************************************************** 
// MacQ15:	1.15 Integer multiplication and addition
//
//          Return z = y + a*b
//				1.15 x 1.15 = 1.30
// ***************************************************************
__inline Q15 MacQ15( Q15 y, Q15 a, Q15 b )
{
Q15 z;
#ifdef USE_ARM946_DSPEXT
//	"smulbb %0, %1, %2\n\t"
//	"smlabb %0, %1, %2\n\t"
asm volatile (
	"smulbb %0, %2, %3\n\t"
	"mov %0, %0, asr#15\n\t"
	"qadd %0, %0, %1\n\t"
	: "=r" (z) 
	: "r" (y), "r" (a), "r" (b)
	: "r3"
	);
return (z);
#else
{
long sum32 = y + ((a*b)>>15);
if      (sum32 > kQ15_Max)
    return (kQ15_Max);
else if (sum32 < kQ15_Min)
    return (kQ15_Min);
else
    return ((Q15)sum32);
}
#endif
}	// ---- end MacQ15() ---- 

// *************************************************************** 
// MultQ15:	Integer multiplication
//
//			16x16=32-bit fixed-point implementation
//				1.15 x 1.15 = 1.30
// ***************************************************************
__inline Q15 MultQ15( Q15 a, Q15 b )
{
Q15 y;

#ifdef USE_ARM946_DSPEXT
// NOTE:  'volatile' instructs compiler to skip optimization of assembly instructions
// __asm__  __volatile__ (  // Use this for stricter compilers
asm volatile (
	"smulbb %0, %1, %2\n\t"
	"mov %0, %0, asr#15\n\t"
	: "=r" (y) 
	: "r" (a), "r" (b)
	: "r3"
	);
#else
y = (a*b)>>15;
#endif

return (y);
}	// ---- end MultQ15() ---- 

#endif  //	end __DSPUTIL_MIX_H__

