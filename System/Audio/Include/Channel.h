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
//#include <RsrcTypes.h>
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
	inline void	Pause()  { if (bInUse_) bPaused_ = true; }
	inline void	Resume() { if (bInUse_) bPaused_ = false; }

	// Ask channel to get data from player and return it to caller's mix buffer.
	// This will add per-channel fx as well as sample rate convert player
	// data to system rate.  If less than numFrames is available
	// the remainder of the channel's buffer will be zero padded.
	// The channel also assumes that the mix buffer is stereo 
	// so mono data is copied to both stereo channel on mix out.
	U32			RenderBuffer( S16 *pMixBuff, U32 numStereoFrames , long addToOutputBuffer );

	inline U8		GetVolume()            { return volume_; }
	inline void		SetVolume( U8 volume ) { volume_ = volume; }

	inline S8		GetPan()       { return pan_; }
	inline void		SetPan(S8 pan) { pan_ = pan; }

	inline float		GetEQ_Frequency()        { return eq_frequency_; }
	inline void		SetEQ_Frequency(float x) { eq_frequency_ = x; }

	inline float		GetEQ_Q()        { return eq_q_; }
	inline void		SetEQ_Q(float x) { eq_q_ = x; }

	inline float		GetEQ_GainDB()        { return eq_gainDB_; }
	inline void		SetEQ_GainDB(float x) { eq_gainDB_ = x; }

	inline void		SetMixerChannelDataPtr(MIXERCHANNEL *d) { pDSP_ = d; }

//	inline long		GetMixBinIndex()       { return mixBinIndex_; }
//	inline void		SetMixBinIndex(long x) { mixBinIndex_ = x; }

	inline long		GetSamplingFrequency()        { return inSampleRate_; }
//	inline void		SetSamplingFrequency(float x) { inSampleRate_ = x; }

	// Return the requested status of the channel 
	inline U8		IsInUse()  { return bInUse_; }
	inline U8		IsPaused() { return bPaused_; }
	inline U8		HasOwnAudioEffectsProcessor() { return bOwnProcessor_; }
	inline CAudioPlayer*	GetPlayer() { return pPlayer_; }

private:
	U8			volume_;	// Volume of the channel
	S8			pan_;		// Pan of the channel 

	float			eq_frequency_;
	float			eq_q_;
	float			eq_gainDB_;

	U32			inSampleRate_;	// Sample rate of data as reported by player

	MIXERCHANNEL		*pDSP_;

	S16 			*pOutBuffer_;	// Pointer to the output buffer
#define kChannel_MaxTempBuffers		2
	S16 			*tmpPs_[kChannel_MaxTempBuffers];	

//	CAudioEffectsProcessor		*pChain_;		// Pointer to a special effects processor
	CAudioPlayer				*pPlayer_;		// Pointer to the player assigned to this channel
	
	U8			bInUse_:1;			// Channel is in use
	U8			bPaused_:1;			// Channel is paused
	U8			bOwnProcessor_:1;	// Channel has own audio effects processor
//	U8			unused_:3;			// Unused
	
	CDebugMPI	*pDebugMPI_;
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_CHANNEL_H

// EOF	
