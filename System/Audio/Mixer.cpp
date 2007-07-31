//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		AudioMixer.cpp
//
// Description:
//		The class to manage the processing of audio data on an audio channel.
//
//==============================================================================
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <AudioTypes.h>
#include <AudioPriv.h>
#include <Mixer.h>
#include <AudioOutput.h>
LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CAudioMixer implementation
//==============================================================================
CAudioMixer::CAudioMixer( U8 numChannels )
{
long i, ch;
	CDebugMPI debugMPI( kGroupAudio );

	masterVol_   = 100;
	numChannels_ = numChannels;

	DefaultBrioMixer(&pDSP_);
	pDSP_.channelCount = numChannels_;
printf("CAudioMixer::CAudioMixer: numChannels_=%d \n", numChannels_);

	// Allocate audio channels
	pChannels_ = new CChannel[ numChannels_ ];
	debugMPI.Assert((pChannels_ != kNull), "CAudioMixer::CAudioMixer: Mixer couldn't allocate channels!\n" );

	// Init midi player ptr
	pMidiPlayer_   = new CMidiPlayer( );
	
// Allocate and configure intermediate buffers and sampling rate conversion
	pMixBuffer_    = new S16[   kAudioOutBufSizeInWords * sizeof(S16)];
	pSRCInBuffer_  = new S16[ 2*kAudioOutBufSizeInWords * sizeof(S16)];
	pSRCOutBuffer_ = new S16[ 2*kAudioOutBufSizeInWords * sizeof(S16)];

long fsRack[3];
fsRack[0] =  8000;
fsRack[1] = 16000;
fsRack[2] = 32000;
for (i = 0; i < kAudioMixer_SRC_MixBinCount; i++)
	{
// Initialize sampling rate converters  
	for (ch = 0; ch < 2; ch++)
		{
		srcMixBinBufferPtrs_[i][ch] = new S16[2*kAudioOutBufSizeInWords * sizeof(S16)];
		DefaultSRC(&src_[i][ch]);
		src_[i][ch].type = kSRC_Interpolation_Type_Triangle; // Linear, Triangle, FIR
		SRC_SetInSamplingFrequency (&src_[i][ch], (float)fsRack[i]);
		SRC_SetOutSamplingFrequency(&src_[i][ch], (float)kAudioSampleRate);
		UpdateSRC(&src_[i][ch]);
		ResetSRC(&src_[i][ch]);
		}
	srcMixBinFilled_[i] = False;
	}

// Configure DSP engine
for (i = 0; i < pDSP_.channelCount; i++)
	{
	pChannels_[i].SetMixerChannelDataPtr(&pDSP_.channels[i]);
	}

for (i = 0; i < kAudioMixer_MaxTempBuffers; i++)
	{
	long bufWords = 2*kAudioOutBufSizeInWords + kSRC_Filter_MaxDelayElements;
	pTmpBuffers_[i] = new S16[ bufWords * sizeof(S16)];
	tmpBufOffsetPtrs_[i] = &pTmpBuffers_[i][kSRC_Filter_MaxDelayElements];
	}
}  // ---- end CAudioMixer::CAudioMixer() ----

//==============================================================================
// CAudioMixer::~CAudioMixer ::
//==============================================================================
CAudioMixer::~CAudioMixer()
{
long i;
	// Deallocate the channels
	if (pChannels_)
	{
		//fixme/dg: will this delete[] the array + the channels in the array too?
		delete [] pChannels_;
	}
	
	// Deallocate the midi player
	if (pMidiPlayer_)
		delete pMidiPlayer_;

// Dellocate buffers 
if (pMixBuffer_)
	free(pMixBuffer_);
if (pSRCInBuffer_)
	free(pSRCInBuffer_);
if (pSRCOutBuffer_)
	free(pSRCOutBuffer_);
for (i = 0; i < kAudioMixer_MaxTempBuffers; i++)
	{
	if (pTmpBuffers_[i])
		free(pTmpBuffers_[i]);
	}

for (long i = 0; i < kAudioMixer_SRC_MixBinCount; i++)
	{
	if (srcMixBinBufferPtrs_[i][0])
		free(srcMixBinBufferPtrs_[i][0]);
	if (srcMixBinBufferPtrs_[i][1])
		free(srcMixBinBufferPtrs_[i][1]);
	}
}  // ---- end ~CAudioMixer::CAudioMixer() ----

//==============================================================================
//==============================================================================
CChannel* CAudioMixer::FindChannelUsing( tAudioPriority priority )
{
	U8 iChan;
	
	// For now, just search for a channel not in use
	for (iChan=0; iChan<numChannels_; iChan++)
	{
		if (!pChannels_[iChan].IsInUse()) 
			return &pChannels_[iChan];
	}
	
	// Reaching this point means all channels are currently in use
	return kNull;
}

//==============================================================================
//==============================================================================
CChannel* CAudioMixer::FindChannelUsing( tAudioID id )
{
	U8 				iChan;
	tAudioID		idFromPlayer;
	CChannel*		pChan;
	CAudioPlayer* 	pPlayer;
	
	// Loop through mixer channels, look for one that's in use and then
	// test the player's ID against the requested id.
	for (iChan=0; iChan<numChannels_; iChan++)
	{
		pChan = &pChannels_[iChan];
		if (pChan->IsInUse()) {
			pPlayer = pChan->GetPlayer();
			idFromPlayer = pPlayer->GetAudioID();
			if ( idFromPlayer == id )
				return &pChannels_[iChan];
		}
	}
	
	// Reaching this point means all no ID matched.
	return kNull;
}

//==============================================================================
//==============================================================================
Boolean CAudioMixer::IsAnyAudioActive( void )
{
	Boolean 	result = false;
	U32 		iChan = 0;
	CChannel*	pChan = NULL;

	// Loop over the number of channels
	for (iChan = 0; iChan < numChannels_; iChan++)
	{
		pChan = &pChannels_[iChan];
	
		if (pChan->IsInUse())
			result = true;
	}
	
	return result;
}

//==============================================================================
//==============================================================================
int CAudioMixer::RenderBuffer( S16 *pOutBuff, U32 numFrames )
{
	U32		i, ch;
	U32 		framesRendered;
	U8		iChan;
	CChannel	*pChan;
long mixBinIndex;
long channelsPerFrame = 2;

//	printf("AudioMixer::RenderBuffer -- START \n");

	// fixme/dg: do proper DebugMPI-based output.
//	printf("AudioMixer::RenderBuffer -- bufPtr: 0x%x, frameCount: %u \n", (unsigned int)pOutBuff, (int)numStereoFrames );
	if ( (numFrames * kAudioBytesPerStereoFrame) != kAudioOutBufSizeInBytes )
		printf("AudioMixer::RenderBuffer -- frameCount doesn't match buffer size!!!\n");
		
	// Clear output buffers
	ClearShorts( pOutBuff, numFrames * channelsPerFrame );
	for (i = 0; i < kAudioMixer_MaxTempBuffers; i++)
		ClearShorts( pTmpBuffers_[i], numFrames * channelsPerFrame );

// printf("numStereoFrames=%ld kAudioBytesPerStereoFrame=%d kAudioOutBufSizeInBytes=%d \n",		numFrames, kAudioBytesPerStereoFrame, kAudioOutBufSizeInBytes);

// Clear stereo mix buffers
for (i = 0; i < kAudioMixer_SRC_MixBinCount; i++)
	{
	ClearShorts( srcMixBinBufferPtrs_[i][0], numFrames*channelsPerFrame);
	ClearShorts( srcMixBinBufferPtrs_[i][1], numFrames*channelsPerFrame);
	}

	//
	// ---- Render each channel to mix buffer with corresponding sampling frequency
	//
	for (iChan = 0; iChan < numChannels_; iChan++)
	{
	pChan = &pChannels_[iChan];

	// Render if channel is in use and not paused
	if (pChan->IsInUse() && !pChan->IsPaused())
		{
		long channelSamplingFrequency = pChan->GetSamplingFrequency();
		U32 framesToRender = (numFrames*channelSamplingFrequency)/32000;
//printf("ichan%d: framesToRender=%ld for %ld Hz\n", iChan, framesToRender, channelSamplingFrequency);

//		ClearShorts( pTmpBuffers_[0], numFrames * channelsPerFrame );
		framesRendered = pChan->RenderBuffer( pTmpBuffers_[0], framesToRender, False ); 
		Add2_Shortsi(pTmpBuffers_[0], pOutBuff, pOutBuff, numFrames*channelsPerFrame); 

//		framesRendered = pChan->RenderBuffer( pOutBuff, framesToRender, True ); 
			
	// If player has finished, release the channel.
		if ( framesRendered < framesToRender ) 
			pChan->Release( false );	// Don't suppress done msg if requested
			
	// Add output to appropriate Mix "Bin" 
		S16* srcMixBinP;
	// FUTURE:  convert 8000->16000Hz with less stringent anti-aliasing filter and let that mix bin's conversion do 16000->32000Hz
		if 	(8000 == channelSamplingFrequency)
		   	srcMixBinP = srcMixBinBufferPtrs_[kAudioMixer_SRC_MixBin_0_8000Hz ][0];
		else if (16000 == channelSamplingFrequency)
		   	srcMixBinP = srcMixBinBufferPtrs_[kAudioMixer_SRC_MixBin_1_16000Hz][0];
		else //if (32000 == channelSamplingFrequency)
		   	srcMixBinP = srcMixBinBufferPtrs_[kAudioMixer_SRC_MixBin_2_32000Hz][0];
//		CopyShorts(pTmpBuffers_[0], pOutBuff, framesToRender);
		Add2_Shortsi(pTmpBuffers_[0], srcMixBinP, srcMixBinP, numFrames*channelsPerFrame); 
		}
	}

	// MIDI player renders to output buffer if it has been activated by client.
	if ( pMidiPlayer_->IsActive() )
		{
//		long rateDivisor = 2;
		long midiFrames = (numFrames/2);
		short *mixBuf = srcMixBinBufferPtrs_[kAudioMixer_SRC_MixBin_1_16000Hz ][0];
		ClearShorts( pTmpBuffers_[0], midiFrames * 2 );
		framesRendered = pMidiPlayer_->RenderBuffer( pTmpBuffers_[0], midiFrames, False ); 
		Add2_Shortsi(pTmpBuffers_[0], mixBuf, mixBuf, midiFrames*2);
		}

// 
// Convert Mix buffers to output sampling rate
//
mixBinIndex = kAudioMixer_SRC_MixBin_2_32000Hz;
CopyShorts(srcMixBinBufferPtrs_[mixBinIndex][0], pOutBuff, numFrames*2);

// Deinterleave and process each channel separately
//mixBinIndex = kAudioMixer_SRC_MixBin_0_8000Hz;
//DeinterleaveShorts(srcMixBinBufferPtrs_[mixBinIndex][0], pTmpBuffers_[2], pTmpBuffers_[3], numFrames);
//RunSRC(pTmpBuffers_[2], pTmpBuffers_[0], numFrames/4, numFrames, &src_[mixBinIndex][0]);
//RunSRC(pTmpBuffers_[3], pTmpBuffers_[1], numFrames/4, numFrames, &src_[mixBinIndex][1]);
//InterleaveShorts(pTmpBuffers_[0], pTmpBuffers_[1], pTmpBuffers_[6], numFrames);
//Add2_Shortsi(pTmpBuffers_[6], pOutBuff, pOutBuff, numFrames*2);
//CopyShorts(pTmpBuffers_[6], pOutBuff, numFrames*2);

mixBinIndex = kAudioMixer_SRC_MixBin_1_16000Hz;
DeinterleaveShorts(srcMixBinBufferPtrs_[mixBinIndex][0], pTmpBuffers_[4], pTmpBuffers_[5], numFrames);
RunSRC(pTmpBuffers_[4], pTmpBuffers_[0], numFrames/2, numFrames, &src_[mixBinIndex][0]);
RunSRC(pTmpBuffers_[5], pTmpBuffers_[1], numFrames/2, numFrames, &src_[mixBinIndex][1]);
InterleaveShorts(pTmpBuffers_[0], pTmpBuffers_[1], pTmpBuffers_[6], numFrames);
Add2_Shortsi(pTmpBuffers_[6], pOutBuff, pOutBuff, numFrames*2);
//CopyShorts(pTmpBuffers_[6], pOutBuff, numFrames*2);
		
#if 0
// fixme/rdg:  needs CAudioEffectsProcessor
	// If at least one audio channel is playing, Process global audio effects
	if (numPlaying && (gAudioContext->pAudioEffects != kNull))
		gAudioContext->pAudioEffects->ProcessAudioEffects( kAudioOutBufSizeInWords, pOutBuff );
#endif

// Scale output by master volume
#define ORIG_MIXER_MASTER_VOLUME
#ifdef ORIG_MIXER_MASTER_VOLUME
	for (i = 0; i < kAudioOutBufSizeInWords; i++)
	{
// NOTE: volume here is interpreted as a linear value
		pOutBuff[i] = (S16)((pOutBuff[i] * (int)masterVol_) >> 7); // fixme; 
	} 
#else
// Run_BrioMixer(short **ins, short **outs, long length, BRIOMIXER *d)
// Initial integration code:  scale stereo/interleaved buffer 
ScaleShortsf(pOutBuff, pOutBuff, length, d->outGainf[0]);

#endif

//printf("AudioMixer::RenderBuffer -- end \n");
	return kNoErr;
}

//==============================================================================
//==============================================================================
int CAudioMixer::WrapperToCallRenderBuffer( S16 *pOut,  unsigned long numStereoFrames, void* pToObject  )
{
	// Cast void ptr to a this ptr:
	CAudioMixer* mySelf = (CAudioMixer*)pToObject;
	
	// Call member function to get a buffer full of stereo data
	return mySelf->RenderBuffer( pOut, numStereoFrames );
}


LF_END_BRIO_NAMESPACE()
// EOF	
