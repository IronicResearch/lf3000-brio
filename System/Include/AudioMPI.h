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
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;		
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	// Default listener is not currently used
	CAudioMPI( const IEventListener* pDefaultListener = NULL );
	virtual ~CAudioMPI();

	//********************************
	// Overall audio control 
	//********************************
	// Controls the audio output driver.  
	tErrType	StartAudio( void );
	tErrType	StopAudio( void );
	tErrType	PauseAudio( void );
	tErrType	ResumeAudio( void );
	
	// Set the output gain of the mixer.
	void 		SetMasterVolume( U8 volume );

	// Plays an audio resource.  Currently only volume and pListener are interpreted.
	// An audio done mesasge will be posted to the listener if provided.
	tAudioID PlayAudio( tRsrcHndl			hRsrc, 
						U8					volume, 
						tAudioPriority		priority,
						S8					pan, 
						IEventListener*		pListener,
						tAudioPayload		payload,
						tAudioOptionsFlags	flags );

	// need policy for unloading audio resources after they have been played. ReleaseAudioRsrc()?

	// MIDI functionality
	// Currently the midiID is ignored, only one file can be played at a time.
	tErrType 	AcquireMidiPlayer( tAudioPriority priority, IEventListener* pHandler, tMidiID* pMidiID );
	tErrType 	MidiNoteOn( tMidiID midiID, U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );
	tErrType 	MidiNoteOff( tMidiID midiID, U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );

	tAudioID 	PlayMidiFile( tMidiID	midiID,
						tRsrcHndl		hRsrc, 
						U8					volume, 
						tAudioPriority		priority,
						IEventListener*		pListener,
						tAudioPayload		payload,
						tAudioOptionsFlags	flags );

	// these are still unimplemented!	
	tErrType 	ReleaseMidiPlayer( tMidiID midiID );
	tMidiID 	GetMidiIDForAudioID( tAudioID audioID );
	tAudioID 	GetAudioIDForMidiID( tMidiID midiID );
	void 		StopMidiPlayer( tMidiID midiID, Boolean surpressDoneMessage );
	void 		PauseMidiPlayer( tMidiID midiID );
	void 		ResumeMidiPlayer( tMidiID midiID );
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
