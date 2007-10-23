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
//{static int c=0; printf("CChannel::CChannel %d \n", c++);}

//	mpChain_ = kNull;
	pPlayer_ = kNull;
	fInUse_  = false;
	fPaused_ = false;
	fOwnProcessor_ = false;
	fReleasing_ = false;
	
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

	pDebugMPI_ = new CDebugMPI( kGroupAudio );
	pDebugMPI_->SetDebugLevel( kDbgLvlVerbose); //kAudioDebugLevel );
//	pDebugMPI_->DebugOut( kDbgLvlVerbose, "CChannel::CChannel: volume_=%d pan_=%d\n", volume_, pan_);
//	printf("CChannel::CChannel: printf volume_=%d pan_=%d \n", volume_, pan_);
}	// ---- end CChannel::CChannel ----

//==============================================================================
// CChannel::~CChannel
//==============================================================================
CChannel::~CChannel()
{
{static int c=0; printf("CChannel::~CChannel %d \n", c++);}
	// Free debug MPI
	if (pDebugMPI_)
		delete pDebugMPI_;
}	// ---- end CChannel::~CChannel ----

//==============================================================================
// CChannel::InitChanWithPlayer
//==============================================================================
tErrType CChannel::InitChanWithPlayer( CAudioPlayer* pPlayer )
{
//{static long c=0; printf("CChannel::InitChanWithPlayer: %ld start \n", c++);}

	// If we're pre-empting, release the active player first.
	if (pPlayer_ != kNull)
		Release( false );		// don't suppress done msg if it has been requested
	
	pPlayer_ = pPlayer;
	fInUse_  = true;

// Convert interface parameters to DSP level data and reset channel
	SetPan(    pPlayer->GetPan() );
	SetVolume( pPlayer->GetVolume() );
	samplingFrequency_ = pPlayer_->GetSampleRate();

	pDSP_->samplingFrequency = (float) samplingFrequency_;
//printf("CChannel::InitChanWithPlayer: samplingFrequency=%g \n", pDSP_->samplingFrequency);
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
			"CChannel::InitChanWithPlayer - samplingFrequency=%g\n", 
			static_cast<float>(pDSP_->samplingFrequency) );	
	MixerChannel_SetSamplingFrequency(pDSP_, samplingFrequency_);
	UpdateMixerChannel(pDSP_);
	ResetMixerChannel (pDSP_);
// FIXX: these buffers will migrate to briomixer.cpp/Mixer.cpp
//	MixerChannel_SetAllTempBuffers(pDSP_, tmpPs_, kChannel_MaxTempBuffers);

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

	// we're resetting the channel
	fReleasing_ = true;
	
	// Send the done message back to the initiator of the PlayAudio() call.
	if (suppressPlayerDoneMsg)
		pPlayer_->SetSendDoneMessage( false );

	// Once this is set, the mixer will no longer call our RenderBuffer()
	// method so we're safe to delete the other resources.
	fPaused_ = false;
	fOwnProcessor_ = 0;

// Set DSP player values to arbitrary 0's
	pan_    = 0;
	volume_ = 0;
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
}	// ---- end CChannel::Release ----

//==============================================================================
// CChannel::SetPan :     Channel stereo position   Left .. Center .. Right
//==============================================================================
void CChannel::SetPan( S8 x )
{
S8 lower = -100;
S8 upper =  100;

pan_ = BoundS8(&x, lower, upper);

// Convert input range to [-100 .. 100]range [0 .. 1] suitable
// for the constant power calculation
// ChangeRangef(x, L1, H1, L2, H2)
float xf = ChangeRangef((float)pan_, (float) lower, (float)upper, 0.0f, 1.0f);

//#define kPanValue_FullLeft ( 0.0)
//#define kPanValue_Center    (0.5)
//#define kPanValue_FullRight (1.0)
// PanValues(xf, panValuesf);
ConstantPowerValues(xf, &panValuesf[kLeft], &panValuesf[kRight]);
//printf("CChannel::SetPan : printf %d -> <%f , %f> \n", x, panValuesf[kLeft], panValuesf[kRight]);
RecalculateLevels();
}	// ---- end CChannel::SetPan ----

//==============================================================================
// CChannel::SetVolume : Convert range [0 .. 100] to [-100 ..0] dB
//==============================================================================
void CChannel::SetVolume( U8 x )
{
//printf("CChannel::SetVolume : printf  %d\n", x);
volume_ = x;
// ChangeRangef(x, L1, H1, L2, H2)

// FIXX: move to decibels, but for now, linear volume
gainf = ChangeRangef((float)x, 0.0f, 100.0f, 0.0f, 1.0f);
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
}	// ---- end CChannel::SetVolume ----

//==============================================================================
// CChannel::RecalculateLevels : Recalculate levels when either pan or volume changes
//==============================================================================
void CChannel::RecalculateLevels()
{
//printf("CChannel::RecalculateLevels : \n");

levelsf[0] =  panValuesf[0]*gainf;
levelsf[1] =  panValuesf[1]*gainf;

// Convert 32-bit floating-point to Q15 fractional integer format 
levelsi[0] = FloatToQ15(levelsf[0]);
levelsi[1] = FloatToQ15(levelsf[1]);

//printf("CChannel::RecalculateLevels : levelsf [0]=%g [1]=%g\n", levelsf[0], levelsf[1]);
}	// ---- end CChannel::RecalculateLevels ----

//==============================================================================
// CChannel::ShouldRender
//==============================================================================
Boolean CChannel::ShouldRender( void ) {
	Boolean result = false;
//printf("CChannel::ShouldRender : fInUse_=%d !fPaused_=%d && !fReleasing_=%d\n", fInUse_, !fPaused_, !fReleasing_);

	if (fInUse_ && !fPaused_ && !fReleasing_)
		result = true;
	
	return result;
}	// ---- end CChannel::ShouldRender ----

//==============================================================================
// CChannel::RenderBuffer
//==============================================================================
U32 CChannel::RenderBuffer(S16 *pOut, S16 *pTmp, int numFrames, Boolean addToOutputBuffer )
{
	U32 numSamples = numFrames * 2;  // 2 channels
	S32 y;

 //	 printf("CChannel::RenderBufferRenderBuffer -- chan bufPtr: 0x%x, channel: 0x%x \n", (unsigned int)pOutBuffer_, (unsigned int)this );
//{static long c=0; printf("CChannel::RenderBuffer: START %ld addToOutputBuffer=%d\n", c++, addToOutputBuffer); }

	// decide how to deal with player done i.e. playerFramesRendered comes back 
	// less than numStereoFrames: does player send done, or channel.
	
// fixme/dg:  needs CAudioEffectsProcessor
#if 0
	// If channel has an effects processor, render the effect into pOutBuffer_
	if (pFxChain_ != kNull)
		pFxChain->ProcessAudioEffects( kAudioOutBufSizeInWords, pOutBuffer_ );
#endif
	int playerFramesRendered = pPlayer_->RenderBuffer( pTmp, numFrames );

//printf("levelsf <%f , %f > \n", levelsf[0], levelsf[1]);
//printf("levelsi <%f , %f > \n", Q15ToFloat(levelsi[0]), Q15ToFloat(levelsi[1]));

// ---- Add rendered output to out buffer
	if (addToOutputBuffer)
		{
		for (U32 i = 0; i < numSamples; i += 2)
			{
		// Integer scaling for gain control
//            y = pOut[i] + ((pTmp[i] * volume_)>>7);	        // ORIG rbg			
//			y = pOut[i] + (S32)(levelsf[0] * (float)pTmp[i]);	// FLOAT			
 			y = pOut[i] + (S32) MultQ15(levelsi[0], pTmp[i]);	// Q15  1.15 Fixed-point		
		// Saturate to 16-bit range				
			if      (y > kS16Max) y = kS16Max;
			else if (y < kS16Min) y = kS16Min;				
			pOut[i] = (S16)y;

 			y = pOut[i+1] + (S32) MultQ15(levelsi[1], pTmp[i+1]);				
		// Saturate to 16-bit range				
			if      (y > kS16Max) y = kS16Max;
			else if (y < kS16Min) y = kS16Min;				
			pOut[i+1] = (S16)y;
			}
		}
// ---- Render to out buffer
	else
		{
		for (U32 i = 0; i < numSamples; i += 2)
			{
		// Integer scaling for gain control
//  		y = ((pTmp[i] * volume_)>>7);		        // ORIG rdg	Darren	
//			y = (S32)(levelsf[0] * (float)pTmp[i]);		// FLOAT
 			y = (S32) MultQ15(levelsi[0], pTmp[i]);		// Q15  1.15 Fixed-point	
		// Saturate to 16-bit range				
			if      (y > kS16Max) y = kS16Max;
			else if (y < kS16Min) y = kS16Min;				
			pOut[i] = (S16)y;

 			y = (S32) MultQ15(levelsi[1], pTmp[i+1]);				
		// Saturate to 16-bit range				
			if      (y > kS16Max) y = kS16Max;
			else if (y < kS16Min) y = kS16Min;				
			pOut[i+1] = (S16)y;
			}
		}

	return (playerFramesRendered);
}	// ---- end CChannel::RenderPlayerToBuffer ----

LF_END_BRIO_NAMESPACE()
// EOF	
