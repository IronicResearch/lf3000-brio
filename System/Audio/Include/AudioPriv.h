#ifndef LF_BRIO_AUDIOPRIVATE_H
#define LF_BRIO_AUDIOPRIVATE_H
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		AudioPriv.h
//
// Description:
//		Defines the private, hidden data structures used by AudioMgrMPI. 
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <StringTypes.h>
#include <CoreModule.h>
#include <KernelMPI.h>
#include <DebugMPI.h>
#include <AudioConfig.h>
#include <AudioTask.h>
#include <AudioMsg.h>
#include <EventListener.h>

//#include <RsrcTypes.h>
//#include <AudioTypes.h>
//#include <AudioRsrcs.h>
//#include <AudioMixer.h>
LF_BEGIN_BRIO_NAMESPACE()


class IEventListener;

typedef U32 tRsrcHndl;

// Constants
const CString	kAudioModuleName	= "Audio";
const tVersion	kAudioModuleVersion	= 2;


//==============================================================================
class CAudioModule : public ICoreModule {
public:	
	// core functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// Overall Audio Control
	tErrType SetDefaultListener( const IEventListener* pListener );

	VTABLE_EXPORT tErrType	StartAudio( void );
	VTABLE_EXPORT tErrType	StopAudio( void );
	VTABLE_EXPORT tErrType	PauseAudio( void );
	VTABLE_EXPORT tErrType	ResumeAudio( void );

	VTABLE_EXPORT void 		SetMasterVolume( U8 volume );

	VTABLE_EXPORT tAudioID StartAudio( tRsrcHndl				hRsrc, 
										U8					volume, 
										tAudioPriority		priority,
										S8					pan, 
										IEventListener		*pListener,
										tAudioPayload		payload,
										tAudioOptionsFlags	flags );

	VTABLE_EXPORT void PauseAudio( tAudioID audioID );
	VTABLE_EXPORT void ResumeAudio( tAudioID audioID ); 
	VTABLE_EXPORT void StopAudio( tAudioID audioID, Boolean surpressDoneMessage ); 

	VTABLE_EXPORT tErrType AcquireMidiPlayer( tAudioPriority priority, IEventListener *pHandler, tMidiID *midiID );
	VTABLE_EXPORT tErrType ReleaseMidiPlayer( tMidiID midiID );
	
	VTABLE_EXPORT tMidiID StartMidiFile( tMidiID	midiID,
						tRsrcHndl			hRsrc, 
						U8					volume, 
						tAudioPriority		priority,
						IEventListener*		pListener,
						tAudioPayload		payload,
						tAudioOptionsFlags	flags );
	VTABLE_EXPORT void StopMidiFile( tMidiID midiID, Boolean surpressDoneMessage );
	VTABLE_EXPORT void PauseMidiFile( tMidiID midiID );
	VTABLE_EXPORT void ResumeMidiFile( tMidiID midiID );

	VTABLE_EXPORT tMidiID GetMidiIDForAudioID( tAudioID audioID );
	VTABLE_EXPORT tAudioID GetAudioIDForMidiID( tMidiID midiID );
	VTABLE_EXPORT tMidiTrackBitMask GetEnabledMidiTracks( tMidiID midiID );
	VTABLE_EXPORT tErrType EnableMidiTracks( tMidiID midiID, tMidiTrackBitMask trackBitMask );
	VTABLE_EXPORT tErrType TransposeMidiTracks( tMidiID midiID, tMidiTrackBitMask tracktBitMask, S8 transposeAmount );
	VTABLE_EXPORT tErrType ChangeMidiInstrument( tMidiID midiID, tMidiTrackBitMask trackBitMask, tMidiInstr instr );
	VTABLE_EXPORT tErrType ChangeMidiTempo( tMidiID midiID, S8 tempo );
	VTABLE_EXPORT tErrType SendMidiCommand( tMidiID midiID, U8 cmd, U8 data1, U8 data2 );
	VTABLE_EXPORT tErrType 	MidiNoteOn( tMidiID midiID, U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );
	VTABLE_EXPORT tErrType 	MidiNoteOff( tMidiID midiID, U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );
	
private:
	CKernelMPI* 			pKernelMPI_;
	CDebugMPI* 				pDebugMPI_;	
	tMessageQueueHndl 		hRecvMsgQueue_;
	tMessageQueueHndl 		hSendMsgQueue_;
	const IEventListener*	pDefaultListener_;

	void SendCmdMessage( CAudioCmdMsg& msg );
	tAudioID WaitForAudioID( void );
	tMidiID WaitForMidiID( void );
	tErrType WaitForStatus( void );
	
	// Limit object creation to the Module Manager interface functions
	CAudioModule();
	virtual ~CAudioModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));
};

LF_END_BRIO_NAMESPACE()	
#endif		// LF_BRIO_AUDIOPRIVATE_H

// EOF	
