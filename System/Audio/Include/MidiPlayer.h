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

#include "program_list.h"
#include "spmidi_load.h"
#include "spmidi_print.h"
#include "engine/spmidi_host.h"

#include "Dsputil.h"

LF_BEGIN_BRIO_NAMESPACE()

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
	CMidiPlayer( tMidiPlayerID id );
	~CMidiPlayer();

	U32			RenderBuffer( S16* pOutBuff, U32 numStereoFrames, long addToOutput  );
	inline U8	GetVolume()		{ return volume_; }
	void		SetVolume( U8 x );

	inline bool		IsFileActive() { return bFileActive_; };

	inline bool		IsActive() { return bActive_; };
	inline void		Activate()   { bActive_ = true; }
	inline void		DeActivate() { bActive_ = false; }

	// Get/Set the class member variables
	inline tMidiPlayerID		GetID() { return id_; }

	// Control the playing of MIDI data
	tErrType 	NoteOn(  U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );
	tErrType 	NoteOff( U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );
	tErrType 	SendCommand( U8 cmd, U8 data1, U8 data2 );

	tErrType 	StartMidiFile( tAudioStartMidiFileInfo* pInfo );
	tErrType 	PauseMidiFile( void );
	tErrType 	ResumeMidiFile( void );
	tErrType 	StopMidiFile( tAudioStopMidiFileInfo* pInfo );

	tErrType	GetEnableTracks( tMidiTrackBitMask* trackBitMask );
	tErrType	SetEnableTracks( tMidiTrackBitMask trackBitMask);
	tErrType	TransposeTracks( /* tMidiTrackBitMask trackBitMask, S8 transposeAmount */);
	tErrType	ChangeProgram( /* tMidiTrackBitMask trackBitMask, tMidiInstr instr */);
	tErrType	ChangeTempo( /* S8 Tempo */); 

private:
	CDebugMPI* 				pDebugMPI_;	
	CKernelMPI* 			pKernelMPI_;	
	SPMIDI_Context*			pContext_;		// Pointer to the Midi context being used
	MIDIFilePlayer*			pFilePlayer_;	// Pointer to the MidiFile player being used
	const IEventListener*	pListener_;		// pointer to caller's listener for done event
	tMidiPlayerID			id_;			// player ID 
	tMidiTrackBitMask		trackBitMask_;	// Track bit mask of the Midi playing
	Boolean					loopMidiFile_;
	S16* 					pMidiRenderBuffer_;
	tMutex     				render_mutex_;

	U8						volume_;
    float                   levelf_;
    Q15                     leveli_;

	U8						bFilePaused_:1;				// Player is paused
	U8						bFileActive_:1;				// Player has active file associated.
	U8						bActive_:1;					// Player has been acquired by client.

	// fixme/dg: make these get set as part of constructor?
	U32 	numFrames_;
	U32 	samplesPerFrame_;
	U32 	bitsPerSample_;

	void SendDoneMsg( void );
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_MIDIPLAYER_H

// EOF	
