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
#include <AudioEffectsProcessor.h>
#include <EventListener.h>

//#include <RsrcTypes.h>
//#include <AudioTypes.h>
//#include <AudioRsrcs.h>
//#include <AudioMixer.h>
LF_BEGIN_BRIO_NAMESPACE()


class IEventListener;

// Constants
const CString	kAudioModuleName	= "Audio";
const tVersion	kAudioModuleVersion	= 2;

/// Audio internal resource types

enum tAudioTypeInt {
	kAudioRsrcUndefined = 0,
	kAudioRsrcMIDI = 1,
	kAudioRsrcOggVorbis = 2,
	kAudioRsrcRaw = 3,
	kAudioRsrcOggTheora = 4
};


//==============================================================================
class CAudioModule : public ICoreModule {
public:	
	// core functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// MPI state storage
	VTABLE_EXPORT	U32		Register( void );
	VTABLE_EXPORT	void	Unregister( U32 id) ;

	// Overall Audio Control
	tErrType SetDefaultListener( const IEventListener* pListener );

	VTABLE_EXPORT tErrType	PauseAudioSystem( void );
	VTABLE_EXPORT tErrType	ResumeAudioSystem( void );

	VTABLE_EXPORT void 		SetMasterVolume( U8 volume );
	VTABLE_EXPORT U8		GetMasterVolume( void );

	// Specific to MPIs
	VTABLE_EXPORT tErrType SetAudioResourcePath( U32 mpiID, const CPath &path );
	VTABLE_EXPORT const CPath* GetAudioResourcePath( U32 mpiID );
	
	VTABLE_EXPORT tAudioID StartAudio( U32					mpiID,
										const CPath 		&path, 
										U8					volume, 
										tAudioPriority		priority,
										S8					pan, 
										const IEventListener *pListener,
										tAudioPayload		payload,
										tAudioOptionsFlags	flags );

	VTABLE_EXPORT tAudioID 	StartAudio( U32					mpiID,
										const CPath 		&path, 
										tAudioPayload		payload,
										tAudioOptionsFlags	flags );

	VTABLE_EXPORT void		PauseAudio( tAudioID id );
	VTABLE_EXPORT void 		ResumeAudio( tAudioID id ); 
	VTABLE_EXPORT void 		StopAudio( tAudioID id, Boolean surpressDoneMessage ); 

	VTABLE_EXPORT Boolean	IsAudioPlaying( tAudioID id );
	VTABLE_EXPORT Boolean	IsAudioPlaying( void );

	VTABLE_EXPORT U32 		GetAudioTime( tAudioID id );

	VTABLE_EXPORT U8		GetAudioVolume( tAudioID id ); // TODO: stub
	VTABLE_EXPORT void		SetAudioVolume( tAudioID id, U8 volume ); // TODO: stub

	VTABLE_EXPORT tAudioPriority	GetAudioPriority( tAudioID id); // TODO: stub
	VTABLE_EXPORT void		SetAudioPriority( tAudioID id, tAudioPriority priority); // TODO: stub

	VTABLE_EXPORT S8		GetAudioPan( tAudioID id ); // TODO: stub
	VTABLE_EXPORT void		SetAudioPan( tAudioID id, S8 pan ); // TODO: stub

	VTABLE_EXPORT const IEventListener*	GetAudioEventListener( tAudioID id ); // TODO: stub
	VTABLE_EXPORT void		SetAudioEventListener( tAudioID id, const IEventListener *pListener ); // TODO: stub

	//********************************
	// Defaults to use when value is not specified in the Start() call.
	//********************************    
	VTABLE_EXPORT U8		GetDefaultAudioVolume( U32 mpiID ); // TODO: stub
	VTABLE_EXPORT void		SetDefaultAudioVolume(  U32 mpiID, U8 volume ); // TODO: stub

	VTABLE_EXPORT tAudioPriority	GetDefaultAudioPriority( U32 mpiID ); // TODO: stub
	VTABLE_EXPORT void		SetDefaultAudioPriority( U32 mpiID, tAudioPriority priority ); // TODO: stub

	VTABLE_EXPORT S8		GetDefaultAudioPan( U32 mpiID ); // TODO: stub
	VTABLE_EXPORT void		SetDefaultAudioPan( U32 mpiID, S8 pan ); // TODO: stub

	VTABLE_EXPORT const IEventListener*	GetDefaultAudioEventListener( U32 mpiID );  // TODO: stub
	VTABLE_EXPORT void		SetDefaultAudioEventListener( U32 mpiID, const IEventListener *pListener );  // TODO: stub

	VTABLE_EXPORT tErrType 	RegisterAudioEffectsProcessor( /* tRsrcType type, */ CAudioEffectsProcessor *pChain ); // TODO: stub
	VTABLE_EXPORT tErrType 	RegisterGlobalAudioEffectsProcessor( CAudioEffectsProcessor *pChain ); // TODO: stub
	VTABLE_EXPORT tErrType 	ChangeAudioEffectsProcessor( tAudioID id, CAudioEffectsProcessor *pChain );  // TODO: stub

	tErrType RegisterGetStereoAudioStreamFcn( /* tRsrcType type, */ tGetStereoAudioStreamFcn pFcn ); // TODO: stub

	VTABLE_EXPORT tErrType	AcquireMidiPlayer( tAudioPriority priority, IEventListener *pHandler, tMidiPlayerID *id );
	VTABLE_EXPORT tErrType	ReleaseMidiPlayer( tMidiPlayerID id );

	VTABLE_EXPORT tAudioID	GetAudioIDForMidiID( tMidiPlayerID id ); // TODO: stub

	VTABLE_EXPORT tErrType StartMidiFile( U32				mpiID,
										tMidiPlayerID		id,
										const CPath 		&path, 
										U8					volume, 
										tAudioPriority		priority,
										IEventListener*		pListener,
										tAudioPayload		payload,
										tAudioOptionsFlags	flags );
	VTABLE_EXPORT tErrType StartMidiFile( 	U32 				mpiID, 
											tMidiPlayerID		id,
											const CPath 		&path, 
											tAudioPayload		payload,
											tAudioOptionsFlags	flags );
	VTABLE_EXPORT void PauseMidiFile( tMidiPlayerID id );
	VTABLE_EXPORT void ResumeMidiFile( tMidiPlayerID id );
	VTABLE_EXPORT void StopMidiFile( tMidiPlayerID id, Boolean surpressDoneMessage );

	VTABLE_EXPORT Boolean IsMidiFilePlaying( tMidiPlayerID id ); // TODO: stub
	VTABLE_EXPORT Boolean IsMidiFilePlaying( void ); // TODO: stub

	VTABLE_EXPORT tMidiTrackBitMask GetEnabledMidiTracks( tMidiPlayerID id );
	VTABLE_EXPORT tErrType SetEnableMidiTracks( tMidiPlayerID id, tMidiTrackBitMask trackBitMask );
	VTABLE_EXPORT tErrType TransposeMidiTracks( tMidiPlayerID id, tMidiTrackBitMask tracktBitMask, S8 transposeAmount );
	VTABLE_EXPORT tErrType ChangeMidiInstrument( tMidiPlayerID id, tMidiTrackBitMask trackBitMask, tMidiInstr instr );
	VTABLE_EXPORT tErrType ChangeMidiTempo( tMidiPlayerID id, S8 tempo );

	VTABLE_EXPORT tErrType MidiNoteOn( tMidiPlayerID id, U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );
	VTABLE_EXPORT tErrType MidiNoteOff( tMidiPlayerID id, U8 channel, U8 noteNum, U8 velocity, tAudioOptionsFlags flags );
	VTABLE_EXPORT tErrType SendMidiCommand( tMidiPlayerID id, U8 cmd, U8 data1, U8 data2 );
	
private:
	CKernelMPI* 			pKernelMPI_;
	CDebugMPI* 				pDebugMPI_;	
	tMessageQueueHndl 		hRecvMsgQueue_;
	tMessageQueueHndl 		hSendMsgQueue_;
	U8						masterVolume_;
	tMutex     				mpiMutex_;

	void 			SendCmdMessage( CAudioCmdMsg& msg );
	tAudioID 		WaitForAudioID( void );
	tMidiPlayerID 	WaitForMidiID( void );
	tErrType 		WaitForStatus( void );
	Boolean 		WaitForBooleanResult( void ); 
	U32 			WaitForU32Result( void ); 
	
	// No public access to start/stop.
	tErrType	StartAudioSystem( void );
	tErrType	StopAudioSystem( void );

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
