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
#include <AudioEffectsProcessor.h>
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
	virtual ~CAudioMPI( void );

	//********************************
	// Audio output driver control. 
	//********************************    

	// Pauses the output driver, keeping the state of the audio system intact.
	// While paused, the audio system consumes no CPU.
	tErrType	PauseAudioSystem( void );

	// Resume the audio system from the previously paused state.
	tErrType	ResumeAudioSystem( void );

	// Set the final output gain of the mixer.  (This includes MIDI.)
	void 		SetMasterVolume( U8 volume );
	U8			GetMasterVolume( void ) const;

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
					const IEventListener*	pListener = kNull,
					tAudioPayload		payload = 0,
					tAudioOptionsFlags	flags = 0 );

	// Same as above, but uses defaults for unspecified params.
	tAudioID 	StartAudio( tRsrcHndl	hRsrc, 
					tAudioPayload		payload,
					tAudioOptionsFlags	flags );
	
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
	// Get/Set the Volume/Priority/Pan using a given audioID
	//********************************    
	// Returns the time in ms since the file started playing.
	U32 	GetAudioTime( tAudioID id ) const;
	
	U8		GetAudioVolume( tAudioID id ) const;
	void	SetAudioVolume( tAudioID id, U8 volume );

	tAudioPriority	GetAudioPriority( tAudioID id ) const;
	void	SetAudioPriority( tAudioID id, tAudioPriority priority );

	S8		GetAudioPan( tAudioID id ) const;
	void	SetAudioPan( tAudioID id, S8 pan );

	const IEventListener*	GetAudioEventListener( tAudioID id ) const;
	void	SetAudioEventListener( tAudioID id, IEventListener *pListener );

	//********************************
	// Defaults to use when value is not specified in the Start() call.
	//********************************    
	U8		GetDefaultAudioVolume( void ) const;
	void	SetDefaultAudioVolume( U8 volume );

	tAudioPriority	GetDefaultAudioPriority( void ) const;
	void	SetDefaultAudioPriority( tAudioPriority priority );

	S8		GetDefaultAudioPan( void ) const;
	void	SetDefaultAudioPan( S8 pan );

	const IEventListener*	GetDefaultAudioEventListener( void ) const;
	void	SetDefaultAudioEventListener( IEventListener *pListener );
	
	//********************************
	// Audio FX functionality
	//********************************    
	tErrType RegisterAudioEffectsProcessor( tRsrcType type, CAudioEffectsProcessor *pChain ); // TODO: stub
	tErrType RegisterGlobalAudioEffectsProcessor( CAudioEffectsProcessor *pChain ); // TODO: stub
	tErrType ChangeAudioEffectsProcessor( tAudioID id, CAudioEffectsProcessor *pChain ); // TODO: stub

	// Registers function to call to get the next chunk of stereo audio stream data // TODO: stub
	tErrType RegisterGetStereoAudioStreamFcn( tRsrcType type, tGetStereoAudioStreamFcn pFcn ); // TODO: stub

	//********************************
	// MIDI functionality
	//********************************    

	// NOTE: Currently the MIDI player ID is ignored because only one file can 
	// be played at a time!  There is only one player.
	// In the future if we implement multiple MIDI players you would use the ID to
	// specify which player to direct a msg.

	// This function activates the MIDI engine.  Don't do this unless you really need to play
	// MIDI, there is a MIPS cost to having the player active even if you aren't using it.
	// Always release it when you are done!
	tErrType 	AcquireMidiPlayer( tAudioPriority priority, IEventListener* pListener, tMidiPlayerID* pID );

	// Deactivate the MIDI engine.
	tErrType 	ReleaseMidiPlayer( tMidiPlayerID id );
	
	// Get the Audio ID associated with a currently playing MidiFile. 
	// You only need this if you want to change the Volume/Priority/Pan/Listener
	// using the methods above.
	tAudioID		GetAudioIDForMidiID( tMidiPlayerID id ); // TODO: stub
	
	// Start playback of a MIDI file.
	// Currently only the volume and pListener options are used.
    tErrType 	StartMidiFile( tMidiPlayerID	id,
						tRsrcHndl			hRsrc, 
						U8					volume, 
						tAudioPriority		priority,
						IEventListener*		pListener,
						tAudioPayload		payload,
						tAudioOptionsFlags	flags );

    // Uses defaults from MPI for volume, priority, and listener.
    tErrType 	StartMidiFile( tMidiPlayerID		id,
    							tRsrcHndl			hRsrc, 
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
	tErrType 	SetEnableMidiTracks( tMidiPlayerID id, tMidiTrackBitMask trackBitMask );
	
	tErrType 	TransposeMidiTracks( tMidiPlayerID id, tMidiTrackBitMask trackBitMask, S8 transposeAmount ); // TODO: stub
	tErrType 	ChangeMidiInstrument( tMidiPlayerID id, tMidiTrackBitMask trackBitMask, tMidiInstr instr ); // TODO: stub
	tErrType 	ChangeMidiTempo( tMidiPlayerID id, S8 tempo ); // TODO: stub

	// Send MIDI data to a player instance.
	// Trigger a single MIDI note on event.
	// WARNING: DON'T DO THIS WHILE A MIDI FILE IS PLAYING, bad things may happen!!!
    tErrType 	MidiNoteOn( tMidiPlayerID id, U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );
	
	// Stop the previously trigger MIDI note.
    tErrType 	MidiNoteOff( tMidiPlayerID id, U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );

    // Send raw MIDI msg to player.
	tErrType 	SendMidiCommand( tMidiPlayerID id, U8 cmd, U8 data1, U8 data2 );
	
private:
	class CAudioModule*	pModule_;
	U32					mpiID_;
};

LF_END_BRIO_NAMESPACE()	
#endif /* LF_BRIO_AUDIOMPI_H */

// EOF
