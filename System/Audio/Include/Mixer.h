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
#include <DebugMPI.h>

#include "sndfile.h"

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
	
	void 			SetMasterVolume( U8 x ) { masterVolume_ = x; }
	
	// Main routine to handle the processing of data through the audio channels
	int RenderBuffer( S16* pOutBuff, unsigned long frameCount );
	
	static int WrapperToCallRenderBuffer( S16* pOutBuff, unsigned long frameCount, void* pToObject  );
											
private:
	CDebugMPI 		*pDebugMPI_;
	
	BRIOMIXER		pDSP_;
#define kAudioMixer_MixBinCount	3	// At present, for sampling rates 8000, 16000, 32000 Hz
#define kAudioMixer_MixBin_Index_FsDiv4 0
#define kAudioMixer_MixBin_Index_FsDiv2 1
#define kAudioMixer_MixBin_Index_FsDiv1 2
#define kAudioMixer_MixBin_Index_Fs     kAudioMixer_MixBin_Index_FsDiv1
	S16			*mixBinBufferPtrs_[kAudioMixer_MixBinCount][2];
	long			 mixBinFilled_[kAudioMixer_MixBinCount];
	long GetMixBinIndex( long samplingFrequency );
	long GetSamplingRateDivisor( long samplingFrequency );

// Sampling rate conversion parameters
	SRC			src_[kAudioMixer_MixBinCount][2];

	U8			masterVolume_;			// fixme/rdg: convert to fixedpoint
	U8 			numChannels_;			// mono or stereo (for now fixed at stereo)
	CChannel*		pChannels_;			// Array of channels
	CMidiPlayer*		pMidiPlayer_;			// player for doing MIDI
	U8				masterVol_;			// fixme/rdg: convert to fixedpoint
	S16*			pMixBuffer_; // Ptr to mixed samples from all active channels

#define kAudioMixer_MaxTempBuffers	8
	S16*			pTmpBuffers_     [kAudioMixer_MaxTempBuffers]; // Ptr to intermediate results
	S16*			tmpBufOffsetPtrs_[kAudioMixer_MaxTempBuffers]; 

// Debug : info for sound file input/output
long readInSoundFile;
long writeOutSoundFile ;

SNDFILE	*inSoundFile;
SF_INFO	inSoundFileInfo ;
char inSoundFilePath[500];	

SNDFILE	*outSoundFile;
SF_INFO	outSoundFileInfo ;

SNDFILE	*OpenSoundFile( char *path, SF_INFO *sfi, long rwType);
int CloseSoundFile( SNDFILE **soundFile );

};

#endif		// LF_BRIO_MIXER_H

LF_END_BRIO_NAMESPACE()
// EOF	
