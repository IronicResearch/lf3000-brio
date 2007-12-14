// *************************************************************************
//
// Header file DSP functions
//
//				Written by Gints Klimanis, 1996-present
// *************************************************************************

#ifndef __DSPUTIL_H__
#define	__DSPUTIL_H__

#include <math.h>
#include <stdio.h>

#include "util.h"#include "midi.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef kLeft
#define kLeft	0
#endif
#ifndef kRight
#define kRight	1
#endif

#define	k2To0i		   (1<<0)
#define	k2To1i		   (1<<1)
#define	k2To2i		   (1<<2)
#define	k2To3i		   (1<<3)
#define	k2To4i		   (1<<4)
#define	k2To5i		   (1<<5)
#define	k2To6i		   (1<<6)
#define	k2To7i		   (1<<7)
#define	k2To8i		   (1<<8)
#define	k2To9i		   (1<<9)
#define	k2To10i		   (1<<10)
#define	k2To11i		   (1<<11)
#define	k2To12i		   (1<<12)
#define	k2To13i		   (1<<13)
#define	k2To14i		   (1<<14)
#define	k2To15i		   (1<<15)
#define	k2To16i		   (1<<16)
#define	k2To17i		   (1<<17)
#define	k2To18i		   (1<<18)
#define	k2To19i		   (1<<19)
#define	k2To20i		   (1<<20)
#define	k2To21i		   (1<<21)
#define	k2To22i		   (1<<22)
#define	k2To23i		   (1<<23)
#define	k2To24i		   (1<<24)
#define k2To29i		   (1<<29)
#define k2To30i		   (1<<30)
// These purist definitions cause problems for 32-bit integers
//#define k2To31i		   (1<<31)
//#define k2To32i		   (1<<32)
#define k2To31i		2147483648
#define k2To32i		4294967296

#define	k2To15m1i		(k2To15i - 1)
#define	k2To16m1i		(k2To16i - 1)
#define	k2To22m1i		(k2To22i - 1)
#define	k2To23m1i		(k2To23i - 1)
#define	k2To24m1i		(k2To24i - 1)
#define k2To29m1i		(k2To29i - 1)
#define k2To30m1i		(k2To30i - 1)
// These purist definitions cause problems for 32-bit integers
//#define k2To31m1i		(k2To31i - 1)
//#define k2To32m1i		(k2To32i - 1)
#define k2To31m1i		(2147483647UL)  // Add UL suffix to avoid compiler warnings
#define k2To32m1i		(4294967295UL)


#define	k2To14		((double) k2To14i)
#define	k2To14f		((float ) k2To14i)
#define	k2To15		((double) k2To15i)
#define	k2To15f		((float ) k2To15i)
#define	k2To16		((double) k2To16i)
#define	k2To16f		((float ) k2To16i)
#define	k2To17		((double) k2To17i)
#define	k2To17f		((float ) k2To17i)
#define	k2To18		((double) k2To18i)
#define	k2To18f		((float ) k2To18i)
#define	k2To19		((double) k2To19i)
#define	k2To19f	 	((float ) k2To19i)

#define	k2To20		((double) k2To20i)
#define	k2To20f		((float ) k2To20i)
#define	k2To21		((double) k2To21i)
#define	k2To21f		((float ) k2To21i)
#define	k2To22		((double) k2To22i)
#define	k2To22f		((float ) k2To22i)
#define	k2To23		((double) k2To23i)
#define	k2To23f		((float ) k2To23i)
#define	k2To24		((double) k2To24i)
#define	k2To24f		((double) k2To24i)

// Note that any power of 2 > 23 will not fit into a 32-bit single precision floating point
// respresentation, limited by the 23-bit mantissa width
//#define	TwoTo24f	(  16777216.0f)

#define	k2To25		((double) k2To25i)
#define	k2To25f		((float ) k2To25i)
#define	k2To26		((double) k2To26i)
#define	k2To26f		((float ) k2To26i)
#define	k2To27		((double) k2To27i)
#define	k2To27f		((float ) k2To27i)
#define	k2To28		((double) k2To28i)
#define	k2To28f		((float ) k2To28i)
#define	k2To29		((double) k2To29i)
#define	k2To29f		((float ) k2To29i)

#define k2To30		((double) k2To30i)
#define k2To30f		((float ) k2To30i)
#define k2To31		((double) k2To31i)
#define k2To31f		((float ) k2To31i)
#define k2To32		((double) k2To32i)
#define k2To32f		((float ) k2To32i)

#define k2ToTheMinus15		(1.0/k2To15)
#define k2ToTheMinus15f	    (1.0f/k2To15f)
#define k2ToTheMinus31		(1.0/k2To31)
#define k2ToTheMinus31f	((float)(1.0/k2To31))

#define	k2To15m1		((double) k2To15m1i)
#define	k2To15m1f		((float ) k2To15m1i)
#define	k2To16m1		((double) k2To16m1i)
#define	k2To16m1f		((float ) k2To16m1i)
#define	k2To22m1		((double) k2To22m1i)
#define	k2To22m1f		((float ) k2To22m1i)
#define	k2To23m1		((double) k2To23m1i)
#define	k2To23m1f		((float ) k2To23m1i)
#define	k2To24m1		((double) k2To24m1i)
#define k2To29m1		((double) k2To29m1i)
#define k2To30m1		((double) k2To30m1i)
#define k2To31m1		((double) k2To31m1i)
#define k2To32m1d		((double) k2To32m1i)
// For triple parameter arrays
#define kMin		0
#define kMax		1
#define kDefault	2

#define DODO
#ifdef DODO
#define  Printf	 printf
#define FPrintf fprintf
#define SPrintf	sprintf	

#else
#define  Printf	{}
#define FPrintf {}
#define SPrintf	{}	
#define SetLogFilePath  {}
#define WriteToLogFile  {}
#define WriteToLogFile2 {}
#endif

int WriteToFile(char *text, char *path);

#define kPrintStringSpace	1000

#define kPrintLevelLow	0x1
#define kPrintLevelHigh	0x2
#define kPrintLevelAll	0x3
#define kPrintLevelBoth	kPrintLevelAll

// These modes append to string
#define kPrintLevelAppend		0x10
#define kPrintLevelAppendLow	(kPrintLevelAppend | kPrintLevelLow )
#define kPrintLevelAppendHigh	(kPrintLevelAppend | kPrintLevelHigh)
#define kPrintLevelAppendAll	(kPrintLevelAppend | kPrintLevelAll )
#define kPrintLevelAppendBoth	kPrintAppendLevelAll

#define kPrintLevelNone	        0x0

typedef signed char     S8;   
typedef unsigned char   U8;   
typedef short           S16;   
typedef long            S32;   
typedef unsigned long   U32;   

typedef short Q15;   // 1.15 format in 16 bits
typedef long  Q31;   // 1.31 format in 32 bits

#define kS8_Max ( 127)
#define kS8_Min (-128)
#define kS8_Maxf ((float)kS8_Max)
#define kS8_Minf ((float)kS8_Min)

#define kU8_Max ( 255)
#define kU8_Min (   0)
#define kU8_Maxf ((float)kU8_Max)
#define kU8_Minf ((float)kU8_Min)

#define kS16_Max ( 32767)
#define kS16_Min (-32768)
#define kS16_Maxf ((float)kS16_Max)
#define kS16_Minf ((float)kS16_Min)

#define kQ15_Max ( 32767)
#define kQ15_Min (-32768)
#define kQ15_Maxf ((float)kQ15_Max)
#define kQ15_Minf ((float)kQ15_Min)

#define kS32_Max ( k2To32m1i)
#define kS32_Min (-k2To32i)
#define kS32_Maxd ((float)kS16_Max)  // Need 64-bit float to preserve 32-bit precision
#define kS32_Mind ((float)kS16_Min)

#define kMaxSamplingFrequency	48000.0
#define kMaxSamplingRate		kMaxSamplingFrequency
#define kMaxSamplingPeriod		(1.0/kMaxSamplingFrequency)
//#define SetSamplingRate(d, x)	((d)->samplingFrequency = (double)(x))

#define kBogusSamplingFrequency	(-1.0)

#define mTolerancef(x, v, e) (x >= (v)*(1.0f - (e)) && x <= (v)*(1.0f + (e)))	
#define mToleranced(x, v, e) (x >= (v)*(1.0  - (e)) && x <= (v)*(1.0  + (e)))	

// ChangeRange: Convert Range [L1 .. H1] to [L2 .. H2]
// y = (x - L1)*((H2 - L2)/(H1 - L1)) + L2
// x		input value
// L1, H1	lower and upper bounds of input  range [L1 .. H1]
// L2, H2	lower and upper bounds of output range [L2 .. H2]
#ifndef ChangeRangef
#define ChangeRangef(x, L1, H1, L2, H2) ((((float )(x)) - ((float )(L1)))*((((float )(H2)) - ((float )(L2)))/(((float )(H1)) - ((float )(L1)))) + ((float )(L2))) 
#endif
#ifndef ChangeRanged
#define ChangeRanged(x, L1, H1, L2, H2) ((((double)(x)) - ((double)(L1)))*((((double)(H2)) - ((double)(L2)))/(((double)(H1)) - ((double)(L1)))) + ((double)(L2))) 
#endif
#ifndef ChangeRange
#define	ChangeRange ChangeRanged
#endif
//void ChangeRangef(float x, float L1, float H1, float L2, float H2);

// ShiftRange: Massage Range [LO1 .. HI1] to [LO2 .. HI2]
// y = (x - LO1)*((HI2 - LO2)/(HI1 - LO1)) + LO2
// x		input 
// LO1, HI1	low and high bounds of input  range 
// LO2, HI2	low and high bounds of output range 
//#define ShiftRangef(x, LO1, HI1, LO2, HI2) ((((float )(x)) - ((float )(LO1)))*((((float )(HI2)) - ((float )(LO2)))/(((float )(HI1)) - ((float )(LO1)))) + ((float )(LO2))) 
float ShiftRangef(float x, float LO1, float HI1, float LO2, float HI2);
//#define ShiftRanged(x, LO1, HI1, LO2, HI2) ((((double)(x)) - ((double)(LO1)))*((((double)(HI2)) - ((double)(LO2)))/(((double)(HI1)) - ((double)(LO1)))) + ((double)(LO2))) 
double ShiftRanged(double x, double LO1, double HI1, double LO2, double HI2);
#define ShiftRangeArrayf(x, r1, r2) (ShiftRangef(x, (float)r1[LO], (float)r1[HI], (float)r2[LO], (float)r2[HI]))
#define ShiftRangeArrayd(x, r1, r2) (ShiftRanged(x, (double)r1[LO], (double)r1[HI], (double)r2[LO], (double)r2[HI]))

#define ChangeRangeArrayf	ShiftRangeArrayf
#define ChangeRangeArrayd	ShiftRangeArrayd

	float
ChangeRangeLog10(float x, float inRangeLo, float inRangeHi, float outRangeLo, float outRangeHi);

short  DoubleToFractionalInteger(double x);
double FractionalIntegerToDouble(short  x);

	float 
SegmentChangeRangef(float x, float inLo , float inMid , float inHi,
							 float outLo, float outMid, float outHi);
	double 
SegmentChangeRanged(double x, double inLo , double inMid , double inHi,
							  double outLo, double outMid, double outHi);
#define SegmentChangeRange		SegmentChangeRangef

float RandRangeF(float lo, float hi);

#define MAX_CHANNELS	2
#define kMaxChannels	2

typedef struct dualfloat {
	float	a, b;
} DualFloat;
typedef struct dualdouble {
	double	a, b;
} DualDouble;

#define PrintOn	 True
#define PrintOff False

#define kPrintFormat_Short		0
#define kPrintFormat_Normal		1
#define kPrintFormat_Long		2
void DurationInSeconds(double seconds,   char *out, char printFormat);
void SizeInBytes		(long    bytes,    char *out, char printFormat);
void FrequencyInHertz (double frequency, char *out, char printFormat);

// NOTE: Compiler messes up double -> 32-bit unsigned, so go to 31-bit and shift left
#define mDoubleToULong(x)	(((unsigned long)(k2To31m1*(double)(x)))<<1)
#define mFloatToULong		mDoubleToULong

#define mWholed(   x) ( (double)(long)(x))
#define mFractiond(x) (((double)(x)) - mWholed(x))
#define mWholef(   x) ( (float )(long)(x))
#define mFractionf(x) (((float )(x)) - mWholef(x))
#define mWhole			mWholed
#define mFraction		mFractiond

#define Roundl(x)	   ((long )((x) + 0.5)) 
#define Rounds(x)	   ((short)((x) + 0.5))

//float VolumeToGain(float x, float *xRange);

#define kPanValue_FullLeft ( 1.0)
#define kPanValue_Center      0.0
#define kPanValue_FullRight (-1.0)
void  PanValues   (float x, float *outs);
void  ConstantPowerValues(float x, float *left, float *right);

#define kE		 2.718281828

#define kLog10_2   0.3010299956639812
#define kLog10_2f ((float) kLog10_2)
#define kLog2_10  (1.0 /   kLog10_2)	// 1/log2(10) = log10(2)
#define kLog2_10f ((float) kLog2_10)	// 1/log2(10) = log10(2)

#define kLogE_2	   0.69314718055994529 // 0.69314718055994530941723212145818
#define kLogE_2f  ((float) kLogE_2)
#define kLn_2		kLogE_2
#define kLn_2f		kLogE_2f

#define kLog2_E	  (1.0 /   kLogE_2)		// 1/logE( 2) = log2(E)
#define kLog2_Ef  ((float) kLog2_E)		// 1/logE( 2) = log2(E)

#define Fabs	fabs
//#define Pow		pow
//#define Exp		exp

#define Exp(x)		  (dspPow2(Log2ofE *(double)(x)))
#define Pow10(x)	  (dspPow2(Log2of10*(double)(x)))

#define Expf(x)		  ((float) Exp(  (x)))
#define Powf(x)		  ((float) Pow(  (x)))
#define dspPow2f(x)	  ((float) dspPow2( (x)))
#define Pow10f(x)	  ((float) Pow10((x)))

#define Fabsf(x)	((float) Fabs((x)))		 
//#define Expf(x)		((float) Exp( (x)))
//#define Powf(x)		((float) Pow( (x)))

double Log2 (double x);
double Log10(double x);
double Log  (double x);
double LogN (double n, double x);

double dspPow2(double x);
double Pow (double x, double y);

//#define acosh( x)	(log((x) + sqrt((x)*(x) - 1)))
//#define asinh( x)	(log((x) + sqrt((x)*(x) + 1)))

#define dspPow2(x) (pow(2.0, (x)))

#define EqualTemperToRatio(s) 			(dspPow2((s)*(1.0f/12.0f)))
#define RatioToEqualTemper(r)			(Log2((r))*12.0f)
#define NToneEqualTemperToRatio(s, c)	 dspPow2((s) *( 0.01f/12.0f)*(c))
#define RatioToNToneEqualTemper(r, c)	(Log2((r))*((12.0f/0.01f)/(c)))
#define MIDINoteToHertz(n)	 			(440.0 * (float)EqualTemperToRatio((n)- 69))
#define PitchToFrequency(n, ref)	 	(ref   * (float)EqualTemperToRatio((n)))

#define EqualTemperToRatiof(s) 			((float)EqualTemperToRatio(s))

int	MIDINoteToNotation(int noteNumber, char *note, int useFlats);
int NotationToMIDINote(char *note);

long  HexToLong      (char *s);
int   HexToShort     (char *s);
short Binary16ToShort(char *s);

//#define M_E			2.718281828
//#define M_SQRT2	 	(1.414213562)
#define M_SQRT2_D2	(0.5*M_SQRT2)
//#define M_PI	 	(3.14159265358979323846)
#define M_2PI	 	(2.0*M_PI)

#define kPi			M_PI // 3.14159265358979323846
#define	k2Pi	    (2.0*kPi)
#define kTwoPi		k2Pi
#define kSqrt2		M_SQRT2 // 1.414213562

#define kPif		((float) kPi)
#define kTwoPif		((float) kTwoPi)
#define kSqrt2f		((float) kSqrt2)

#define	kHalfPi	(0.5*kPi)
#define	kHalfPif ((float) kHalfPi)
#define kSqrt2d2	(kSqrt2/2.0)
#define kSqrt2d2k2Tof	 ((float) kSqrt2d2)

#define Log10of2	 0.3010299956639812
#define Log10of2f	 0.3010299956639812f
#define Log2of10	(1.0 /Log10of2 )		// 1/log2(10) = log10(2)
#define Log2of10f	(1.0f/Log10of2f)		// 1/log2(10) = log10(2)
#define LogEof2		0.69314718055994529
#define LogEof2f	0.69314718055994529f
#define Log2ofE		(1.0 /LogEof2 )			// 1/logE( 2) = log2(E)
#define Log2ofEf	(1.0f/LogEof2f)			// 1/logE( 2) = log2(E)

//	Exp		e^x
//	Pow		x^y
//	Log		logE(x)	or ln(x)
//	Log10	log10(x)

//	Sin		sine(x)
//	Cos		cosine(x)
//	Tan		tangent(x)
//  Atan	arctan(x, y)
//  Sqrt	square root

#define RCTimeConstant(r, c) 		(    (TwoPi*(r)*(c)))
#define RCFrequency(  r, c) 		(1.0/(TwoPi*(r)*(c)))


// WARNING: Workaround for screwed up double -> 32-bit unsigned long, 
// so convert to 31-bits and shift left once
#define DoubleToULong(x)	(((unsigned long)(k2To31m1*(double)(x)))<<1)
#define FloatToULong		DoubleToULong

long  HexToLong      (char *s);
int   HexToShort     (char *s);
short Binary16ToShort(char *s);

// Messy - function is prettier but more overhead
// More curve macros in time
#define SQUARE127f(x)   ((1.0f/(  127.0f*127.0f  ))*((float)(x))*((float)(x)))
#define SQUARE32767f(x) ((1.0f/(32767.0f*32767.0f))*((float)(x))*((float)(x)))
#define SQUARE65535f(x) ((1.0f/(65535.0f*65535.0f))*((float)(x))*((float)(x)))

#define SQUAREf(x)    (((float)(x))* (float)(x))
#define CUBEf(x)      (((float)(x))* (float)(x) * (float)(x))

#define INVERSESQUAREf(x) (1.0f - ((1.0f - (float)(x))*(1.0f - (float)(x))))


#define NORMALTwoTo15f(x) ((1.0f/k2To15f)*(float )(x))
#define NORMALTwoTo15d(x) ((1.0 /k2To15 )*(double)(x))
#define NORMALTwoTo15     NORMALTwoTo15f

#define NORMAL32768f	  NORMALTwoTo15f
#define NORMAL32768d	  NORMALTwoTo15d
#define NORMAL32768		  NORMALTwoTo15f

#define NORMALTwoTo29d(x)   ((1.0/k2To29)*(double)(x))
#define NORMALTwoTo29f(x)   ((float)NORMALTwoTo29d((x)))
#define NORMALTwoTo29       NORMALTwoTo29f

#define NORMALTwoTo30d(x)   ((1.0/k2To30)*(double)(x))
#define NORMALTwoTo30f(x)   ((float)NORMALTwoTo30d((x)))
#define NORMALTwoTo30       NORMALTwoTo30f

#define NORMALTwoTo30m1d(x) ((1.0/k2To30m1)*(double)(x))
#define NORMALTwoTo30m1f(x) ((float)NORMALTwoTo30m1d((x)))
#define NORMALTwoTo30m1     NORMALTwoTo30m1f

#define NORMALTwoTo31d(x)   ((1.0/k2To31)*(double)(x))
#define NORMALTwoTo31f(x)   ((float)NORMALTwoTo31d((x)))
#define NORMALTwoTo31       NORMALTwoTo31f

#define NORMALTwoTo31m1d(x) ((1.0 /k2To31m1)*(double)(x))
#define NORMALTwoTo31m1f(x) ((float)NORMALTwoTo31m1d((x)))
#define NORMALTwoTo31m1     NORMALTwoTo31m1f

#define NORMALTwoTo32d(x)   ((1.0/k2To32)*(double)(x))
#define NORMALTwoTo32f(x)   ((float)NORMALTwoTo32d((x)))
#define NORMALTwoTo32       NORMALTwoTo32f

#define NORMALTwoTo32m1d(x) ((1.0 /k2To32m1d )*(double)(x))
#define NORMALTwoTo32m1f(x) ((float)NORMALTwoTo32m1d((x)))
#define NORMALTwoTo32m1     NORMALTwoTo32m1f

#define NormalTwoTo30m1d(x) ((1.0/k2To30m1)*(double)(x))
#define NormalTwoTo30m1f(x) ((float)NormalTwoTo30m1d((x)))
#define NormalTwoTo31m1d(x) ((1.0/k2To31m1)*(double)(x))
#define NormalTwoTo31m1f(x) ((float)NormalTwoTo31m1d((x)))
#define NormalTwoTo32m1d(x) ((1.0/k2To32m1d)*(double)(x))
#define NormalTwoTo32m1f(x) ((float)NormalTwoTo32m1d((x)))

#define kMIDI_Cm1	0
#define kMIDI_Dbm1	1
#define kMIDI_Dm1	2
#define kMIDI_Ebm1	3
#define kMIDI_Em1	4
#define kMIDI_Fm1	5
#define kMIDI_Gbm1	6
#define kMIDI_Gm1	7
#define kMIDI_Abm1	8
#define kMIDI_Am1	9
#define kMIDI_Bbm1	10
#define kMIDI_Bm1	11
#define kMIDI_C0	12
#define kMIDI_Db0	13
#define kMIDI_D0	14
#define kMIDI_Eb0	15
#define kMIDI_E0	16
#define kMIDI_F0	17
#define kMIDI_Gb0	18
#define kMIDI_G0	19
#define kMIDI_Ab0	20
#define kMIDI_A0	21
#define kMIDI_Bb0	22
#define kMIDI_B0	23
#define kMIDI_C1	24
#define kMIDI_Db1	25
#define kMIDI_D1	26
#define kMIDI_Eb1	27
#define kMIDI_E1	28
#define kMIDI_F1	29
#define kMIDI_Gb1	30
#define kMIDI_G1	31
#define kMIDI_Ab1	32
#define kMIDI_A1	33
#define kMIDI_Bb1	34
#define kMIDI_B1	35
#define kMIDI_C2	36
#define kMIDI_Db2	37
#define kMIDI_D2	38
#define kMIDI_Eb2	39
#define kMIDI_E2	40
#define kMIDI_F2	41
#define kMIDI_Gb2	42
#define kMIDI_G2	43
#define kMIDI_Ab2	44
#define kMIDI_A2	45
#define kMIDI_Bb2	46
#define kMIDI_B2	47
#define kMIDI_C3	48
#define kMIDI_Db3	49
#define kMIDI_D3	50
#define kMIDI_Eb3	51
#define kMIDI_E3	52
#define kMIDI_F3	53
#define kMIDI_Gb3	54
#define kMIDI_G3	55
#define kMIDI_Ab3	56
#define kMIDI_A3	57
#define kMIDI_Bb3	58
#define kMIDI_B3	59
#define kMIDI_C4	60
#define kMIDI_Db4	61
#define kMIDI_D4	62
#define kMIDI_Eb4	63
#define kMIDI_E4	64
#define kMIDI_F4	65
#define kMIDI_Gb4	66
#define kMIDI_G4	67
#define kMIDI_Ab4	68
#define kMIDI_A4	69
#define kMIDI_Bb4	70
#define kMIDI_B4	71
#define kMIDI_C5	72
#define kMIDI_Db5	73
#define kMIDI_D5	74
#define kMIDI_Eb5	75
#define kMIDI_E5	76
#define kMIDI_F5	77
#define kMIDI_Gb5	78
#define kMIDI_G5	79
#define kMIDI_Ab5	80
#define kMIDI_A5	81
#define kMIDI_Bb5	82
#define kMIDI_B5	83
#define kMIDI_C6	84
#define kMIDI_Db6	85
#define kMIDI_D6	86
#define kMIDI_Eb6	87
#define kMIDI_E6	88
#define kMIDI_F6	89
#define kMIDI_Gb6	90
#define kMIDI_G6	91
#define kMIDI_Ab6	92
#define kMIDI_A6	93
#define kMIDI_Bb6	94
#define kMIDI_B6	95
#define kMIDI_C7	96
#define kMIDI_Db7	97
#define kMIDI_D7	98
#define kMIDI_Eb7	99
#define kMIDI_E7	100
#define kMIDI_F7	101
#define kMIDI_Gb7	102
#define kMIDI_G7	103
#define kMIDI_Ab7	104
#define kMIDI_A7	105
#define kMIDI_Bb7	106
#define kMIDI_B7	107
#define kMIDI_C8	108
#define kMIDI_Db8	109
#define kMIDI_D8	110
#define kMIDI_Eb8	111
#define kMIDI_E8	112
#define kMIDI_F8	113
#define kMIDI_Gb8	114
#define kMIDI_G8	115
#define kMIDI_Ab8	116
#define kMIDI_A8	117
#define kMIDI_Bb8	118
#define kMIDI_B8	119
#define kMIDI_C9	120
#define kMIDI_Db9	121
#define kMIDI_D9	122
#define kMIDI_Eb9	123
#define kMIDI_E9	124
#define kMIDI_F9	125
#define kMIDI_Gb9	126
#define kMIDI_G9	127
// Above MIDI range 
#define kMIDI_Ab9	128
#define kMIDI_A9	129
#define kMIDI_Bb9	130
#define kMIDI_B9	131
#define kMIDI_C10	132
#define kMIDI_Db10	133
#define kMIDI_D10	134
#define kMIDI_Eb10	135
#define kMIDI_E10	136
#define kMIDI_F10	137
#define kMIDI_Gb10	138
#define kMIDI_G10	139

#define kHz_Cm1 	8.175799f		// MIDI Note#=0
#define kHz_Dbm1	8.661957f
#define kHz_Dm1 	9.177024f
#define kHz_Ebm1	9.722718f
#define kHz_Em1 	10.300861f
#define kHz_Fm1 	10.913382f
#define kHz_Gbm1	11.562326f
#define kHz_Gm1 	12.249857f
#define kHz_Abm1	12.978272f
#define kHz_Am1 	13.750000f
#define kHz_Bbm1	14.567618f
#define kHz_Bm1 	15.433853f
#define kHz_C0 		16.351598f
#define kHz_Db0 	17.323914f
#define kHz_D0 		18.354048f
#define kHz_Eb0 	19.445436f
#define kHz_E0 		20.601722f
#define kHz_F0 		21.826764f
#define kHz_Gb0 	23.124651f
#define kHz_G0 		24.499715f
#define kHz_Ab0 	25.956544f
#define kHz_A0 		27.500000f
#define kHz_Bb0 	29.135235f
#define kHz_B0 		30.867706f
#define kHz_C1 		32.703196f
#define kHz_Db1 	34.647829f
#define kHz_D1 		36.708096f
#define kHz_Eb1 	38.890873f
#define kHz_E1 		41.203445f
#define kHz_F1 		43.653529f
#define kHz_Gb1 	46.249303f
#define kHz_G1 		48.999429f
#define kHz_Ab1 	51.913087f
#define kHz_A1 		55.000000f
#define kHz_Bb1 	58.270470f
#define kHz_B1 		61.735413f
#define kHz_C2 		65.406391f
#define kHz_Db2 	69.295658f
#define kHz_D2 		73.416192f
#define kHz_Eb2 	77.781746f
#define kHz_E2 		82.406889f
#define kHz_F2 		87.307058f
#define kHz_Gb2 	92.498606f
#define kHz_G2 		97.998859f
#define kHz_Ab2 	103.826174f
#define kHz_A2 		110.000000f
#define kHz_Bb2 	116.540940f
#define kHz_B2 		123.470825f
#define kHz_C3 		130.812783f
#define kHz_Db3 	138.591315f
#define kHz_D3 		146.832384f
#define kHz_Eb3 	155.563492f
#define kHz_E3 		164.813778f
#define kHz_F3 		174.614116f
#define kHz_Gb3 	184.997211f
#define kHz_G3 		195.997718f
#define kHz_Ab3 	207.652349f
#define kHz_A3 		220.000000f
#define kHz_Bb3 	233.081881f
#define kHz_B3 		246.941651f
#define kHz_C4 		261.625565f
#define kHz_Db4 	277.182631f
#define kHz_D4 		293.664768f
#define kHz_Eb4 	311.126984f
#define kHz_E4 		329.627557f
#define kHz_F4 		349.228231f
#define kHz_Gb4 	369.994423f
#define kHz_G4 		391.995436f
#define kHz_Ab4 	415.304698f
#define kHz_A4 		440.000000f
#define kHz_Bb4 	466.163762f
#define kHz_B4 		493.883301f
#define kHz_C5 		523.251131f
#define kHz_Db5 	554.365262f
#define kHz_D5 		587.329536f
#define kHz_Eb5 	622.253967f
#define kHz_E5 		659.255114f
#define kHz_F5 		698.456463f
#define kHz_Gb5 	739.988845f
#define kHz_G5 		783.990872f
#define kHz_Ab5 	830.609395f
#define kHz_A5 		880.000000f
#define kHz_Bb5 	932.327523f
#define kHz_B5 		987.766603f
#define kHz_C6 		1046.502261f
#define kHz_Db6 	1108.730524f
#define kHz_D6 		1174.659072f
#define kHz_Eb6 	1244.507935f
#define kHz_E6 		1318.510228f
#define kHz_F6 		1396.912926f
#define kHz_Gb6 	1479.977691f
#define kHz_G6 		1567.981744f
#define kHz_Ab6 	1661.218790f
#define kHz_A6 		1760.000000f
#define kHz_Bb6 	1864.655046f
#define kHz_B6 		1975.533205f
#define kHz_C7 		2093.004522f
#define kHz_Db7 	2217.461048f
#define kHz_D7 		2349.318143f
#define kHz_Eb7 	2489.015870f
#define kHz_E7 		2637.020455f
#define kHz_F7 		2793.825851f
#define kHz_Gb7 	2959.955382f
#define kHz_G7 		3135.963488f
#define kHz_Ab7 	3322.437581f
#define kHz_A7 		3520.000000f
#define kHz_Bb7 	3729.310092f
#define kHz_B7 		3951.066410f
#define kHz_C8 		4186.009045f
#define kHz_Db8 	4434.922096f
#define kHz_D8 		4698.636287f
#define kHz_Eb8 	4978.031740f
#define kHz_E8 		5274.040911f
#define kHz_F8 		5587.651703f
#define kHz_Gb8 	5919.910763f
#define kHz_G8 		6271.926976f
#define kHz_Ab8 	6644.875161f
#define kHz_A8 		7040.000000f
#define kHz_Bb8 	7458.620184f
#define kHz_B8 		7902.132820f
#define kHz_C9 		8372.018090f
#define kHz_Db9 	8869.844191f
#define kHz_D9 		9397.272573f
#define kHz_Eb9 	9956.063479f
#define kHz_E9 		10548.081821f
#define kHz_F9 		11175.303406f
#define kHz_Gb9 	11839.821527f
#define kHz_G9 		12543.853951f		// MIDI Note#=127
// Above MIDI range 
#define kHz_Ab9 	13289.750323		// MIDI Note#=128
#define kHz_A9 		14080.000000		  
#define kHz_Bb9 	14917.240369		  
#define kHz_B9 		15804.265640		  
#define kHz_C10 	16744.036179		  
#define kHz_Db10 	17739.688383		  
#define kHz_D10 	18794.545147		  
#define kHz_Eb10 	19912.126958		  
#define kHz_E10 	21096.163642		  
#define kHz_F10 	22350.606812		  
#define kHz_Gb10 	23679.643054		  
#define kHz_G10 	25087.707903		// MIDI Note#=139

#define DecibelToLinear(d)		(pow(10.0, ((double)(d))*(1.0/20.0)))
#define LinearToDecibel(x)		(log10((x))*20.0)

#define DecibelToLinearf(d)		((float) DecibelToLinear((d)))
#define LinearToDecibelf(x)		((float) LinearToDecibel((x)))
//#define DecibelToLinear(d)		(pow(10.0, ((double)(d))*(1.0/20.0)))
//#define LinearToDecibel(x)		(log10((x))*20.0)

// These constants are not very precise
#define kDecibelToLinearf_6dBf     1.999862f
#define kDecibelToLinearf_3dBf     1.414165f
#define kDecibelToLinearf_1p5dBf   1.189187f
#define kDecibelToLinearf_0dBf     0.999999f
#define kDecibelToLinearf_m1p5dBf  0.840911f
#define kDecibelToLinearf_m3dBf    0.707131f
#define kDecibelToLinearf_m6dBf    0.500035f
#define kDecibelToLinearf_m0dBf  kDecibelToLinearf_0dBf

// Careful:  these macros will not work when values at bottom and top
//				of integer numerical range are compared
#define BOUND(x, lo, hi) {\
    if      ((x) < (lo)) (x) = (lo);\
    else if ((x) > (hi)) (x) = (hi);\
}
#define BOUNDR(x, r) {\
    if      ((x) < (r)[LO]) (x) = (r)[LO];\
    else if ((x) > (r)[HI]) (x) = (r)[HI];\
}

S8    BoundS8( S8  *x, S8  lo, S8 hi);
U8    BoundU8( U8  *x, U8  lo, U8 hi);
S16   BoundS16(S16 *x, S16 lo, S16 hi);
S32   BoundS32(S32 *x, S32 lo, S32 hi);
float Boundf(float *x, float lo, float hi);

#define CEILING(x, hi) {\
	if ((x) > (hi))		(x) = (hi);\
}
#define FLOOR(x, lo) {\
	if ((x) < (lo))		(x) = (lo);\
}
#define CEILINGWRAP(x, lo, hi) {\
	if ((x) >= (hi))	(x) = (lo)+ mFractionf(x);\
}
//#define FLOORWRAP(x, lo, hi) { REMOVED BACKSLASH BECAUCE OF ARM COMPILER ERROR
//	if ((x) <= (lo))	(x) = (lo)+ mFractionf(x);
//}
// No return value
#define SATURATE_16BIT(x) {\
	if      ((x) >  k2To15m1i) (x) =  k2To15m1i;\
	else if ((x) < -k2To15i  ) (x) = -k2To15i;\
}


// Hyperbolic trigonometric functions *not* found in standard C library
#define ln	log										// log is logE
#define asinh(x)	ln((x) + sqrt((x)*(x) + 1))		// -infinite < x < infinite
#define acosh(x)	ln((x) + sqrt((x)*(x) - 1))		// x >= 1
#define asech(x)	acosh(1.0/(x))					// 0 < x < 1
#define acsch(x)	asinh(1.0/(x))					// x != 0
#define acoth(x)	atanh(1.0/(x))					// |x| > 1

#define Ln			ln
#define Asinh		asinh
#define Acosh		acosh
#define Asech		asech
#define Acsch		acsch
#define Acoth		acoth

void SinCosf   (float w, float *outs);		
void CosSinf   (float w, float *outs);
void AbsSinCosf(float w, float *outs);
void AbsCosSinf(float w, float *outs);

//#define MAX(a, b)	((a) > (b) ? (a) : (b))
//#define MIN(a, b)	((a) < (b) ? (a) : (b))

void SetDoubles (double *d, long length, double value);
void SetFloats  (float  *d, long length, float  value);
void SetShorts  (short  *d, long length, short  value);

#define ClearDoubles(p, l)	(SetDoubles((p), (l), 0.0 ))
//#define ClearFloats( p, l)	(SetFloats( (p), (l), 0.0f))
void ClearFloats(float  *d, long length);

//#define ClearShorts( p, l)	(SetShorts( (p), (l), 0   ))
void ClearShorts(short *d, long length);
void ClearLongs (long  *d, long length);

void CopyShorts (short *in, short *out, long length);
void CopyFloats (float *in, float *out, long length);
void CopyLongs  (float *in, float *out, long length);

void ReverseFloats(float *in, float *out,    long length);
void FanOutFloats (float *in, float *outs[], long length, long N);
void FanOutShorts (short *in, short *outs[], long length, long N);

long CompareFloats(float *a, float *b, long length);
long CompareShorts(short *a, short *b, long length);
long CompareLongs (long  *a, long  *b, long length);

short MaxAbsShorts(short *a, long length, long stride);

void PrintFloats	  (float *d, long length);
void PrintFloatsDB	  (float *d, long length);
char PrintFloatsToFile(float *d, long length, char *path);
char PrintFloatsToFile_ZeroClamp(float *d, long length, char *path, float lo, float hi);
char PrintDualAxisFloatsToFile(float *x, float *y, long length, char *path);
//void PrintBinary16(FILE *h, short x);

void PrintFloatLine   (float *d, long length);
void PrintAxisFloats  (float *d, long length, double x0, double x1);
void PrintAxisFloatsDB(float *d, long length, double x0, double x1);
char PrintAxisFloatsToFile(float *d, long length, float loRange, float hiRange, char *path);

void DecibelFloats (float  *in, float  *out, long length);
void DecibelDoubles(double *in, double *out, long length);

void ShortsToFloats		(short *in, float *out, long length);
void FloatsToShorts		(float *in, short *out, long length, int saturate);

void DeinterleaveShorts		    (short *in, short *outL,   short *outR,      long outLength);
void DeinterleaveNShorts		(short *in, short *outs[], long   outLength, int  interleave);
void DeinterleaveShortsToFloats (short *in, float *outL,   float *outR,      long outLength);
void DeinterleaveNShortsToFloats(short *in, float *outs[], long   outLength, int  interleave);

void InterleaveFloatsToShorts (float *inL, float *inR, short *out, long inLength, int saturate, int stride);
void InterleaveNFloatsToShorts(float *ins[], short *out, long inLength, int interleave, int saturate);
void InterleaveNShorts	  	  (short *ins[], short *out, long inLength, int interleave);
void InterleaveShorts         (short *inL, short *inR, short *out, long inLength);

long Clamp (float *in, float *out, long length, float floor, float ceiling, float x);
long GateDB(float *in, float *out, long length, float levelDB, float kDB);
long Gate  (float *in, float *out, long length, float floor, float ceiling);
#define GateZero(i, o, l)	(Gate((i), (o), (l), -1e-12f, 1e-12f))

//void ScaleFloats(float *in, float *out, long length, float k);
//void ScaleShorts(short *in, short *out, long length, float k);


long Bound				    (float *in, float *out, long length, float floor, float ceiling);
void Mask				    (float *in, float *out, long length, long mask);	
void ScaleAdd			    (float *in, float *out, long length, float g);
void ScaleRamp			    (float *in, float *out, long length, float g, float delta);
void ScaleFloatsToShorts	(float *in, short *out, long length, float g, int saturate);
void ScaleFloatsToShortsAsm (float *in, short *out, long length, float g, int saturate);
void ScaleFloatsToLongs     (float *in, long  *out, long length, float g, int saturate);
void ScaleFloatsToLongsAsm  (float *in, long  *out, long length, float g, int saturate);
void Square				    (float *in, float *out, long length);
//#define Square(in, out, length)		Dot((in), (in), (out), (length))
void Envelope				(float *in, float *out, long length, 
									   float attack, float release, float loThreshold, float hiThreshold, float *lastX);

void Scale	  (float *in, float *out, long length, float k);
void Ramp     (float *out, long length, float start, float end);

void ScaleShortsf(short *in, short *out, long length, float k);
void ScaleShortsi(short *in, short *out, long length, float k);
void ScaleShortsi_Q15(Q15 *in, Q15 *out, long length, Q15 k);

void ScaleAddShortsf(short *in, short *out, long length, float k);
void MACShortsi_Q15(Q15 *in, Q15 *out, long length, Q15 k);

void AddShorts(short *in, short *out, long length, long saturate);

// Mixer value functions
void Add2_Shortsi(short *inA, short *inB, short *outY, long length);
void Mix2_Shortsf(short *inA, short *inB, short *outY, long length, float kA, float kB);
void Mix2_Shortsi(short *inA, short *inB, short *outY, long length, short kA, short kB);

void AccS16toS32(S32 *sumY, S16 *inX, long length, long addToOutput);
void AccS16toS16(S16 *sumY, S16 *inX, long length, long addToOutput);

void ShiftLeft_S16 (S16 *inX, S16 *outY, long length, long x);
void ShiftRight_S16(S16 *inX, S16 *outY, long length, long x);

void Pan_Shortsf(short *x, short *yLeft, short *yRight, long length, float gainLeft, float gainRight);
void Pan_Shortsi(short *x, short *yLeft, short *yRight, long length, short gainLeft, short gainRight);

void Add		    (float *a , float *b, float *out, long length);
void Subtract		(float *a , float *b, float *out, long length);
void DotFloats		(float *a , float *b, float *out, long length);
void Mix2Floats     (float *a , float *b, float *out, long length, float gainA, float gainB);
void MixNFloats     (float **ins, float *out, long length, int count, float *gains);

void DecibelFloats(float *in, float *out, long length);
void ScaleDB(float *in, float *out, long length, float gainDB);
void RampDB (float *in, float *out, long length, float gainDB, float deltaDB);
void Dot    (float *inA, float *inB, float *out, long length);
void Sum    (float *inA, float *inB, float *out, long length);

void Clip     (float *in, float *out, long length, float lo, float hi);
void Floor    (float *in, float *out, long length, float lo);
void Ceiling  (float *in, float *out, long length, float hi);
void GateScale(float *in, float *out, long length, float lo, float hi, float k);

void CompressClip(float *in, float *out, long length, float threshold, float limit, float ratio);

// Sampling Rate conversion
void Average2(float *in, float *out, long length);
void AverageN(float *in, float *out, long length, int count);
void Average2Shorts(short *in, short *out, long length);

//
// Noise generation routines
//
void OtherNoise (float *out, long length);
void NoiseWhite (float *out, long length, unsigned long *z, float gain, char additive);
void NoiseRed   (float *out, long length, unsigned long *z, float gain, char additive);
void NoiseViolet(float *out, long length, unsigned long *z, float gain, char additive);

// Cyclical
void Sawtooth(float *out, long length, float floor, float ceiling);
void Triangle(float *out, long length, float floor, float ceiling);
void Pulse   (float *out, long length, float floor, float ceiling, float duty);
void Sine	 (float *out, long length, double amplitude, double phase);


void SetUpSampleNHoldOscillator(unsigned long *z, unsigned long *counter, unsigned long *delta, 
										  float normalPeriod, float phase);
void SetUpSawtoothOscillator   (unsigned long *z, unsigned long *delta, float normalFrequency, float phase);
void PrepareSineOscillator     (double *h, double *z, double amplitude, double normalFrequency, double phase);
void SetUpSquareOscillator     (unsigned long *z, unsigned long *delta, float normalFrequency, float phase);
void SetUpTriangleOscillator   (unsigned long *z, unsigned long *delta, float normalFrequency, float phase);

void SampleNHoldOscillator(float *out, long length, unsigned long *z, unsigned long *counter, unsigned long delta);
void SawtoothOscillator   (float *out, long length, unsigned long *z, unsigned long delta);
void ComputeSineOscillator(float *out, long length, double *z, double *h);
void SquareOscillator	  (float *out, long length, unsigned long *z, unsigned long delta);
void TriangleOscillator   (float *out, long length, unsigned long *z, unsigned long delta);

    void 
SawtoothOscillator_S16(short *outP, long length, unsigned long *z, unsigned long delta);

void BlastSineOscillator(float *out, long length, double *z, double *h, float gain, int *init);

double SamplesToSeconds(int samples, double samplingFrequency);
int    SecondsToSamples(double time, double samplingFrequency);

#define SecondsToMilliSecondsf(x) ((x) * 1000.0f)
#define SecondsToMilliSecondsd(x) ((x) * 1000.0 )
#define SecondsToMilliSeconds	SecondsToMilliSecondsf

#define MilliSecondsToSecondsf(x) ((x) * 0.001f)
#define MilliSecondsToSecondsd(x) ((x) * 0.001 )
#define MilliSecondsToSeconds	MilliSecondsToSecondsf

#ifndef RINT
#define RINT(x) ((int)(x + 0.5))
#endif

FILE *CreateTextFile  (char *path);
FILE *CreateFileOrExit(char *path);

int RemoveCharacter	(char *s, char target);
int RemoveNonNumericals	(char *s);
int IsNumerical		(char *s);
int IsPositiveNumerical (char *s);

// Hex24 routines
int ByteToHex(char c, int capitalize, char *outS);
int FloatToHexFrac24(float x, int capitalize, char *outS);

// Q fixed point integer routines
// Define fixed point multiplication of the high 16-bit halves of
// two 32-bit fixed-point numbers.

Q15 FloatToQ15(float x);
Q31 FloatToQ15v2(float x);
float Q15ToFloat(Q15 x);

Q31 FloatToQ31(float x);
float Q31ToFloat(Q31 x);

__inline Q15 AddQ15 ( Q15 a, Q15 b );
__inline Q15 SubQ15 ( Q15 a, Q15 b );
//__inline Q15 MultQ15( Q15 a, Q15 b );
//__inline Q15 MacQ15 ( Q15 x, Q15 a, Q15 b );

__inline Q31 AddQ31 ( Q31 a, Q31 b );
__inline Q31 SubQ31 ( Q31 a, Q31 b );
__inline Q31 MultQ31( Q31 a, Q31 b );
__inline Q31 MacQ31 ( Q31 x, Q31 a, Q31 b );

void PrintQ15asFloat(Q15 x);
void PrintQ31asFloat(Q31 x);

// *************************************************************** 
// MultQ15:	Integer multiplication
//
//			16x16=32-bit fixed-point implementation
//				1.15 x 1.15 = 1.30
// ***************************************************************
__inline Q15 MultQ15( Q15 a, Q15 b )
{
Q15 y;

//	__asm {
//	    SMULWT    y, a, b
//	    QADD    y, y, y
//	}
// Attempt at GCC version (doesn't compile)
//	asm("smulwt    (product), (x), (y)\n\t"
//	    "qadd    (product), (product), (product)\n\t"
//	);
#ifdef USE_ARM946_DSPEXT
// NOTE:  volatile instructs compiler to skip optimization of assembly instructions
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
#else
z = y + ((a*b)>>15);
#endif

return (z);
}	// ---- end MacQ15() ---- 

// Test for Mobileer FXP31 routines
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

__inline FXP31 testFXP31_MULT( FXP31 x, FXP31 y );

__inline FXP31 testMIX_SCALE_SHIFT_ADD( FXP31 accum_1, FXP31 signal_2, FXP31 gain_3, int shiftby_4 );



void Test_Dsputil();

#ifdef __cplusplus
}
#endif

#endif  // end __DSPUTIL_H__
