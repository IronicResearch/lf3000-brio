#ifndef LF_BRIO_AUDIOPRIVATE_H
#define LF_BRIO_AUDIOPRIVATE_H
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// AudioPriv.h
//
//		Defines private, hidden data structures used by AudioMPI 
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <StringTypes.h>
#include <CoreModule.h>

#include <KernelMPI.h>
#include <DebugMPI.h>

#include <AudioConfig.h>
#include <AudioTypesPriv.h>
#include <AudioEffectsProcessor.h>
#include <EventListener.h>

LF_BEGIN_BRIO_NAMESPACE()

// Backdoor GAS and SAS functions need this for passing around the tAudioState
// struct.  This code will be eliminated when those backdoor functions are gone.
const U32	kAUDIO_MAX_MSG_SIZE	=	512;

class IEventListener;

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

	// MPI state storage
	VTABLE_EXPORT	U32		Register( void );
	VTABLE_EXPORT	void	Unregister( U32 id) ;

	// Overall Audio Control
	tErrType SetDefaultListener( const IEventListener* pListener );

	VTABLE_EXPORT tErrType	PauseAudioSystem(  void );
	VTABLE_EXPORT tErrType	ResumeAudioSystem( void );

	VTABLE_EXPORT void		SetMasterVolume( U8 volume );
	VTABLE_EXPORT U8		GetMasterVolume( void );
	VTABLE_EXPORT void		SetOutputEqualizer( U8 enable );
	VTABLE_EXPORT U8		GetOutputEqualizer( void );

	// Specific to MPIs
	VTABLE_EXPORT tErrType SetAudioResourcePath( U32 mpiID,
												 const CPath &path );
	VTABLE_EXPORT CPath* GetAudioResourcePath( U32 mpiID );
	
	VTABLE_EXPORT tAudioID StartAudio( U32 mpiID,
									   const CPath &path, 
									   U8 volume, 
									   tAudioPriority priority,
									   S8 pan, 
									   const IEventListener *pListener,
									   tAudioPayload payload,
									   tAudioOptionsFlags flags );

	VTABLE_EXPORT tAudioID StartAudio( U32 mpiID,
									   const CPath	&path, 
									   tAudioPayload payload,
									   tAudioOptionsFlags flags );
	
	VTABLE_EXPORT void PauseAudio( tAudioID id );
	VTABLE_EXPORT void ResumeAudio( tAudioID id ); 
	VTABLE_EXPORT void StopAudio( tAudioID id, Boolean noDoneMessage ); 

	VTABLE_EXPORT Boolean IsAudioPlaying( tAudioID id );
	VTABLE_EXPORT Boolean IsAudioPlaying( void );

	VTABLE_EXPORT U32 GetAudioTime( tAudioID id );

	VTABLE_EXPORT U8 GetAudioVolume( tAudioID id );
	VTABLE_EXPORT void SetAudioVolume( tAudioID id, U8 volume );

	// LF INTERNAL functions to get internal audio state (now Mixer)
	VTABLE_EXPORT U8 GetAudioState( tAudioState *d );
	VTABLE_EXPORT void SetAudioState( tAudioState *d );

	VTABLE_EXPORT tAudioPriority GetAudioPriority( tAudioID id);
	VTABLE_EXPORT void SetAudioPriority( tAudioID id,
												   tAudioPriority priority);

	VTABLE_EXPORT S8 GetAudioPan( tAudioID id ); 
	VTABLE_EXPORT void SetAudioPan( tAudioID id, S8 pan ); 

	VTABLE_EXPORT const IEventListener *GetAudioEventListener( tAudioID id );
	VTABLE_EXPORT void SetAudioEventListener( tAudioID id,
											  const IEventListener *pListener );

	//********************************
	// Defaults to use when value is not specified in the Start() call.
	//********************************	  
	VTABLE_EXPORT U8 GetDefaultAudioVolume( U32 mpiID );
	VTABLE_EXPORT void SetDefaultAudioVolume( U32 mpiID, U8 volume );

	VTABLE_EXPORT tAudioPriority GetDefaultAudioPriority( U32 mpiID );
	VTABLE_EXPORT void SetDefaultAudioPriority( U32 mpiID,
												tAudioPriority priority );

	VTABLE_EXPORT S8 GetDefaultAudioPan( U32 mpiID );
	VTABLE_EXPORT void SetDefaultAudioPan( U32 mpiID, S8 pan );

	VTABLE_EXPORT const IEventListener *GetDefaultAudioEventListener( U32 mpiID );
	VTABLE_EXPORT void SetDefaultAudioEventListener( U32 mpiID,
													 const IEventListener *pListener );
	VTABLE_EXPORT tErrType RegisterAudioEffectsProcessor( CAudioEffectsProcessor *pChain );
	VTABLE_EXPORT tErrType RegisterGlobalAudioEffectsProcessor( CAudioEffectsProcessor *pChain );
	VTABLE_EXPORT tErrType	ChangeAudioEffectsProcessor( tAudioID id,
														 CAudioEffectsProcessor *pChain );

	tErrType RegisterGetStereoAudioStreamFcn( tGetStereoAudioStreamFcn pFcn );

	VTABLE_EXPORT tErrType AcquireMidiPlayer( tAudioPriority priority,
											  IEventListener *pHandler,
											  tMidiPlayerID *id );
	VTABLE_EXPORT tErrType ReleaseMidiPlayer( tMidiPlayerID id );
	
	VTABLE_EXPORT tAudioID GetAudioIDForMidiID( tMidiPlayerID id );

	VTABLE_EXPORT tErrType StartMidiFile( U32 mpiID,
										  tMidiPlayerID id,
										  const CPath &path, 
										  U8 volume, 
										  tAudioPriority priority,
										  IEventListener* pListener,
										  tAudioPayload payload,
										  tAudioOptionsFlags flags );

	VTABLE_EXPORT tErrType StartMidiFile( U32 mpiID, 
										  tMidiPlayerID id,
										  const CPath &path, 
										  tAudioPayload payload,
										  tAudioOptionsFlags flags );

	VTABLE_EXPORT void PauseMidiFile( tMidiPlayerID id );
	VTABLE_EXPORT void ResumeMidiFile( tMidiPlayerID id );
	VTABLE_EXPORT void StopMidiFile( tMidiPlayerID id, Boolean noDoneMessage );

	VTABLE_EXPORT Boolean IsMidiFilePlaying( tMidiPlayerID id );
	VTABLE_EXPORT Boolean IsMidiFilePlaying( void );

	VTABLE_EXPORT tMidiTrackBitMask GetEnabledMidiTracks( tMidiPlayerID id );

	VTABLE_EXPORT tErrType SetEnableMidiTracks( tMidiPlayerID id,
												tMidiTrackBitMask mask );

	VTABLE_EXPORT tErrType TransposeMidiTracks( tMidiPlayerID id,
												tMidiTrackBitMask mask,
												S8 amount );

	VTABLE_EXPORT tErrType ChangeMidiInstrument( tMidiPlayerID id,
												 tMidiTrackBitMask mask,
												 tMidiPlayerInstrument instr );

	VTABLE_EXPORT tErrType ChangeMidiTempo( tMidiPlayerID id, S8 tempo );

	VTABLE_EXPORT tErrType MidiNoteOn( tMidiPlayerID id,
									   U8 channel,
									   U8 note,
									   U8 velocity,
									   tAudioOptionsFlags flags );

	VTABLE_EXPORT tErrType MidiNoteOff( tMidiPlayerID id,
										U8 channel,
										U8 note,
										U8 velocity,
										tAudioOptionsFlags flags );

	VTABLE_EXPORT tErrType SendMidiCommand( tMidiPlayerID id,
											U8 cmd,
											U8 data1,
											U8 data2 );
	
 private:
	CKernelMPI*				pKernelMPI_;
	CDebugMPI*				pDebugMPI_;	
	tMutex					mpiMutex_;

	// Limit object creation to the Module Manager interface functions
	CAudioModule();
	virtual ~CAudioModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
		::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));
};

LF_END_BRIO_NAMESPACE()	
#endif		// LF_BRIO_AUDIOPRIVATE_H
