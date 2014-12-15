// *************************************************************** 
// mix_smallf.cpp:		Audio channel mixing routines
//			32-bit fixed point or a development
//
//          Small version for Crammer product
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mix_common.h"
#include "mix_smallf.h"

// *************************************************************** 
// ScaleShortsf:	Scale 'short' from in buffer to out buffer
//			32-bit floating-point implementation
//				( ok for "in place" operation )
// ***************************************************************
    void 
ScaleShortsf(short *in, short *out, long length, float k)
{
for (long i = 0; i < length; i++)
    out[i] = (short)(k*(float)in[i]);
}	// ---- end ScaleShortsf() ---- 

// *************************************************************** 
// MacShortsf:	MultiplyAccumulate 'short' from in buffer and add to out buffer
//			32-bit floating-point implementation
//              out(x) = out(x) + k*in(x)
// ***************************************************************
    void 
MacShortsf(short *in, short *out, long length, float k, long saturateQ15)
{
if (saturateQ15)
    {
    for (long i = 0; i < length; i++)
        {
        float sum = (long) (((float)out[i]) + k*(float)in[i]);
        if      (sum > kQ15_Maxf)
            sum = kQ15_Maxf;
        else if (sum < kQ15_Minf)
            sum = kQ15_Minf;
        out[i] = (short)sum;
        }
    }
else
    {
    for (long i = 0; i < length; i++)
        out[i] = (short)(((float)out[i]) + k*(float)in[i]);
    }
}	// ---- end MacShortsf() ---- 

// *************************************************************** 
// RunMixerChannelf:	master router function
//
// ***************************************************************
    void 
RunMixerChannelf(short **inPs, short **outPs, long length, MIXERCHANNEL *d)
{
long i = 0;
short **tmpPs = d->tmpPs;

//{static long c=0; printf("RunMixerChannelf%d : start\n", c++);}
printf("RunMixerChannelf: gain <%f %f> addToOutput=%d\n", d->gainf[kLeft ], d->gainf[kRight], d->addToOutput);

switch (d->type)
	{
	default:
	case kMixerChannel_Type_In1_Out1:
	{
// Add/Copy to Submix buffer
	if (d->addToOutput) 
	    MacShortsf(inPs[0], outPs[0], length, d->gainf[kLeft], True);
	else
	    ScaleShortsf(inPs[0], outPs[0], length, d->gainf[kLeft]);
	}
	break;

// "pan" signal to stereo
	case kMixerChannel_Type_In1_Out2:
    {
// Add/Copy to Output/Submix buffers with panning
	if (d->addToOutput) 
		{
    	MacShortsf(inPs[0], outPs[kLeft ], length, d->gainf[kLeft ], True);
    	MacShortsf(inPs[0], outPs[kRight], length, d->gainf[kRight], True);
		}
	else
		{
    	ScaleShortsf(inPs[0], outPs[kLeft ], length, d->gainf[kLeft ]);
    	ScaleShortsf(inPs[0], outPs[kRight], length, d->gainf[kRight]);
		}
    }
	break;

// Process stereo channels independently 
	case kMixerChannel_Type_In2_Out2:
	{
	// Apply Gain 
	for (long ch = 0; ch < 2; ch++)
		{
	// Add/Copy to Output buffers with Center panning
		if (d->addToOutput) 
		    MacShortsf(inPs[ch], outPs[ch], length, d->gainf[ch], True);
		else
		    ScaleShortsf(inPs[ch], outPs[ch], length, d->gainf[ch]);
		}
	}
	break;
	}
}	// ---- end RunMixerChannelf() ---- 

// *************************************************************** 
// RunMixerf:	
// ***************************************************************
    void 
RunMixerf(short **ins, short **outs, long length, MIXER *d)
{
//{static long c=0; printf("RunMixerf %d: start\n", c++);}

for (long i = 0; i < d->channelCount; i++)
    {
    RunMixerChannelf(&ins[i], outs, length, &d->channels[i]);
    }
}	// ---- end RunMixerf() ---- 


