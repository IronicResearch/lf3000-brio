// *************************************************************** 
// briomixer.h:		Header file for Brio mixer , Lightning v1.0
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#ifndef __BRIOMIXER_H__
#define	__BRIOMIXER_H__

#include <math.h>

#include "mix.h"
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

#define kBrioMixer_Type_Basic	   0

#define kBrioMixer_In_Microphone	0
#define kBrioMixer_In_VOX_CH1		1
#define kBrioMixer_In_VOX_CH2		2
#define kBrioMixer_In_SFX_CH1_Left	3
#define kBrioMixer_In_SFX_CH1_Right	4
#define kBrioMixer_In_SFX_CH2_Left	5
#define kBrioMixer_In_SFX_CH2_Right	6
#define kBrioMixer_In_MSX_CH1_Left	7
#define kBrioMixer_In_MSX_CH1_Right	8
#define kBrioMixer_In_MSX_CH2_Left	9
#define kBrioMixer_In_MSX_CH2_Right	10
#define kBrioMixer_In_Count		11

#define kBrioMixer_Out_Headset_Left	0
#define kBrioMixer_Out_Headset_Right	1
#define kBrioMixer_Out_TV_Left		2
#define kBrioMixer_Out_TV_Right		3
#define kBrioMixer_Out_Speaker_Left	4
#define kBrioMixer_Out_Speaker_Right	5
#define kBrioMixer_Out_Count		6

#define kBrioMixer_Out_Stereo_Left	0
#define kBrioMixer_Out_Stereo_Right	1
#define kBrioMixer_Out_Stereo_Count	2

#define kBrioMixer_FxSend_Reverb	0
#define kBrioMixer_FxSend_Stereo_Count	1

#define kBrioMixer_MaxChannels	12

typedef struct briomixer {
	long type; 
	long useFixedPoint;

// ---- Input section	
	long enableChannel_Reverb;
	MIXERCHANNEL channels[kBrioMixer_MaxChannels];
	long channelEnabled[kBrioMixer_MaxChannels];
	long channelCount;

// ---- Out section	
	long enableOut;
	long computeTripleOuts;
	long enableOutEQ;
//	EQ    out_EQ[kBrioMixer_Out_Count];
	float out_GainDB[kBrioMixer_Out_Count];

#define kBrioMixer_SubMixBuffer_Count	7
#define kBrioMixer_TempBuffer_Count	3
#define kBrioMixer_FxSendBuffer_Count	1
#define kBrioMixer_OutBuffer_Count	6
#define kBrioMixer_TotalBuffer_Count	(kBrioMixer_SubMixBuffer_Count+kBrioMixer_TempBuffer_Count+kBrioMixer_FxSendBuffer_Count+kBrioMixer_OutBuffer_Count)
	short  *subMixBuffers[kBrioMixer_SubMixBuffer_Count];
	short  *tempBuffers  [kBrioMixer_TempBuffer_Count  ];
	short  *fxSendBuffers[kBrioMixer_FxSendBuffer_Count];
	short  *outBuffers   [kBrioMixer_OutBuffer_Count   ];

// Low-level algorithm data
	float   outGainf   [kBrioMixer_Out_Count];
	Q15     outGaini   [kBrioMixer_Out_Count];

	long  computingDone;
	float samplingFrequency;
} BRIOMIXER;

void DefaultBrioMixer(BRIOMIXER *d);
void UpdateBrioMixer (BRIOMIXER *d);
void ResetBrioMixer  (BRIOMIXER *d);
void PrepareBrioMixer(BRIOMIXER *d);

void RunBrioMixerf(short **ins, short **outs, long length, BRIOMIXER *d);
void RunBrioMixeri(short **ins, short **outs, long length, BRIOMIXER *d);

// Parameter set functions
void BrioMixer_SetAllTempBuffers  (BRIOMIXER *d, short **bufs, long count);
void BrioMixer_SetAllSubMixBuffers(BRIOMIXER *d, short **bufs, long count);
void BrioMixer_SetOutMixBuffers   (BRIOMIXER *d, short **bufs, long count);
void BrioMixer_SetFxSendBuffers   (BRIOMIXER *d, short **bufs, long count);

void BrioMixer_SetSubMixBuffer(BRIOMIXER *d, long index, short *buf);
void BrioMixer_SetTempBuffer  (BRIOMIXER *d, long index, short *buf);
void BrioMixer_SetOutBuffer   (BRIOMIXER *d, long index, short *buf);
void BrioMixer_SetFxSendBuffer(BRIOMIXER *d, long index, short *buf);

void BrioMixer_ClearAllTempBuffers  (BRIOMIXER *d, long length);
void BrioMixer_ClearAllSubMixBuffers(BRIOMIXER *d, long length);
void BrioMixer_ClearAllOutBuffers   (BRIOMIXER *d, long length);
void BrioMixer_ClearAllFxSendBuffers(BRIOMIXER *d, long length);
void BrioMixer_ClearAllBuffers      (BRIOMIXER *d, long length);

// Mixer channel functions
void Test_BrioMixer();

#endif  //	end __BRIOMIXER_H__
