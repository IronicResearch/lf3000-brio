// *************************************************************************
//
// Header file DSP functions
//
//				Written by Gints Klimanis
// *************************************************************************

#ifndef __DSPUTIL2_H__
#define	__DSPUTIL2_H__

//#define OSC_USE_WAVEUTIL
//#define OSC_USE_SRC

//#include "dsputil.h"

//#ifdef __cplusplus
//extern "C" {
//#endif


// ----------------------------------------------------
// ---- Oscillator  ----
// ----------------------------------------------------
#define kOscillatorWaveform_Triangle		0
#define kOscillatorWaveform_SawtoothUp		1
#define kOscillatorWaveform_Sawtooth		kOscillatorWaveform_SawtoothUp
#define kOscillatorWaveform_SawtoothDown	2
#define kOscillatorWaveform_Sine			3
#define kOscillatorWaveform_Square			4
#define kOscillatorWaveform_Pulse			5
#define kOscillatorWaveform_WhiteNoise		6
#define kOscillatorWaveform_PinkNoise		7
#define kOscillatorWaveform_RedNoise		8
#define kOscillatorWaveform_VioletNoise		9
#define kOscillatorWaveform_SampleNHold		10
#define kOscillatorWaveform_Quadrature		11

#define kOscillatorWaveform_SamplePlay		12

#define kOscillator_LoopType_Off		0
#define kOscillator_LoopType_Forward	1

#define kOscillator_Interpolation_None		0
#define kOscillator_Interpolation_Linear	1
#define kOscillator_Interpolation_Sinc		2

#define kOscillator_AllocatedWaveform_Sine			"Sine"
#define kOscillator_AllocatedWaveform_Sawtooth		"Sawtooth"
#define kOscillator_AllocatedWaveform_Triangle		"Triangle"
#define kOscillator_AllocatedWaveform_WhiteNoise	"WhiteNoise"

#define kOscillator_StringSpace		kPrintStringSpace

typedef struct oscillator {
// High level parameters
	int		waveform;	 

	float	gain;		// Linear
	float	frequency;	// Hertz
	float	pitch;		// Semitones
	double	phase;		// Radians
	float	bias;	    // Amount to add to output
	int		channels;

// Low level data
#define kOscillator_DelayElements_PinkNoise	7
#define kOscillator_Coefficients_PinkNoise	7
#define kOscillator_MaxDelayElements	kOscillator_DelayElements_PinkNoise

	double	h[kOscillator_Coefficients_PinkNoise];		// Sine & pink noise
	double	z[kOscillator_MaxDelayElements];	// Delay elements 
	unsigned long udelta, uz;
	unsigned long counter;
	int		done;			// Don't compute oscillator

// For    sine wave: phase  =  90*(Pi/180) = Pi/2
// For  cosine wave: phase  = 180*(Pi/180) = Pi
// For -cosine wave: phase  = 180*( 0/180) = 0

	float	*modSources[2];

// sample info
	short	*sampleData[MAX_CHANNELS];
	int		sampleDataAllocatedInternally;
	long	sampleLength;
	float	sampleTime;
	int		loopType;	

	float   index ;
	long	iIndex;
	float   outToInRatio;  // outRate/ inRate
	float   inToOutRatio;  // inRate /outRate

	int		interpolationMode;
#ifdef OSC_USE_WAVEUTIL
	WAVEFILE  waveFile;
#endif

	int		lowLevelDataBogus;
	double	samplingFrequency;
} OSCILLATOR;

void SetOscillator                  (OSCILLATOR *d, float frequency, float gain, int waveform);
#define OscillatorInvalidate(d)	((d)->lowLevelDataBogus = True)
void SetOscillator_SamplingFrequency(OSCILLATOR *d, double x);
//void SetOscillator_Frequency        (OSCILLATOR *d, double x);
#define SetOscillator_Frequency(d, x)	((d)->frequency = (x))
#define SetOscillator_Gain(     d, x)	((d)->gain      = (x))
#define SetOscillator_Phase(    d, x)	((d)->phase     = (x))
#define SetOscillator_Waveform( d, x)	((d)->waveform  = (x))
#define SetOscillator_Bias(     d, x)	((d)->bias      = (x))
#define SetOscillator_Pitch(    d, x)	((d)->pitch     = (x))
#define SetOscillator_SampleLength(d, x)((d)->sampleLength     = (x))

#define SetOscillator_InterpolationMode(d, x)	((d)->interpolationMode = (x))

void SetOscillator_SamplePlayRate(OSCILLATOR *d, float ratio);
void SetOscillator_SamplePitch   (OSCILLATOR *d, float semitones);
void SetOscillator_SampleTime    (OSCILLATOR *d, float seconds);

void  DefaultOscillator(OSCILLATOR *d);
void  UpdateOscillator (OSCILLATOR *d); 
void  PrepareOscillator(OSCILLATOR *d);
void  ResetOscillator  (OSCILLATOR *d);

void  PrintOscillator(OSCILLATOR *d, char *outputSpace);
char *TranslateOscillatorWaveformID(int x);
	
int   Oscillator_LoadSample(OSCILLATOR *d, char *filePath);
int   Oscillator_SetSamplePtr(OSCILLATOR *d, short *sampleData);

void  ComputeOscillator				(float *out    , long length, OSCILLATOR *d, int addToOutput);
void  ComputeOscillator_Stereo		(float *outs[2], long length, OSCILLATOR *d, int addToOutput);

void  ComputeOscillator_Sine        (float *out, long length, OSCILLATOR *d, int addToOutput);
void  ComputeOscillator_SawtoothUp  (float *out, long length, OSCILLATOR *d, int addToOutput);
void  ComputeOscillator_SawtoothDown(float *out, long length, OSCILLATOR *d, int addToOutput);
void  ComputeOscillator_Square		(float *out, long length, OSCILLATOR *d, int addToOutput);
void  ComputeOscillator_Triangle    (float *out, long length, OSCILLATOR *d, int addToOutput);
void  ComputeOscillator_WhiteNoise  (float *out, long length, OSCILLATOR *d, int addToOutput);
void  ComputeOscillator_PinkNoise   (float *out, long length, OSCILLATOR *d, int addToOutput);
void  ComputeOscillator_RedNoise    (float *out, long length, OSCILLATOR *d, int addToOutput);
void  ComputeOscillator_VioletNoise (float *out, long length, OSCILLATOR *d, int addToOutput);

void  ComputeOscillator_SineBias    (float *out, long length, OSCILLATOR *d);
void  ComputeOscillator_SampleNHold (float *out, long length, OSCILLATOR *d, int addToOutput);
void  ComputeOscillator_SineQuadrature(float *outSine, float *outCosine, long length, OSCILLATOR *d);

//#ifdef __cplusplus
//}
//#endif

#endif  //	__DSPUTIL2_H__
