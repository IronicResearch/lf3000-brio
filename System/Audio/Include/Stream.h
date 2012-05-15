#ifndef LF_BRIO_STREAM_H
#define LF_BRIO_STREAM_H

//==============================================================================
// Copyright (c) 2002-2008 LeapFrog Enterprises, Inc.
//==============================================================================
//
// Stream.h
//
// Defines class to manage processing of audio data on an audio stream.
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <AudioTypes.h>
#include <AudioPlayer.h>

#include <DebugMPI.h>

#include <Dsputil.h>

LF_BEGIN_BRIO_NAMESPACE()

// Support for separate rendering thread ring buffer
#undef USE_RENDER_THREAD	
#define kNumRingBufs		8

class CAudioPlayer;
//==============================================================================
// Class:
//		CStream
//
// Description:
//		Class to manage the processing of audio data on an audio stream.
//==============================================================================
class CStream {
 public:
	CStream();
	~CStream();

	tErrType	InitWithPlayer( CAudioPlayer* pPlayer );
	tErrType	Release( Boolean noPlayerDoneMsg );

	// Ask stream to get data from player and return it to caller's mix buffer.
	// This will add per-stream fx as well as sample rate convert player
	// data to system rate.	 If less than numFrames is available
	// the remainder of the stream's buffer will be zero padded.
	// The stream also assumes that the mix buffer is stereo 
	// so mono data is copied to both audio channels on mix out.
	U32			Render( S16 *pOut, int numStereoFrames );

	inline U8	GetVolume()		{ return volume_; }
	void		SetVolume(U8 x);

	inline S8	GetPan()			{ return pan_; }
	void		SetPan(S8 x);

	inline U32	GetSamplingFrequency()			{ return samplingFrequency_; }
	void		SetSamplingFrequency(U32 x)		{ samplingFrequency_ = x;}

	void		SetPlayer(CAudioPlayer *pPlayer) { pPlayer_ = pPlayer; }

	float		panValuesf[kAudioNumOutputChannels];
	Q15			panValuesi[kAudioNumOutputChannels];

	float		gainf;
	Q15			gaini;
	float		levelsf	  [kAudioNumOutputChannels]; // gain * panValue
	Q15			levelsi	  [kAudioNumOutputChannels];
	
	// Return requested status
	inline CAudioPlayer*	GetPlayer() { return pPlayer_; }

	Boolean isDone_;
	
#ifdef USE_RENDER_THREAD
	inline Boolean IsDone() { return pPlayer_ && pPlayer_->IsDone() && nStreamIdx_ == nDoneIdx_; }
#else
	inline Boolean IsDone() { return pPlayer_ && pPlayer_->IsDone(); }
#endif
	
	inline U32	GetFramesToRender()	{ return framesToRender_; }
	
#ifdef USE_RENDER_THREAD
	inline S16*	GetRenderBuf()	{ return pRingBuf_[nRenderIdx_ % kNumRingBufs]; }
	inline S16* GetStreamBuf()	{ return pRingBuf_[nStreamIdx_ % kNumRingBufs]; }
	
	U32			PreRender(S16* pOut, U32 numStereoFrames);
	U32			PostRender(S16* pOut, U32 numStereoFrames);
#endif
	
 private:
	tAudioPriority	priority_;	
	U8				volume_;	
	S8				pan_   ;		 
	U32				samplingFrequency_;
	U32				framesToRender_;			// effective frames per sample rate
	CAudioPlayer	*pPlayer_;		 
	void			RecalculateLevels();

#ifdef USE_RENDER_THREAD
	S16*			pRingBuf_[kNumRingBufs];	// ring buffer for pre-rendering
	int				nRenderIdx_;				// buffer index for render input
	int				nStreamIdx_;				// buffer index for stream output
	U32				nFrames_[kNumRingBufs]; 	// matching array of frames rendered
	int				nDoneIdx_;					// buffer index when done rendering
#endif

	Boolean			bExternalStream_;			// uses external ALSA dmix stream
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_STREAM_H

