// *************************************************************** 
// mix.h:	Header file for channel mixer routines
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#ifndef __MIX_H__
#define	__MIX_H__

#include <math.h>
#include "util.h"

#include "mix.h"

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
#define kMixerChannel_Type_In2_Out1	2
#define kMixerChannel_Type_In2_Out2	3

typedef struct mixerchannel {
// High Level parameters
	float   inGainDB;
	long     enableEQ;
//	EQ	equalizer[2];
	float   reverbSendDB;
	float   pan;
	float   postGainDB;

	long 	useFixedPoint;
	long    type;
	long    addToOutput;

#define kMixerChannel_TempBufferCount	2
	short	*tmpPs[kMixerChannel_TempBufferCount];

// Low level algorithm data  (many float variables will be displaced by their Q15 equivalents
	float   inGainf;
	float   reverbSendf;
	float	panValuesf[2];
	float   postGainf;
	Q15     inGaini;
	Q15     reverbSendi;
	Q15	panValuesi[2];
	Q15     postGaini;

	float   samplingFrequency;
} MIXERCHANNEL;

void DefaultMixerChannel(MIXERCHANNEL *d);
void UpdateMixerChannel (MIXERCHANNEL *d);
void ResetMixerChannel  (MIXERCHANNEL *d);
void PrepareMixerChannel(MIXERCHANNEL *d);

void MixerChannel_SetSamplingFrequency(MIXERCHANNEL *d, float x);
void MixerChannel_SetAllTempBuffers(MIXERCHANNEL *d, short **bufs, long count);

void RunMixerChannelf(short **ins, short **outs, long length, MIXERCHANNEL *d);
void RunMixerChanneli(short **ins, short **outs, long length, MIXERCHANNEL *d);

// Mixer channel functions
void Test_Mixer();

#endif  //	end __MIX_H__
