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
	VTABLE_EXPORT tErrType	PauseAudio( void );
	VTABLE_EXPORT tErrType	ResumeAudio( void );
	VTABLE_EXPORT tErrType	StopAudio( void );

	VTABLE_EXPORT void 		SetMasterVolume( U8 volume );
	VTABLE_EXPORT U8		GetMasterVolume( void );

	VTABLE_EXPORT tAudioID StartAudio( tRsrcHndl			hRsrc, 
										U8					volume, 
										tAudioPriority		priority,
										S8					pan, 
										IEventListener		*pListener,
										tAudioPayload		payload,
										tAudioOptionsFlags	flags );

	VTABLE_EXPORT U32 		GetAudioTime( tAudioID id );

	VTABLE_EXPORT void		PauseAudio( tAudioID id );
	VTABLE_EXPORT void 		ResumeAudio( tAudioID id ); 
	VTABLE_EXPORT void 		StopAudio( tAudioID id, Boolean surpressDoneMessage ); 

	VTABLE_EXPORT Boolean	IsAudioPlaying( tAudioID id );
	VTABLE_EXPORT Boolean	IsAudioPlaying( void );

	
	VTABLE_EXPORT tErrType	AcquireMidiPlayer( tAudioPriority priority, IEventListener *pHandler, tMidiPlayerID *id );
	VTABLE_EXPORT tErrType	ReleaseMidiPlayer( tMidiPlayerID id );
	
	VTABLE_EXPORT tMidiPlayerID StartMidiFile( tMidiPlayerID	id,
						tRsrcHndl			hRsrc, 
						U8					volume, 
						tAudioPriority		priority,
						IEventListener*		pListener,
						tAudioPayload		payload,
						tAudioOptionsFlags	flags );
	VTABLE_EXPORT void StopMidiFile( tMidiPlayerID id, Boolean surpressDoneMessage );
	VTABLE_EXPORT void PauseMidiFile( tMidiPlayerID id );
	VTABLE_EXPORT void ResumeMidiFile( tMidiPlayerID id );

	VTABLE_EXPORT Boolean IsMidiFilePlaying( tMidiPlayerID id );
	VTABLE_EXPORT Boolean IsMidiFilePlaying( void );

	VTABLE_EXPORT tMidiTrackBitMask GetEnabledMidiTracks( tMidiPlayerID id );
	VTABLE_EXPORT tErrType EnableMidiTracks( tMidiPlayerID id, tMidiTrackBitMask trackBitMask );
	VTABLE_EXPORT tErrType TransposeMidiTracks( tMidiPlayerID id, tMidiTrackBitMask tracktBitMask, S8 transposeAmount );
	VTABLE_EXPORT tErrType ChangeMidiInstrument( tMidiPlayerID id, tMidiTrackBitMask trackBitMask, tMidiInstr instr );
	VTABLE_EXPORT tErrType ChangeMidiTempo( tMidiPlayerID id, S8 tempo );

	VTABLE_EXPORT tErrType SendMidiCommand( tMidiPlayerID id, U8 cmd, U8 data1, U8 data2 );
	VTABLE_EXPORT tErrType 	MidiNoteOn( tMidiPlayerID id, U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );
	VTABLE_EXPORT tErrType 	MidiNoteOff( tMidiPlayerID id, U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );
	
private:
	CKernelMPI* 			pKernelMPI_;
	CDebugMPI* 				pDebugMPI_;	
	tMessageQueueHndl 		hRecvMsgQueue_;
	tMessageQueueHndl 		hSendMsgQueue_;
	const IEventListener*	pDefaultListener_;

	void 			SendCmdMessage( CAudioCmdMsg& msg );
	tAudioID 		WaitForAudioID( void );
	tMidiPlayerID 	WaitForMidiID( void );
	tErrType 		WaitForStatus( void );
	Boolean 		WaitForBooleanResult( void ); 
	U32 			WaitForU32Result( void ); 
	
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
