#ifndef LF_BRIO_CHANNEL_H
#define LF_BRIO_CHANNEL_H

//==============================================================================
// Copyright (c) 2002-2008 LeapFrog Enterprises, Inc.
//==============================================================================
//
// Channel.h
//
// Defines class to manage processing of audio data on an audio channel.
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <AudioTypes.h>
#include <AudioPlayer.h>

#include <DebugMPI.h>

#include <Dsputil.h>

LF_BEGIN_BRIO_NAMESPACE()

class CAudioPlayer;
//==============================================================================
// Class:
//		CChannel
//
// Description:
//		Class to manage the processing of audio data on an audio channel. 
//==============================================================================
class CChannel {
 public:
	CChannel();
	~CChannel();

	tErrType	InitWithPlayer( CAudioPlayer* pPlayer );
	tErrType	Release( Boolean noPlayerDoneMsg );

	// Set pause state
	inline void	Pause()	 { if (fInUse_) fPaused_ = true;  }
	inline void	Resume() { if (fInUse_) fPaused_ = false; }
	inline void	SetInUse(Boolean x) { fInUse_ = x; }

	// Ask channel to get data from player and return it to caller's mix buffer.
	// This will add per-channel fx as well as sample rate convert player
	// data to system rate.	 If less than numFrames is available
	// the remainder of the channel's buffer will be zero padded.
	// The channel also assumes that the mix buffer is stereo 
	// so mono data is copied to both stereo channel on mix out.
	U32			Render( S16 *pOut, int numStereoFrames );

	inline U8	GetVolume()		{ return volume_; }
	void		SetVolume(U8 x);

	inline S8	GetPan()			{ return pan_; }
	void		SetPan(S8 x);

	inline U32	GetSamplingFrequency()			{ return samplingFrequency_; }
	void		SetSamplingFrequency(U32 x)		{ samplingFrequency_ = x;}

	inline CAudioPlayer *GetPlayerPtr()			{ return pPlayer_; }
	void		SetPlayer(CAudioPlayer *pPlayer, long releaseExistingPlayer);

#define kAudioMixerChannel_MaxOutChannels 2
	float		panValuesf[kAudioMixerChannel_MaxOutChannels];
	Q15			panValuesi[kAudioMixerChannel_MaxOutChannels];

	float		gainf;
	Q15			gaini;
	float		levelsf	  [kAudioMixerChannel_MaxOutChannels]; // gain * panValue
	Q15			levelsi	  [kAudioMixerChannel_MaxOutChannels];

	// Mixer needs to know if the channel is in a state appropriate
	// for calling Render().  Keeps flag state inside of channel.
	Boolean		ShouldRender( void );
	
	inline tAudioPriority	GetPriority()							{ return priority_; }
	inline void				SetPriority( tAudioPriority priority )	{ priority_ = priority; }

	// Return requested status
	inline Boolean			IsInUse()  { return fInUse_; }
	inline Boolean			IsPaused() { return fPaused_; }
	inline CAudioPlayer*	GetPlayer() { return pPlayer_; }

	Boolean isDone_;
	Boolean fInUse_;		
	void SendDoneMsg( void );

 private:
	tAudioPriority	priority_;	
	U8			volume_;	
	S8			pan_   ;		 
	U32			samplingFrequency_;	
	CAudioPlayer				*pPlayer_;		 
	Boolean fPaused_;		
	Boolean fReleasing_;	// Channel is in process of being reset
	void   RecalculateLevels();
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_CHANNEL_H

