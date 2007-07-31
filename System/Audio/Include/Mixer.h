#ifndef LF_BRIO_MIXER_H
#define LF_BRIO_MIXER_H


//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		AudioMixer.h
//
// Description:
//		Defines the class to manage the low-level mixing of independent audio channels.
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <AudioTypes.h>
#include <Channel.h>
#include <MidiPlayer.h>

#include <briomixer.h>
#include <src.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Class:
//		CAudioMixer
//
// Description:
//		Class to manage the low-level mixing of independent audio channels. 
//==============================================================================
class CAudioMixer {
public:
	CAudioMixer( U8 numChannels );
	~CAudioMixer();
		
	// Find the best channel to use for audio at a given priority
	CChannel*		FindChannelUsing( tAudioPriority priority );
	
	// Find a channel based on the ID of the audio its playing.
	CChannel*		FindChannelUsing( tAudioID id );
	
	// Returns true if any audio playing on a mixer channel.
	// Note: this doesn't include MIDI.
	Boolean IsAnyAudioActive( void );

	// Get the MIDI player for the mixer.
	CMidiPlayer*	GetMidiPlayer( void ) { return pMidiPlayer_; }
	
	void 			SetMasterVolume( U8 vol ) { masterVol_ = vol; }
	
	// Main routine to handle the processing of data through the audio channels
	int RenderBuffer( S16* pOutBuff, unsigned long frameCount );
	
	static int WrapperToCallRenderBuffer( S16* pOutBuff, unsigned long frameCount, void* pToObject  );
											
private:
	BRIOMIXER		pDSP_;
#define kAudioMixer_SRC_MixBinCount	3	// For sampling rates 8, 16, 32 kHz
#define kAudioMixer_SRC_MixBin_0_8000Hz	 0
#define kAudioMixer_SRC_MixBin_1_16000Hz 1
#define kAudioMixer_SRC_MixBin_2_32000Hz 2
	SRC			src_[kAudioMixer_SRC_MixBinCount][2];
	S16			*srcMixBinBufferPtrs_[kAudioMixer_SRC_MixBinCount][2];
	long			 srcMixBinFilled_[kAudioMixer_SRC_MixBinCount];

	U8 			numChannels_;			// mono or stereo (for now fixed at stereo)
	CChannel*		pChannels_;			// Array of channels

	CMidiPlayer*		pMidiPlayer_;			// player for doing MIDI
	U8			masterVol_;			// fixme/rdg: convert to fixedpoint
	S16*			pMixBuffer_; // Ptr to mixed samples from all active channels

#define kAudioMixer_MaxTempBuffers	8
	S16*			pTmpBuffers_[kAudioMixer_MaxTempBuffers]; // Ptr to intermediate result buffer
	S16*			tmpBufOffsetPtrs_[kAudioMixer_MaxTempBuffers]; 

	S16*			pSRCInBuffer_;			// Ptr to sample rate converter input buffer
	S16*			pSRCOutBuffer_;			// Ptr to sample rate converter output buffer

};

#endif		// LF_BRIO_MIXER_H

LF_END_BRIO_NAMESPACE()
// EOF	
