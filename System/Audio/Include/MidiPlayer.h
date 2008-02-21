#ifndef LF_BRIO_MIDIPLAYER_H
#define LF_BRIO_MIDIPLAYER_H

//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// MidiPlayer.h
//
//					Class to manage playing of MIDI data
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <AudioTypes.h>
#include <AudioPlayer.h>

#include <MidiLoader.h>

#include "Dsputil.h"

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Class:
//		CMidiPlayer
//
// Description:
//		Class to manage the playing of MIDI data. 
//==============================================================================
class CMidiPlayer {
 public:
	CMidiPlayer( tMidiPlayerID id );
	~CMidiPlayer();

	U32			Render( S16* pOut, U32 numStereoFrames );

// Return status
	inline U8 ShouldSendDoneMessage()	 { return bSendDoneMessage_;   }
	inline U8 ShouldSendLoopEndMessage() { return bSendLoopEndMessage_;}

	inline bool		IsFileActive() { return bFileActive_; };

	inline bool		IsActive()	 { return bActive_; };
	inline void		Activate()	 { bActive_ = true; }
	inline void		DeActivate() { bActive_ = false; bFileActive_ = false;}

	// Get/Set class member variables
	inline tMidiPlayerID	GetID() { return id_; }
	inline U8	GetVolume()			{ return volume_; }
	void		SetVolume( U8 x );

	inline S8	GetPan()			{ return pan_; }
	void		SetPan(S8 x);

	// MIDI channel messages
	tErrType	NoteOn( U8 channel,
						U8 note,
						U8 velocity,
						tAudioOptionsFlags flags );

	tErrType	NoteOff( U8 channel,
						 U8 note,
						 U8 velocity,
						 tAudioOptionsFlags flags );

	tErrType	SendCommand( U8 cmd, U8 data1, U8 data2 );

	// MIDI file transport control
	tErrType	StartMidiFile( tAudioStartMidiFileInfo *pInfo );
	tErrType	StopMidiFile(  tAudioStopMidiFileInfo  *pInfo );
	tErrType	PauseMidiFile(	void );
	tErrType	ResumeMidiFile( void );

	tErrType	GetEnableTracks( tMidiTrackBitMask *d );
	tErrType	SetEnableTracks( tMidiTrackBitMask	d);
	tErrType	TransposeTracks( tMidiTrackBitMask	d, S8 semitones );
	tErrType	ChangeProgram( tMidiTrackBitMask	d,
							   tMidiPlayerInstrument number );
	tErrType	ChangeTempo( S8 tempo); 
	
 private:
	CDebugMPI*				pDebugMPI_;	
	
//#define USE_MIDI_PLAYER_MUTEX
#ifdef USE_MIDI_PLAYER_MUTEX
	CKernelMPI*				pKernelMPI_;	
	tMutex					*renderMutex_;
#endif

	// Mobileer MIDIengine variables
	SPMIDI_Context*			pContext_;		
	MIDIFilePlayer*			pFilePlayer_;	
	SPMIDI_Orchestra		*spMIDI_orchestra_;

	const IEventListener*	pListener_;
	tAudioOptionsFlags		optionsFlags_;	
	U8						bSendDoneMessage_;
	U8						bSendLoopEndMessage_;

	tMidiPlayerID			id_;			 
	tMidiTrackBitMask		trackBitMask_;

	// DSP information
	U8			pan_;
	U8			volume_;

#define kAudioMixerChannel_MaxOutChannels 2		
	float		panValuesf[kAudioMixerChannel_MaxOutChannels];
	Q15			panValuesi[kAudioMixerChannel_MaxOutChannels];
	float		gainf;
	Q15			gaini;
	float		levelsf[kAudioMixerChannel_MaxOutChannels];
	Q15			levelsi[kAudioMixerChannel_MaxOutChannels];

	// State activity flags
	Boolean		shouldLoop_;
	S32			loopCount_;
	S32			loopCounter_;

	U8			bFilePaused_;				
	U8			bFileActive_;				
	U8			bActive_;

	// Data configuration
	U32		framesPerIteration_;
	U32		channels_;
	U32		bitsPerSample_;

	void SendLoopEndMsg( void );
	void SendDoneMsg(	 void );

	void RecalculateLevels();
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_MIDIPLAYER_H
	
