//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
//==============================================================================
//
// Stream.cpp
//
// Description: manages processing of audio data on a mixer stream 
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <AudioTypes.h>
#include <AudioPriv.h>
#include <Stream.h>

#include <EventMPI.h>

#include <RawPlayer.h>
#include <VorbisPlayer.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CStream implementation
//==============================================================================
CStream::CStream()
{
	pPlayer_ = kNull;

	// Set DSP values
	pan_	= kPan_Default;
	volume_ = kVolume_Default;
	samplingFrequency_ = 0;
	framesToRender_ = 0;
	isDone_ = false;
	// FIXME: valgrind suspect(s)
	gainf	= 1.0;
	panValuesf[kLeft ] = 0.707;
	panValuesf[kRight] = 0.707;

#ifdef USE_RENDER_THREAD
	// Rendering bufs
	for (int i = 0; i < kNumRingBufs; i++)
		pRingBuf_[i] = new S16[kAudioOutBufSizeInWords];
	nRenderIdx_ = nStreamIdx_ = nDoneIdx_ = 0;
#endif
}	// ---- end CStream ----

// ==============================================================================
// ~CStream
// ==============================================================================
CStream::~CStream()
{
#ifdef USE_RENDER_THREAD
	for (int i = kNumRingBufs-1; i >= 0; i--)
		delete[] pRingBuf_[i];
#endif
}	// ---- end ~CStream ----

// ============================================================================
// SetPan :		CStream stereo position	  [Left .. Center .. Right]
// ============================================================================
void CStream::SetPan( S8 x )
{
	pan_ = BoundS8(&x, kPan_Min, kPan_Max);

	// Convert input range to [-100 .. 100]range [0 .. 1] suitable for the
	// constant power calculation
	float xf = ChangeRangef((float)pan_, (float) kPan_Min, (float)kPan_Max, 0.0f, 1.0f);

	// FIXME: float sin/cos routine...
	ConstantPowerValues(xf, &panValuesf[kLeft], &panValuesf[kRight]);
	RecalculateLevels();
}	// ---- end SetPan ----

// ============================================================================
// SetVolume : Convert range [0 .. 127] to [-100 .. +4.15] dB
// ============================================================================
void CStream::SetVolume( U8 x )
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
void CStream::RecalculateLevels()
{
	// FIXME: SetPan() vs SetVolume() dependency
	levelsf[kLeft ] =  panValuesf[kLeft ]*gainf;
	levelsf[kRight] =  panValuesf[kRight]*gainf;

	// Convert 32-bit floating-point to Q15 fractional integer format
	levelsi[kLeft ] = FloatToQ15(levelsf[kLeft ]);
	levelsi[kRight] = FloatToQ15(levelsf[kRight]);

}	// ---- end RecalculateLevels ----

// ==============================================================================
// Release:
// ==============================================================================
tErrType CStream::Release( Boolean noPlayerDoneMsg )
{	
	if (!pPlayer_)
		return (kNoErr);

	pPlayer_ = kNull;
	isDone_ = false;

	return (kNoErr);
}	// ---- end Release ----

// ==============================================================================
// InitWithPlayer
// ==============================================================================
tErrType CStream::InitWithPlayer( CAudioPlayer* pPlayer )
{

	// Convert interface parameters to DSP level data and reset channel
	pPlayer_	= pPlayer;

	// Check sample rate of player to fit Brio mixer params
	U32 rate = pPlayer->GetSampleRate();
	if (rate > kAudioSampleRate) {
		rate = kAudioSampleRate;
	}
	else if (rate > kAudioSampleRate_Div2 && rate < kAudioSampleRate) {
		rate = kAudioSampleRate_Div2;
	}
	else if (rate < kAudioSampleRate_Div2) {
		rate = kAudioSampleRate_Div4;
	}
	SetSamplingFrequency(rate);

	// Calculate effective frames to render for player sample rate
	framesToRender_ = kAudioFramesPerBuffer * rate / kAudioSampleRate;
	isDone_ = false;
	
#ifdef USE_RENDER_THREAD
	// Reset ring buffer indexes for new player
	nRenderIdx_ = nStreamIdx_ = nDoneIdx_ = 0;
	memset(pRingBuf_[0], 0, kAudioOutBufSizeInBytes);
#endif
	
	return (kNoErr);
}	// ---- end InitWithPlayer ----

#ifdef USE_RENDER_THREAD
// ==============================================================================
// PreRender -- Renders samples into ring buffer
// ==============================================================================
U32 CStream::PreRender(S16* /* pOut */, U32 framesToRender)
{
	// Render buffer index can only exceed Stream index up to ring depth
	if (nRenderIdx_ > nStreamIdx_ + kNumRingBufs-1)
		return 0;
	S16* pOut = GetRenderBuf();
	U32* pFrames = &nFrames_[nRenderIdx_ % kNumRingBufs]; 
	*pFrames = Render(pOut, framesToRender);
	if (*pFrames == 0)
		return 0;
	if (isDone_)
		nDoneIdx_ = nRenderIdx_;
	nRenderIdx_++;
	return *pFrames;
}

// ==============================================================================
// PostRender -- Copies rendered samples from ring buffer
// ==============================================================================
U32 CStream::PostRender(S16* pOut, U32 framesToRender)
{
	// Stream buffer index always lags Render index unless done
	S16* pBuf = GetStreamBuf();
	U32* pFrames = &nFrames_[nStreamIdx_ % kNumRingBufs]; 
	memcpy(pOut, pBuf, *pFrames * kAudioBytesPerStereoFrame);
	if (nStreamIdx_ < nRenderIdx_)
		nStreamIdx_++;
	return *pFrames;
}
#endif

// ==============================================================================
// Render
// ==============================================================================
U32 CStream::Render(S16 *pOut, int framesToRender )
{
	int samplesToRender = framesToRender * 2;  // 2 channels
	int samplesRendered = 0;
	int framesRendered	= 0;

	ClearShorts(pOut, samplesToRender);

	// Call player to render a stereo buffer
	if (pPlayer_)
	{
		framesRendered	= pPlayer_->Render( pOut, framesToRender );
		samplesRendered = framesRendered*2;
		isDone_ = pPlayer_->IsDone();
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
