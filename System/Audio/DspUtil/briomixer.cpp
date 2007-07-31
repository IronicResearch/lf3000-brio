// *************************************************************** 
// briomixer.cpp:		BrioMixer for Lighting 1.0
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "Dsputil.h"
#include "briomixer.h"


// *************************************************************** 
// DefaultBrioMixer:		Set Default high-level parameter values
// ***************************************************************
    void 
DefaultBrioMixer(BRIOMIXER *d)
{
long i = 0;

d->useFixedPoint = False;
d->channelCount  = kBrioMixer_MaxChannels;

d->enableChannel_Reverb = False;
d->enableOutEQ          = False;
d->computeTripleOuts    = False;

// ---- Input section	
for (i = 0; i < kBrioMixer_MaxChannels; i++)
	{
	DefaultMixerChannel(&d->channels[i]);
	d->channelEnabled[i] = False;
	}

// ---- Output section	
for (i = 0; i < kBrioMixer_Out_Count; i++)
	{
//	DefaultEQ(&d->out_EQ[i]);
	d->out_GainDB[i] = 0.0f;
	}

for (i = 0; i < kBrioMixer_SubMixBuffer_Count; i++)
	d->subMixBuffers[i] = NULL;
for (i = 0; i < kBrioMixer_TempBuffer_Count; i++)
	d->tempBuffers[i] = NULL;
for (i = 0; i < kBrioMixer_OutBuffer_Count; i++)
	d->outBuffers[i] = NULL;

d->samplingFrequency = 1.0f;
}	// ---- end DefaultBrioMixer() ---- 

// *************************************************************** 
// UpdateBrioMixer:	Convert high-level parameter values to low level data
// ***************************************************************
    void 
UpdateBrioMixer(BRIOMIXER *d)
{
long i = 0;

{static long c=0; printf("UpdateBrioMixer %d: start\n", c++);}

// ---- Input section	
for (i = 0; i < d->channelCount; i++)
	UpdateMixerChannel(&d->channels[i]);

// ---- Out section	
for (i = 0; i < kBrioMixer_Out_Count; i++)
	{
//	UpdateEQ(&d->out_EQ[i]);
	d->outGainf[i] = DecibelToLinear(d->out_GainDB[i]);
	}
}	// ---- end UpdateBrioMixer() ---- 

// *************************************************************** 
// ResetBrioMixer:	Reset unit to initial state
// ***************************************************************
    void 
ResetBrioMixer(BRIOMIXER *d)
{
long i = 0;

for (i = 0; i < kBrioMixer_MaxChannels; i++)
	{
	ResetMixerChannel(&d->channels[i]);
	}

// ---- Output section	
for (i = 0; i < kBrioMixer_Out_Count; i++)
	{
//	ResetEQ(&d->out_EQ[i]);
	}

// Reset Reverb state here

// Reset Dynamic Range Control here
}	// ---- end ResetBrioMixer() ---- 

// *************************************************************** 
// PrepareBrioMixer:	Update() + Reset()
// ***************************************************************
    void 
PrepareBrioMixer(BRIOMIXER *d)
{
UpdateBrioMixer(d);
ResetBrioMixer(d);
}	// ---- end PrepareBrioMixer() ---- 

// *************************************************************** 
// BrioMixer_ClearAllTempBuffers:	Zero contents of all Temp buffers
// ***************************************************************
    void 
BrioMixer_ClearAllTempBuffers(BRIOMIXER *d, long length)
{
for (long i = 0; i < kBrioMixer_TempBuffer_Count; i++)
	ClearShorts(d->tempBuffers[i], length);
}	// ---- end BrioMixer_ClearAllTempBuffers() ---- 

// *************************************************************** 
// BrioMixer_ClearAllSubMixBuffers:	Zero contents of all SubMix buffers
// ***************************************************************
    void 
BrioMixer_ClearAllSubMixBuffers(BRIOMIXER *d, long length)
{
for (long i = 0; i < kBrioMixer_SubMixBuffer_Count; i++)
	ClearShorts(d->subMixBuffers[i], length);
}	// ---- end BrioMixer_ClearAllSubMixBuffers() ---- 

// *************************************************************** 
// BrioMixer_ClearAllOutBuffers:	Zero contents of all Out buffers
// ***************************************************************
    void 
BrioMixer_ClearAllOutBuffers(BRIOMIXER *d, long length)
{
for (long i = 0; i < kBrioMixer_OutBuffer_Count; i++)
	ClearShorts(d->outBuffers[i], length);
}	// ---- end BrioMixer_ClearAllOutBuffers() ---- 

// *************************************************************** 
// BrioMixer_ClearAllFxSendBuffers:	Zero contents of all FxSend buffers
// ***************************************************************
    void 
BrioMixer_ClearAllFxSendBuffers(BRIOMIXER *d, long length)
{
for (long i = 0; i < kBrioMixer_FxSendBuffer_Count; i++)
	ClearShorts(d->fxSendBuffers[i], length);
}	// ---- end BrioMixer_ClearAllFxSendBuffers() ---- 

// *************************************************************** 
// BrioMixer_ClearAllBuffers:	Zero contents of all buffers
// ***************************************************************
    void 
BrioMixer_ClearAllBuffers(BRIOMIXER *d, long length)
{
BrioMixer_ClearAllTempBuffers   (d, length);BrioMixer_ClearAllSubMixBuffers (d, length);BrioMixer_ClearAllOutBuffers    (d, length);BrioMixer_ClearAllFxSendBuffers (d, length);
}	// ---- end BrioMixer_ClearAllBuffers() ---- 

// *************************************************************** 
// BrioMixer_SetAllTempBuffers:		Assign ptrs to buffers 
// ***************************************************************
    void 
BrioMixer_SetAllTempBuffers(BRIOMIXER *d, short **bufs, long count)
{
for (long i = 0; i < count; i++)
	d->tempBuffers[i] = bufs[i];
}	// ---- end BrioMixer_SetAllTempBuffers() ---- 

// *************************************************************** 
// BrioMixer_SetAllSubMixBuffers:	Assign ptrs to buffers	
// ***************************************************************
    void 
BrioMixer_SetAllSubMixBuffers(BRIOMIXER *d, short **bufs, long count)
{
for (long i = 0; i < count; i++)
	d->subMixBuffers[i] = bufs[i];
}	// ---- end BrioMixer_SetAllSubMixBuffers() ---- 

// *************************************************************** 
// BrioMixer_SetAllOutBuffers:		Assign ptrs to buffers	
// ***************************************************************
    void 
BrioMixer_SetAllOutBuffers(BRIOMIXER *d, short **bufs, long count)
{
for (long i = 0; i < count; i++)
	d->outBuffers[i] = bufs[i];
}	// ---- end BrioMixer_SetAllOutBuffers() ---- 

// *************************************************************** 
// BrioMixer_SetSubMixBuffer:	Set specified buffer
// ***************************************************************
    void 
BrioMixer_SetSubMixBuffer(BRIOMIXER *d, long index, short *buf)
{
d->subMixBuffers[index] = buf;
}	// ---- end BrioMixer_SetSubMixBuffer() ---- 

// *************************************************************** 
// BrioMixer_SetTempBuffer:	Set specified  buffer
// ***************************************************************
    void 
BrioMixer_SetTempBuffer(BRIOMIXER *d, long index, short *buf)
{
d->tempBuffers[index] = buf;
}	// ---- end BrioMixer_SetTempBuffer() ---- 

// *************************************************************** 
// BrioMixer_SetOutBuffer:	Set specified buffer
// ***************************************************************
    void 
BrioMixer_SetOutBuffer(BRIOMIXER *d, long index, short *buf)
{
d->outBuffers[index] = buf;
}	// ---- end BrioMixer_SetOutBuffer() ---- 

// *************************************************************** 
// BrioMixer_SetFxSendBuffer:	Set specified buffer
// ***************************************************************
    void 
BrioMixer_SetFxSendBuffer(BRIOMIXER *d, long index, short *buf)
{
d->fxSendBuffers[index] = buf;
}	// ---- end BrioMixer_SetFxSendBuffer() ---- 

// *************************************************************** 
// Run_BrioMixerf:	Brio Mixer implementation for Lighting 1.0
//
//			32-bit floating-point implementation
// ***************************************************************
    static void 
Run_BrioMixerf(short **inPs, short **outPs, long length, BRIOMIXER *d)
{
long i = 0;
long index;
short *inP, *outP;
float k;
short **tmpPs = d->tempBuffers;
short **subPs = d->subMixBuffers;

{static long c=0; printf("Run_BrioMixerf%d : start\n", c++);}

// Clear temp and output stereo buffers
//ClearShorts(tmpPs[0], length);
//ClearShorts(tmpPs[1], length);

// ---- Calculate Microphone monophonic input 
index = kBrioMixer_In_Microphone;
//	AddShorts(tmpPs[0], subPs[1], length, True);

// ---- Calculate VOX (CH1, CH2) monophonic inputs 
for (index = kBrioMixer_In_VOX_CH1; index <= kBrioMixer_In_VOX_CH2; index++)
	{
	inP = inPs[index];
	if (inP)
		{
		// Apply Gain and Equalization (EQ)
		// FIXXX: combine gain and EQ
		ScaleShortsf(inP, tmpPs[0], length, d->channels[index].inGainf);
//		RunEQ(tmpPs[0], tmpPs[0], );

		// Send to reverb
//	ScaleAddShorts(tmpPs[0], d->fxSendBuffers[kBrioMixer_FxSend_Reverb], length, True);

		// Pan to Submix buffers 
		// FIXXX: combine pan and gain
		ScaleShortsf(   tmpPs[0], tmpPs[0], length, d->channels[index].postGainf);
		ScaleAddShortsf(tmpPs[0], subPs[0], length, d->channels[index].panValuesf[kLeft ]);
		ScaleAddShortsf(tmpPs[0], subPs[1], length, d->channels[index].panValuesf[kRight]);
		}
	}

// ---- Calculate SFX (CH1, CH2) monophonic/stereo inputs 
for (index = kBrioMixer_In_SFX_CH1_Left; index <= kBrioMixer_In_SFX_CH2_Left; index += 2)
	{
	inP = inPs[index];
	if (inP)
		{
	// SFX channels can be mono or stereo, so check and process accordingly
		// Mono case
		if (!inPs[index+1])
			{
		// Apply Gain and Equalization (EQ)
		// FIXXX: combine gain and EQ
			ScaleShortsf(inP, tmpPs[0], length, d->channels[index].inGainf);
		// Send to reverb
	//		ScaleAddShorts(tmpPs[0], d->fxSendBuffers[kBrioMixer_FxSend_Reverb], length, True);

		// Pan to Submix buffers with Center panning
			ScaleShortsf(tmpPs[0], tmpPs[0], length, d->channels[index].inGainf);
			AddShorts(tmpPs[0], subPs[0], length, True);
			AddShorts(tmpPs[0], subPs[1], length, True);
			}
		// Stereo case
		else
			{
			for (long ch = kLeft; ch <= kRight; ch++)
				{
			// Apply Gain and Equalization (EQ) 
			// FIXXX: combine gain and EQ
				ScaleShortsf(inPs[index+ch], tmpPs[0], length, d->channels[index+ch].inGainf);
		//		RunEQ(tmpPs[0], tmpPs[0], length, d->channels[index+ch].eq);
			// Send to reverb
		//		ScaleAddShorts(tmpPs[0], d->fxSendBuffers[kBrioMixer_FxSend_Reverb], length, True);

			// Copy to Submix buffers with Stereo panning
				ScaleAddShortsf(tmpPs[0], subPs[ch], length, d->channels[index+ch].postGainf);
				}
			}

		}
	}

// ---- Calculate Music MSX (CH1, CH2) mono/stereo inputs 
// Combine Music channels (some mix of stereo and mono) and process stereo stream
// kBrioMixer_In_MSX_CH1_Left
{
ClearShorts(tmpPs[0], length);
ClearShorts(tmpPs[1], length);
for (index = kBrioMixer_In_MSX_CH1_Left; index <= kBrioMixer_In_MSX_CH1_Left; index += 2)
	{
// Mono case
	if (inPs[index] && !inPs[index+1])
		{
	//  Scale by gain and then copy to output temporary mix (Center Pan)
		ScaleAddShortsf(inPs[index], tmpPs[kLeft ], length, d->channels[index+kLeft].postGainf);
		ScaleAddShortsf(inPs[index], tmpPs[kRight], length, d->channels[index+kLeft].postGainf);
		}
// Stereo case
	else if (inPs[index] && inPs[index+1])
		{
	//  Scale by gain and then copy to output temporary mix (Center Pan)
		ScaleAddShortsf(inPs[index  ], tmpPs[kLeft ], length, d->channels[index+kLeft].postGainf);
		ScaleAddShortsf(inPs[index+1], tmpPs[kRight], length, d->channels[index+kRight].postGainf);
		}

	// Process summed music channels as a stereo stream
		for (long ch = kLeft; ch <= kRight; ch++)
			{
		// Apply Equalization (EQ)
//			Run_EQ(tmpPs[ch], tmpPs[ch], length, &eq[x]);
		// Send to reverb
	//		ScaleAddShorts(tmpPs[ch], d->fxSendBuffers[kBrioMixer_FxSend_Reverb], length, True);

		// Scale to Submix buffers with Center panning
			ScaleAddShortsf(tmpPs[ch], subPs[ch], length, d->channels[index].postGainf);
			}
		}
	}

//
// ----------- Compute section  (Headset/TV/Speaker or Stereo output)
//
if (d->computeTripleOuts)
	{
// ---- Process Submix for HeadSet output	
// Apply Equalization (EQ) and Gain
// FIXXXX: combine these by tucking gain into EQ coefficients
//	Run_EQ(subPs[kLeft], tmpPs[0], length, &d->out_EQ[x]);
	index = kBrioMixer_Out_Headset_Left;
	ScaleShortsf(subPs[kLeft ], d->outBuffers[index+0], length, d->outGainf[index+0]);
	ScaleShortsf(subPs[kRight], d->outBuffers[index+1], length, d->outGainf[index+1]);

// ---- Process Submix for TV output	
// Apply Equalization (EQ) and Gain
// FIXXXX: combine these by tucking gain into EQ coefficients
//	Run_EQ(subPs[kLeft], tmpPs[0], length, &d->out_EQ[x]);
	index = kBrioMixer_Out_TV_Left;
	ScaleShortsf(subPs[kLeft ], d->outBuffers[index+0], length, d->outGainf[index+0]);
	ScaleShortsf(subPs[kRight], d->outBuffers[index+1], length, d->outGainf[index+1]);

// ---- Process Submix for Speaker output	
// Apply Equalization (EQ) and Gain
// FIXXXX: combine these by tucking gain into EQ coefficients
//	Run_EQ(subPs[kLeft], tmpPs[0], length, &d->out_EQ[x]);
	index = kBrioMixer_Out_Speaker_Left;
	ScaleShortsf(subPs[kLeft ], d->outBuffers[index+0], length, d->outGainf[index+0]);
	ScaleShortsf(subPs[kRight], d->outBuffers[index+1], length, d->outGainf[index+1]);
	}
// Transfer Submix output to stereo output
else
	{
	CopyShorts(subPs[kLeft ], d->outBuffers[kLeft ], length);
	CopyShorts(subPs[kRight], d->outBuffers[kRight], length);
	}

}	// ---- end Run_BrioMixerf() ---- 

// *************************************************************** 
// Run_BrioMixeri:	Brio Mixer implementation for Lighting 1.0
//
//			16/32-bit fixed-point implementation
// ***************************************************************
    static void 
Run_BrioMixeri(short **ins, short **outs, long length, BRIOMIXER *d)
{
{static long c=0; printf("Run_BrioMixeri%d : start\n", c++);}

}	// ---- end Run_BrioMixeri() ---- 

// *************************************************************** 
// Run_BrioMixer:	Brio Mixer implementation for Lighting 1.0
//
// ***************************************************************
    void 
Run_BrioMixer(short **ins, short **outs, long length, BRIOMIXER *d)
{
{static long c=0; printf("Run_BrioMixer%d : start\n", c++);}

if (d->useFixedPoint)
	Run_BrioMixeri(ins, outs, length, d);
else
	Run_BrioMixerf(ins, outs, length, d);
}	// ---- end Run_BrioMixer() ---- 

// *************************************************************** 
// Test_BrioMixer:	 Keep this function at the end of the file
// ***************************************************************
void Test_BrioMixer()
{

{static long c = 0; printf("Test_BrioMixer() %d: start \n", c++);}

}	// ---- end Test_BrioMixer() ---- 




