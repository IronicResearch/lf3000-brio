#ifndef LF_BRIO_AUDIOMPI_H
#define LF_BRIO_AUDIOMPI_H

//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
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
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <ResourceTypes.h>
//#include <AudioTypes.h>
#include <CoreMPI.h>
//#include <EventListener.h>


//==============================================================================
// Class:
//		CAudioMgrMPI
//
// Description:
//		Module Public Interface (MPI) class for the Audio Manager module. 
//==============================================================================
class CAudioMPI : public ICoreMPI {
public:
	// MPI core functions
	virtual	Boolean		IsValid() const;	
	
	virtual	tErrType	GetMPIVersion(tVersion &version) const;		   
	virtual	tErrType	GetMPIName(ConstPtrCString &pName) const;		

	virtual	tErrType	GetModuleVersion(tVersion &version) const;
	virtual	tErrType	GetModuleName(ConstPtrCString &pName) const;	
	virtual	tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;


	// class-specific functionality
	CAudioMPI();
	virtual ~CAudioMPI();

	//********************************
	// Overall audio control 
	//********************************
	tErrType	StartAudio( void );
	tErrType	StopAudio( void );
	tErrType	PauseAudio( void );
	tErrType	ResumeAudio( void );
	
private:
	class CAudioModule*	mpModule;
};

/*
	//********************************
	// Registration
	//********************************
	// Registers the function to call to get the next chunk of stereo audio stream data
	tErrType	RegisterGetStereoAudioStreamFcn(tRsrcType type, tGetStereoAudioStreamFcn pFcn); 
	// Registers the audio effects processor for a given AudioRsrcType
	tErrType	RegisterAudioEffectsProcessor(tRsrcType type, CAudioEffectsProcessor *pChain); 
	// Registers the global audio effects processor
	tErrType	RegisterGlobalAudioEffectsProcessor(CAudioEffectsProcessor *pChain); 

	//********************************
	// Audio effect control
	//********************************
	// Changes the audio effects processor for a given audio ID
	tErrType	ChangeAudioEffectsProcessor(tAudioID audioID, CAudioEffectsProcessor *pChain); 

	//******************************************************************
	// Master volume control: range is from 0 to 255
	//******************************************************************
	void	SetMasterVolume(U8 volume);
	U8		GetMasterVolume( void );


	//********************************
	// Audio player control
	//********************************
	// Play a single audio resource
	tAudioID PlayAudio(tRsrcHndl			hRsrc, 
						U8					volume, 
						tAudioPriority		priority,
						S8					pan, 
						IEventListener		*pListener,
						tAudioPayload		payload,
						tAudioOptionsFlags	flags);
	tAudioID PlayAudio(tRsrcHndl			hRsrc, 
						tAudioPayload		payload,
						tAudioOptionsFlags	flags);

	// Control the playing of a given audioID 
	void	StopAudio(tAudioID audioID, Boolean surpressDoneMessage = false);
	void	PauseAudio(tAudioID audioID);
	void	ResumeAudio(tAudioID audioID);

	// Get/Set the Volume/Priority/Pan/EventHandler of a given audioID
	U8		GetAudioVolume(tAudioID audioID); 
	void	SetAudioVolume(tAudioID audioID, U8 volume); 
	tAudioPriority	GetAudioPriority(tAudioID audioID); 
	void	SetAudioPriority(tAudioID audioID, tAudioPriority priority); 
	S8		GetAudioPan(tAudioID audioID); 
	void	SetAudioPan(tAudioID audioID, S8 pan); 
//	IEventHandler*	GetAudioEventHandler(tAudioID audioID); 
//	void	SetAudioEventHandler(tAudioID audioID, IEventHandler *pHandler); 

	// Get/Set the default Volume/Priority/Pan/EventHandler 
	// to use when one hasn't been specified
	U8		GetDefaultAudioVolume(); 
	void	SetDefaultAudioVolume(U8 volume); 
	tAudioPriority	GetDefaultAudioPriority(); 
	void	SetDefaultAudioPriority(tAudioPriority priority); 
	S8		GetDefaultAudioPan(); 
	void	SetDefaultAudioPan(S8 pan); 
//	IEventHandler*	GetDefaultAudioEventHandler(); 
//	void	SetDefaultAudioEventHandler(IEventHandler *pHandler); 

	// Return whether the audio is currently playing
	Boolean IsAudioBusy(tAudioID audioID);
	Boolean IsAnyAudioBusy();

	//********************************
	// Midi player control
	//********************************
	// Acquire a new MidiPlayer to control directly  
//	tErrType	AcquireMidiPlayer(tAudioPriority priority, IEventHandler *pHandler, tMidiID *midiID);
	tErrType	ReleaseMidiPlayer(tMidiID midiID);
	// Get the Midi/Audio ID associated with a currently playing Audio/Midi ID
	// Only need these if want to play notes using existing MidiPlayer
	tMidiID		GetMidiIDForAudioID(tAudioID audioID);
	tAudioID	GetAudioIDForMidiID(tMidiID midiID);

	// Control the playing of a given MidiPlayer 
	void		StopMidiPlayer(tMidiID midiID, Boolean surpressDoneMessage = false);
	void		PauseMidiPlayer(tMidiID midiID);
	void		ResumeMidiPlayer(tMidiID midiID);

	tMidiTrackBitMask	GetEnabledMidiTracks(tMidiID midiID);
	tErrType	EnableMidiTracks(tMidiID midiID, tMidiTrackBitMask trackBitMask);
	tErrType	TransposeMidiTracks(tMidiID midiID, tMidiTrackBitMask trackBitMask, S8 transposeAmount);
	tErrType	ChangeMidiInstrument(tMidiID midiID, tMidiTrackBitMask trackBitMask, tMidiInstr instr);
	tErrType	ChangeMidiTempo(tMidiID midiID, S8 tempo); 
	tErrType 	SendMidiCommand(tMidiID midiID, U8 cmd, U8 data1, U8 data2);

	// Play a note on a given track of the MidiPlayer
	tErrType 	PlayMidiNote(tMidiID	midiID,
						U8 			track, 
						U8			pitch, 
						U8			velocity, 
						U16			noteCount,
						tAudioOptionsFlags	flags);
*/


#endif /* LF_BRIO_AUDIOMPI_H */

// EOF
