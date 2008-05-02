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

#include <ButtonMPI.h>
#include <DebugMPI.h>
#include <KernelMPI.h>

#include <src.h>
#include <shape.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Class: CAudioMixer
//
// Description: manage the low-level mixing of audio channels. 
//==============================================================================
const tEventType kMixerTypes[] = { kAudioCompletedEvent };

class CAudioMixer : private IEventListener
{
 public:
	CAudioMixer( int numChannels );
	~CAudioMixer();
		
	CChannel* FindChannel( tAudioID id );
	
	Boolean IsAnyAudioActive( void );

	CMidiPlayer *GetMidiPlayerPtr( void ) { return pMidiPlayer_; }
	tAudioID GetMidiPlayer_AudioID( void ) { return (1); }

	tAudioID AddPlayer( tAudioStartAudioInfo *pInfo, char *sExt );
	void RemovePlayer( tAudioID id, Boolean noDoneMessage );
	void PausePlayer( tAudioID id );
	void ResumePlayer( tAudioID id );
	Boolean IsPlayerPlaying( tAudioID id );

	CMidiPlayer *CreateMIDIPlayer();
	void DestroyMIDIPlayer();

	void SetMasterVolume( U8 x ) ; 
	U8 GetMasterVolume( void ) { return audioState_.masterVolume; }

	void Pause(); 
	void Resume(); 
	Boolean IsPaused() { return isPaused_; }

	Boolean IsSpeakerEnabled( ) {
		return ((Boolean)audioState_.speakerEnabled);
	}
	void EnableSpeaker( Boolean x );

	int Render( S16 *pOut, U32 frameCount );
	
	static int WrapperToCallRender( S16 *pOut, U32 frameCount, void *pObject );

	tAudioState audioState_;

	void GetAudioState(tAudioState *d);
	void SetAudioState(tAudioState *d);
	
	// ---- DEBUG File I/O
	// Debug : info for sound file input/output
	int OpenInSoundFile (char *path);
	int OpenOutSoundFile(char *path);

	tPriorityPolicy GetPriorityPolicy(void);
	tErrType SetPriorityPolicy(tPriorityPolicy policy);

 private:
	CButtonMPI *pButtonMPI_;
	CChannel* FindFreeChannel( tAudioPriority priority );
	CAudioPlayer *CreatePlayer( tAudioStartAudioInfo *pInfo, char *sExt );
	void DestroyPlayer(CAudioPlayer *pPlayer);
	void HandlePlayerEvent( CAudioPlayer *pPlayer, tEventType type );
	Boolean HandlePlayerLooping(CAudioPlayer *pPlayer);
	tEventStatus Notify( const IEventMessage &msgIn );

	float			samplingFrequency_;

	void SetDSP();
	void UpdateDSP();
	void ResetDSP();
	void SetSamplingFrequency( float x );
	tAudioID GetNextAudioID(void);
	tAudioID GetNextMidiID(void);

	// These are versions of Mixer functions that can be called if you already
	// hold the mixer lock.
	CChannel* FindChannelInternal( tAudioID id );
	void RemovePlayerInternal( tAudioID id, Boolean noDoneMessage );

// Didj is hard-limited to 4 channels : 3 audio + 1 MIDI input channels (stereo)
#define kAudioMixer_MaxInAudioChannels	4		// 3 active but can have more if others paused	 
#define kAudioMixer_MaxActiveAudioChannels	3	 
#define kAudioMixer_MaxInMIDIChannels	1	 
#define kAudioMixer_MaxInChannels		(kAudioMixer_MaxInAudioChannels)	
#define kAudioMixer_MaxOutChannels		2

	// Priority Support Features
	typedef Boolean ConditionFunction(CAudioPlayer *pPlayer);
	// Find a channel that can be halted in order to play a player of priority
	// priority, subject to the approval of ConditionFunction.
	CChannel *FindKillableChannel(ConditionFunction *cond,
								  tAudioPriority priority);

	// Channel parameters
	U8			numInChannels_; // for now, all output in stereo (including replicated mono)
	CChannel*	pChannels_;			// Array of channels
	S16			pChannelBuf_[kAudioOutBufSizeInWords];	

// Mix Bin Parameters
#define kAudioMixer_MixBinCount			3	// At present, for sampling rates :	 fs, fs/2, fs/4 
#define kAudioMixer_MixBin_Index_FsDiv4 0
#define kAudioMixer_MixBin_Index_FsDiv2 1
#define kAudioMixer_MixBin_Index_FsDiv1 2
#define kAudioMixer_MixBin_Index_Fs kAudioMixer_MixBin_Index_FsDiv1
#define kAudioMixer_MixBinBufferLength_Words  \
	(kAudioOutBufSizeInWords + kSRC_Filter_MaxDelayElements)
	S16 pMixBinBufs_[kAudioMixer_MixBinCount][kAudioMixer_MixBinBufferLength_Words];
	long mixBinFilled_[kAudioMixer_MixBinCount];
	long fsRack_[kAudioMixer_MixBinCount];

	long GetMixBinIndex( long samplingFrequency );
	long GetSamplingRateDivisor( long samplingFrequency );

// Sampling rate conversion (SRC) parameters
#define kAudioMixer_SRCCount (kAudioMixer_MixBinCount-1)
	SRC src_[kAudioMixer_SRCCount][kAudioMixer_MaxOutChannels];
	float masterGainf_[kAudioMixer_MaxOutChannels];
	Q15 masterGaini_[kAudioMixer_MaxOutChannels];

	// Headphone gain
	float headphoneGainDB_;
	float headphoneGainF_;
	Q15 headphoneGainWholeI_;
	float headphoneGainFracF_;
	Q15 headphoneGainFracI_;

	//	Soft Clipper parameters
	WAVESHAPER	outSoftClipper_[kAudioMixer_MaxOutChannels];

	// MIDI parameters - only one player for now
	CMidiPlayer *pMidiPlayer_;

#define kAudioMixer_MaxTempBuffers	7
#define kAudioMixer_TempBufferWords \
	(kAudioMixer_MaxOutChannels*kAudioOutBufSizeInWords)
	S16 pTmpBufs_[kAudioMixer_MaxTempBuffers][kAudioMixer_TempBufferWords];
	S16* pTmpBufOffsets_[kAudioMixer_MaxTempBuffers]; 

	Boolean isPaused_;

	tAudioID nextAudioID;
	tAudioID nextMidiID;

	class CMixerMessage : public IEventMessage 
	{
	public:
		CMixerMessage(CAudioPlayer *pPlayer, tEventType type):
			IEventMessage(type)
		{
			pPlayer_ = pPlayer;
		}

		virtual U16	GetSizeInBytes() const
		{
			return sizeof(CMixerMessage);
		}
		
		CAudioPlayer *pPlayer_;
	};
	
	tPriorityPolicy currentPolicy;

};

#endif		// LF_MIXER_H

LF_END_BRIO_NAMESPACE()

