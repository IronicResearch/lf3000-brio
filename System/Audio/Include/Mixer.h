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
	
	// Get the MIDI player for the mixer.
	CMidiPlayer*	GetMidiPlayer( void ) { return pMidiPlayer_; }
	
	// Set the mixer's master volume.
	void 			SetMasterVolume( U8 vol ) { masterVol_ = vol; }
	
	// Main routine to handle the processing of data through the audio channels
	int RenderBuffer( S16* pOutBuff, unsigned long frameCount );
	
	static int WrapperToCallRenderBuffer( S16* pOutBuff, 
											unsigned long frameCount,
											void* pToObject  );
											
private:
	U8 				numChannels_;			// mono or stereo (for now fixed at stereo)
	CChannel*		pChannels_;				// Array of channels
	CMidiPlayer*	pMidiPlayer_;			// player for doing MIDI
	U8				masterVol_;				// fixme/rdg: convert to fixedpoint
	S16*			pMixBuffer_;			// Pointer to mixed samples from all active chans.
	S16*			pSRCInBuffer_;			// Pointer to the sample rate converter input buffer
	S16*			pSRCOutBuffer_;			// Pointer to the sample rate converter output buffer
	S16*			pSRCOverflowBuffer_;	// Pointer to the sample rate converter output buffer

};

#endif		// LF_BRIO_MIXER_H

LF_END_BRIO_NAMESPACE()
// EOF	
