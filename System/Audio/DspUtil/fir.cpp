// *************************************************************** 
// fir.cpp:		Code for Finite Impulse Response (FIR) filters
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "util.h"
#include "fir.h"


// FIR low pass pseudo-Half Band filter
// Edges (normalized)       :  0...0.1904761905 , 0.275...0.5 
// Specified deviations (dB): 0.4237859814, -26.02059991
// Worst deviations     (dB): 0.3934868464, -26.6782971
#define kFIR_HalfBandV1_Length 15
static float firHalfBand_V1_GainCompensationDB = -0.393f;
static float firHalfBandHz_V1[kFIR_HalfBandV1_Length] = {
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

// FIR band pass filter 
// Edges (normalized)       : 0...0.125 , 0.1875...0.3125 , 0.375...0.5 
// Specified deviations (dB): -26.02059991, 0.4237859814, -26.02059991
// Worst deviations     (dB): -17.52475309, 1.083071175, -17.52475309
#define kFIR_BandPassHz_Length	15
static float firBandPass_GainCompensationDB = -0.393f;
float firBandPassHz[kFIR_BandPassHz_Length] = {
 1.054007141981783E-016,     
 4.626067510926039E-002,     
 3.284771181906916E-017,    
 1.250000000000000E-001,     
-9.783117603060870E-018,    
-2.962606751092606E-001,     
 4.717363652484429E-017,     
 3.828008305593115E-001,     
 4.717363652484429E-017,     
-2.962606751092606E-001,     
-9.783117603060870E-018,     
 1.250000000000000E-001,     
 3.284771181906916E-017,     
 4.626067510926039E-002,     
 1.054007141981783E-016      
};

// FIR band stop filter 
// Edges (normalized)       : 0...0.125 , 0.1875...0.3125 , 0.375...0.5 
// Specified deviations (dB): 0.4237859814, -26.02059991, 0.4237859814
// Worst deviations     (dB): 0.4433194267, -25.65037324, 0.4433194267
#define kFIR_BandStopHz_Length	17
static float firBandStop_GainCompensationDB = -0.444f;
float firBandStopHz[kFIR_BandStopHz_Length] = {
 5.808898833541933E-002,     
 4.784520410580150E-017,     
-2.745784022961546E-002,     
-1.922226675218418E-017,     
-1.060661931723916E-001,     
-3.953511333669201E-017,     
 2.916808937762975E-001,     
-1.505127721803406E-017,     
 6.195989697824791E-001,     
-1.505127721803406E-017,     
 2.916808937762975E-001,     
-3.953511333669201E-017,     
-1.060661931723916E-001,     
-1.922226675218418E-017,     
-2.745784022961546E-002,     
 4.784520410580150E-017,     
 5.808898833541933E-002      
};

// FIR highpass filter 
// Edges (normalized)       : 0...0.225 , 0.275...0.5 
// Specified deviations (dB): -26.02059991, 0.4237859814
// Worst deviations     (dB): -20.05716543, 0.8213132248
#define kFIR_HighPassHz_Length	15
static float firHighPass_GainCompensationDB = -0.822f;
float firHighPassHz[kFIR_HighPassHz_Length] = {
 7.170315600633288E-002,     
-1.318044505937485E-004,     
-5.754117820612641E-002,     
-1.028839231777762E-004,     
 1.020287792782554E-001,     
-8.722206843596569E-005,     
-3.170894032860515E-001,     
 4.998456180270049E-001,     
-3.170894032860515E-001,     
-8.722206843596569E-005,     
 1.020287792782554E-001,     
-1.028839231777762E-004,     
-5.754117820612641E-002,     
-1.318044505937485E-004,     
 7.170315600633288E-002      
};

// FIR lowpass filter 
// Edges (normalized)       : 0...0.004761904762 , 0.1267857143...0.5 
// Specified deviations (dB): 0.4237859814, -26.02059991
// Worst deviations     (dB): 0.04450512642, -43.69790572
// 15 coefficients: 
#define kFIR_LowPassHz_Length	15
static float firLowPass_GainCompensationDB = -0.045f;
float firLowPassHz[kFIR_LowPassHz_Length] = {
 1.012450863421694E-002,     
 2.135048804652548E-002,     
 3.902844636949630E-002,     
 6.074499819991502E-002,     
 8.367209352087522E-002,     
 1.040691000995740E-001,     
 1.181803454848006E-001,     
 1.232306918154969E-001,     
 1.181803454848006E-001,     
 1.040691000995740E-001,     
 8.367209352087522E-002,     
 6.074499819991502E-002,     
 3.902844636949630E-002,     
 2.135048804652548E-002,     
 1.012450863421694E-002      
};

// Low Pass FIR Half Band filter
// Stop Band Ripple               : -32.8 dB
// Pass band as fraction of band  : 0.8
// Weight on transition band error: 1.0E-6
#define kFIR_LowPass_HalfBandHz_Length	15
static float firLowPass_HalfBand_GainCompensationDB = 0.0f;
float firLowPass_HalfBandHz[kFIR_LowPass_HalfBandHz_Length] = {
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
// DefaultFIR:		Set Default high-level parameter values
// ***************************************************************
    void 
DefaultFIR(FIR *d)
{
//long i = 0;

d->type = kFIR_Type_LowPass;
d->outLevelDB = 0.0f;

d->samplingFrequency = 1.0f;
}	// ---- end DefaultFIR() ---- 

// *************************************************************** 
// UpdateFIR:	Convert high-level parameter values to low level data
// ***************************************************************
    void 
UpdateFIR(FIR *d)
{
long i = 0;

switch (d->type)
	{
	default:
	case kFIR_Type_LowPass:
		d->order =  kFIR_LowPassHz_Length;
		d->outLevelDB = firLowPass_GainCompensationDB;
		CopyFloats(firLowPassHz, d->h, d->order);
	break;
	case kFIR_Type_HighPass:
		d->order =  kFIR_HighPassHz_Length;
		d->outLevelDB = firHighPass_GainCompensationDB;
		CopyFloats(firHighPassHz, d->h, d->order);
	break;
	case kFIR_Type_BandPass:
		d->order =  kFIR_BandPassHz_Length;
		d->outLevelDB = firBandPass_GainCompensationDB;
		CopyFloats(firBandPassHz, d->h, d->order);
	break;
	case kFIR_Type_BandStop:
		d->order =  kFIR_BandStopHz_Length;
		d->outLevelDB = firBandStop_GainCompensationDB;
		CopyFloats(firBandStopHz, d->h, d->order);
	break;
	case kFIR_Type_HalfBand:
		d->order =  kFIR_LowPass_HalfBandHz_Length;
		d->outLevelDB = firLowPass_HalfBand_GainCompensationDB;
		CopyFloats(firLowPass_HalfBandHz, d->h, d->order);
	break;
	}
d->outLevel = DecibelToLinear(d->outLevelDB);

// Scale coefficients by output level
for (i = 0; i < d->order; i++)
	d->h[i] *= d->outLevel;

// Convert 32-bit floating point coefficients to signed 16-bit fixed point
for (i = 0; i < d->order; i++)
	{
	d->hI[i] = (short)(32767.0f * d->h[i]);
//	printf("UpdateFIR  hI[%2d] = %d <- %g \n", i, d->hI[i], d->h[i]);
	}

//printf("UpdateFIR: outLevelDB = %g -> %g \n", d->outLevelDB, d->outLevel);
}	// ---- end UpdateFIR() ---- 

// *************************************************************** 
// ResetFIR:	Reset unit to initial state
// ***************************************************************
    void 
ResetFIR(FIR *d)
{
long i;

for (i = 0; i < kFIR_MaxDelayElements; i++)
	d->z[i] = 0;
}	// ---- end ResetFIR() ---- 

// *************************************************************** 
// PrepareFIR:	Update() + Reset()
// ***************************************************************
    void 
PrepareFIR(FIR *d)
{
UpdateFIR(d);
ResetFIR(d);
}	// ---- end PrepareFIR() ---- 

// *************************************************************** 
// RunFIR_Shorts:	Main calculation routine, 32-bit floating point
// ***************************************************************
	void
RunFIR_Shorts(short *in, short *out, long length, FIR *d)
{
long    i, j;
long lAcc;
short *zP = in;

//{static long c=0; printf("RunFIR_Shorts %d \n", c++);}

for (i = 0; i < length; i++)
	{
	float acc = 0.0f;
        zP = &in[i-(d->order-1)];
// Compute FIR filter 
        for (j = 0; j < d->order; j++)
		acc += d->h[j]*(float)zP[j];

// Saturate and output:  
	lAcc = (long) acc;
	if      (lAcc >  32767)
		lAcc = 32767;
	else if (lAcc < -32768)
		lAcc = -32768;
	out[i] = (short) lAcc;
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->order; i++)
	in[-1-i] = in[length-1-i];
}	// ---- end RunFIR_Shorts() ---- 

// *************************************************************** 
// RunFIRFixedPoint_Shorts:	Main calculation routine, 16-bit fixed point
// ***************************************************************
	void
RunFIRFixedPoint_Shorts(short *in, short *out, long length, FIR *d)
{
long i;
{static long c=0; printf("RunFIRFixedPoint_Shorts %d \n", c++);}

for (i = 0; i < length; i++)
	{
	long lAcc = 0;
        short *zP = &in[i-(d->order-1)];
// Compute FIR filter 
        for (long j = 0; j < d->order; j++)
		lAcc += d->hI[j]*zP[j];

// Saturate and output:  FIXXX add guard bits to accumulator, then add saturation code
	lAcc = lAcc>>15;  // instead of 16 for guard bits, TEMPORARY
//	if      (lAcc >  (32767))
//		lAcc = (32767);
//	else if (lAcc < (-32768)
//		lAcc = (-32768;
	out[i] = (short) lAcc;
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->order; i++)
	in[-1-i] = in[length-1-i];
}	// ---- end RunFIRFixedPoint_Shorts() ---- 

