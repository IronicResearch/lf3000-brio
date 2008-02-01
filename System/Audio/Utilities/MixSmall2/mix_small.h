// *************************************************************** 
// mix_small.h:	Header file for reduced mixer routines
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#ifndef __MIX_SMALL_H__
#define	__MIX_SMALL_H__

#include <math.h>

#include "dsputil_mix.h"
#include "mix_common.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

void DefaultMixerChannel(MIXERCHANNEL *d);
void UpdateMixerChannel (MIXERCHANNEL *d);

void MixerChannel_SetTempBuffers(MIXERCHANNEL *d, short **bufs, long count);

void RunMixerChanneli(Q15   **ins, Q15   **outs, long length, MIXERCHANNEL *d);

// ************** Mixer definitions
#define kMixer_HeadroomDB_Default ( -3.0f)
#define kMixer_HeadroomDB_Min     (-96.0f)
#define kMixer_HeadroomDB_Max     (  0.0f)

#define kHeadroom_Max      kS16_Max
#define kHeadroom_Min      kS16_Min

#define kMixerChannel_Level_Min     (-96.0)
#define kMixerChannel_Level_Default   (0.0)
#define kMixerChannel_Level_Max       (0.0)

void DefaultMixer(MIXER *d);
void UpdateMixer (MIXER *d);

int  SetMixer_HeadroomDB(MIXER *d, float x);
void SetMixer_OutGainDB(MIXER *d, float x);

int  SetMixer_ChannelCount(MIXER *d, long x);
void SetMixer_ChannelTempBuffers(MIXER *d, short **bufs, long count);
void SetMixer_ChannelParameters(MIXER *d, long index, long type, float pan, float gainDB);

void RunMixeri(Q15   **ins, Q15   **outs, long length, MIXER *d);
void RunMixer (Q15   **ins, Q15   **outs, long length, MIXER *d);

// Mixer channel functions
void Test_Mixer();

#endif  //	end __MIX_SMALL_H__
