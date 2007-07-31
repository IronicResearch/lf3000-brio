// *************************************************************** 
// mix.cpp:		Audio channel mixing routines,
//			16-bit or 32-bit fixed point or a development
//			fixed-point/floating-point hybrid.
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "Dsputil.h"
#include "mix.h"

// *************************************************************** 
// DefaultMixerChannel:		Set Default high-level parameter values
// ***************************************************************
    void 
DefaultMixerChannel(MIXERCHANNEL *d)
{
long i = 0;

d->useFixedPoint = False;
d->type = kMixerChannel_Type_In1_Out1;
d->addToOutput = False;

d->enableEQ = False;
d->inGainDB = 0.0f;

for (i = 0; i < kMixerChannel_TempBufferCount; i++)
	d->tmpPs[i] = NULL;

// for (i = 0; i < 2; i++)
// DefaultEQ(&d->equalizer[i]);
d->reverbSendDB = 0.0f;
d->pan        = kPanValue_Center;
d->postGainDB = 0.0f;

d->samplingFrequency = 1.0f;
}	// ---- end DefaultMixerChannel() ---- 

// *************************************************************** 
// UpdateMixerChannel:	Convert high-level parameter values to low level data
// ***************************************************************
    void 
UpdateMixerChannel(MIXERCHANNEL *d)
{
long i = 0;

//{static long c=0; printf("UpdateMixerChannel %d: start\n", c++);}

d->inGainf = DecibelToLinearf(d->inGainDB);
//	UpdateEQ(&d->EQ;
d->reverbSendf = DecibelToLinear(d->reverbSendDB);

PanValues(d->pan, d->panValuesf);
//printf("UpdateMixerChannel: pan: x=%g -> <%g, %g>\n", d->pan, d->panValuesf[Left], d->panValuesf[Right]);

d->postGainf = DecibelToLinear(d->postGainDB);
}	// ---- end UpdateMixerChannel() ---- 

// *************************************************************** 
// ResetMixerChannel:	Reset unit to initial state
// ***************************************************************
    void 
ResetMixerChannel(MIXERCHANNEL *d)
{
long i = 0;
//{static long c=0; printf("ResetMixerChannel %d: start\n", c++);}
}	// ---- end ResetMixerChannel() ---- 

// *************************************************************** 
// PrepareMixerChannel:	Update() + Reset()
// ***************************************************************
    void 
PrepareMixerChannel(MIXERCHANNEL *d)
{
UpdateMixerChannel(d);
ResetMixerChannel(d);
}	// ---- end PrepareMixerChannel() ---- 

//============================================================================
// MixerChannel_SetSamplingFrequency:		
//============================================================================
	void
MixerChannel_SetSamplingFrequency(MIXERCHANNEL *d, float x)
{
d->samplingFrequency = x;

// Set frequencies of internal DSP modules
//for (long ch = 0; ch < 2; ch++)
//	Equalizer_SetSamplingFrequency(&d->equalizer[ch], x);
}	// ---- end MixerChannel_SetSamplingFrequency() ---- 

// *************************************************************** 
// MixerChannel_SetAllTempBuffers:	Assign ptrs to buffers 
// ***************************************************************
    void 
MixerChannel_SetAllTempBuffers(MIXERCHANNEL *d, short **bufs, long count)
{
for (long i = 0; i < count; i++)
	d->tmpPs[i] = bufs[i];
}	// ---- end MixerChannel_SetAllTempBuffers() ---- 

// *************************************************************** 
// Run_MixerChannel:	master router function
//
// ***************************************************************
    void 
Run_MixerChannel(short **inPs, short **outPs, long length, MIXERCHANNEL *d)
{
long i = 0;
//long index;
//short *inP, *outP;
//float k;
short **tmpPs = d->tmpPs;
//short **subPs = d->subMixBuffers;

{static long c=0; printf("Run_MixerChannel%d : start\n", c++);}

switch (d->type)
	{
	default:
	case kMixerChannel_Type_In1_Out1:
	{
	// Apply Gain and Equalization (EQ)
	// FIXXX: combine gain and EQ
	ScaleShortsf(inPs[0], tmpPs[0], length, d->inGainf);
	// RunEqualizer(tmpPs[0], tmpPs[0], length, &d->equalizer[0]);

	// Send to reverb
	// ScaleAddShorts(tmpPs[0], d->fxSendBuffers[kBrioMixer_FxSend_Reverb], length, True);

	// Add/Copy to Submix buffer
	if (d->addToOutput) 
		AddShorts(tmpPs[0], outPs[0], length, True);
	else
		CopyShorts(tmpPs[0], outPs[0], length);
	}
	break;
// Processing chain is applied, then pan  signal to stereo
	case kMixerChannel_Type_In1_Out2:
	// Apply Gain and Equalization (EQ)
		ScaleShortsf(inPs[0], tmpPs[0], length, d->inGainf);
	//	RunEqualizer(tmpPs[0], tmpPs[0], length, &d->equalizer[0]);

		// Send to reverb
	//	ScaleAddShorts(tmpPs[0], d->fxSendBuffers[kBrioMixer_FxSend_Reverb], length, True);

	// Add/Copy to Output/Submix buffers with Center panning
	if (d->addToOutput) 
		{
		AddShorts(tmpPs[0], outPs[0], length, True);
		AddShorts(tmpPs[0], outPs[1], length, True);
		}
	else
		{
		CopyShorts(tmpPs[0], outPs[0], length);
		CopyShorts(tmpPs[0], outPs[1], length);
		}
	break;
// Sum two channels, then process as single channel
	case kMixerChannel_Type_In2_Out1:
	{
	// Sum , Apply Gain and Equalization (EQ)
	Add2_Shortsi(inPs[0], inPs[1], tmpPs[0], length);
	ScaleShortsf(tmpPs[0], tmpPs[0], length, d->inGainf);
	// RunEqualizer(tmpPs[0], tmpPs[0], length, &d->equalizer[0]);

	// Send to reverb
	// ScaleAddShorts(tmpPs[0], d->fxSendBuffers[kBrioMixer_FxSend_Reverb], length, True);

	// Add/Copy to Output/Submix buffers with Center panning
	if (d->addToOutput) 
		AddShorts(tmpPs[0], outPs[1], length, True);
	else
		CopyShorts(tmpPs[0], outPs[0], length);
	}
	break;
// Process stereo channels independently 
	case kMixerChannel_Type_In2_Out2:
	{
	// Apply Gain and Equalization (EQ)
	for (long ch = 0; ch < 2; ch++)
		{
		ScaleShortsf(inPs[ch], tmpPs[ch], length, d->inGainf);
	//	RunEqualizer(tmpPs[ch], tmpPs[ch], length, &d->equalizer[ch]);

		// Send to reverb
	//	ScaleAddShorts(tmpPs[ch], d->fxSendBuffers[kBrioMixer_FxSend_Reverb], length, True);

		// Copy to Output buffers with Center panning
		if (d->addToOutput) 
			AddShorts(tmpPs[ch], outPs[ch], length, True);
		else
			CopyShorts(tmpPs[ch], outPs[ch], length);
		}
	}
	break;
	}

}	// ---- end Run_MixerChannel() ---- 

// *************************************************************** 
// Test_Mixer:	 Keep this function at the end of the file
// ***************************************************************
void Test_Mixer()
{

{static long c=0; printf("Test_Mixer() %d: start \n", c++);}

}	// ---- end Test_Mixer() ---- 




