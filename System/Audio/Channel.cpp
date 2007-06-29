//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		Channel.cpp
//
// Description:
//		The class to manage the processing of audio data on an audio channel.
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <AudioTypes.h>
#include <AudioPriv.h>
#include <Channel.h>

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CChannel implementation
//==============================================================================

CChannel::CChannel()
{
	// Initialize the class variables
//	mpChain_ = kNull;
	pPlayer_ = kNull;
	bInUse_ = 0;
	bPaused_ = 0;
	bOwnProcessor_ = 0;
	volume_ = 0;
	pan_ = 0;
	
	// Allocate the channel's output buffer
	pOutBuffer_ = new S16[ kAudioOutBufSizeInWords ];
}

//==============================================================================
//==============================================================================
CChannel::~CChannel()
{
	// Free the channel buffers
	if (pOutBuffer_)
		delete pOutBuffer_;
}

//==============================================================================
//==============================================================================
tErrType CChannel::InitChanWithPlayer( CAudioPlayer* pPlayer )
{
	// If we're pre-empting, release the active player first.
	if (pPlayer_ != kNull)
		Release( false );		// don't suppress done msg if it has been requested
	
	pPlayer_ = pPlayer;
	bInUse_ = 1;
	SetPan( pPlayer->GetPan() );
	SetVolume( pPlayer->GetVolume() );
//	printf("CChannel::InitChanWithPlayer - setting volume %f\n", volume_);
	inSampleRate_ = pPlayer_->GetSampleRate();
//	InitConversionRate(&convRate_, inSampleRate_, kAudioSampleRate);
	
	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CChannel::Release( Boolean suppressPlayerDoneMsg )
{
//	pFxChain_ = kNull;

//	printf("CChannel::Release - deleting player 0x%x\n", (unsigned int)pPlayer_);

	if (suppressPlayerDoneMsg)
		pPlayer_->SetSendDoneMessage( false );

	delete pPlayer_;
	pPlayer_ = kNull;
	bInUse_ = 0;
	bPaused_ = 0;
	bOwnProcessor_ = 0;
	pan_ = 0;
	volume_ = 0;
	inSampleRate_ = 0;

	return kNoErr;
}

//==============================================================================
//==============================================================================
U32 CChannel::RenderBuffer( S16 *pMixBuff, U32 numStereoFrames  )
{
	U32 playerFramesRendered = 0;
	U32 numStereoSamples = numStereoFrames * kAudioBytesPerSample;
		
 //	 printf("CChannel::RenderBufferRenderBuffer -- chan bufPtr: 0x%x, channel: 0x%x \n", (unsigned int)pOutBuffer_, (unsigned int)this );

	// Initialize the output buffer to 0.  This has the side effect of zero
	// padding the output if the player runs out of data.
	bzero( pOutBuffer_, kAudioOutBufSizeInBytes );

	// Have player render its data into our output buffer.  If the player
	// contains mono data, it will be rendered out as stereo data.
	playerFramesRendered = pPlayer_->RenderBuffer( pOutBuffer_, numStereoFrames );
	
//	printf("Twenty Samples from channel buffer after player render:\n");
//	for (int i = 0; i < 20; i++) {
//		printf("0x%x " , pOutBuffer_[i]);
//	}
//	printf("\n");
	
	// decide how to deal with player done i.e. playerFramesRendered comes back 
	// less than numStereoFrames: does player send done, or channel.
	
// fixme/dg:  needs CAudioEffectsProcessor
#if 0
	// If the channel has an effects processor, render the effect into the
	// pOutBuffer_, then sample rate convert after.
	if (pFxChain_ != kNull)
		pFxChain->ProcessAudioEffects( kAudioOutBufSizeInWords, pOutBuffer_ );
#endif

	// First Check if conversion is necesssary
//		if ( kAudioSampleRate != inSampleRate_)
//	if ( 0 )
//	{
		// convert channel's output buffer and MIX(!) it to caller's buffer.
//		ConvertData(&convRate_, numStereoSamples, pOutBuffer_, pMixBuff);
//	}
	// Otherwise data is already at the correct sample rate
	// so just need to add it to the mix buffer
//	else
//	{
		S16	*pChanData = pOutBuffer_;
		S32	sum;
		for (U32 i = 0; i < numStereoSamples; i++)
		{
#if LF_BRIO_AUDIO_DO_FLOATINGPOINT_GAIN_CONTROL
			float chanOutputSample;
			// Be sure the total sum stays within range
			chanOutputSample = (float)*pChanData++;
			chanOutputSample *= volume_; 			// Apply channel gain
			sum = *pMixBuff + (S16)chanOutputSample;			
#else
			// Integer scaling for gain control.
			S32 scaledOutputSample = (*pChanData++ * volume_) >> 7; // fixme
			sum = *pMixBuff + scaledOutputSample;				

//			sum = *pMixBuff + *pChanData++;		// working, no volume		
#endif
			if (sum > kS16Max) sum = kS16Max;
			else if (sum < kS16Min) sum = kS16Min;
			
			*pMixBuff++ = (S16)sum;
		}
//	}

	return playerFramesRendered;
}

// EOF	
