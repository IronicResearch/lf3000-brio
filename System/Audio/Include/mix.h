// *************************************************************** 
// mix.h:	Header file for channel mixer routines
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#ifndef __MIX_H__
#define	__MIX_H__

#if 0
#include <math.h>
#include "util.h"

#include "eq.h"
#include "shape.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

typedef struct mix {
	long type; 

	long  computingDone;
	float samplingFrequency;
} MIX;

#define kMixerChannel_Type_In1_Out1	0
#define kMixerChannel_Type_In1_Out2	1
#define kMixerChannel_Type_In2_Out2	2

typedef struct mixerchannel {
// High Level parameters
	float   inGainDB;
//	long    useEQ;

#define kMixerChannel_EQ_MaxBands 2
//	EQ	equalizer[kMixerChannel_EQ_MaxBands];       // NOTE:  not enough for 2 In/2 Out channel type
//	float   reverbSendDB;
	float   pan;
	float   outGainDB;

	long 	useFixedPoint;
	long    type;
	long    addToOutput;

#define kMixerChannel_TempBufferCount	2
	short	*tmpPs[kMixerChannel_TempBufferCount];

// Low level algorithm data  (many float variables will be displaced by their Q15 equivalents
	float   inGainf;
//	float   reverbSendf;
	float	panValuesf[2];
	float   outGainf;

	Q15     inGaini;
//	Q15     reverbSendi;
	Q15	    panValuesi[2];
	Q15     outGaini;

	float   samplingFrequency;
} MIXERCHANNEL;

void DefaultMixerChannel(MIXERCHANNEL *d);
void UpdateMixerChannel (MIXERCHANNEL *d);
void ResetMixerChannel  (MIXERCHANNEL *d);
void PrepareMixerChannel(MIXERCHANNEL *d);

void MixerChannel_SetSamplingFrequency(MIXERCHANNEL *d, float x);
void MixerChannel_SetAllTempBuffers(MIXERCHANNEL *d, short **bufs, long count);

void RunMixerChannelf(short **ins, short **outs, long length, MIXERCHANNEL *d);
void RunMixerChanneli(Q15   **ins, Q15   **outs, long length, MIXERCHANNEL *d);

#define kMixer_MaxInChannels  (19 * 2)	// maximum number of channels into mixer
#define kMixer_MaxOutChannels 2
#define kMixer_OutEQ_MaxBands 3

typedef struct mixer {
// High Level parameters
    long channelCount;
    MIXERCHANNEL  channels[kMixer_MaxInChannels];

// Output section
	float   outGainDB;

	long    useOutEQ;
	EQ	    outEQ[kMixer_MaxOutChannels][kMixer_OutEQ_MaxBands];

	long    useOutSoftClipper;
    WAVESHAPER outSoftClipper[kMixer_MaxOutChannels];

// Low Level parameters
    float outGainf;
    Q15   outGaini;
    float samplingFrequency;
} MIXER;

void DefaultMixer(MIXER *d);
void UpdateMixer (MIXER *d);
void ResetMixer  (MIXER *d);
void PrepareMixer(MIXER *d);

void Mixer_SetAllChannelTempBuffers(MIXER *d, short **bufs, long count);
void Mixer_SetSamplingFrequency(MIXER *d, float x);

void RunMixerf(short **ins, short **outs, long length, MIXER *d);
void RunMixeri(Q15   **ins, Q15   **outs, long length, MIXER *d);

// Mixer channel functions
void Test_Mixer();
#endif

#endif  //	end __MIX_H__
