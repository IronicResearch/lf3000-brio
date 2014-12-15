// *************************************************************** 
// mix_smallf.h:	Header file for floating-point  mixer routines
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#ifndef __MIX_SMALLF_H__
#define	__MIX_SMALLF_H__

#include <math.h>
#include <mix_common.h>

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

void ScaleShortsf(short *in, short *out, long length, float k);
void MacShortsf(short *in, short *out, long length, float k, long saturateQ15);

void RunMixerChannelf(Q15   **ins, Q15   **outs, long length, MIXERCHANNEL *d);

void RunMixerf(short **ins, short **outs, long length, MIXER *d);


#endif  //	end __MIX_SMALLF_H__
