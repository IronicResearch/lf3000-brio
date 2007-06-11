#ifndef LF_BRIO_MIDIPLAYER_H
#define LF_BRIO_MIDIPLAYER_H

//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		MidiPlayer.h
//
// Description:
//		Defines the class to manage the playing of Midi data.
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
//#include <RsrcTypes.h>
#include <AudioTypes.h>
#include <AudioPlayer.h>

// Midi Engine includes
#include <spmidi.h>
#include <spmidi_util.h>
#include <midifile_player.h>

//==============================================================================
// Class:
//		CMidiPlayer
//
// Description:
//		Class to manage the playing of MIDI data. 
//==============================================================================
//class CMidiPlayer : public CAudioPlayer {
class CMidiPlayer {
public:
	CMidiPlayer();
	~CMidiPlayer();

	U32			RenderBuffer( S16* pOutBuff, U32 numStereoFrames );
	bool		IsFileActive() { return bFileActive_; };

	// Get/Set the class member variables
	inline tMidiID				GetMidiID() { return midiID_; }
	inline tMidiTrackBitMask	GetEnabledTracks() { return trackBitMask_; }
	tErrType					EnableTracks(tMidiTrackBitMask trackBitMask);

	// Control the playing of the Midi
	tErrType 	NoteOn( U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );
	tErrType 	NoteOff( U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );
	tErrType 	StartMidiFile( tAudioStartMidiFileInfo* pInfo );
	tErrType 	PauseMidiFile( tAudioPauseMidiFileInfo* pInfo );
	tErrType 	ResumeMidiFile( tAudioResumeMidiFileInfo* pInfo );
	tErrType 	StopMidiFile( tAudioStopMidiFileInfo* pInfo );

	tErrType	TransposeTracks( tMidiTrackBitMask tracktBitMask, S8 transposeAmount );
	tErrType	ChangeInstrument( tMidiTrackBitMask trackBitMask, tMidiInstr instr );
	tErrType	ChangeTempo( S8 Tempo); 
	tErrType 	SendCommand( U8 cmd, U8 data1, U8 data2);

private:
	SPMIDI_Context*			pContext_;		// Pointer to the Midi context being used
	MIDIFilePlayer*			pFilePlayer_;	// Pointer to the MidiFile player being used
	const IEventListener*	pListener_;		// pointer to caller's listener for done event
	tMidiID					midiID_;		// MidiID of the Midi playing 
	tMidiTrackBitMask		trackBitMask_;	// Track bit mask of the Midi playing
	bool					loopMidiFile_;
	S16* 					pMidiRenderBuffer_;
	float					volume_;
	U8						bFilePaused_:1;				// Player is paused
	U8						bFileActive_:1;				// Player has active file associated.

	// fixme/dg: make these get set as part of constructor?
	U32 	numFrames_;
	U32 	samplesPerFrame_;
	U32 	bitsPerSample_;

	void SendDoneMsg( void );
};

#endif		// LF_BRIO_MIDIPLAYER_H

// EOF	
