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
class CMidiPlayer : public CAudioPlayer {
 public:

	CMidiPlayer( tAudioStartAudioInfo *pInfo, tAudioID id );

	~CMidiPlayer();

	void	RewindFile();
	U32		GetAudioTime_mSec( void );

	U32			Render( S16* pOut, U32 numStereoFrames );

// Return status
	inline U8 ShouldSendDoneMessage()	 { return bSendDoneMessage_;   }
	inline U8 ShouldSendLoopEndMessage() { return bSendLoopEndMessage_;}

	inline bool		IsFileActive() { return bFileActive_; };

	// Get/Set class member variables
	inline tAudioID	GetID() { return id_; }
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
	tErrType	StartMidiFile( tAudioStartAudioInfo *pInfo );
	tErrType	StopMidiFile( Boolean noDoneMsg );
	
	// As soon as we eliminate direct calls to this class from the MPI, we can
	// eliminate these declarations.
	void		Pause(	void );
	void		Resume( void );

	tErrType	GetEnableTracks( tMidiTrackBitMask *d );
	tErrType	SetEnableTracks( tMidiTrackBitMask	d);
	tErrType	TransposeTracks( tMidiTrackBitMask	d, S8 semitones );
	tErrType	ChangeProgram( tMidiTrackBitMask	d,
							   tMidiPlayerInstrument number );
	tErrType	ChangeTempo( S8 tempo); 
	
 private:
	CDebugMPI*				pDebugMPI_;	
	CKernelMPI*				pKernelMPI_;
	
	// Mobileer MIDIengine variables
	SPMIDI_Context*			pContext_;		
	MIDIFilePlayer*			pFilePlayer_;	
	SPMIDI_Orchestra		*spMIDI_orchestra_;

	tMidiTrackBitMask		trackBitMask_;

	U8 *pMidiFileImage;

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

	Boolean bSendDoneMessage_;
	Boolean bSendLoopEndMessage_;
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_MIDIPLAYER_H
	
