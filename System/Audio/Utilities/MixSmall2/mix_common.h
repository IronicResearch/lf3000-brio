// *************************************************************** 
// mix_common.h:	Header file of common definitions for reduced mixer routines
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#ifndef __MIX_COMMON_H__
#define	__MIX_COMMON_H__

#include <math.h>

#include "dsputil_mix.h"


#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

// ************** Mixer Channel definitions
#define kMixerChannel_Type_In1_Out1	0
#define kMixerChannel_Type_In1_Out2	1
#define kMixerChannel_Type_In2_Out2	2
//#define kMixerChannel_Type_In2_Out1	3

typedef struct mixerchannel {
// High Level parameters
	float   headroomDB;
	float   inGainDB;
	float   pan;

	long 	useFixedPoint;
	long    type;
	long    addToOutput;

#define kMixerChannel_TempBufferCount	2
	short	*tmpPs[kMixerChannel_TempBufferCount];

// Low level algorithm data  (many float variables will be displaced by their Q15 equivalents)
	float   gainf[2];
	Q15     gaini[2]; // Headroom + inGain + pan
} MIXERCHANNEL;

typedef struct mixer {
#define kMixer_MaxChannels 5
// High Level parameters
    long channelCount;
    MIXERCHANNEL  channels[kMixer_MaxChannels];

	float   outGainDB;
	float   headroomDB;
	long 	useFixedPoint;

// Low Level parameters
    float outGainf;
    Q15   outGaini;
} MIXER;

void DefaultMixer(MIXER *d);
void UpdateMixer (MIXER *d);

int  SetMixer_HeadroomDB(MIXER *d, long x);
int  SetMixer_OutGainDB(MIXER *d, long x);
int  SetMixer_ChannelCount(MIXER *d, long x);
void SetMixer_ChannelTempBuffers(MIXER *d, short **bufs, long count);
void SetMixer_ChannelParameters(MIXER *d, long index, long type, long pan, float gainDB);

//void RunMixer (Q15   **ins, Q15   **outs, long length, MIXER *d);

// Mixer channel functions
void Test_Mixer();

#endif  //	end __MIX_COMMON_H__
