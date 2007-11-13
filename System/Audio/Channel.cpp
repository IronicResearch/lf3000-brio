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

#define kPan_Default    0
#define kPan_Min    (-100)
#define kPan_Max      100

#define kVolume_Default  100
#define kVolume_Min        0
#define kVolume_Max      100

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CChannel implementation
//==============================================================================
CChannel::CChannel()
{
// Initialize class variables
//{static int c=0; printf("CChannel::CChannel %d \n", c++);}

//	mpChain_ = kNull;
	pPlayer_ = kNull;
	fInUse_  = false;
	fPaused_ = false;
	fOwnProcessor_ = false;
	fReleasing_ = false;
	
// Channel Interface parameters
	pan_    = kPan_Default;
	volume_ = kVolume_Default;

// Channel DSP Engine
	pDSP_ = NULL;
//	DefaultMixerChannel(&pDSP_);
//	pDSP_->inGainDB = 0.0f;
//	pDSP_->EQ;
//	pDSP_->reverbSendDB = 0.0f;
//	pDSP_->pan         = kPanValue_Center;
//	pDSP_->postGainDB = 0.0f;

	pDebugMPI_ = new CDebugMPI( kGroupAudio );
	pDebugMPI_->SetDebugLevel( kAudioDebugLevel ); // kDbgLvlVerbose, kAudioDebugLevel
//	pDebugMPI_->DebugOut( kDbgLvlVerbose, "CChannel::CChannel: volume_=%d pan_=%d\n", volume_, pan_);
//	printf("CChannel::CChannel: printf volume_=%d pan_=%d \n", volume_, pan_);
}	// ---- end CChannel ----

//==============================================================================
// ~CChannel
//==============================================================================
CChannel::~CChannel()
{
//{static int c=0; printf("CChannel::~CChannel %d \n", c++);}
	// Free debug MPI
	if (pDebugMPI_)
		delete pDebugMPI_;
}	// ---- end ~CChannel ----

//==============================================================================
// InitChanWithPlayer
//==============================================================================
tErrType CChannel::InitChanWithPlayer( CAudioPlayer* pPlayer )
{
//{static long c=0; printf("CChannel::InitChanWithPlayer: %ld start \n", c++);}

	// If we're pre-empting, release the active player first.
	if (pPlayer_)
		Release( false );		// don't suppress done msg if it has been requested
	
	pPlayer_ = pPlayer;
	fInUse_  = true;

// Convert interface parameters to DSP level data and reset channel
	SetPan(    pPlayer->GetPan() );
	SetVolume( pPlayer->GetVolume() );
	samplingFrequency_ = pPlayer_->GetSampleRate();

	pDSP_->samplingFrequency = (float) samplingFrequency_;
//printf("CChannel::InitChanWithPlayer: samplingFrequency=%g \n", pDSP_->samplingFrequency);
//	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
//			"CChannel::InitChanWithPlayer - samplingFrequency=%g\n", 
//			static_cast<float>(pDSP_->samplingFrequency) );	
	MixerChannel_SetSamplingFrequency(pDSP_, samplingFrequency_);
	UpdateMixerChannel(pDSP_);
	ResetMixerChannel (pDSP_);
// FIXX: these buffers will migrate to briomixer.cpp/Mixer.cpp
//	MixerChannel_SetAllTempBuffers(pDSP_, tmpPs_, kChannel_MaxTempBuffers);

//	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
//			"CChannel::InitChanWithPlayer - setting volume %d\n", 
//			static_cast<int>(volume_) );	

// Convert high level parameters to values suitable for DSP
//printf("CChannel::InitChanWithPlayer : pan=%d volume=%d \n", pan_, volume_);

	return kNoErr;
}	// ---- end InitChanWithPlayer ----

//==============================================================================
// Release
//==============================================================================
tErrType CChannel::Release( Boolean suppressPlayerDoneMsg )
{
//	pFxChain_ = kNull;

//	pDebugMPI_->DebugOut( kDbgLvlVerbose,
//		"CChannel::Release - deleting player 0x%x\n", (unsigned int)pPlayer_);

	// we're resetting the channel
	fReleasing_ = true;
	
	// Send the done message back to the initiator of the PlayAudio() call.
	if (suppressPlayerDoneMsg)
		pPlayer_->SetSendDoneMessage( false );

	// Once this is set, the mixer will no longer call our RenderBuffer()
	// method so we're safe to delete the other resources.
	fPaused_ = false;
	fOwnProcessor_ = 0;

// Set DSP values
	pan_    = kPan_Default;
	volume_ = kVolume_Default;
	samplingFrequency_ = 0;

	// The player's dtors are protected with a mutex so they are
	// safe to be deleted.  They dtors will wait until RenderBuffer()
	// is complete before delete internal resources.
	delete pPlayer_;
	pPlayer_ = kNull;

	// no longer in use
	fReleasing_ = false;
	fInUse_     = false;

	return kNoErr;
}	// ---- end Release ----

//==============================================================================
// SetPan :     Channel stereo position   Left .. Center .. Right
//==============================================================================
void CChannel::SetPan( S8 x )
{
pan_ = BoundS8(&x, kPan_Min, kPan_Max);

// Convert input range to [-100 .. 100]range [0 .. 1] suitable
// for the constant power calculation
// ChangeRangef(x, L1, H1, L2, H2)
float xf = ChangeRangef((float)pan_, (float) kPan_Min, (float)kPan_Max, 0.0f, 1.0f);

// PanValues(xf, panValuesf);
ConstantPowerValues(xf, &panValuesf[kLeft], &panValuesf[kRight]);
//printf("CChannel::SetPan : printf %d -> <%f , %f> \n", x, panValuesf[kLeft], panValuesf[kRight]);
RecalculateLevels();
}	// ---- end SetPan ----

//==============================================================================
// SetVolume : Convert range [0 .. 100] to [-100 ..0] dB
//==============================================================================
void CChannel::SetVolume( U8 x )
{
//printf("CChannel::SetVolume : printf  %d\n", x);
volume_ = x; //BoundU8(&x, kVolume_Min, kVolume_Max);

// ChangeRangef(x, L1, H1, L2, H2)
// FIXX: move to decibels, but for now, linear volume
gainf = ChangeRangef((float)x, (float) kVolume_Min, (float)kVolume_Max, 0.0f, 1.0f);
gainf *= kDecibelToLinearf_m3dBf; // DecibelToLinearf(-3.0);
//gainf = ChangeRangef((float)x, 0.0f, 100.0f, -100.0f, 0.0f);
//gainf =  DecibelToLinearf(gainf);

//pDebugMPI_->DebugOut( kDbgLvlVerbose, 
//			"CChannel::SetVolume - %d -> %f\n", static_cast<int>(volume_) , static_cast<int>(gainf) );	
//printf( "CChannel::SetVolume - %d -> %g\n", volume_, gainf );	

//printf("%f dB -> %f \n", -3.01, DecibelToLinearf(-3.01));
//printf("%f dB -> %f \n", -6.02, DecibelToLinearf(-6.02));
//printf("sqrt(2)/2 = %g\n", 0.5f*sqrt(2.0));

RecalculateLevels();
}	// ---- end SetVolume ----

//==============================================================================
// RecalculateLevels : Recalculate levels when either pan or volume changes
//==============================================================================
void CChannel::RecalculateLevels()
{
//printf("CChannel::RecalculateLevels : \n");

levelsf[kLeft ] =  panValuesf[kLeft ]*gainf;
levelsf[kRight] =  panValuesf[kRight]*gainf;

// Convert 32-bit floating-point to Q15 fractional integer format 
levelsi[kLeft ] = FloatToQ15(levelsf[kLeft ]);
levelsi[kRight] = FloatToQ15(levelsf[kRight]);

//printf("CChannel::RecalculateLevels : levelsf L=%g R=%g\n", levelsf[kLeft], levelsf[kRight]);
}	// ---- end RecalculateLevels ----

//==============================================================================
// ShouldRender
//==============================================================================
Boolean CChannel::ShouldRender( void ) {
	Boolean result = false;
//printf("CChannel::ShouldRender : fInUse_=%d !fPaused_=%d && !fReleasing_=%d\n", fInUse_, !fPaused_, !fReleasing_);

	if (fInUse_ && !fPaused_ && !fReleasing_)
		result = true;
	
	return result;
}	// ---- end ShouldRender ----

//==============================================================================
// RenderBuffer
//==============================================================================
U32 CChannel::RenderBuffer(S16 *pOut, int numFrames )
{
	U32 numSamples = numFrames * 2;  // 2 channels
	S32 y;

 //	 printf("CChannel::RenderBufferRenderBuffer -- chan bufPtr: 0x%x, channel: 0x%x \n", (unsigned int)pOutBuffer_, (unsigned int)this );
//{static long c=0; printf("CChannel::RenderBuffer: START %ld \n", c++); }

	// decide how to deal with player done i.e. playerFramesRendered comes back 
	// less than numStereoFrames: does player send done, or channel.
	
// fixme/dg:  needs CAudioEffectsProcessor
#if 0
	// If channel has an effects processor, render the effect into pOutBuffer_
	if (pFxChain_ != kNull)
		pFxChain->ProcessAudioEffects( kAudioOutBufSizeInWords, pOutBuffer_ );
#endif
	int playerFramesRendered = pPlayer_->RenderBuffer( pOut, numFrames );

//printf("levelsf <%f , %f > \n", levelsf[kLeft], levelsf[kRight]);
//printf("levelsi <%f , %f > \n", Q15ToFloat(levelsi[kLeft]), Q15ToFloat(levelsi[kRight]));

// ---- Render to out buffer
for (U32 i = 0; i < numSamples; i += 2)
	{
// Integer scaling for gain control
//  		y = ((pOut[i] * volume_)>>7);		        // ORIG rdg	Darren	
//			y = (S32)(levelsf[kLeft] * (float)pOut[i]);	// FLOAT
		y = (S32) MultQ15(levelsi[kLeft], pOut[i]);	// Q15  1.15 Fixed-point	
// Saturate to 16-bit range				
	if      (y > kS16Max) y = kS16Max;
	else if (y < kS16Min) y = kS16Min;				
	pOut[i] = (S16)y;

		y = (S32) MultQ15(levelsi[kRight], pOut[i+1]);				
// Saturate to 16-bit range				
	if      (y > kS16Max) y = kS16Max;
	else if (y < kS16Min) y = kS16Min;				
	pOut[i+1] = (S16)y;
	}

return (playerFramesRendered);
}	// ---- end RenderPlayerToBuffer ----

LF_END_BRIO_NAMESPACE()
// EOF	
