// **********************************************************************
//
// eq.cpp		Routines for Audio Equalizer
//
//					Written by Gints Klimanis
// **********************************************************************
#include <string.h>

#include "eq.h"

// ************************************************************************
// ComputeEQ_LowPassHz:	Compute coefficiepnts for low pass filter
// ************************************************************************ 
    void
ComputeEQ_LowPassHz(float *h, float frequency)
// frequency		fc/fs	normalized frequency
{
float b[3], a[3];
float wc    = kTwoPi*frequency;
float alpha = sinh(1.0/(2.0*kEQ_DefaultQ))*sin(wc);
float cosWc = cos(wc);
float a0m1;

// b1=(1-cosWc)   b0=b2=b1/2
b[1] = (1.0 - cosWc);
b[0] = b[1]*0.5;
//b[2] = (1.0 - cosWc)*0.5;	NOTE: b2 = b0

a[0] =  1.0 + alpha;
a[1] = -2.0*cosWc;
a[2] =  1.0 - alpha;

a0m1 = 1.0/a[0];
h[0] = b[0]*a0m1;
h[1] = b[1]*a0m1;
h[2] = h[0];

h[3] = a[1]*a0m1;
h[4] = a[2]*a0m1;
}   // ---- end ComputeEQ_LowPassHz() ---- 

// ************************************************************************
// ComputeEQ_HighPassHz:	Compute coefficients for high pass filter
// ************************************************************************ 
    void
ComputeEQ_HighPassHz(float *h, float frequency)
// frequency		fc/fs	normalized frequency
{
float b[3], a[3];
float wc    = kTwoPi*frequency;
float alpha = sinh(1.0/(2.0*kEQ_DefaultQ))*sin(wc);
float cosWc = cos(wc);
float a0m1;

b[0] =  (1.0 + cosWc)*0.5;
b[1] = -(1.0 + cosWc);
//b[2] =  (1.0 + cosWc)*0.5;NOTE: b2 = b0

a[0] =  1.0 + alpha;
a[1] = -2.0*cosWc;
a[2] =  1.0 - alpha;

a0m1 = 1.0/a[0];
h[0] = b[0]*a0m1;
h[1] = b[1]*a0m1;
h[2] = h[0];

h[3] = a[1]*a0m1;
h[4] = a[2]*a0m1;
}   // ---- end ComputeEQ_HighPassHz() ---- 

// ************************************************************************
// ComputeEQ_LowShelfHz:	Compute coefficients for low shelf filter
// ************************************************************************ 
    void
ComputeEQ_LowShelfHz(float *h, float frequency, float slope, float gainDB)
// frequency		fc/fs	normalized frequency
// slope			range [0 .. 1] (or else you get special fx curves)
{
float b[3], a[3];
float G     = pow(10.0, gainDB*(1.0/40.0));
float wc    = kTwoPi*frequency;
float beta  = sqrt((G*G + 1.0)/slope - (G - 1.0)*(G - 1.0));
float cosWc = cos(wc);
float sinWc = sin(wc);
float a0m1;
//printf("ComputeEQ_LowShelfHz: fc=%g slope=%g gainDB=%g\n", frequency, slope, gainDB);

b[0] =     G*((G+1.0) - (G-1.0)*cosWc + beta*sinWc);
b[1] = 2.0*G*((G-1.0) - (G+1.0)*cosWc);
b[2] =     G*((G+1.0) - (G-1.0)*cosWc - beta*sinWc);

a[0] =        (G+1.0) + (G-1.0)*cosWc + beta*sinWc;
a[1] = -2.0 *((G-1.0) + (G+1.0)*cosWc);
a[2] =        (G+1.0) + (G-1.0)*cosWc - beta*sinWc;

a0m1 = 1.0/a[0];
h[0] = b[0]*a0m1;
h[1] = b[1]*a0m1;
h[2] = b[2]*a0m1;

h[3] = a[1]*a0m1;
h[4] = a[2]*a0m1;
}   // ---- end ComputeEQ_LowShelfHz() ---- 

// ************************************************************************
// ComputeEQ_HighShelfHz:	Compute coefficients for high shelf filter
// ************************************************************************ 
    void
ComputeEQ_HighShelfHz(float *h, float frequency, float slope, float gainDB)
// frequency		fc/fs	normalized frequency
// slope			range [0 .. 1] (or else you get special fx curves)
{
float b[3], a[3];
float G     = pow(10.0, -gainDB*(1.0/40.0));
float wc    = kTwoPi*frequency;
float beta  = sqrt((G*G + 1.0)/slope - (G - 1.0)*(G - 1.0));
float cosWc = cos(wc);
float sinWc = sin(wc);
float a0m1;

b[0] =         (G+1.0) - (G-1.0)*cosWc + beta*sinWc;
b[1] =  2.0*  ((G-1.0) - (G+1.0)*cosWc);
b[2] =         (G+1.0) - (G-1.0)*cosWc - beta*sinWc;

a[0] =      G*((G+1.0) + (G-1.0)*cosWc + beta*sinWc);
a[1] = -2.0*G*((G-1.0) + (G+1.0)*cosWc);
a[2] =      G*((G+1.0) + (G-1.0)*cosWc - beta*sinWc);

a0m1 = 1.0/a[0];
h[0] = b[0]*a0m1;
h[1] = b[1]*a0m1;
h[2] = b[2]*a0m1;

h[3] = a[1]*a0m1;
h[4] = a[2]*a0m1;
}   // ---- end ComputeEQ_HighShelfHz() ---- 

// ************************************************************************
// ComputeEQ_ParametricHz:	Compute coefficients for parametric equalizer
// ************************************************************************ 
    void
ComputeEQ_ParametricHz(float *h, float frequency, float q, float gainDB)
// frequency		fc/fs	normalized frequency
// q				range [sqrt(2)/2 .. N]
{
float b[3], a[3];
float G     = pow(10.0, gainDB*(1.0/40.0));
float wc    = kTwoPi*frequency;
float alpha = sinh(1.0/(q+q))*sin(wc);
float cosWc = cos(wc);
float a0m1, Gm1;

b[0] =  1.0 + alpha*G;
b[1] = -2.0 * cosWc;
b[2] =  1.0 - alpha*G;

Gm1 = alpha/G;
a[0] =  1.0 + Gm1;
//a[1] =  -2.0 * cosWc;
a[2] =  1.0 - Gm1;

a0m1 = 1.0/a[0];
h[0] = b[0]*a0m1;
h[1] = b[1]*a0m1;
h[2] = b[2]*a0m1;

h[3] = h[1];	// a1 = b1
h[4] = a[2]*a0m1;
}   // ---- end ComputeEQ_ParametricHz() ----
// ************************************************************************
// ComputeEQHz:	Compute coefficients for specified filter mode
// ************************************************************************ 
    void
ComputeEQHz(float *h, float frequency, float q, float gainDB, int mode)
// frequency		fc/fs	normalized frequency
{
switch (mode)
	{
	case kEQ_Mode_ByPass:
		h[kEQ_b0] = 1.0;
		h[kEQ_b1] = 0.0;
		h[kEQ_b2] = 0.0;
		h[kEQ_a1] = 0.0;
		h[kEQ_a2] = 0.0;
	break;
	case kEQ_Mode_LowPass:
		ComputeEQ_LowPassHz(h, frequency);
	break;
	case kEQ_Mode_HighPass:
		ComputeEQ_HighPassHz (h, frequency);
	break;
	case kEQ_Mode_LowShelf:
		ComputeEQ_LowShelfHz(h, frequency, q, gainDB);
	break;
	case kEQ_Mode_HighShelf:
		ComputeEQ_HighShelfHz (h, frequency, q, gainDB);
	break;

	case kEQ_Mode_Parametric:
		ComputeEQ_ParametricHz(h, frequency, q, gainDB);
	break;
	default:
		Printf("ComputeEQHz: bogus filter mode=%d\n", mode);
	break;
	}

//printf("ComputeEQHz: h(z)=<%f %f %f %f %f> mode=%d\n", h[0], h[1], h[2], h[3], h[4], mode);
}   // ---- end ComputeEQHz() ---- 

// ************************************************************************
// DefaultEQ:	 Default parameter set
// ************************************************************************ 
    void
DefaultEQ(EQ *d)
{
d->frequency = 5705.0f; 
d->q         = 20.0f;
d->gainDB    =  0.0f;
d->mode      = kEQ_Mode_LowPass;

d->lowLevelDataBogus = True;
d->samplingFrequency = kBogusSamplingFrequency;
}	// ---- end DefaultEQ() ---- 

// ************************************************************************
// SetEQ_Parameters:	Set high-level parameters 
// ************************************************************************ 
	void 
SetEQ_Parameters(EQ *d, float frequency, float q, float gainDB, int mode)
// frequency		*not* normalized to sampling rate
// q 		        Q or Slope, depending on EQ mode
{

//if (0.0 != dFc || 0.0 != dQ || 0.0 != dGainDB || d->mode != mode)
	d->lowLevelDataBogus = True;

#ifdef SAFE
printf("\SetEQ_Parameters:  fc=%f,  q=%f,  gainDB=%f mode='%s'\n",
		d->frequency, d->q, d->gainDB, TranslateEQ_ModeID(d->mode));
printf("SetEQ_Parameters: Nfc=%f, Nq=%f, NgainDB=%f Nmode='%s'\n",
		frequency, q, gainDB, TranslateEQ_ModeID(d->mode));
printf("SetEQ_Parameters: dFc=%f, dQ=%f, dGainDB=%g\n", dFc, dQ, dGainDB);
#endif

d->frequency = frequency;
d->q         = q;
d->gainDB    = gainDB;
d->mode      = mode;
}	// ---- end SetEQ_Parameters() ---- 

// ************************************************************************
// SetEQ_SamplingFrequency:	Set module sampling frequency 
// ************************************************************************ 
	void 
SetEQ_SamplingFrequency(EQ *d, float samplingFrequency)
{
d->lowLevelDataBogus = (d->samplingFrequency != samplingFrequency);
d->samplingFrequency      = samplingFrequency;
// FIXXX: add bounding code
}	// ---- end SetEQ_SamplingFrequency() ---- 

// ************************************************************************
// SetEQ_Frequency: 
// ************************************************************************ 
	void 
SetEQ_Frequency(EQ *d, float x)
// frequency	Hertz,	not normalized to sampling rate
{
// Delta = old value - new value
float dFc = x - d->frequency;

if (0.0 != dFc)
	d->lowLevelDataBogus = True;
d->frequency = x;
}	// ---- end SetEQ_Frequency() ---- 

// ************************************************************************
// SetEQ_Q: 
// ************************************************************************ 
	void 
SetEQ_Q(EQ *d, float x)
{
// Delta = old value - new value
float delta = x - d->q;

if (0.0 != delta)
	d->lowLevelDataBogus = True;
d->q = x;
}	// ---- end SetEQ_Q() ---- 

// ************************************************************************
// SetEQ_GainDB: 
// ************************************************************************ 
	void 
SetEQ_GainDB(EQ *d, float x)
{
// Delta = old value - new value
float delta = x - d->gainDB;

if (0.0 != delta)
	d->lowLevelDataBogus = True;
d->gainDB = x;
}	// ---- end SetEQ_GainDB() ---- 

// ************************************************************************
// AdjustGainEQHz:	Adjust gain of coefficients 
//
//				Note:  does not alter gain parameter in data structure
//				Also, does not engage inter-sample ramping code
// ************************************************************************ 
    void
AdjustGainEQHz(float *h, float gainDB)
{
float k = DecibelToLinear(gainDB);

//Printf("AdjustGainEQHz: gainDB=%g -> gain=%g\n", gainDB, gain);

h[kEQ_b0] *= k;
h[kEQ_b1] *= k;
h[kEQ_b2] *= k;
}	// ---- end AdjustGainEQHz() ---- 

// ************************************************************************
// DampingFromQ:	Return damping value for specified Q
// ************************************************************************ 
    float
DampingFromQ(float q)
{
return (2.0*sinh(1.0/(q+q)));
}   // ---- end DampingFromQ() ---- 

// ************************************************************************
// QFromDamping:	Return Q value for specified damping
// ************************************************************************ 
//    float
//QFromDamping(float damping)
//{
//return (0.5/Asinh(0.5*damping));
//}   // ---- end QFromDamping() ---- 

// ************************************************************************
// DampingFromQ_v2:	Return damping value for specified Q
// ************************************************************************ 
//    float
//DampingFromQ_v2(float frequency, float q)
//{
//float w0 = kTwoPi*frequency;
//return (tan(w0/(q+q))/sin(w0));
//}   // ---- end DampingFromQ_v2() ---- 

// ************************************************************************
// QFromDamping_v2:	Return Q value for specified damping
// ************************************************************************ 
//    float
//QFromDamping_v2(float frequency, float damping)
//{
//float w0 = kTwoPi*frequency;
//
//return (0.5*w0/atan(damping*sin(w0)));
//}   // ---- end QFromDamping_v2() ---- 

// ************************************************************************
// UpdateEQ:	Compute coefficients (when  lowLevelDataBogus=True)
//				NOTE: does not reset state
// ************************************************************************ 
    void
UpdateEQ(EQ *d)
{
int		i;
float	fc;

if (kBogusSamplingFrequency == d->samplingFrequency)
	Printf("UpdateEQ: Hey.  SamplingFrequency=%g not set !\n", d->samplingFrequency);

// Limit frequency to 99% of bandwidth = 
fc = d->frequency/d->samplingFrequency;
//if (fc > 0.99*0.5*d->samplingFrequency)
//	d->mode = kEQ_Mode_ByPass;
if (fc > 0.99*0.5*d->samplingFrequency)
	fc = 0.99*0.5*d->samplingFrequency;

//if (d->lowLevelDataBogus)
	{
//	if (2 == d->order)
//		dspComputeEQHz(d->h[0], d->frequency/d->samplingFrequency, d->q, d->gainDB, d->mode);
	}

ComputeEQHz(d->hf, fc, d->q, d->gainDB, d->mode);
for (i = 0; i < kEQ_Coeffs; i++)
    {
	d->h32[i] = FloatToQ31(0.5f*d->hf[i]);   // Really, just 24 bits from 32-bit float
	d->h16[i] = (d->h32[i])>>16;
    }

//#define EQ_CHECK_HZ
#ifdef EQ_CHECK_HZ
printf("---- UpdateEQ: %d ----\n");
printf("mode=%d '%s' fc=%g/%g q=%g gainDB=%g\n", d->mode, TranslateEQ_ModeID(d->mode), d->frequency, d->samplingFrequency, d->q, d->gainDB);

//printf("%d b0  %g\n", kEQ_b0, d->hf[kEQ_b0]);
//printf("%d b1  %g\n", kEQ_b1, d->hf[kEQ_b1]);
//printf("%d b2  %g\n", kEQ_b2, d->hf[kEQ_b2]);
//printf("%d a1  %g\n", kEQ_a1, d->hf[kEQ_a1]);
//printf("%d a2  %g\n", kEQ_a2, d->hf[kEQ_a2]);
for (i = 0; i < kEQ_Coeffs; i++)
    printf("%d %g\n", i, d->hf[i]);
//printf("\n");
//for (i = 0; i < kEQ_Coeffs; i++)
//    printf("%d %04X -> %g\n", i, 0xffff & d->h16[i], Q15ToFloat(d->h16[i]));
//printf("\n");
//for (i = 0; i < kEQ_Coeffs; i++)
//    printf("%d %08X -> %g\n", i, d->h32[i], Q31ToFloat(d->h32[i]));
#endif

d->lowLevelDataBogus = False;
}	// ---- end UpdateEQ() ---- 

// ************************************************************************
// ResetEQ:	 Reset state and delta parameters
// ************************************************************************ 
    void
ResetEQ(EQ *d)
{
// Clear delay elements
for (int i = 0; i < kEQ_Zs; i++)
    {
    d->zf[i] = 0.0;

    d->z16[i] = 0;
    d->z32[i] = 0;
    }
}	// ---- end ResetEQ() ---- 

// ************************************************************************
// PrepareEQ:	Update() + Reset() + one-time low level initializations
// ************************************************************************ 
    void
PrepareEQ(EQ *d)
{
d->lowLevelDataBogus = True;

UpdateEQ(d);
ResetEQ (d);

//PrintEQ(d, kPrintLevelAll, NULL);
}	// ---- end PrepareEQ() ---- 

// ************************************************************************
// ComputeEQf: 2nd Order IIR bi-quadratic
// ************************************************************************ 
    void
ComputeEQf(S16 *x, S16 *y, long length, EQ *d)
{
float *h = d->hf;	// Filter transfer function: {b0, b1, b2, a1, a2}
float *z = d->zf;

//{static long c=0; printf("ComputeEQf %d: start \n", c++);}
/*
**************** Direct Form I (Good for audio use) ************
(single truncation point:  accumulator 'sum')

x  ---> b0 -> sum ->--------->
    |        ^^ ^^         | 
	v        || ||         v 
   ___       || ||        ___
  | -1|      || ||       | -1|
  |z  |      || ||       |z  |
   ---       || ||        ---
    |        || ||         | 
    |-> b1---|| ||-- -a1 <-| 
    |         | |          | 
	v         | |          v 
   ___        | |         ___
  | -1|       | |        | -1|
  |z  |       | |        |z  |
   ---        | |         ---
	|		  | |          |
	 -> b2 ---- ---- -a2 <-								Feedback
Topology		Delay Units		Accumulators	AudioQuality
-------------   ----------		------------    ------------
DirectForm			4			      1	            good
Canonical			2 				  2              bad
DirectFormTranspose	4				  1             good
CanonicalTranspose	2				  2              bad
 */
// Topology: DirectForm: 
for (long i = 0; i < length; i++)
	{
	float sum  = (h[0]*((float)x[i]) + h[1]*z[0] + h[2]*z[1] - h[3]*z[2] - h[4]*z[3]);

// Update unit delays
	z[kEQ_y2] = z[kEQ_y1];
	z[kEQ_y1] = sum;
	z[kEQ_x2] = z[kEQ_x1];
	z[kEQ_x1] = x[i];
	y[i] = (S16) sum;	// For in-place operation, output must follow delay updates 
	}
}   // ---- end ComputeEQf() ---- 

// ************************************************************************
// ComputeEQf2:     2nd Order IIR bi-quadratic
// ************************************************************************ 
    void
ComputeEQf2(S16 *x, S16 *y, long length, EQ *d)
{
float *h = d->hf;	// Filter transfer function: {b0, b1, b2, a1, a2}
float *z = d->zf;
float    k = 0.5f;
float invK = 1.0f/k;

//printf("ComputeEQf2: here\n");

// Topology: DirectForm: 
for (long i = 0; i < length; i++)
	{
	float sum  = 0.0f;
    sum += k*h[0]*(float)x[i];
    sum += k*h[1]*z[0];
    sum += k*h[2]*z[1];
    sum -= k*h[3]*z[2];
    sum -= k*h[4]*z[3];
    sum *= k;

// Update unit delays
	z[kEQ_y2] = z[kEQ_y1];
	z[kEQ_y1] = sum;
	z[kEQ_x2] = z[kEQ_x1];
	z[kEQ_x1] = x[i];
	y[i] = (S16) (sum);	// For in-place operation, output must follow delay updates 
	}
}   // ---- end ComputeEQf2() ---- 

// ************************************************************************
// ComputeEQi: 2nd Order IIR bi-quadratic
//                  Topology: Direct form implementation
// ************************************************************************ 
    void
ComputeEQi(Q15 *x, Q15 *y, long length, EQ *d)
{
Q15  *h = d->h16;	// Filter transfer function: {b0, b1, b2, a1, a2}
Q15  *z = d->z16;

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
}   // ---- end ComputeEQi() ---- 

// ************************************************************************
// ComputeEQ: 
// ************************************************************************ 
    void
ComputeEQ(Q15 *x, Q15 *y, long length, EQ *d)
{
if (d->useFixedPoint)
    ComputeEQi(x, y, length, d);
else
    ComputeEQf(x, y, length, d);
}   // ---- end ComputeEQ() ---- 

// ********************************************************************** 
// TranslateEQ_ModeID:   Translate ID to English string
// ********************************************************************** 
    char * 
TranslateEQ_ModeID(int id)
{
switch (id)
	{
	 case kEQ_Mode_ByPass:
		return ("ByPass");

	 case kEQ_Mode_LowPass:
		return ("LowPass");
	 case kEQ_Mode_HighPass:
		return ("HighPass");

	 case kEQ_Mode_LowShelf:
		return ("LowShelf");
	 case kEQ_Mode_HighShelf:
		return ("HighShelf");

	 case kEQ_Mode_Parametric:
		return ("Parametric");
	}
return ("Bogus");
}	// ---- end TranslateEQ_ModeID() ---- 
