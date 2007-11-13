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
//		Defines the class to manage the playing of MIDI data.
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <AudioTypes.h>
#include <AudioPlayer.h>

// Mobileer Midi Engine includes
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

	U32			RenderBuffer( S16* pOut, U32 numStereoFrames  );

	inline U8	GetVolume()		{ return volume_; }
	void		SetVolume( U8 x );

	inline S8	GetPan()		{ return pan_; }
	void		SetPan(S8 x);

	inline bool		IsFileActive() { return bFileActive_; };

	inline bool		IsActive() { return bActive_; };
	inline void		Activate()   { bActive_ = true; }
	inline void		DeActivate() { bActive_ = false; }

	// Get/Set the class member variables
	inline tMidiPlayerID		GetID() { return id_; }

	// MIDI channel messages
	tErrType 	NoteOn(  U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );
	tErrType 	NoteOff( U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );
	tErrType 	SendCommand( U8 cmd, U8 data1, U8 data2 );

// MIDI file transport control
	tErrType 	StartMidiFile( tAudioStartMidiFileInfo* pInfo );
	tErrType 	PauseMidiFile( void );
	tErrType 	ResumeMidiFile( void );
	tErrType 	StopMidiFile( tAudioStopMidiFileInfo* pInfo );

	tErrType	GetEnableTracks( tMidiTrackBitMask* d );
	tErrType	SetEnableTracks( tMidiTrackBitMask d);
	tErrType	TransposeTracks( tMidiTrackBitMask d, S8 transposeAmount );
	tErrType	ChangeProgram( tMidiTrackBitMask d, tMidiPlayerInstrument instr );
	tErrType	ChangeTempo( S8 Tempo); 

private:
	CDebugMPI* 				pDebugMPI_;	
	CKernelMPI* 			pKernelMPI_;	

// Mobileer engine variables
	SPMIDI_Context*			pContext_;		
	MIDIFilePlayer*			pFilePlayer_;	
	SPMIDI_Orchestra        *spmidi_orchestra_;

	const IEventListener*	pListener_;		// pointer to caller's listener for done event
	tAudioOptionsFlags	optionsFlags_;	
	U8			bDoneMessage_:1;		// Caller requests done message 

	tMidiPlayerID			id_;			// player ID 
	tMidiTrackBitMask		trackBitMask_;	// Track bit mask of the Midi playing
	S16* 					pMidiRenderBuffer_;
	tMutex     				render_mutex_;

// DSP information
	U8			pan_;
	U8			volume_;

#define kAudioMixerChannel_MaxOutChannels 2     
	float		panValuesf[kAudioMixerChannel_MaxOutChannels];
	Q15			panValuesi[kAudioMixerChannel_MaxOutChannels];
	float		gainf;
	Q15			gaini;
    float       levelsf[kAudioMixerChannel_MaxOutChannels]; // gain * panValue
    Q15         levelsi[kAudioMixerChannel_MaxOutChannels];

// Some MIDI state activity flags
	Boolean		shouldLoop_;
    S32         loopCount_;
    S32         loopCounter_;

	U8			bFilePaused_:1;				
	U8			bFileActive_:1;				// Player has active file association
	U8			bActive_:1;					// Player has been acquired by client.

// Data configuration
	U32 	framesPerIteration_;
	U32 	samplesPerFrame_;
	U32 	bitsPerSample_;

	void SendDoneMsg( void );
    void RecalculateLevels();
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_MIDIPLAYER_H
	
