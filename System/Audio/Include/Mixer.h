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
	CChannel*		FindBestChannel( tAudioPriority priority );
	CMidiPlayer*	GetMidiPlayer( void ) { return pMidiPlayer_; }
	void 			SetMasterVolume( U8 vol ) { masterVol_ = ((float)vol) / 100.0F;}
	
	// Main routine to handle the processing of data through the audio channels
	// for one audio tick 
	int RenderBuffer( S16* pOutBuff, unsigned long frameCount );
	
	static int WrapperToCallRenderBuffer( S16* pOutBuff, 
											unsigned long frameCount,
											void* pToObject  );
											
private:
	U8 				numChannels_;
	CChannel*		pChannels_;			// Array of channels
	CMidiPlayer*	pMidiPlayer_;		// player for doing MIDI
	float			masterVol_;
};

#endif		// LF_BRIO_MIXER_H

// EOF	
