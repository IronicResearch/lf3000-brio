#ifndef LF_BRIO_AUDIOMPI_H
#define LF_BRIO_AUDIOMPI_H
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		AudioMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the Audio module. 
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <ResourceTypes.h>
#include <AudioTypes.h>
#include <CoreMPI.h>
//#include <EventListener.h>
LF_BEGIN_BRIO_NAMESPACE()

class IEventListener;


//==============================================================================
// Class:
//		CAudioMgrMPI
//
// Description:
//		Module Public Interface (MPI) class for the Audio Manager module. 
//==============================================================================
class CAudioMPI : public ICoreMPI {
public:
	//********************************
	// ICoreMPI functionality
	//********************************

	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;		
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	//********************************
	// Class-specific functionality
	//********************************    

	// The default listener is not currently used.
	CAudioMPI( const IEventListener* pDefaultListener = NULL );
	virtual ~CAudioMPI();

	//********************************
	// Audio output driver control. 
	//********************************    

	// Opens the audio output driver and starts the Brio audio system.
	tErrType	StartAudio( void );

	// Pauses the output driver, keeping the state of the audio system intact.
	// While paused, the audio system consumes no CPU.
	tErrType	PauseAudio( void );

	// Resume the audio system from the previously paused state.
	tErrType	ResumeAudio( void );

	// Stop the audio output driver and clear the audio system state.
	tErrType	StopAudio( void );
	

	// Set the final output gain of the mixer.  (This includes MIDI.)
	void 		SetMasterVolume( U8 volume );
	U8			GetMasterVolume( void );

	//********************************
	// Audio Playback. 
	//********************************    
	
	// Plays an audio resource.
	// An audio done event will be posted to the listener if provided.
	// Currently only volume and pListener are interpreted.
	tAudioID 	StartAudio( tRsrcHndl	hRsrc, 
					U8					volume, 
					tAudioPriority		priority,
					S8					pan, 
					IEventListener*		pListener,
					tAudioPayload		payload,
					tAudioOptionsFlags	flags );

	// Returns the time in ms since the file started playing.
	U32 		GetAudioTime( tAudioID id );

	// Pause a playing audio resource.
	void 		PauseAudio( tAudioID id );

	// Resume a playing audio resource from the previously paused position.
	void 		ResumeAudio( tAudioID id ); 

	// Stop the audio resource and free its mixer channel.  Optionally post
	// an audioDone event via the EventMgr.
	void 		StopAudio( tAudioID id, Boolean surpressDoneMessage ); 

	// Is this piece of audio still playing?
	Boolean		IsAudioPlaying( tAudioID id );
	
	// Is the audio system playing anything?
	Boolean		IsAudioPlaying( void );

	//********************************
	// MIDI functionality
	//********************************    

	// NOTE: Currently the midiID is ignored because only one file can be played at a time!
	// in the future if we implement multiple MIDI players you would use the midiID to
	// specify which player to direct the note or midifile msgs.

	// This function activates the MIDI engine.  Don't do this unless you really need to play
	// MIDI, there is a MIPS cost to having the player active even if you aren't using it.
	// Always release it when you are done!
	tErrType 	AcquireMidiPlayer( tAudioPriority priority, IEventListener* pHandler, tMidiPlayerID* pID );

	// Deactivate the MIDI engine.
	tErrType 	ReleaseMidiPlayer( tMidiPlayerID id );
	
	// Start playback of a MIDI file.
	// Currently only the volume and pListener options are used.
    tErrType 	StartMidiFile( tMidiPlayerID	id,
						tRsrcHndl			hRsrc, 
						U8					volume, 
						tAudioPriority		priority,
						IEventListener*		pListener,
						tAudioPayload		payload,
						tAudioOptionsFlags	flags );

	// Is this MIDI file still playing?
	Boolean		IsMidiFilePlaying( tMidiPlayerID id );
	
	// Is the MIDI system playing anything?
	Boolean		IsMidiFilePlaying( void );

	// Pause playback of a MIDI file. 
	void 		PauseMidiFile( tMidiPlayerID id );
	
	// Resume playback of a MIDI file.
    void 		ResumeMidiFile( tMidiPlayerID id );

	// Stop playback of a MIDI file and free the MIDI player. Optionally post
	// an audioDone event via the EventMgr.
    void 		StopMidiFile( tMidiPlayerID id, Boolean surpressDoneMessage );

    // Get and set properities of a playing MIDI file.
    tMidiTrackBitMask GetEnabledMidiTracks( tMidiPlayerID id );
	tErrType 	EnableMidiTracks( tMidiPlayerID id, tMidiTrackBitMask trackBitMask );
	tErrType 	TransposeMidiTracks( tMidiPlayerID id, tMidiTrackBitMask tracktBitMask, S8 transposeAmount );
	tErrType 	ChangeMidiInstrument( tMidiPlayerID id, tMidiTrackBitMask trackBitMask, tMidiInstr instr );
	tErrType 	ChangeMidiTempo( tMidiPlayerID id, S8 tempo );

	// Send MIDI data to a player instance.
	// Trigger a single MIDI note on event.
	// WARNING: DON'T DO THIS WHILE A MIDI FILE IS PLAYING, bad things may happen!!!
    tErrType 	MidiNoteOn( tMidiPlayerID id, U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );
	
	// Stop the previously trigger MIDI note.
    tErrType 	MidiNoteOff( tMidiPlayerID id, U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );

	tErrType 	SendMidiCommand( tMidiPlayerID id, U8 cmd, U8 data1, U8 data2 );
	
private:
	class CAudioModule*	pModule_;
};

LF_END_BRIO_NAMESPACE()	
#endif /* LF_BRIO_AUDIOMPI_H */

// EOF
