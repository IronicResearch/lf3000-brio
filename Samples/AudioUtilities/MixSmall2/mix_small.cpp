// *************************************************************** 
// mix_small.cpp:		Audio channel mixing routines
//			16-bit or 32-bit fixed point or a development
//			fixed-point/floating-point hybrid.
//
//          Small version for Crammer project
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>

#include "mix_small.h"

// *************************************************************** 
// DefaultMixerChannel:		Set Default high-level parameter values
// ***************************************************************
    void 
DefaultMixerChannel(MIXERCHANNEL *d)
{
d->useFixedPoint = False;
d->type = kMixerChannel_Type_In1_Out1;
d->addToOutput = False;

d->inGainDB = 0.0f;
d->pan      = kPanValue_Center;

for (long i = 0; i < kMixerChannel_TempBufferCount; i++)
	d->tmpPs[i] = NULL;
}	// ---- end DefaultMixerChannel() ---- 

// *************************************************************** 
// UpdateMixerChannel:	Convert high-level parameter values to low level data
// ***************************************************************
    void 
UpdateMixerChannel(MIXERCHANNEL *d)
{
float	panValuesf[2];
//{static long c=0; printf("UpdateMixerChannel: %d: start\n", c++);}

float k = DecibelToLinearf(d->inGainDB + d->headroomDB);
//printf("UpdateMixerChannel: gainDB=%g -> %g\n", d->inGainDB, k);

PanValues_ConstantPower(d->pan, panValuesf);
//printf("UpdateMixerChannel: pan   =%g -> <%g, %g>\n", d->pan, panValuesf[kLeft], panValuesf[kRight]);
d->gainf[kLeft ] = k*panValuesf[kLeft ];d->gainf[kRight] = k*panValuesf[kRight];

d->gaini[kLeft ] = FloatToQ15(d->gainf[kLeft ]);
d->gaini[kRight] = FloatToQ15(d->gainf[kRight]);

}	// ---- end UpdateMixerChannel() ---- 

// *************************************************************** 
// MixerChannel_SetTempBuffers:	Assign ptrs to buffers 
// ***************************************************************
    void 
MixerChannel_SetTempBuffers(MIXERCHANNEL *d, short **bufs, long count)
{
for (long i = 0; i < count; i++)
	d->tmpPs[i] = bufs[i];
}	// ---- end MixerChannel_SetTempBuffers() ---- 

// *************************************************************** 
// RunMixerChanneli:	master mixing function
// ***************************************************************
    void 
RunMixerChanneli(Q15 **inPs, Q15 **outPs, long length, MIXERCHANNEL *d)
{
long i = 0;
Q15 **tmpPs = d->tmpPs;

//{static long c=0; printf("Run_MixerChanneli%d : start type=%d\n", c++, d->type);}

switch (d->type)
	{
	default:
	case kMixerChannel_Type_In1_Out1:
	{
#ifdef SAFE
	ScaleShortsi_Q15(inPs[0], tmpPs[0], length, d->gaini[kLeft]);
	// Add/Copy to Submix buffer
	if (d->addToOutput) 
		AddShorts(tmpPs[0], outPs[0], length, True);
	else
		CopyShorts(tmpPs[0], outPs[0], length);
#endif
	if (d->addToOutput) 
		MACShortsi_Q15(inPs[0], outPs[0], length, d->gaini[kLeft]);
	else
    	ScaleShortsi_Q15(inPs[0], tmpPs[0], length, d->gaini[kLeft]);
	}
	break;

// Processing chain is applied, then pan  signal to stereo
	case kMixerChannel_Type_In1_Out2:
    {
	// Add/Copy to Output/Submix buffers with Center panning
	if (d->addToOutput) 
		{
		MACShortsi_Q15(inPs[0], outPs[0], length, d->gaini[kLeft ]);
		MACShortsi_Q15(inPs[0], outPs[1], length, d->gaini[kRight]);
		}
	else
		{
		ScaleShortsi_Q15(inPs[0], outPs[0], length, d->gaini[kLeft ]);
		ScaleShortsi_Q15(inPs[0], outPs[1], length, d->gaini[kRight]);
		}
    }
	break;

// Process stereo channels independently 
	case kMixerChannel_Type_In2_Out2:
	{
	// Apply Gain 
	for (long ch = 0; ch < 2; ch++)
		{
	// Copy to Output buffers with Center panning
		if (d->addToOutput) 
		    MACShortsi_Q15(inPs[ch], outPs[ch], length, d->gaini[ch]);
		else
		    ScaleShortsi_Q15(inPs[ch], outPs[ch], length, d->gaini[ch]);
		}
	}
	break;
	}
}	// ---- end RunMixerChanneli() ---- 

// *************************************************************** 
// DefaultMixer:		Set Default high-level parameter values
// ***************************************************************
    void 
DefaultMixer(MIXER *d)
{
long i = 0;

d->outGainDB  = 0.0f;
d->headroomDB = kMixer_HeadroomDB_Default;
d->useFixedPoint = True;

for (i = 0; i < kMixer_MaxChannels; i++)
    {
//    short *buf = (short *) malloc(1024*sizeof(short));
    DefaultMixerChannel(&d->channels[i]);
//    SetMixerChannel_TempBuffers(&d->channels[i], &buf, 1);

// Mixer Channel Type_: In1_Out1, In1_Out2, In2_Out2  (NOT NEEDED: In2_Out1)
    d->channels[i].type = kMixerChannel_Type_In1_Out1;
    }

d->channelCount = 2;
}	// ---- end DefaultMixer() ---- 

// *************************************************************** 
// UpdateMixer:	Convert high-level parameter values to low-level data
// ***************************************************************
    void 
UpdateMixer(MIXER *d)
{
//{static long c=0; printf("UpdateMixer %d: start\n", c++);}

d->outGainf = DecibelToLinearf(d->outGainDB);
d->outGaini = FloatToQ15(d->outGainf);

for (long i = 0; i < d->channelCount; i++)
    {
    MIXERCHANNEL *ch = &d->channels[i];
    ch->headroomDB = d->headroomDB;
    UpdateMixerChannel(ch);
    ch->addToOutput = True;
    }
d->channels[0].addToOutput = False;
}	// ---- end UpdateMixer() ---- 

// *************************************************************** 
// SetMixer_ChannelParameters:	Specify fundamental parameters
// ***************************************************************
    void 
SetMixer_ChannelParameters(MIXER *d, long index, long type, float pan, float gainDB)
{
//printf("SetMixer_ChannelParameters: ch=%d type=%d pan=%g> gainDB=%g\n", index, type, pan, gainDB);
MIXERCHANNEL *mc = &d->channels[index];

gainDB += d->outGainDB;
Boundf(&gainDB, kMixerChannel_Level_Min, kMixerChannel_Level_Max);
mc->inGainDB = gainDB;

Boundf(&pan, kPanValue_FullLeft, kPanValue_FullRight);
mc->pan      = pan;

mc->type     = type;
}	// ---- end SetMixer_ChannelParameters() ---- 

// *************************************************************** 
// SetMixer_OutGainDB:	    Set to range [-96 .. 0] dB
// ***************************************************************
    void 
SetMixer_OutGainDB(MIXER *d, float x)
{
//if (x < kMixer_HeadroomDB_Min || x > kMixer_HeadroomDB_Max)
//    {
//    printf("SetMixer_HeadroomDB:  value %g out of parameter range [%g .. %g]\n",
//            x, kMixer_HeadroomDB_Min, kMixer_HeadroomDB_Max);
//    return (False);
//    }
d->outGainDB = x;
}	// ---- end SetMixer_OutGainDB() ---- 

// *************************************************************** 
// SetMixer_HeadroomDB:	    Set headroom in range [-96 .. 0] dB
//              Return Boolean success
// ***************************************************************
    int 
SetMixer_HeadroomDB(MIXER *d, float x)
{
if (x < kMixer_HeadroomDB_Min || x > kMixer_HeadroomDB_Max)
    {
    printf("SetMixer_HeadroomDB:  value %g out of parameter range [%g .. %g]\n",
            x, kMixer_HeadroomDB_Min, kMixer_HeadroomDB_Max);
    return (False);
    }
d->headroomDB = x;
return (True);
}	// ---- end SetMixer_HeadroomDB() ---- 

// *************************************************************** 
// SetMixer_ChannelCount:	Specify fundamental parameters
//              Return Boolean success
// ***************************************************************
    int 
SetMixer_ChannelCount(MIXER *d, long x)
{
if (x < 1 || x > kMixer_MaxChannels)
    {
    printf("SetMixer_ChannelCount: failed to set %d channels.  Use range [%d .. %d]\n",
            x, 1, kMixer_MaxChannels);
    return (False);
    }
d->channelCount = x;
return (True);
}	// ---- end SetMixer_ChannelCount() ---- 

// *************************************************************** 
// RunMixeri:	Run fixed-point mixer
// ***************************************************************
    void 
RunMixeri(Q15 **ins, Q15 **outs, long length, MIXER *d)
{
//{static long c=0; printf("RunMixeri %d: start\n", c++);}

for (long i = 0; i < d->channelCount; i++)
    RunMixerChanneli(&ins[i], outs, length, &d->channels[i]);
}	// ---- end RunMixer() ---- 

// *************************************************************** 
// RunMixer:	Run fixed-point mixer
// ***************************************************************
//    void 
//RunMixer(Q15 **ins, Q15 **outs, long length, MIXER *d)
//{
//{static long c=0; printf("RunMixeri %d: start\n", c++);}
//if (d->useFixedPoint)
//    RunMixeri(ins, outs, length, d);
//else
//    RunMixerf(ins, outs, length, d);
//}	// ---- end RunMixer() ---- 

// *************************************************************** 
// Mixer_SetChannelTempBuffers:	Assign ptrs to buffers 
// ***************************************************************
    void
Mixer_SetChannelTempBuffers(MIXER *d, short **bufs, long count)
{
for (long i = 0; i < d->channelCount; i++)
    {
    MixerChannel_SetTempBuffers(&d->channels[i], bufs, count);
    }
}	// ---- end Mixer_SetChannelTempBuffers() ---- 

// *************************************************************** 
// Test_Mixer:	 Keep this function at the end of the file
// ***************************************************************
void Test_Mixer()
{

{static long c=0; printf("Test_Mixer() %d: start \n", c++);}

}	// ---- end Test_Mixer() ---- 




