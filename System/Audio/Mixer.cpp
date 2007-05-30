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
	tErrType err = kNoErr;
	CDebugMPI debugMPI( kGroupAudio );

	masterVol_ = 1.0;
	
	numChannels_ = numChannels;

	// Allocate the audio channels
	pChannels_ = new CChannel[ numChannels_ ];
	debugMPI.Assert((pChannels_ != kNull), "Mixer couldn't allocate channels!\n" );
	
	// Init midi player ptr.
	pMidiPlayer_ = new CMidiPlayer( );
}

//==============================================================================
//==============================================================================
CAudioMixer::~CAudioMixer()
{
	tErrType err = kNoErr;

	// Deallocate the channels
	if (pChannels_)
	{
		//fixme/dg: will this delete[] the array + the channels in the array too?
		delete [] pChannels_;
	}
	
	// Deallocate the midi player
	if (pMidiPlayer_)
		delete pMidiPlayer_;
}

//==============================================================================
//==============================================================================
int CAudioMixer::RenderBuffer( S16 *pOutBuff, U32 numStereoFrames )
{
	U32			i;
	U32 		framesRendered;
	U8			iChan;
	CChannel	*pChan;

	// fixme/dg: do proper DebugMPI-based output.
//	printf("AudioMixer::RenderOutput -- bufPtr: %x, frameCount: %d \n", pOutBuff, numStereoFrames );
	
	if ( (numStereoFrames * kAudioBytesPerStereoFrame) != kAudioOutBufSizeInBytes )
		printf("AudioMixer::RenderOutput -- frameCount doesn't match buffer size!!!\n");
		
	// Initialize the output buffer to 0
	bzero( pOutBuff, kAudioOutBufSizeInBytes );
	
	// Loop over the number of channels
	for (iChan = 0; iChan < numChannels_; iChan++)
	{
		pChan = &pChannels_[iChan];

		// Only need to process if channel is in use and not paused
		if (pChan->IsInUse() && !pChan->IsPaused())
		{
			// Have the channel get data and render it to the output buffer,
			// applying any per-channel effects if active.
			framesRendered = pChan->RenderBuffer( pOutBuff, numStereoFrames ); 
			
			// Check to see if the player has finished
			// If it has, release the channel.
			if ( framesRendered < numStereoFrames ) {
				pChan->Release();
			}
		}
	}

	// Have the MIDI player render to the output buffer,
	if ( kNull != pMidiPlayer_ )
		framesRendered = pMidiPlayer_->RenderBuffer( pOutBuff, numStereoFrames ); 

// fixme/rdg:  needs CAudioEffectsProcessor
#if 0
	// If at least one audio channel is playing
	// Process the global audio effects
	if (numPlaying && (gAudioContext->pAudioEffects != kNull))
		gAudioContext->pAudioEffects->ProcessAudioEffects( kAudioOutBufSizeInWords, pOutBuff );
#endif

	// FIXME - MG:Adjust for pan which is now set at 0.5 for each side
	// fixme/rdg: optimize out float math?
	S16* pMixBuff = (S16 *)pOutBuff;
	float samp;
	for (i = 0; i < kAudioOutBufSizeInWords; i++)
	{
		samp = (float)*pMixBuff;
		*pMixBuff++ = (S16)(samp * masterVol_); 
	} 

	return kNoErr;
}

//==============================================================================
//==============================================================================
int CAudioMixer::WrapperToCallRenderBuffer( S16 *pOutBuff, 
								unsigned long numStereoFrames, 
								void* pToObject  )
{
	// Cast void ptr to a this ptr:
	CAudioMixer* mySelf = (CAudioMixer*)pToObject;
	
	// Call member function.
	return mySelf->RenderBuffer( pOutBuff, numStereoFrames );
}

//==============================================================================
//==============================================================================
CChannel* CAudioMixer::FindBestChannel( tAudioPriority priority )
{
	U8 iChan;
	// For now, just search for a channel not in use
	for (iChan=0; iChan<numChannels_; iChan++)
	{
		if (!pChannels_[iChan].IsInUse()) return &pChannels_[iChan];
	}
	// Reaching this point means all channels are currently in use
	return kNull;
}

// EOF	
