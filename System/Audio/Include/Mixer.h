#ifndef LF_MIXER_H
#define LF_MIXER_H

//==============================================================================
// Copyright (c) 2002-2007 LeapFrog Enterprises, Inc.
//==============================================================================
//
// Mixer.h
//
//		Defines class to manage low-level mixing of audio channels.
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
#include <eq.h>
#include <src.h>
#include <shape.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Class: CAudioMixer
//
// Description: manage the low-level mixing of audio channels. 
//==============================================================================
class CAudioMixer {
public:
	CAudioMixer( int numChannels );
	~CAudioMixer();
		
	CChannel*		FindFreeChannel(      tAudioPriority priority );
	CChannel*		FindChannel(          tAudioID id );
	long    		FindFreeChannelIndex( tAudioID id );
	
	Boolean IsAnyAudioActive( void );  // Is audio (excluding MIDI?) playing on any channel?

	CMidiPlayer *GetMidiPlayerPtr( void ) { return pMidiPlayer_; }

	tErrType	  AddPlayer(    tAudioStartAudioInfo *pInfo, char *sExt, tAudioID newID );
	CAudioPlayer *CreatePlayer( tAudioStartAudioInfo *pInfo, char *sExt, tAudioID newID );

	void 		SetMasterVolume( U8 x ) ; 

	void 		Pause ( ); 
	void 		Resume( ); 
	Boolean		IsPaused( ) { return isPaused_; }

	Boolean     GetEnabledOutputSpeakerDSP( ) { return ((Boolean)useOutSpeakerDSP_); }
	void        EnableOutputSpeakerDSP( Boolean x );
	
	int Render( S16 *pOut, U32 frameCount );
	
	static int WrapperToCallRender( S16 *pOut, U32 frameCount, void *pObject );
										
    tAudioState audioState_;
//    DSP_BLOCK   *dspBlock;

// Level Meter data
    S16 outLevels_ShortTime[2];
    S16 outLevels_LongTime [2];
    S16 temp_ShortTime     [2];
    S32 longTimeHoldCounter;
    S32 longTimeHoldInterval;
    S32 shortTimeCounter;
    S32 shortTimeInterval;
    float longTimeDecayF;
    S16   longTimeDecayI;

	void GetAudioState(tAudioState *d);
	void SetAudioState(tAudioState *d);
	
// ---- DEBUG File I/O
// Debug : info for sound file input/output
    int OpenInSoundFile (char *path);
    int OpenOutSoundFile(char *path);

private:
	CDebugMPI 		*pDebugMPI_;

//#define NEW_ADD_PLAYER
#define OLD_ADD_PLAYER
#ifdef NEW_ADD_PLAYER
    CAudioPlayer *playerToAdd_;
    CChannel     *targetChannel_;
#endif

//	BRIOMIXER		pDSP_;
    float           samplingFrequency_;
    long            useOutSpeakerDSP_;

    void SetDSP();
    void UpdateDSP();
    void ResetDSP();
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
//	S16 			*channel_tmpPtrs_[kAudioMixer_MaxInChannels];
	S16 			*pChannelBuf_;	

// Mix Bin Parameters
#define kAudioMixer_MixBinCount	        3	// At present, for sampling rates :  fs, fs/2, fs/4 
#define kAudioMixer_MixBin_Index_FsDiv4 0
#define kAudioMixer_MixBin_Index_FsDiv2 1
#define kAudioMixer_MixBin_Index_FsDiv1 2
#define kAudioMixer_MixBin_Index_Fs     kAudioMixer_MixBin_Index_FsDiv1
	S16			*pMixBinBufs_ [kAudioMixer_MixBinCount];
	long	     mixBinFilled_[kAudioMixer_MixBinCount];
    long fsRack_[kAudioMixer_MixBinCount];

	long GetMixBinIndex        ( long samplingFrequency );
	long GetSamplingRateDivisor( long samplingFrequency );

// Sampling rate conversion (SRC) parameters
#define kAudioMixer_SRCCount (kAudioMixer_MixBinCount-1)
	SRC			src_[kAudioMixer_SRCCount][kAudioMixer_MaxOutChannels];

	U8			masterVolume_;			
    float       masterGainf_[kAudioMixer_MaxOutChannels];
    Q15         masterGaini_[kAudioMixer_MaxOutChannels];

//  Output EQ parameters
#define kAudioMixer_MaxEQBands  3
//    long        useOutEQ_;
    long        outEQ_BandCount_;
    EQ          outEQ_[kAudioMixer_MaxOutChannels][kAudioMixer_MaxEQBands];

// File I/O debug stuff
//    long readInSoundFile_  ;
//    long writeOutSoundFile_;
    long inSoundFileDone_  ;
    SNDFILE	*inSoundFile_;
    SF_INFO	inSoundFileInfo_;

    SNDFILE	*outSoundFile_;
    SF_INFO	outSoundFileInfo_;

// Input debug stuff
    long  inputIsDC;
    float inputDCValueDB;
    float inputDCValuef;
    Q15   inputDCValuei;

#ifdef NEED_SAWTOOTH
    long  inputIsSawtoothWave_;
    unsigned long z_;
    unsigned long delta_;
    float normalFrequency_;
    float phase_;
#endif

//  Soft Clipper parameters
    WAVESHAPER  outSoftClipper_[kAudioMixer_MaxOutChannels];

// MIDI parameters - only one player for now
	CMidiPlayer *pMidiPlayer_;

#define kAudioMixer_MaxTempBuffers	7
	S16*	pTmpBufs_      [kAudioMixer_MaxTempBuffers]; 
	S16*	pTmpBufOffsets_[kAudioMixer_MaxTempBuffers]; 

    Boolean     isPaused_;

// Some Debug variables
};

#endif		// LF_MIXER_H

LF_END_BRIO_NAMESPACE()

