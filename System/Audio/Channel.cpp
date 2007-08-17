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

LF_BEGIN_BRIO_NAMESPACE()

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
// Initialize class variables
//	mpChain_ = kNull;
	pPlayer_ = kNull;
	bInUse_  = 0;
	bPaused_ = 0;
	bOwnProcessor_ = 0;

// Channel Interface parameters
	volume_ = 0;
	pan_    = 0;

// Channel DSP Engine
	pDSP_ = NULL;
//	DefaultMixerChannel(&pDSP_);
//	pDSP_->inGainDB = 0.0f;
//	pDSP_->EQ;
//	pDSP_->reverbSendDB = 0.0f;
//	pDSP_->pan         = kPanValue_Center;
//	pDSP_->postGainDB = 0.0f;

	// Allocate the channel's output buffer
	pOutBuffer_ = new S16[ 2*kAudioOutBufSizeInWords ];  // Factor 2 to allow for 2x upsampling
	for (long i = 0; i < kChannel_MaxTempBuffers; i++)
		tmpPs_[i] = new S16[ 2*kAudioOutBufSizeInWords ];	

	pDebugMPI_ = new CDebugMPI( kGroupAudio );

	pDebugMPI_->SetDebugLevel( kAudioDebugLevel );
}	// ---- end CChannel::CChannel ----

//==============================================================================
// CChannel::~CChannel
//==============================================================================
CChannel::~CChannel()
{
	// Free tchannel buffers
	if (pOutBuffer_)
		delete pOutBuffer_;
	
	for (long i = 0; i < kChannel_MaxTempBuffers; i++)
		{
		if (tmpPs_[i])
			free(tmpPs_[i]);	
		}

	// Free debug MPI
	if (pDebugMPI_)
		delete pDebugMPI_;
}	// ---- end CChannel::~CChannel ----

//==============================================================================
// CChannel::InitChanWithPlayer
//==============================================================================
tErrType CChannel::InitChanWithPlayer( CAudioPlayer* pPlayer )
{
long ch = 0;
	// If we're pre-empting, release the active player first.
	if (pPlayer_ != kNull)
		Release( false );		// don't suppress done msg if it has been requested
	
	pPlayer_ = pPlayer;
	bInUse_  = 1;
ch = 0;

// Convert interface parameters to DSP level data and reset channel
	SetPan(    pPlayer->GetPan() );
	SetVolume( pPlayer->GetVolume() );
	inSampleRate_ = pPlayer_->GetSampleRate();

	pDSP_->samplingFrequency = (float) inSampleRate_;
//printf("CChannel::InitChanWithPlayer: samplingFrequency=%g \n", pDSP_->samplingFrequency);
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
			"CChannel::InitChanWithPlayer - samplingFrequency=%g\n", 
			static_cast<float>(pDSP_->samplingFrequency) );	
	MixerChannel_SetSamplingFrequency(pDSP_, inSampleRate_);
	UpdateMixerChannel(pDSP_);
	ResetMixerChannel (pDSP_);
// FIXX: these buffers will migrate to briomixer.cpp/Mixer.cpp
	MixerChannel_SetAllTempBuffers(pDSP_, tmpPs_, kChannel_MaxTempBuffers);

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
			"CChannel::InitChanWithPlayer - setting volume %d\n", 
			static_cast<int>(volume_) );	

// Convert high level parameters to values suitable for DSP
//printf("CChannel::InitChanWithPlayer : pan=%d volume=%d \n", pan_, volume_);

	return kNoErr;
}	// ---- end CChannel::InitChanWithPlayer ----

//==============================================================================
// CChannel::Release
//==============================================================================
tErrType CChannel::Release( Boolean suppressPlayerDoneMsg )
{
//	pFxChain_ = kNull;

	pDebugMPI_->DebugOut( kDbgLvlVerbose,
		"CChannel::Release - deleting player 0x%x\n", 
		(unsigned int)pPlayer_);

	// Send the done message back to the initiator of the PlayAudio() call.
	if (suppressPlayerDoneMsg)
		pPlayer_->SetSendDoneMessage( false );

	// Once this is set, the mixer will no longer call our RenderBuffer()
	// method so we're safe to delete the other resources.
	bInUse_  = false;
	bPaused_ = false;
	bOwnProcessor_ = 0;

// Set DSP player values to arbitrary 0's
	pan_    = 0;
	volume_ = 0;
	inSampleRate_ = 0;

	// The player's dtors are protected with a mutex so they are
	// safe to be deleted.  They dtors will wait until RenderBuffer()
	// is complete before delete internal resources.
	delete pPlayer_;
	pPlayer_ = kNull;

	return kNoErr;
}	// ---- end CChannel::Release ----

//==============================================================================
// CChannel::SetPan
//==============================================================================
void CChannel::SetPan( S8 x )
{
pan_ = x;
// ChangeRangef(x, L1, H1, L2, H2)
//#define kPanValue_FullLeft ( 1.0)
//#define kPanValue_Center      0.0
//#define kPanValue_FullRight (-1.0)
float xf = ChangeRangef((float)x, -100, 100, -1.0, 1.0);

//PanValues   (xf, panValuesf);
ConstantPowerValues(xf, &panValuesf[kLeft], &panValuesf[kRight]);
//printf("CChannel::SetPan: %d -> <%f, %f> \n", (int)x, panValuesf[kLeft],  panValuesf[kRight]);
}	// ---- end CChannel::SetPan ----

//==============================================================================
// CChannel::SetVolume : Convert range [0 .. 100] to [-100 ..0 dB]
//==============================================================================
void CChannel::SetVolume( U8 x )
{
volume_ = x;
// ChangeRangef(x, L1, H1, L2, H2)
//long xI = (long)x;

}	// ---- end CChannel::SetVolume ----

//==============================================================================
// CChannel::RenderBuffer
//==============================================================================
U32 CChannel::RenderBuffer( S16 *outP, U32 numFrames , long addToOutputBuffer )
{
	U32 playerFramesRendered = 0;
	U32 numStereoSamples = numFrames * kAudioBytesPerSample;
		
 //	 printf("CChannel::RenderBufferRenderBuffer -- chan bufPtr: 0x%x, channel: 0x%x \n", (unsigned int)pOutBuffer_, (unsigned int)this );

	// Initialize the output buffer to 0.  This has the side effect of zero
	// padding the output if the player runs out of data.
	bzero( tmpPs_[0], kAudioOutBufSizeInBytes );

	// Have player render its data into our output buffer.  If the player
	// contains mono data, it will be rendered out as stereo data.
	playerFramesRendered = pPlayer_->RenderBuffer( tmpPs_[0], numFrames );
		
	// decide how to deal with player done i.e. playerFramesRendered comes back 
	// less than numStereoFrames: does player send done, or channel.
	
// fixme/dg:  needs CAudioEffectsProcessor
#if 0
	// If channel has an effects processor, render the effect into pOutBuffer_
	if (pFxChain_ != kNull)
		pFxChain->ProcessAudioEffects( kAudioOutBufSizeInWords, pOutBuffer_ );
#endif

// Add rendered output to out buffer
	if (addToOutputBuffer)
		{
		for (U32 i = 0; i < numStereoSamples; i++)
			{
		// Integer scaling for gain control.  fixme
			S32 y = outP[i] + ((tmpPs_[0][i] * volume_)>>7);				
		// Saturate to 16-bit range				
			if      (y > kS16Max) y = kS16Max;
			else if (y < kS16Min) y = kS16Min;
				
			outP[i] = (S16)y;
			}
		}
// Copy rendered output to out buffer
	else
		{
		for (U32 i = 0; i < numStereoSamples; i++)
			{
		// Integer scaling for gain control.  fixme
			S32 y = ((tmpPs_[0][i] * volume_)>>7);
		// Saturate to 16-bit range				
			if      (y > kS16Max) y = kS16Max;
			else if (y < kS16Min) y = kS16Min;
				
			outP[i] = (S16)y;
			}
		}


	return playerFramesRendered;
}	// ---- end CChannel::RenderBuffer ----

LF_END_BRIO_NAMESPACE()
// EOF	
