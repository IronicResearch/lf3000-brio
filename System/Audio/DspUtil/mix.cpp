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

d->inGainDB = 0.0f;

for (i = 0; i < kMixerChannel_TempBufferCount; i++)
	d->tmpPs[i] = NULL;

//d->useEQ = False;
//for (i = 0; i < kMixerChannel_EQ_MaxBands; i++)
// DefaultEQ(&d->equalizer[i]);

//d->reverbSendDB = 0.0f;
d->pan        = kPanValue_Center;
d->outGainDB = 0.0f;

d->samplingFrequency = 1.0f;
}	// ---- end DefaultMixerChannel() ---- 

// *************************************************************** 
// UpdateMixerChannel:	Convert high-level parameter values to low level data
// ***************************************************************
    void 
UpdateMixerChannel(MIXERCHANNEL *d)
{
//{static long c=0; printf("UpdateMixerChannel %d: start\n", c++);}

d->inGainf = DecibelToLinearf(d->inGainDB);
d->inGaini = FloatToQ15(d->inGainf);
//for (long i = 0; i < kMixerChannel_EQ_MaxBands; i++)
//	UpdateEQ(&d->EQ[i]);
//d->reverbSendf = DecibelToLinear(d->reverbSendDB);

PanValues(d->pan, d->panValuesf);
//printf("UpdateMixerChannel: pan: x=%g -> <%g, %g>\n", d->pan, d->panValuesf[Left], d->panValuesf[Right]);

d->outGainf = DecibelToLinear(d->outGainDB);
}	// ---- end UpdateMixerChannel() ---- 

// *************************************************************** 
// ResetMixerChannel:	Reset unit to initial state
// ***************************************************************
    void 
ResetMixerChannel(MIXERCHANNEL *d)
{
//{static long c=0; printf("ResetMixerChannel %d: start\n", c++);}
//for (long i = 0; i < kMixerChannel_EQ_MaxBands; i++)
//	ResetEQ(&d->EQ[i]);
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

// ============================================================================
// MixerChannel_SetSamplingFrequency:		
// ============================================================================
	void
MixerChannel_SetSamplingFrequency(MIXERCHANNEL *d, float x)
{
d->samplingFrequency = x;
// FIXX: add bounding code
// Set frequencies of internal DSP modules
//for (long ch = 0; ch < kMixerChannel_EQ_MaxBands; ch++)
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
// RunMixerChannelf:	master router function
//
// ***************************************************************
    void 
RunMixerChannelf(short **inPs, short **outPs, long length, MIXERCHANNEL *d)
{
long i = 0;
//short *inP, *outP;
short **tmpPs = d->tmpPs;
//short **subPs = d->subMixBuffers;

//{static long c=0; printf("RunMixerChannelf%d : start\n", c++);}

switch (d->type)
	{
	default:
	case kMixerChannel_Type_In1_Out1:
	{
	// Apply Gain and Equalization (EQ)
	// FIXXX: combine gain and EQ
	ScaleShortsf(inPs[0], tmpPs[0], length, d->inGainf);
//  for (i = 0; i < kMixerChannel_EQ_MaxBands; i++)
//	    RunEqualizer(tmpPs[0], tmpPs[0], length, &d->equalizer[i]);

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
//  for (i = 0; i < kMixerChannel_EQ_MaxBands; i++)
//	    RunEqualizer(tmpPs[0], tmpPs[0], length, &d->equalizer[i]);

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
//	case kMixerChannel_Type_In2_Out1:
//	{
	// Sum , Apply Gain and Equalization (EQ)
//	Add2_Shortsi(inPs[0], inPs[1], tmpPs[0], length);
//	ScaleShortsf(tmpPs[0], tmpPs[0], length, d->inGainf);
//  for (i = 0; i < kMixerChannel_EQ_MaxBands; i++)
//	    RunEqualizer(tmpPs[0], tmpPs[0], length, &d->equalizer[i]);

	// Send to reverb
	// ScaleAddShorts(tmpPs[0], d->fxSendBuffers[kBrioMixer_FxSend_Reverb], length, True);

	// Add/Copy to Output/Submix buffers with Center panning
//	if (d->addToOutput) 
//		AddShorts(tmpPs[0], outPs[1], length, True);
//	else
//		CopyShorts(tmpPs[0], outPs[0], length);
//	}
//	break;

// Process stereo channels independently 
	case kMixerChannel_Type_In2_Out2:
	{
	// Apply Gain and Equalization (EQ)
	for (long ch = 0; ch < 2; ch++)
		{
		ScaleShortsf(inPs[ch], tmpPs[ch], length, d->inGainf);
// NOTE:  not enough EQ structs for this mixer type.  Think about whether you need this channel type
//      for (i = 0; i < kMixerChannel_EQ_MaxBands; i++)
//	        RunEqualizer(tmpPs[0], tmpPs[0], length, &d->equalizer[i]);

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
}	// ---- end RunMixerChannelf() ---- 

// *************************************************************** 
// RunMixerChanneli:	master router function
// ***************************************************************
    void 
RunMixerChanneli(Q15 **inPs, Q15 **outPs, long length, MIXERCHANNEL *d)
{
long i = 0;
//long index;
//short *inP, *outP;
//float k;
Q15 **tmpPs = d->tmpPs;
//short **subPs = d->subMixBuffers;

//{static long c=0; printf("Run_MixerChanneli%d : start type=%d\n", c++, d->type);}

switch (d->type)
	{
	default:
	case kMixerChannel_Type_In1_Out1:
	{
	// Apply Gain and Equalization (EQ)
	// FIXXX: combine gain and EQ
#ifdef SAFE
	ScaleShortsi_Q15(inPs[0], tmpPs[0], length, d->inGaini);
//  if (useEQ)
//      {
//      for (i = 0; i < kMixerChannel_EQ_MaxBands; i++)
//	        RunEqualizer(tmpPs[0], tmpPs[0], length, &d->equalizer[i]);
//      }

	// Send to reverb
	// ScaleAddShorts(tmpPs[0], d->fxSendBuffers[kBrioMixer_FxSend_Reverb], length, True);

	// Add/Copy to Submix buffer
	if (d->addToOutput) 
		AddShorts(tmpPs[0], outPs[0], length, True);
	else
		CopyShorts(tmpPs[0], outPs[0], length);
#endif
	if (d->addToOutput) 
		MACShortsi_Q15(inPs[0], outPs[0], length, d->inGaini);
	else
    	ScaleShortsi_Q15(inPs[0], tmpPs[0], length, d->inGaini);
	}
	break;

// Processing chain is applied, then pan  signal to stereo
	case kMixerChannel_Type_In1_Out2:
    {
	// Apply Gain and Equalization (EQ)
		ScaleShortsi(inPs[0], tmpPs[0], length, d->inGaini);
//      if (useEQ)
//          {
//          for (i = 0; i < kMixerChannel_EQ_MaxBands; i++)
//	            RunEqualizer(tmpPs[0], tmpPs[0], length, &d->equalizer[i]);
//          }

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
    }
	break;

// Process stereo channels independently (GKNOTE: is this type needed ?  Should use two 1 In/1 Out)
	case kMixerChannel_Type_In2_Out2:
	{
	// Apply Gain and Equalization (EQ)
	for (long ch = 0; ch < 2; ch++)
		{
		ScaleShortsi_Q15(inPs[ch], tmpPs[ch], length, d->inGaini);
// NOTE: not enough EQ structs for this 
//      if (useEQ)
//          {
//          for (i = 0; i < kMixerChannel_EQ_MaxBands; i++)
//	            RunEqualizer(tmpPs[0], tmpPs[0], length, &d->equalizer[i]);
//          }
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
}	// ---- end RunMixerChanneli() ---- 

// *************************************************************** 
// DefaultMixer:		Set Default high-level parameter values
// ***************************************************************
    void 
DefaultMixer(MIXER *d)
{
long i = 0, ch = 0;

d->useOutEQ          = False;
d->useOutSoftClipper = False;
d->outGainDB = 0.0f;

// ---- Configure channel section
d->channelCount = 5;
//for (i = 0; i < kMixerChannel_TempBufferCount; i++)
//	d->tmpPs[i] = NULL;
for (i = 0; i < kMixer_MaxInChannels; i++)
    {
//    short *buf = (short *) malloc(1024*sizeof(short));
    DefaultMixerChannel(&d->channels[i]);
//    MixerChannel_SetAllTempBuffers(&d->channels[i], &buf, 1);

    d->channels[i].type = kMixerChannel_Type_In1_Out1; // In1_Out1, In1_Out2, In2_Out2
    }

// ---- Configure output section
for (ch = 0; ch < kMixer_MaxOutChannels; ch++)
    {
// Configure EQ
    for (i = 0; i < kMixer_OutEQ_MaxBands; i++)
         DefaultEQ(&d->outEQ[ch][i]);
    SetEQ_Parameters(&d->outEQ[ch][0], 1000.0f, 2.0f, 0.0f, kEQ_Mode_Parametric);
    SetEQ_Parameters(&d->outEQ[ch][1], 2000.0f, 2.0f, 0.0f, kEQ_Mode_Parametric);
    SetEQ_Parameters(&d->outEQ[ch][2], 4000.0f, 2.0f, 0.0f, kEQ_Mode_Parametric);

// Configure Soft Clipper
    DefaultWaveShaper(&d->outSoftClipper[ch]);
    SetWaveShaper_Parameters(&d->outSoftClipper[ch], kWaveShaper_Type_V4, 0.0f, 0.0f);
    }

d->samplingFrequency = 1.0f;
}	// ---- end DefaultMixer() ---- 

// *************************************************************** 
// UpdateMixer:	Convert high-level parameter values to low-level data
// ***************************************************************
    void 
UpdateMixer(MIXER *d)
{
long ch = 0, i = 0;

//{static long c=0; printf("UpdateMixer %d: start\n", c++);}

d->outGainf = DecibelToLinearf(d->outGainDB);
d->outGaini = FloatToQ15(d->outGainf);

for (ch = 0; ch < d->channelCount; ch++)
    {
    UpdateMixerChannel(&d->channels[ch]);
    d->channels[ch].addToOutput = True;

    for (i = 0; i < kMixer_OutEQ_MaxBands; i++)
        UpdateEQ(&d->outEQ[ch][i]);
    UpdateWaveShaper(&d->outSoftClipper[ch]);
    }
d->channels[0].addToOutput = False;  // First mixer channel isn't additive

}	// ---- end UpdateMixer() ---- 

// *************************************************************** 
// ResetMixer:	Set to initial state
// ***************************************************************
    void 
ResetMixer(MIXER *d)
{
//{static long c=0; printf("ResetMixer %d: start\n", c++);}
long ch = 0;
for (ch = 0; ch < d->channelCount; ch++)
    {
    ResetMixerChannel(&d->channels[ch]);

    for (long i = 0; i < kMixer_OutEQ_MaxBands; i++)
        UpdateEQ(&d->outEQ[ch][i]);
    UpdateWaveShaper(&d->outSoftClipper[ch]);
    }
}	// ---- end ResetMixer() ---- 

// *************************************************************** 
// PrepareMixer:	Update() + Reset()
// ***************************************************************
    void 
PrepareMixer(MIXER *d)
{
UpdateMixer(d);
ResetMixer(d);
}	// ---- end PrepareMixerChannel() ---- 

// *************************************************************** 
// RunMixerf:	Set to initial state
// ***************************************************************
    void 
RunMixerf(short **ins, short **outs, long length, MIXER *d)
{
//{static long c=0; printf("RunMixerf %d: start\n", c++);}

for (long i = 0; i < d->channelCount; i++)
    {
    RunMixerChannelf(&ins[2*i], outs, length, &d->channels[i]);
    }
}	// ---- end RunMixerf() ---- 

// *************************************************************** 
// RunMixeri:	Run fixed-point mixer
// ***************************************************************
    void 
RunMixeri(Q15 **ins, Q15 **outs, long length, MIXER *d)
{
//{static long c=0; printf("RunMixeri %d: start\n", c++);}

for (long i = 0; i < d->channelCount; i++)
    {
    RunMixerChanneli(&ins[2*i], outs, length, &d->channels[i]);
    }
}	// ---- end RunMixer() ---- 

// *************************************************************** 
// Mixer_SetAllChannelTempBuffers:	Assign ptrs to buffers 
// ***************************************************************
    void 
Mixer_SetAllChannelTempBuffers(MIXER *d, short **bufs, long count)
{
for (long i = 0; i < d->channelCount; i++)
    {
    MixerChannel_SetAllTempBuffers(&d->channels[i], bufs, count);
    }
}	// ---- end Mixer_SetAllChannelTempBuffers() ---- 

// ============================================================================
// Mixer_SetSamplingFrequency:		
// ============================================================================
	void
Mixer_SetSamplingFrequency(MIXER *d, float x)
{
d->samplingFrequency = x;
// FIXX: add bounding code

// Set frequencies of internal DSP modules
for (long ch = 0; ch < kMixerChannel_EQ_MaxBands; ch++)
    {
    for (long i = 0; i < kMixerChannel_EQ_MaxBands; i++)
    	SetEQ_SamplingFrequency(&d->outEQ[ch][i], x);
	SetWaveShaper_SamplingFrequency(&d->outSoftClipper[ch], x);
    }
}	// ---- end Mixer_SetSamplingFrequency() ---- 

// *************************************************************** 
// Test_Mixer:	 Keep this function at the end of the file
// ***************************************************************
void Test_Mixer()
{

{static long c=0; printf("Test_Mixer() %d: start \n", c++);}

}	// ---- end Test_Mixer() ---- 




