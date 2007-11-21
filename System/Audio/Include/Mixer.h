#ifndef LF_MIXER_H
#define LF_MIXER_H


//==============================================================================
// Copyright (c) 2002-2007 LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		Mixer.h
//
// Description:
//		Defines the class to manage the low-level mixing of audio channels.
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
#include <eq.h>
#include <src.h>
#include <shape.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Class:
//		CAudioMixer
//
// Description:
//		Class to manage the low-level mixing of audio channels. 
//==============================================================================
class CAudioMixer {
public:
	CAudioMixer( int numChannels );
	~CAudioMixer();
		
	CChannel*		FindChannelUsing( tAudioPriority priority );
	CChannel*		FindChannelUsing( tAudioID id );
	long    		FindChannelIndex( tAudioID id );
	
	// Is audio (excluding MIDI) playing on a mixer channel.
	Boolean IsAnyAudioActive( void );

	CMidiPlayer*	GetMidiPlayerPtr( void ) { return pMidiPlayer_; }

	tErrType	    AddPlayer( tAudioStartAudioInfo *pAudioInfo, char *sExt, tAudioID newID );
	CAudioPlayer	*CreatePlayer( tAudioStartAudioInfo *pAudioInfo, char *sExt, tAudioID newID );

	void 			SetMasterVolume( U8 x ) ; 

	Boolean     GetOutputEqualizer( ) { return ((Boolean)useOutEQ_); }
	void        SetOutputEqualizer( Boolean x );
	
// Main routine 
	int RenderBuffer( S16* pOutBuff, unsigned long frameCount );
	
	static int WrapperToCallRenderBuffer( S16 *pOutBuff, unsigned long frameCount, void *pToObject  );
											
private:
	CDebugMPI 		*pDebugMPI_;

    CAudioPlayer *playerToAdd_;
    CChannel     *targetChannel_;
	
	BRIOMIXER		pDSP_;
    float           samplingFrequency_;

    void UpdateDSP();
    void ResetDSP();
    void PrepareDSP();
    void SetSamplingFrequency( float x );

// Didj is hard-limited to 4 channels : 3 audio + 1 MIDI input channels (stereo)
#define kAudioMixer_MaxInAudioChannels	3    
#define kAudioMixer_MaxInMIDIChannels	1    
#define kAudioMixer_MaxInChannels	    (kAudioMixer_MaxInAudioChannels+kAudioMixer_MaxInMIDIChannels)    
#define kAudioMixer_MaxOutChannels	    2

// Channel parameters
	U8 			numInChannels_;			// input: mono or stereo (for now, allow stereo)
	CChannel*		pChannels_;			// Array of channels
//#define kChannel_MaxTempBuffers		2
	S16 			*channel_tmpPtrs_[kAudioMixer_MaxInChannels];
	S16 			*pChannel_OutBuffer_;	

// Mix Bin Parameters
#define kAudioMixer_MixBinCount	        3	// At present, for sampling rates :  fs, fs/2, fs/4 
#define kAudioMixer_MixBin_Index_FsDiv4 0
#define kAudioMixer_MixBin_Index_FsDiv2 1
#define kAudioMixer_MixBin_Index_FsDiv1 2
#define kAudioMixer_MixBin_Index_Fs     kAudioMixer_MixBin_Index_FsDiv1
	S16			*mixBinBufferPtrs_[kAudioMixer_MixBinCount][kAudioMixer_MaxOutChannels];
	long			 mixBinFilled_[kAudioMixer_MixBinCount];
    long fsRack_[kAudioMixer_MixBinCount];

	long GetMixBinIndex        ( long samplingFrequency );
	long GetSamplingRateDivisor( long samplingFrequency );

// Sampling rate conversion (SRC) parameters
	SRC			src_[kAudioMixer_MixBinCount][kAudioMixer_MaxOutChannels];

	U8			masterVolume_;			
    float       masterGainf_[kAudioMixer_MaxOutChannels];
    Q15         masterGaini_[kAudioMixer_MaxOutChannels];

//  Output EQ parameters
#define kAudioMixer_MaxEQBands  3
    long        useOutEQ_;
    long        outEQ_BandCount_;
    EQ          outEQ_[kAudioMixer_MaxOutChannels][kAudioMixer_MaxEQBands];

//  Soft Clipper parameters
    long        useOutSoftClipper_;
    WAVESHAPER  outSoftClipper_[kAudioMixer_MaxOutChannels];

// MIDI parameters
	CMidiPlayer*	pMidiPlayer_;

// Mixer buffer parameters
//	S16*			pMixBuffer_;  // Mix of all active channels

#define kAudioMixer_MaxTempBuffers	8
	S16*	pTmpBufs_        [kAudioMixer_MaxTempBuffers]; 
	S16*	tmpBufOffsetPtrs_[kAudioMixer_MaxTempBuffers]; 

// Some Debug variables
    float   preGainDB;
    float   postGainDB;
    float   preGainf;
    float   postGainf;
    Q15     preGaini;
    Q15     postGaini;
    void UpdateDebugGain();
};

#endif		// LF_MIXER_H

LF_END_BRIO_NAMESPACE()

