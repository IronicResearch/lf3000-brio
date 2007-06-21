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
	
	//********************************
	// Audio Playback. 
	//********************************    

	// Set the output gain of the mixer.  (This includes MIDI.)
	void 		SetMasterVolume( U8 volume );

	// Plays an audio resource.
	// An audio done event will be posted to the listener if provided.
	// Currently only volume and pListener are interpreted.
	tAudioID 	StartAudio( tRsrcHndl			hRsrc, 
					U8					volume, 
					tAudioPriority		priority,
					S8					pan, 
					IEventListener*		pListener,
					tAudioPayload		payload,
					tAudioOptionsFlags	flags );

	// Pause a playing audio resource.
	void 		PauseAudio( tAudioID audioID );

	// Resume a playing audio resource from the previously paused position.
	void 		ResumeAudio( tAudioID audioID ); 

	// Stop the audio resource and free its mixer channel.  Optionally post
	// an audioDone event via the EventMgr.
	void 		StopAudio( tAudioID audioID, Boolean surpressDoneMessage ); 

	//********************************
	// MIDI functionality
	//********************************    

	// NOTE: Currently the midiID is ignored because only one file can be played at a time!

	// This function is currently unimplemented.
	tErrType 	AcquireMidiPlayer( tAudioPriority priority, IEventListener* pHandler, tMidiID* pMidiID );
	
	// Trigger a single MIDI note on event.
	// WARNING: DON'T DO THIS WHILE A MIDI FILE IS PLAYING, bad things may happen!!!
    tErrType 	MidiNoteOn( tMidiID midiID, U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );
	
	// Stop the previously trigger MIDI note.
    tErrType 	MidiNoteOff( tMidiID midiID, U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );

	// Start playback of a MIDI file.
	// Currently only the volume and pListener options are used.
	tMidiID 	StartMidiFile( tMidiID	midiID,
						tRsrcHndl		hRsrc, 
						U8					volume, 
						tAudioPriority		priority,
						IEventListener*		pListener,
						tAudioPayload		payload,
						tAudioOptionsFlags	flags );

	// Pause playback of a MIDI file. 
	void 		PauseMidiFile( tMidiID midiID );
	
	// Resume playback of a MIDI file.
    void 		ResumeMidiFile( tMidiID midiID );

	// Stop playback of a MIDI file and free the MIDI player. Optionally post
	// an audioDone event via the EventMgr.
    void 		StopMidiFile( tMidiID midiID, Boolean surpressDoneMessage );

	// these are still unimplemented!	
	tErrType 	ReleaseMidiPlayer( tMidiID midiID );
	tMidiID 	GetMidiIDForAudioID( tAudioID audioID );
	tAudioID 	GetAudioIDForMidiID( tMidiID midiID );
	tMidiTrackBitMask GetEnabledMidiTracks( tMidiID midiID );
	tErrType 	EnableMidiTracks( tMidiID midiID, tMidiTrackBitMask trackBitMask );
	tErrType 	TransposeMidiTracks( tMidiID midiID, tMidiTrackBitMask tracktBitMask, S8 transposeAmount );
	tErrType 	ChangeMidiInstrument( tMidiID midiID, tMidiTrackBitMask trackBitMask, tMidiInstr instr );
	tErrType 	ChangeMidiTempo( tMidiID midiID, S8 tempo );
	tErrType 	SendMidiCommand( tMidiID midiID, U8 cmd, U8 data1, U8 data2 );

	
private:
	class CAudioModule*	pModule_;
};

LF_END_BRIO_NAMESPACE()	
#endif /* LF_BRIO_AUDIOMPI_H */

// EOF
