//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
//==============================================================================
//
// Channel.cpp
//
// Description: manages processing of audio data on a mixer channel
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <AudioTypes.h>
#include <AudioPriv.h>
#include <Channel.h>

#include <EventMPI.h>

#include <RawPlayer.h>
#include <VorbisPlayer.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CChannel implementation
//==============================================================================
CChannel::CChannel()
{
	pPlayer_ = kNull;

	fInUse_		= false;
	fPaused_	= false;
	fReleasing_ = false;
	isDone_		= false;

	// Set DSP values
	pan_	= kPan_Default;
	volume_ = kVolume_Default;
	samplingFrequency_ = 0;

}	// ---- end CChannel ----

// ==============================================================================
// ~CChannel
// ==============================================================================
CChannel::~CChannel()
{
}	// ---- end ~CChannel ----

// ============================================================================
// SetPan :		Channel stereo position	  [Left .. Center .. Right]
// ============================================================================
void CChannel::SetPan( S8 x )
{
	pan_ = BoundS8(&x, kPan_Min, kPan_Max);

	// Convert input range to [-100 .. 100]range [0 .. 1] suitable for the
	// constant power calculation
	float xf = ChangeRangef((float)pan_, (float) kPan_Min, (float)kPan_Max, 0.0f, 1.0f);
	
	ConstantPowerValues(xf, &panValuesf[kLeft], &panValuesf[kRight]);
	RecalculateLevels();
}	// ---- end SetPan ----

// ============================================================================
// SetVolume : Convert range [0 .. 127] to [-100 .. +4.15] dB
// ============================================================================
void CChannel::SetVolume( U8 x )
{
	
	volume_ = BoundU8(&x, 0, 100);

	gainf  = ChangeRangef((float)x, (float) kVolume_Min, (float)kVolume_Max,
						  0.0f, 1.0f);
	// Convert to squared curve, which is milder than log10
	gainf *= gainf;
	gainf *= kChannel_Headroomf; // DecibelToLinearf(-Channel_HeadroomDB);
	RecalculateLevels();
}	// ---- end SetVolume ----

// ==============================================================================
// RecalculateLevels : Recalculate levels when either pan or volume changes
// ==============================================================================
void CChannel::RecalculateLevels()
{

	levelsf[kLeft ] =  panValuesf[kLeft ]*gainf;
	levelsf[kRight] =  panValuesf[kRight]*gainf;

	// Convert 32-bit floating-point to Q15 fractional integer format
	levelsi[kLeft ] = FloatToQ15(levelsf[kLeft ]);
	levelsi[kRight] = FloatToQ15(levelsf[kRight]);

}	// ---- end RecalculateLevels ----

// ==============================================================================
// ShouldRender
// ==============================================================================
Boolean CChannel::ShouldRender( void ) 
{
	Boolean shouldRender = (fInUse_ && !fPaused_ && !fReleasing_ && !isDone_); // && pPlayer_);
	
	return (shouldRender); 
}	// ---- end ShouldRender ----

// ==============================================================================
// Release:
// ==============================================================================
tErrType CChannel::Release( Boolean noPlayerDoneMsg )
{	
	if (!pPlayer_)
		return (kNoErr);

	fReleasing_ = true;
	fPaused_	= true;

	pPlayer_ = kNull;

	// No longer in use
	fReleasing_ = false;
	fInUse_		= false;

	return (kNoErr);
}	// ---- end Release ----

// ==============================================================================
// InitWithPlayer
// ==============================================================================
tErrType CChannel::InitWithPlayer( CAudioPlayer* pPlayer )
{
	
	// Convert interface parameters to DSP level data and reset channel
	pPlayer_	= pPlayer;
	SetSamplingFrequency(pPlayer->GetSampleRate());

	fPaused_	= false;
	fReleasing_ = false;
	isDone_		= false;
	// Finally, allow this channel to be added to Mixer
	fInUse_		= true;

	return (kNoErr);
}	// ---- end InitWithPlayer ----

// ==============================================================================
// Render
// ==============================================================================
U32 CChannel::Render(S16 *pOut, int framesToRender )
{
	// GK CHECK FIXX numFrames IS THIS FRAMES OR SAMPLES !!!!!!! THIS APPEARS TO
	// BE SAMPLES
	int samplesToRender = framesToRender * 2;  // 2 channels
	int samplesRendered = 0;
	int framesRendered	= 0;

	ClearShorts(pOut, samplesToRender);

	// Call player to render a stereo buffer
	if (pPlayer_)
	{
		framesRendered	= pPlayer_->Render( pOut, framesToRender );
		samplesRendered = framesRendered*2;
		isDone_ = (framesToRender > framesRendered);
	}

	// Scale stereo out buffer (Assumes all audio player output is two channel)
	for (int i = 0; i < samplesRendered; i += 2)
	{
		// Integer scaling : gain + stereo pan
		pOut[i	] = (S16) MultQ15(levelsi[kLeft ], pOut[i  ]);	// Q15	1.15 Fixed-point	
		pOut[i+1] = (S16) MultQ15(levelsi[kRight], pOut[i+1]);				
	}

	return (framesRendered);
}	// ---- end Render ----

LF_END_BRIO_NAMESPACE()
