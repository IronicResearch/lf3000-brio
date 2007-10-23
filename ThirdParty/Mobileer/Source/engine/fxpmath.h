#ifndef _FXPMATH_H
#define _FXPMATH_H
/* $Id: fxpmath.h,v 1.15 2006/02/14 20:08:47 philjmsl Exp $ */
/**
 *
 * @file fxpmath.h
 * @brief Fixed-point math support.
 * 
 * These tools provide DSP style arithmetic where numbers are fractions
 * between -1.0 and +1.0.
 * 
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

/* Define fixed-point data types. The number indicates the number of fractional bits. */
/* FXP31 ranges from 0x80000000 to 0x7FFFFFFF */ 
typedef long FXP31;
typedef long FXP27;
/* FXP24 ranges from 0xFF000000 to 0x00FFFFFF */ 
typedef long FXP24;
typedef long FXP16;
/* FXP15 ranges from 0xFFFF8000 to 0x00007FFF */
/* FXP15 is a common 16 bit signed value, eg. a WAV sample. */
typedef long FXP15;
typedef long FXP14;
typedef long FXP8;
typedef long FXP7;

/* Allow DSP optimization to be overridden by compiler setting. */
#ifndef SPMIDI_DSP_BLACKFIN
	#ifndef __ADSPBLACKFIN__
		#define SPMIDI_DSP_BLACKFIN   (0)
	#else
		#define SPMIDI_DSP_BLACKFIN   (__ADSPBLACKFIN__)
	#endif
#endif

#ifndef SPMIDI_DSP_ARM946
	#ifndef __TARGET_FEATURE_DSPMUL
		#define SPMIDI_DSP_ARM946     (0)
	#else
		#define SPMIDI_DSP_ARM946     (__TARGET_FEATURE_DSPMUL)
	#endif
#endif

/* Define fixed point multiplication of the high 16-bit halves of
 * two 32-bit fixed-point numbers.
 */
#if SPMIDI_DSP_BLACKFIN
inline  FXP31_MULT( FXP31 x, FXP31 y )
{
	FXP31 product;
	asm("%0 = %1.H * %2.H;" 
		:"=r"(product)     /* output */ 
		:"r"(x),"r"(y)  /* input  */ 
		);
	return product;
}
#elif SPMIDI_DSP_ARM946_P1

/* Use ARM DSP Extensions. */

__inline FXP31 FXP31_MULT( FXP31 x_1 , FXP31 y_2 )
{
FXP31 product_0;
//{static int c=0; if (!c) printf("FXP31_MULT: %d using ARM946_P1 \n", c++);}

// ORIGINAL CODE FROM PHIL
//	__asm {
//	    SMULWT    product, x, y
//	    QADD    product, product, product
//	}
asm volatile (
	"smulwt %0, %1, %2\n\t"
	"qadd   %0, %0, %0\n\t"
	: "=r" (product_0) 
        : "r" (x_1), "r" (y_2)
        );
return (product_0);
}
#else
/* Portable 'C' macro to multiply two 1.31 fixed-point values. */
#define FXP31_MULT(a,b)   (((a)>>15) * ((b)>>16))
#endif

/* Generate a 16.16 result by multiplying a 1.31 and a 16.16 fixed-point values. */
#define FXP_MULT_16_31_16( fxp31, fxp16 ) ( ((fxp31 >> 18) * (fxp16 >> 2)) >> 11 )

#define FXP31_MIN_VALUE   ((FXP31)0x80000000)
#define FXP31_MAX_VALUE   ((FXP31)0x7FFFFFFF)

#define FXP27_MIN_VALUE   (-(FXP27)0x08000000)
#define FXP27_MAX_VALUE   ((FXP27)0x07FFFFFF)
#define FXP27_MULT(a,b)   (((a)>>13) * ((b)>>14))

#define FXP15_MIN_VALUE   (-(FXP15)0x8000)
#define FXP15_MAX_VALUE   ((FXP15)0x00007FFF)
#define FXP15_MULT(a,b)   ((a)*(b)>>15)

/* This macro is a code mixing operation.
 * It scales the input signal by the gain and adds it to an accumulator.
 */
/* Inline function for optimized ARM version.
 * Note this macro is missing the QADD so we have to adjust shiftby accordingly.
 */
#if SPMIDI_DSP_ARM946_P2
__inline FXP31 MIX_SCALE_SHIFT_ADD( FXP31 accum_1, FXP31 signal_2, FXP31 gain_3, int shiftby_4 )
{

int temp_0;  // R4
//{static int c=0; if (!c) printf("MIX_SCALE_SHIFT_ADD %d: using ARM946_P2 \n", c++);}

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
return (temp_0);
}
#else

/* Portable 'C' version of core mixing element. */
#define MIX_SCALE_SHIFT_ADD( accum, signal, gain, shiftby ) \
	(accum +  (FXP31_MULT( signal, gain )  >> shiftby))
	
#endif


#endif /* _FXPMATH_H */
