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
#include <CoreTypes.h>
#include <SystemTypes.h>
//#include <RsrcTypes.h>
#include <AudioTypes.h>
#include <AudioPlayer.h>
#include <RawPlayer.h> // fixme/dg: hack for RawPlayer dtor not getting called

#define kFracBits 16

struct tAudioConversion {
	U32 oPosFrac;		// Fractional position of the output buffer in the input buffer
	U32 oPos;			// Current position in the output buffer

	U32 oPosIncFrac;	// Fractional position increment in the output buffer
	U32 oPosInc; 

	U32 iPos;			// Current position in the input buffer
	S16 iLastLeft;		// Last left-side sample in the input buffer
	S16 iLastRight;		// Last right-side sample in the input buffer
};

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
	inline void	Pause() { if (bInUse_) bPaused_ = true; }
	inline void	Resume() {  if (bInUse_) bPaused_ = false; }

	// Ask channel to get data from player and return it to caller's mix buffer.
	// This will add per-channel fx as well as sample rate convert player
	// data to system rate.  If less than numFrames is available
	// the remainder of the channel's buffer will be zero padded.
	// The channel also assumes that the mix buffer is stereo 
	// so mono data is copied to both stereo channel on mix out.
	U32			RenderBuffer( S16 *pMixBuff, U32 numStereoFrames  );

	inline U8		GetVolume() { return (U8)(volume_ * 100.0F); }
	inline void		SetVolume( U8 volume ) { volume_ = ((float)volume) / 100.0F; }
	inline S8		GetPan() { return pan_; }
	inline void		SetPan(S8 pan) { pan_ = pan; }

	// Return the requested status of the channel 
	inline U8				IsInUse() { return bInUse_; }
	inline U8				IsPaused() { return bPaused_; }
	inline U8				HasOwnAudioEffectsProcessor() { return bOwnProcessor_; }
	inline CAudioPlayer*	GetPlayer() { return pPlayer_; }

private:
	float						volume_;	// Volume of the channel
	S8							pan_;		// Pan of the channel 

	S16 						*pOutBuffer_;	// Pointer to the output buffer
	U32							inSampleRate_;	// Sample rate of data as reported by player

//	CAudioEffectsProcessor		*pChain_;		// Pointer to a special effects processor
	CAudioPlayer				*pPlayer_;		// Pointer to the player assigned to this channel
	
	U8		bInUse_:1;			// Channel is in use
	U8		bPaused_:1;			// Channel is paused
	U8		bOwnProcessor_:1;	// Channel has own audio effects processor
//	U8		unused_:3;			// Unused

	tAudioConversion			convRate_;	// Conversion rate parameters
};

#endif		// LF_BRIO_CHANNEL_H

// EOF	
