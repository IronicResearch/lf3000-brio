#ifndef LF_BRIO_CHANNEL_H
#define LF_BRIO_CHANNEL_H

//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		Channel.h
//
// Description:
//		Defines the class to manage the processing of audio data on an audio channel.
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <AudioTypes.h>
#include <AudioPlayer.h>
#include <RawPlayer.h> // fixme/dg: hack for RawPlayer dtor not getting called
#include <DebugMPI.h>

#include <Dsputil.h>
#include <mix.h>

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

	// Acquire this channel to play this audio rsrc
	tErrType	InitChanWithPlayer( CAudioPlayer* pPlayer );
	tErrType	Release( Boolean suppressPlayerDoneMsg );

	// Set the pause state of the channel
	inline void	Pause()  { if (fInUse_) fPaused_ = true; }
	inline void	Resume() { if (fInUse_) fPaused_ = false; }

	// Ask channel to get data from player and return it to caller's mix buffer.
	// This will add per-channel fx as well as sample rate convert player
	// data to system rate.  If less than numFrames is available
	// the remainder of the channel's buffer will be zero padded.
	// The channel also assumes that the mix buffer is stereo 
	// so mono data is copied to both stereo channel on mix out.
	U32			RenderBuffer( S16 *pOutP, S16 *pTmp, int numStereoFrames, Boolean addToOutputBuffer );

	// Mixer needs to know if the channel is in a state appropriate
	// for calling RenderBuffer().  Keeps flag state inside of channel.
	Boolean		ShouldRender( void );
	
	inline U8	GetVolume()		{ return volume_; }
	void		SetVolume( U8 x );

	inline S8	GetPan()		{ return pan_; }
	void		SetPan(S8 x);

	inline tAudioPriority	GetPriority()            				{ return priority_; }
	inline void				SetPriority( tAudioPriority priority ) 	{ priority_ = priority; }

	inline float	GetEQ_Frequency()   { return eq_frequency_; }
	inline float	GetEQ_Q()           { return eq_q_; }
	inline float	GetEQ_GainDB()      { return eq_gainDB_; }

	inline void		SetEQ_Parameters(float frequency, float q, float gainDB) { eq_frequency_ = frequency; eq_q_ = q; eq_gainDB_ = gainDB;}

	inline void		SetMixerChannelDataPtr(MIXERCHANNEL *d) { pDSP_ = d; }

//	inline long		GetMixBinIndex()       { return mixBinIndex_; }
//	inline void		SetMixBinIndex(long x) { mixBinIndex_ = x; }

	inline long		GetSamplingFrequency()        { return samplingFrequency_; }
//	inline void		SetSamplingFrequency(float x) { samplingFrequency_ = x; }

	// Return the requested status of the channel 
	inline Boolean			IsInUse()  { return fInUse_; }
	inline Boolean			IsPaused() { return fPaused_; }
	inline Boolean			HasOwnAudioEffectsProcessor() { return fOwnProcessor_; }
	inline CAudioPlayer*	GetPlayer() { return pPlayer_; }

//    S16 *outP;   // TEMPORARY.  Used only during rearchitecture
private:
	U8			volume_;	
	S8			pan_;		 

#define kAudioMixerChannel_MaxOutChannels 2
	float		panValuesf[kAudioMixerChannel_MaxOutChannels];
	Q15			panValuesi[kAudioMixerChannel_MaxOutChannels];
	float		gainf;
	Q15			gaini;
    float       levelsf[kAudioMixerChannel_MaxOutChannels]; // gain * panValue
    Q15         levelsi[kAudioMixerChannel_MaxOutChannels];

	tAudioPriority	priority_;	// channel priority

	float		eq_frequency_;
	float		eq_q_;
	float		eq_gainDB_;

	U32			samplingFrequency_;	

	MIXERCHANNEL	*pDSP_;

//	CAudioEffectsProcessor		*pChain_;		// Pointer to a special effects processor
	CAudioPlayer				*pPlayer_;		// Pointer to the player assigned to this channel
	
	Boolean fInUse_;		
	Boolean fPaused_;		
	Boolean fReleasing_;	// Channel is in the process of being reset
	Boolean fOwnProcessor_;	// Channel has own audio effects processor
	
	CDebugMPI	*pDebugMPI_;

    void   RecalculateLevels();
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_CHANNEL_H

// EOF	
