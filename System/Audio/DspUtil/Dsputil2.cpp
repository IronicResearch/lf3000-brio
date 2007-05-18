// **********************************************************************
//
// DspUtil2.cpp		
//
//					Written by Gints Klimanis
// **********************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "Dsputil.h"
#include "Dsputil2.h"

#define kSamplingFrequency  44100.0f

// **********************************************************************
// DefaultOscillator:  
// ********************************************************************** 
	void
DefaultOscillator(OSCILLATOR *d)
{
int i;
d->waveform = kOscillatorWaveform_Triangle;		

d->channels  = 1;
d->gain		 = 1.0f;
d->frequency = 1.0f;				// Hertz
d->phase	 = kHalfPi;
d->bias      = 0.0f;
d->pitch     = 0.0f;

#ifdef OSC_USE_WAVEUTIL
waveFileDefault(&d->waveFile);
#endif
for (i = 0; i < MAX_CHANNELS; i++)
	d->sampleData[i] = NULL;
d->sampleLength = 0;
d->sampleTime   = 10.0f;
d->loopType     = kOscillator_LoopType_Off; //Forward;
d->interpolationMode = kOscillator_Interpolation_Linear;
d->outToInRatio   = 1.0f;

d->modSources[0] = NULL;
d->modSources[1] = NULL;

d->sampleDataAllocatedInternally = False;
d->lowLevelDataBogus = True;
d->done = False;
d->samplingFrequency = kBogusSamplingFrequency;
}	// ---- end DefaultOscillator() ---- 

// **********************************************************************
// SetOscillator:	  Set parameters 								
// ********************************************************************** 
	void
SetOscillator(OSCILLATOR *d, float frequency, float gain, int waveform) 
//	frequency		Hertz, not normalized to sampling frequency
{
d->frequency = frequency;
d->gain      = gain;
d->waveform  = waveform;
}	   // ---- end SetOscillator() ---- 

// ************************************************************************
// SetOscillator_SamplingFrequency:	Set module sampling frequency 
// ************************************************************************ 
	void 
SetOscillator_SamplingFrequency(OSCILLATOR *d, double x)
{
d->lowLevelDataBogus = (d->samplingFrequency != x);
d->samplingFrequency = x;
}	// ---- end SetOscillator_SamplingFrequency() ---- 

// ************************************************************************
// SetOscillator_SamplePitch:	Set sample playback pitch 
// ************************************************************************ 
	void 
SetOscillator_SamplePitch(OSCILLATOR *d, float semitones)
{
d->pitch     = semitones;
d->frequency = (float) PitchToFrequency(semitones, 440.0);

d->outToInRatio = (float) EqualTemperToRatio(semitones);
d->inToOutRatio = 1.0f/d->outToInRatio;

//printf("SetOscillator_SamplePitch: semitones %g -> ratio =%g\n", semitones, d->outToInRatio);
}	// ---- end SetOscillator_SamplePitch() ---- 

// ************************************************************************
// SetOscillator_SamplePlayRate:	Set sample playback rate:   out/in 
// ************************************************************************ 
	void 
SetOscillator_SamplePlayRate(OSCILLATOR *d, float ratio)
{
d->outToInRatio = ratio;
d->inToOutRatio = 1.0f/d->outToInRatio;
}	// ---- end SetOscillator_SamplePlayRate() ---- 

// ************************************************************************
// Oscillator_LoadSample:	Load oscillator WAV file 
//
//					Return Boolean success
// ************************************************************************ 
	int 
Oscillator_LoadSample(OSCILLATOR *d, char *filePath)
{
int		i;

if (d->sampleDataAllocatedInternally)
	{
	for (i = 0; i < MAX_CHANNELS; i++)
		{
		if (d->sampleData[i])
			{
			free(d->sampleData[i]);
			d->sampleData[i] = NULL;
			}
		}
	}
d->sampleDataAllocatedInternally = False;
d->sampleLength = 0;

if (!strcmp(filePath, kOscillator_AllocatedWaveform_Sine))
	{
	d->sampleLength = (long)(0.5 + d->sampleTime * d->samplingFrequency);
	}
else
	{
#ifdef OSC_USE_WAVEUTIL
	if (!waveFileOpenIn(&d->waveFile, filePath))
		{
		printf("Oscillator_LoadSample: unable to load file '%s'\n", filePath);
		return (False);
		}

	// Read the entire file into buffer
	d->channels     = waveFileGetChannels(&d->waveFile);
	d->sampleLength = waveFileGetSampleCount(&d->waveFile)/d->channels;
#endif
	d->sampleData[Left ]   = (short *) malloc((d->sampleLength + 40)*sizeof(short));
	d->sampleData[Right]   = (short *) malloc((d->sampleLength + 40)*sizeof(short));
	if (!d->sampleData[Left])
		{
#ifdef OSC_USE_WAVEUTIL
		waveFileCloseIn(&d->waveFile);
#endif
		return (False);
		}
	d->sampleDataAllocatedInternally = True;
	if (2 == d->channels)
		{
		short *tmpStereo = (short *) malloc(2*(d->sampleLength + 40)*sizeof(short));
#ifdef OSC_USE_WAVEUTIL
		waveFileReadSamples(&d->waveFile, tmpStereo, 2*d->sampleLength);
#endif
		DeinterleaveShorts (tmpStereo, d->sampleData[Left ], d->sampleData[Right], d->sampleLength);
		free(tmpStereo);
		}
	else
	{
#ifdef OSC_USE_WAVEUTIL
		waveFileReadSamples(&d->waveFile, d->sampleData[Left], d->sampleLength);
#endif
	}

//printf("%d channels, %d samples ,'%s'\n", d->channels, d->sampleLength, filePath);
#ifdef OSC_USE_WAVEUTIL
	waveFileCloseIn(&d->waveFile);
#endif
	}

return (True);
}	// ---- end Oscillator_LoadSample() ---- 

// **********************************************************************
// UpdateOscillator:	  Compute coefficients 
//				NOTE: does not reset state 								
// **********************************************************************
	void
UpdateOscillator(OSCILLATOR *d) 
{
if (kBogusSamplingFrequency == d->samplingFrequency)
	printf("UpdateOscillator: Hey.  samplingFrequency=%g not set !\n", d->samplingFrequency);

// Update OSCILLATOR frequency
switch (d->waveform)
	{
	case kOscillatorWaveform_Sine:
		{
		d->h[0] =  cos(kTwoPi*d->frequency/d->samplingFrequency);
		d->z[0] =  sin(d->phase)*sqrt((1.0 - d->h[0])/(1.0 + d->h[0]));
		d->z[1] = -cos(d->phase);
		}
	break;
	case kOscillatorWaveform_PinkNoise:
// 3 pole, 3 zero IIR filter:  +/- 0.3 dB of True pink over 10 octaves
		d->h[0] =  65257.0/65536.0; // pole1
		d->h[1] =  62122.0/65536.0; // pole2
		d->h[2] =  35106.0/65536.0; // pole3
		d->h[3] =  64516.0/65536.0; // zero1
		d->h[4] =  54652.0/65536.0; // zero2
		d->h[5] =   4960.0/65536.0; // zero3
	break;
	default:
	case kOscillatorWaveform_Triangle:
	case kOscillatorWaveform_SawtoothUp:
	case kOscillatorWaveform_SawtoothDown:
	case kOscillatorWaveform_Square:
//		d->udelta = (unsigned long)(d->frequency/d->samplingFrequency);
		d->udelta = DoubleToULong(d->frequency/d->samplingFrequency);
	break;
	case kOscillatorWaveform_SampleNHold:
		d->udelta  = (unsigned long) (0.5f + d->samplingFrequency/d->frequency);
		d->counter = d->udelta;
	break;
	}
//printf("UpdateOscillator: fc=%g fs=%g udelta=%X\n", d->frequency, d->samplingFrequency, d->udelta);
}	   // ---- end UpdateOscillator() ---- 

// **********************************************************************
// ResetOscillator:  	 Reset state
// ********************************************************************** 
	void
ResetOscillator(OSCILLATOR *d)
{
//{static long c=0; printf("ResetOscillator%d: start waveform=%d\n", c++, d->waveform);}

switch (d->waveform)
	{
	case kOscillatorWaveform_SamplePlay:
			d->index  = 0.0;
			d->iIndex = 0;
			d->done   = False;
	break;

	case kOscillatorWaveform_Sine:
		{
		double amplitudeH = sqrt((1.0 - d->h[0])/(1.0 + d->h[0]));
		d->z[0] =  sin(d->phase)*amplitudeH;
		d->z[1] = -cos(d->phase);
		}   
	break;
	case kOscillatorWaveform_WhiteNoise:
	case kOscillatorWaveform_PinkNoise:
	case kOscillatorWaveform_RedNoise:
	case kOscillatorWaveform_VioletNoise:
		{
		for (int i = 0; i < kOscillator_MaxDelayElements; i++)
			d->z[i] = 0.0;
		d->uz = (unsigned long)(0.5 + kTwoTo32m1*d->phase/kTwoPi);
		}   
	break;

	case kOscillatorWaveform_SampleNHold:
		d->uz      = (unsigned long)(0.5 + kTwoTo32m1*d->phase/kTwoPi);
		d->counter = d->udelta;
	break;

	case kOscillatorWaveform_Triangle:
	case kOscillatorWaveform_SawtoothUp:
	case kOscillatorWaveform_SawtoothDown:
	case kOscillatorWaveform_Square:
	default:
		d->uz = (unsigned long)(0.5 + kTwoTo32m1*d->phase/kTwoPi);
	//printf("ResetOscillator: phase=%g -> %g %X\n", d->phase, (double)d->uz, d->uz);
	break;
	}
}	// ---- end ResetOscillator() ---- 

// **********************************************************************
// PrepareOscillator:  	 Update() + Reset() + one-time low level initializations
// ********************************************************************** 
	void
PrepareOscillator(OSCILLATOR *d)
{
d->lowLevelDataBogus = True;
UpdateOscillator(d);
ResetOscillator (d);
}	// ---- end PrepareOscillator() ---- 

// **********************************************************************
// ComputeOscillator:	   Return single value of Oscillator function							
// **********************************************************************
	void 
ComputeOscillator(float *out, long length, OSCILLATOR *d, int addToOutput)
{
switch (d->waveform)
	{
	case kOscillatorWaveform_Sine:
		ComputeOscillator_Sine        (out, length, d, addToOutput);
	break;

	case kOscillatorWaveform_Triangle:
		ComputeOscillator_Triangle    (out, length, d, addToOutput);
	break;
	case kOscillatorWaveform_SawtoothDown:
		ComputeOscillator_SawtoothDown(out, length, d, addToOutput);
	break;
	case kOscillatorWaveform_SawtoothUp:
		ComputeOscillator_SawtoothUp  (out, length, d, addToOutput);
	break;
	case kOscillatorWaveform_Square:
		ComputeOscillator_Square      (out, length, d, addToOutput);
	break;

	case kOscillatorWaveform_WhiteNoise:
		ComputeOscillator_WhiteNoise  (out, length, d, addToOutput);
	break;
	case kOscillatorWaveform_PinkNoise:
		ComputeOscillator_PinkNoise    (out, length, d, addToOutput);
	break;
	case kOscillatorWaveform_RedNoise:
		ComputeOscillator_RedNoise    (out, length, d, addToOutput);
	break;
	case kOscillatorWaveform_VioletNoise:
		ComputeOscillator_VioletNoise (out, length, d, addToOutput);
	break;
	case kOscillatorWaveform_SampleNHold:
		ComputeOscillator_SampleNHold (out, length, d, addToOutput);
	break;
	default:
		printf("ComputeOscillator: bogus waveform=%d\n", d->waveform);
	break;
	}
//printf("ComputeOscillator: wave='%s' gain=%g \n", TranslateOscillatorWaveformID(d->waveform), d->gain);
}	   // ---- end ComputeOscillator() ---- 

// **********************************************************************
// ComputeOscillator_SawtoothDown:   Compute sawtooth wave (band-unlimited)
//					
//					Return output in Range [0 .. gain]
// ********************************************************************** 
    void
ComputeOscillator_SawtoothDown(float *out, long length, OSCILLATOR *d, int addToOutput)
// z		ptr to last state
// delta	counter increment
{
unsigned long delta = d->udelta;
unsigned long z0    = d->uz;
float k = (float)(d->gain*(1.0/kTwoTo32m1));

if (addToOutput)
	{
	for (long i = 0; i < length; i++)
		{
	// Convert to range [0 .. gain] 
		out[i] += k*(float)(kTwoTo32m1i - z0); //d->gain*NormalTwoTo32m1f(kTwoTo32m1i - z0);
		z0     += delta;
		}
	}
else
	{
	for (long i = 0; i < length; i++)
		{
	// Convert to range [0 .. gain] 
		out[i] = k*(float)(kTwoTo32m1i - z0); //d->gain*NormalTwoTo32m1f(kTwoTo32m1i - z0);
		z0    += delta;
		}
	}

d->uz = z0;
//printf("ComputeOscillator_SawtoothDown: delta=%X z0=%X\n", delta, z0);
}	// ---- end ComputeOscillator_SawtoothDown() ---- 

// **********************************************************************
// ComputeOscillator_Triangle    Compute triangle wave (band-unlimited)
//
//     Return output in Range [-gain .. gain]
// **********************************************************************
    void
ComputeOscillator_Triangle(float *outBuffer, long bufferLength, OSCILLATOR *d, int addToOutput)
{
// z  ptr to last state
// delta counter increment
long tmp;
unsigned long delta = d->udelta;
unsigned long z0    = d->uz;
float k = (float)(d->gain*(1.0/kTwoTo30m1));
	
if (addToOutput)
	{
	for (long i = 0; i < bufferLength; i++)
		{
		z0    = d->uz;
		d->uz = z0 + delta;

		// Reflect down to 2^31 range
		if (z0 > kTwoTo31m1i)
			z0 = kTwoTo31m1i - z0 + kTwoTo31m1i;
		tmp = z0 - (unsigned long) kTwoTo30m1i;

		// Convert to range [-gain .. gain] 
		outBuffer[i] += k*(float)tmp; // d->gain*NormalTwoTo30m1f(tmp);
		}
	}
else
	{
	for (long i = 0; i < bufferLength; i++)
		{
		z0    = d->uz;
		d->uz = z0 + delta;

		// Reflect down to 2^31 range
		if (z0 > kTwoTo31m1i)
			z0 = kTwoTo31m1i - z0 + kTwoTo31m1i;
		tmp = z0 - (unsigned long) kTwoTo30m1i;

		// Convert to range [-gain .. gain] 
		outBuffer[i] = k*(float)tmp; // d->gain*NormalTwoTo30m1f(tmp);
		}
	}
}	// ---- end ComputeOscillator_Triangle() ---- 

// **********************************************************************
// ComputeOscillator_SawtoothUp:   Compute sawtooth wave (band-unlimited)
//     
//     Return output in Range [0 .. gain]
// ********************************************************************** 
    void
ComputeOscillator_SawtoothUp(float *outBuffer, long bufferLength, OSCILLATOR *d, int addToOutput)
{
unsigned long delta = d->udelta;	// delta wave increment
unsigned long z0    = d->uz;		// last state
float k = (float)(d->gain*(1.0/kTwoTo32m1));

if (addToOutput)
	{
	for (long i = 0; i < bufferLength; i++)
		{
		// Convert to range [0 .. gain] 
		outBuffer[i] += k*(float)z0; // d->gain*NormalTwoTo32m1f(z0);
		z0           += delta;
		}
	}
else
	{		//normal
	for (long i = 0; i < bufferLength; i++)
		{
		// Convert to range [0 .. gain] 
		outBuffer[i] = k*(float)z0; // d->gain*NormalTwoTo32m1f(z0);
		z0          += delta;
		}
	}
//printf ("k =%2.20f, z0 =%x  delta =%x\n", k, z0, delta);
d->uz = z0;
}	// ---- end ComputeOscillator_SawtoothUp() ---- 

// **********************************************************************
// ComputeOscillator_Square:   Compute square wave (band-unlimited)
//     
//     Return output in Range [-gain .. gain]
// ********************************************************************** 
    void
ComputeOscillator_Square(float *outBuffer, long bufferLength, OSCILLATOR *d, int addToOutput)
{
unsigned long delta = d->udelta;	// delta wave increment
unsigned long z0    = d->uz;		// last state
//{static long c=0; printf("ComputeOscillator_Square%d: start\n", c++);}

if (addToOutput)
	{
	for (long i = 0; i < bufferLength; i++)
		{
		if (z0 > kTwoTo31m1i)
			outBuffer[i] += d->gain;
		else
			outBuffer[i] -= d->gain;
		z0 += delta;
		}
	}
else
	{		
	for (long i = 0; i < bufferLength; i++)
		{
		if (z0 > kTwoTo31m1i)
			outBuffer[i] =  d->gain;
		else
			outBuffer[i] = -d->gain;
		z0 += delta;
		}
	}
//printf ("k =%2.20f, z0 =%x  delta =%x\n", k, z0, delta);
d->uz = z0;
}	// ---- end ComputeOscillator_Square() ---- 

// **********************************************************************
// ComputeOscillator_Sine:   Compute sinusoid
//
// ********************************************************************** 
    void 
ComputeOscillator_Sine(float *outBuffer, long bufferLength, OSCILLATOR *d, int addToOutput)
{
long i;
double sum1, sum2;
double z1   = d->z[0];
double z2   = d->z[1];
double tune = d->h[0];

float gain = d->gain;
// Compute transformer normalized waveguide filter 
if (addToOutput)		//normal
	{
	for (i = 0; i < bufferLength; i++)
		{
		outBuffer[i] += d->gain *  (float) z2;
		sum2    = (z1  + z2) * tune;
		sum1    = sum2 - z2;
		z2      = z1   + sum2;
		z1      = sum1;
		}
	}
else
	{
	for (i = 0; i < bufferLength; i++)
		{
		outBuffer[i] = d->gain*(float) z2;
		sum2   = (z1  + z2)*tune;
		sum1   = sum2 - z2;
		z2     = z1   + sum2;
		z1     = sum1;
		}
	}

d->z[0] = z1;		// Save delay state for next iteration
d->z[1] = z2;
}	// ---- end ComputeOscillator_Sine() ---- 

// **********************************************************************
// ComputeOscillator_SineBias:   Compute biased sinusoid
//
// ********************************************************************** 
    void 
ComputeOscillator_SineBias(float *out, long length, OSCILLATOR *d)
{
long i;
double sum1, sum2;
double z1   = d->z[0];
double z2   = d->z[1];
double tune = d->h[0];
float  gain = d->gain;

// Compute transformer normalized waveguide filter 
for (i = 0; i < length; i++)
	{
	out[i] = d->bias + d->gain*(float) z2;  
	sum2   = (z1  + z2)*tune;
	sum1   = sum2 - z2;
	z2     = z1   + sum2;
	z1     = sum1;
	}

d->z[0] = z1;		// Save delay state for next iteration
d->z[1] = z2;
}	// ---- end ComputeOscillator_SineBias() ---- 

// **********************************************************************
// ComputeOscillator_SineQuadrature:   Compute sine anc cosine
//
//				Busted.  The gain difference between outputs varies inversely
//						with frequency.
// ********************************************************************** 
    void 
ComputeOscillator_SineQuadrature(float *outSine, float *outCosine, long length, OSCILLATOR *d)
{
long i;
double sum1, sum2;
double z1   = d->z[0];
double z2   = d->z[1];
double tune = d->h[0];

float gain = d->gain;
// Compute transformer normalized waveguide filter 
for (i = 0; i < length; i++)
	{
	outSine  [i] = d->gain*(float) z2;
	outCosine[i] = d->gain*(float) z1;
	sum2   = (z1  + z2)*tune;
	sum1   = sum2 - z2;

	z2     = z1   + sum2;
	z1     = sum1;
	}

d->z[0] = z1;		// Save delay state for next iteration
d->z[1] = z2;
}	// ---- end ComputeOscillator_SineQuadrature() ---- 

// **********************************************************************
// ComputeOscillator_WhiteNoise    Compute white noise (band-unlimited)
//
//					Return output in Range	[-gain .. gain]
// **********************************************************************
    void
ComputeOscillator_WhiteNoise(float *out, long length, OSCILLATOR *d, int addToOutput)
// z		ptr to last state
// delta	counter increment
{
unsigned long z0 = d->uz;
float k = (float)(d->gain*(1.0/kTwoTo31m1));

//{static long c=0; printf("ComputeOscillator_WhiteNoise%d: z0=%d delta=%d\n", c++, z0, delta);}

// Compute linear congruator 
if (addToOutput)
	{
	for (long i = 0; i < length; i++)
		{
		z0      = 1103515245*z0 + 12345;
	// Convert range [0..2^32 - 1] to range range [-gain..gain] 
		out[i] += k*(float)z0; //d->gain*NormalTwoTo31m1f((long)z0);
		}
	}
else
	{
	for (long i = 0; i < length; i++)
		{
		z0     = 1103515245*z0 + 12345;
	// Convert range [0..2^32 - 1] to range range [-gain..gain] 
		out[i] = k*(float)z0; //d->gain*NormalTwoTo31m1f((long)z0);
		}
	}
d->uz = z0;
}	// ---- end ComputeOscillator_WhiteNoise() ----

// **********************************************************************
// ComputeOscillator_PinkNoise    Compute pink noise (band-unlimited)
//
//		Pink noise is approximated with -10dB/decade (3.01 dB/octave) low pass filter 
 
//					Return output in Range	[-gain .. gain]
// **********************************************************************
    void
ComputeOscillator_PinkNoise(float *out, long length, OSCILLATOR *d, int addToOutput)
// z		ptr to last state
// delta	counter increment
{
int j = 0;
unsigned long z0 = d->uz;
float k = (float)(d->gain*(1.0/kTwoTo31m1));
double x, y;
double *z = d->z;
double *h = d->h;

//{static long c=0; printf("ComputeOscillator_PinkNoise%d: z0=%d \n", c++, z0);}

// 3 pole, 3 zero IIR filter:  +/- 0.3 dB of True pink over 10 octaves
//		h[3] =  64516.0/65536.0; // zero1
//		h[0] =  65257.0/65536.0; // pole1

//		h[4] =  54652.0/65536.0; // zero2
//		h[1] =  62122.0/65536.0; // pole2

//		h[5] =   4960.0/65536.0; // zero3
//		h[2] =  35106.0/65536.0; // pole3

for (long i = 0; i < length; i++)
	{
	z0 = 1103515245*z0 + 12345;
	x  = k*(float) z0;

//#define PINK_NOISE_V1
//#define PINK_NOISE_V2
//#define PINK_NOISE_V3
#define PINK_NOISE_V4
//#define PINK_NOISE_V5

#ifdef PINK_NOISE_V4
// +/-0.5dB accuracy
	z[0] = 0.99765 * z[0] + x * 0.0990460;
	z[1] = 0.96300 * z[1] + x * 0.2965164;
	z[2] = 0.57000 * z[2] + x * 1.0526913;
//	y    = z[0] + z[1] + z[2] + x * 0.1848;
	y    = z[0];
y *= 0.25;
#endif

#ifdef PINK_NOISE_V5
// +/-0.05dB accuracy

  z[0] =  0.99886 * z[0] + x * 0.0555179;
  z[1] =  0.99332 * z[1] + x * 0.0750759;
  z[2] =  0.96900 * z[2] + x * 0.1538520;
  z[3] =  0.86650 * z[3] + x * 0.3104856;
  z[4] =  0.55000 * z[4] + x * 0.5329522;
  z[5] = -0.7616  * z[5] - x * 0.0168980;
  y    = z[0] + z[1] + z[2] + z[3] + z[4] + z[5] + z[6] + x * 0.5362;
  z[6] = x * 0.115926;
#endif

#ifdef PINK_NOISE_V3
// y[n] = b0*x[n] + b1*x[n-1] - a1*y[n-1]
	for (j = 0; j < 3; j++)
		{
		y           = x + h[3+j]*z[2*j] - h[j]*z[2*j+1];
		z[2*j  ] = x;
		z[2*j+1] = y;
		x           = y;
		}
	y *= 1.0/2.0;
#endif

#ifdef PINK_NOISE_V2
// y[n] = b0*x[n] + b1*x[n-1] - a1*y[n-1]
	y  =   x + h[3]*z[0] - h[0]*z[3];
	y +=       h[4]*z[1] - h[1]*z[4];
	y +=       h[5]*z[2] - h[2]*z[5];

	z[2] = z[1];
	z[1] = z[0];
	z[0] = x;

	z[5] = z[4];
	z[4] = z[3];
	z[3] = y;
#endif

// Convert range [0..2^32 - 1] to [-gain..gain] 
//	out[i] = k*(float)z0;
	out[i] = (float)y;
	}
d->uz = z0;
}	// ---- end ComputeOscillator_PinkNoise() ----

// **********************************************************************
// ComputeOscillator_RedNoise    Compute red noise (band-unlimited)
//
//					Return output in Range	[-gain .. gain]
// **********************************************************************
    void
ComputeOscillator_RedNoise(float *out, long length, OSCILLATOR *d, int addToOutput)
// z		ptr to last state
// delta	counter increment
{
long sum;
unsigned long z0 = d->uz;
float k = (float)(d->gain*(1.0/kTwoTo31m1));

//{static long c=0; printf("ComputeOscillator_RedNoise%d: z0=%d delta=%d\n", c++, z0, delta);}

// Compute linear congruator 
if (addToOutput)
	{
	for (long i = 0; i < length; i++)
		{
		sum = ((long) z0)>>1;
	// Run linear congruator. 
		z0  = 1103515245*z0 + 12345;

	// Note: avoid 32-bit overflow: pre-shift
		sum += ((long) z0)>>1;

	// Convert range [-2^31..+2^31] to range [-gain..gain] 
		out[i] += k*(float) sum; // gain*NormalTwoTo31m1f(sum);
		}
	}
else
	{
	for (long i = 0; i < length; i++)
		{
		sum = ((long) z0)>>1;
	// Run linear congruator. 
		z0  = 1103515245*z0 + 12345;

	// Note: avoid 32-bit overflow:  pre-shift
		sum += ((long) z0)>>1;

	// Convert range [-2^31..+2^31] to range [-gain..gain] 
		out[i] = k*(float) sum; // gain*NormalTwoTo31m1f(sum);
		}
	}
d->uz = z0;
}	// ---- end ComputeOscillator_RedNoise() ----

// **********************************************************************
// ComputeOscillator_VioletNoise    Compute violet noise (band-unlimited)
//
//					Return output in Range	[-gain .. gain]
// **********************************************************************
    void
ComputeOscillator_VioletNoise(float *out, long length, OSCILLATOR *d, int addToOutput)
// z		ptr to last state
// delta	counter increment
{
long sum;
unsigned long z0 = d->uz;
float k = (float)(d->gain*(1.0/kTwoTo31m1));

//{static long c=0; printf("ComputeOscillator_VioletNoise%d: z0=%d delta=%d\n", c++, z0, delta);}

// Compute linear congruator 
if (addToOutput)
	{
	for (long i = 0; i < length; i++)
		{
		sum = ((long) z0)>>1;
	// Run linear congruator. 
		z0  = 1103515245*z0 + 12345;

	// Note: avoid 32-bit overflow: pre-shift
		sum -= ((long) z0)>>1;

	// Convert range [-2^31..+2^31] to range [-gain..gain] 
		out[i] += k*(float) sum; //d->gain*NormalTwoTo31m1f(sum);
		}
	}
else
	{
	for (long i = 0; i < length; i++)
		{
		sum = ((long) z0)>>1;
	// Run linear congruator. 
		z0  = 1103515245*z0 + 12345;

	// Note: avoid 32-bit overflow:  pre-shift
		sum -= ((long) z0)>>1;

	// Convert range [-2^31..+2^31] to range [-gain..gain] 
		out[i] = k*(float) sum; //d->gain*NormalTwoTo31m1f(sum);
		}
	}
d->uz = z0;
}	// ---- end ComputeOscillator_VioletNoise() ----

// **********************************************************************
// ComputeOscillator_SampleNHold    Compute SampleNHold (band-unlimited)
//
//					Return output in Range	[-gain .. gain]
// **********************************************************************
    void
ComputeOscillator_SampleNHold(float *out, long length, OSCILLATOR *d, int addToOutput)
// z		ptr to last state
// delta	counter increment
{
unsigned long z0 = d->uz;
float k = (float)(d->gain*(1.0/kTwoTo31m1));

//{static long c=0; printf("ComputeOscillator_SampleNHold%d: z0=%d delta=%d count=%d\n", c++, z0, d->udelta, d->counter);}

// Compute linear congruator 
if (addToOutput)
	{
	for (long i = 0; i < length; i++)
		{
		if (d->counter == d->udelta)
			{
			z0         = 1103515245*z0 + 12345;
			d->counter = 0;
			}
		else
			d->counter += 1;
	// Convert range [0..2^32 - 1] to range range [-gain..gain] 
		out[i] += k*(float)z0; //d->gain*NormalTwoTo31m1f((long)z0);
		}
	}
else
	{
	for (long i = 0; i < length; i++)
		{
		if (d->counter == d->udelta)
			{
			z0         = 1103515245*z0 + 12345;
			d->counter = 0;
			}
		else
			d->counter += 1;
	// Convert range [0..2^32 - 1] to range range [-gain..gain] 
		out[i] = k*(float)z0; //d->gain*NormalTwoTo31m1f((long)z0);
		}
	}
d->uz = z0;
}	// ---- end ComputeOscillator_SampleNHold() ----

// ************************************************************************
// PrintOscillator:	Print parameters, unless 'outputSpace'  provided
// ************************************************************************ 
	void 
PrintOscillator(OSCILLATOR *d, char *outputSpace)
{
char space[kOscillator_StringSpace];
char *s = space;

if (outputSpace)
	s = outputSpace;
// Decide whether to write into string or append to it
//if (!(printLevel & kPrintLevelAppend))
//	s[0] = '\0';

sprintf(s, 
"\nPrintOscillator %d\n\
HighLevel: samplingFrequency=%g\n\
gain=%g frequency=%g phase=%g waveform='%s'\n",
		d, 
		d->samplingFrequency,
		d->gain, 
		d->frequency,
		d->phase,
		TranslateOscillatorWaveformID(d->waveform));

if (!outputSpace)
	printf("%s", s);
}	// ---- end PrintOscillator() ---- 

// ********************************************************************** 
// TranslateOscillatorWaveformID:   Translate ID to English string
// ********************************************************************** 
    char * 
TranslateOscillatorWaveformID(int x)
{
switch (x)
	{
	 case kOscillatorWaveform_Triangle:
		return ("Triangle");
	 case kOscillatorWaveform_SawtoothUp:
		return ("SawtoothUp");
	 case kOscillatorWaveform_SawtoothDown:
		return ("SawtoothDown");
	 case kOscillatorWaveform_Sine:
		return ("Sine");
	 case kOscillatorWaveform_Square:
		return ("Square");
	 case kOscillatorWaveform_Pulse:
		return ("Pulse");
	 case kOscillatorWaveform_Quadrature:
		return ("Quadrature");

	 case kOscillatorWaveform_WhiteNoise:
		return ("WhiteNoise");
	 case kOscillatorWaveform_PinkNoise:
		return ("PinkNoise");
	 case kOscillatorWaveform_RedNoise:
		return ("RedNoise");
	 case kOscillatorWaveform_VioletNoise:
		return ("VioletNoise");
	 case kOscillatorWaveform_SampleNHold:
		return ("SampleNHold");
	}
return ("Bogus");
}	// ---- end TranslateOscillatorWaveformID() ---- 


