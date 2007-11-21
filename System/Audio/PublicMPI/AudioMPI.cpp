//==============================================================================
// Copyright (c) 2002-2007 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		AudioMgrMPI.cppn
//
// Description:
//		Implements the Module Public Interface (MPI) for the Audio Manager module.
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <StringTypes.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <Module.h>
#include <AudioTypes.h>
#include <AudioMPI.h>
#include <AudioPriv.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================
//==============================================================================
// MPI Name & Version
//
//		! NOTE: this may not match the MPI name & version implemented by the
//				installed module.  Always use the MPI's core functions to
//				resolve which version of the MPI is installed & available 
//				at runtime.
//==============================================================================
static const CString kMPIName = "AudioMPI";


//==============================================================================
// Global variables
//==============================================================================

//============================================================================
// CAudioEventMessage
//============================================================================
//------------------------------------------------------------------------------
CAudioEventMessage::CAudioEventMessage( const tAudioMsgAudioCompleted& data ) 
	: IEventMessage(kAudioCompletedEvent)
{
	audioMsgData.audioCompleted = data;
}

CAudioEventMessage::CAudioEventMessage( const tAudioMsgMidiCompleted& data ) 
	: IEventMessage(kMidiCompletedEvent)
{
	audioMsgData.midiCompleted = data;
}

CAudioEventMessage::CAudioEventMessage( const tAudioMsgCuePoint& data ) 
	: IEventMessage(kAudioCuePointEvent)
{
	audioMsgData.audioCuePoint = data;
}

//------------------------------------------------------------------------------
U16	CAudioEventMessage::GetSizeInBytes() const
{
	return sizeof(CAudioEventMessage);
}



//==============================================================================
// CAudioMPI implementation
//==============================================================================

CAudioMPI::CAudioMPI( const IEventListener* pListener ) : pModule_(NULL)
{
	tErrType err;
	
	ICoreModule*	pModule;
	err = Module::Connect(pModule, kAudioModuleName, 
									kAudioModuleVersion);
									
	if (kNoErr == err)
	{
		pModule_ = reinterpret_cast<CAudioModule*>(pModule);
		mpiID_ = pModule_->Register();
		if (pListener)
			pModule_->SetDefaultAudioEventListener( mpiID_, pListener );
	}
}

//----------------------------------------------------------------------------
CAudioMPI::~CAudioMPI()
{
	pModule_->Unregister( mpiID_ );
	Module::Disconnect( pModule_ );
}

//============================================================================
// Informational functions
//============================================================================
Boolean CAudioMPI::IsValid() const
{
	return (pModule_ != NULL) ? true : false;
}

//----------------------------------------------------------------------------
const CString* CAudioMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CAudioMPI::GetModuleVersion() const
{
	if(!pModule_)
		return kUndefinedVersion;
	return pModule_->GetModuleVersion();
}

//----------------------------------------------------------------------------
const CString* CAudioMPI::GetModuleName() const
{
	if (!pModule_)
		return &kNullString;
	return pModule_->GetModuleName();
}

//----------------------------------------------------------------------------
const CURI* CAudioMPI::GetModuleOrigin() const
{
	if (!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
}

// ==============================================================================
// PauseAudioSystem
// ==============================================================================
tErrType CAudioMPI::PauseAudioSystem() 
{
	if ( pModule_ )
		return pModule_->PauseAudioSystem();

	return kMPINotConnectedErr;
}

// ==============================================================================
// ResumeAudioSystem
// ==============================================================================
tErrType CAudioMPI::ResumeAudioSystem() 
{
	if ( pModule_ )
		return pModule_->ResumeAudioSystem();

	return kMPINotConnectedErr;
}

// ==============================================================================
// RegisterAudioEffectsProcessor
// ==============================================================================
tErrType CAudioMPI::RegisterAudioEffectsProcessor(  CAudioEffectsProcessor *pChain ) 
{
	if ( !pModule_ )
		return kNoImplErr;
	
	return pModule_->RegisterAudioEffectsProcessor( pChain );
}

// ==============================================================================
// RegisterGlobalAudioEffectsProcessor
// ==============================================================================
tErrType CAudioMPI::RegisterGlobalAudioEffectsProcessor( CAudioEffectsProcessor *pChain ) 
{
	if ( !pModule_ )
		return kNoImplErr;
	
	return pModule_->RegisterGlobalAudioEffectsProcessor( pChain );
}

// ==============================================================================
// ChangeAudioEffectsProcessor
// ==============================================================================
tErrType CAudioMPI::ChangeAudioEffectsProcessor( tAudioID id, CAudioEffectsProcessor *pChain ) 
{
	if ( !pModule_ )
		return kNoImplErr;
	
	return pModule_->ChangeAudioEffectsProcessor( id, pChain );
}

// ==============================================================================
// GetMasterVolume
// ==============================================================================
U8 CAudioMPI::GetMasterVolume() const
{
	if ( !pModule_ )
		return 0;
	
	return pModule_->GetMasterVolume();
}

// ==============================================================================
// SetMasterVolume
// ==============================================================================
void CAudioMPI::SetMasterVolume( U8 volume ) 
{
	if ( pModule_ )
		pModule_->SetMasterVolume( volume );
}

// ==============================================================================
// GetSpeakerEqualizer
// ==============================================================================
// Get/Set the speaker hardware equalizer.  The speaker equalizer may be
// set or cleared when a kHeadphoneJackDetect message is received
// This will not work on emulation, Lightning specific.  Need to hook into
// lf1000-audio driver via ioctl().
Boolean	CAudioMPI::GetSpeakerEqualizer(void) const
{
	if ( !pModule_  )
		return 0;
return (pModule_->GetOutputEqualizer());
}

// ==============================================================================
// SetSpeakerEqualizer
// ==============================================================================
void CAudioMPI::SetSpeakerEqualizer(Boolean enable)
{
//printf("CAudioMPI::SetOutputEqualizer: enable=%d\n", enable);
	if ( pModule_ )
		pModule_->SetOutputEqualizer( enable );
}

// ==============================================================================
// SetAudioResourcePath
// ==============================================================================
tErrType	CAudioMPI::SetAudioResourcePath( const CPath &path )
{
	if ( !pModule_ )
		return kNoImplErr;

	return pModule_->SetAudioResourcePath( mpiID_, path );
}

//==============================================================================
// SetAudioResourcePath
//==============================================================================
const CPath* 	CAudioMPI::GetAudioResourcePath( void ) const
{
	if ( !pModule_ )
		return kNull;
	
	return pModule_->GetAudioResourcePath( mpiID_ );
}

//==============================================================================
// StartAudio
//==============================================================================
tAudioID CAudioMPI::StartAudio( const CPath			&path, 
								U8					volume, 
								tAudioPriority		priority,
								S8					pan, 
								const IEventListener *pListener,
								tAudioPayload		payload,
								tAudioOptionsFlags	flags )
{
//printf("AudioMPI:StartAudio start \n");
	
	if ( !pModule_ )
		return kNoAudioID;
	
	// Try to play and will return valid ID if successful
	return pModule_->StartAudio( mpiID_, path, volume, priority, pan, pListener, payload, flags );
}

//==============================================================================
//==============================================================================
tAudioID CAudioMPI::StartAudio( const CPath 		&path, 
								tAudioPayload		payload,
								tAudioOptionsFlags	flags )
{
	if ( !pModule_ )
		return kNoAudioID;
	
	return pModule_->StartAudio( mpiID_, path, payload, flags );
}


//==============================================================================
//==============================================================================
U32 CAudioMPI::GetAudioTime( tAudioID id ) const
{
	if ( !pModule_ )
		return 0;
	
	return pModule_->GetAudioTime( id );

}

//==============================================================================
//==============================================================================
void CAudioMPI::StopAudio( tAudioID id, Boolean suppressDoneMessage ) 
{
	if ( pModule_ )
		pModule_->StopAudio( id, suppressDoneMessage );
}


//==============================================================================
//==============================================================================
void CAudioMPI::PauseAudio( tAudioID id ) 
{
	if ( pModule_ )
		pModule_->PauseAudio( id );
}

//==============================================================================
//==============================================================================
void CAudioMPI::ResumeAudio( tAudioID id ) 
{
	if ( pModule_ )
		pModule_->ResumeAudio( id );
}


//==============================================================================
//==============================================================================
U8 CAudioMPI::GetAudioVolume( tAudioID id ) const
{
	if ( !pModule_ )
		return 0;
	
	return pModule_->GetAudioVolume( id );
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetAudioVolume( tAudioID id, U8 volume ) 
{
	if ( pModule_ )
		pModule_->SetAudioVolume( id, volume );
}

//==============================================================================
//==============================================================================
tAudioPriority CAudioMPI::GetAudioPriority( tAudioID id ) const
{
	if ( !pModule_ )
		return 0;
	
	return pModule_->GetAudioPriority( id );
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetAudioPriority( tAudioID id, tAudioPriority priority ) 
{
	if ( pModule_ )
		pModule_->SetAudioPriority( id, priority );
}

//==============================================================================
//==============================================================================
S8 CAudioMPI::GetAudioPan( tAudioID id ) const
{
	if ( !pModule_ )
		return 0;
	
	return pModule_->GetAudioPan( id );
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetAudioPan( tAudioID id, S8 pan ) 
{
	if ( pModule_ )
		pModule_->SetAudioPan( id, pan );
}

//==============================================================================
//==============================================================================
const IEventListener* CAudioMPI::GetAudioEventListener( tAudioID id ) const
{
	if ( !pModule_ )
		return NULL;
	
	return pModule_->GetAudioEventListener( id );
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetAudioEventListener( tAudioID id, IEventListener *pListener ) 
{
	if ( pModule_ )
		pModule_->SetAudioEventListener( id, pListener );
}

//==============================================================================
//==============================================================================
U8 CAudioMPI::GetDefaultAudioVolume( void ) const
{
	if ( !pModule_ )
		return 0;
	
	return pModule_->GetDefaultAudioVolume( mpiID_ );
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetDefaultAudioVolume( U8 volume ) 
{
	if ( pModule_ )
		pModule_->SetDefaultAudioVolume( mpiID_, volume );
}

//==============================================================================
//==============================================================================
tAudioPriority CAudioMPI::GetDefaultAudioPriority( void ) const
{
	if ( !pModule_ )
		return 0;
	
	return pModule_->GetDefaultAudioPriority( mpiID_ );
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetDefaultAudioPriority( tAudioPriority priority ) 
{
	if ( pModule_ )
		pModule_->SetDefaultAudioPriority( mpiID_, priority );
}

//==============================================================================
//==============================================================================
S8 CAudioMPI::GetDefaultAudioPan( void ) const
{
	if ( !pModule_ )
		return 0;
	
	return pModule_->GetDefaultAudioPan( mpiID_ );
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetDefaultAudioPan( S8 pan ) 
{
	if ( pModule_)
		pModule_->SetDefaultAudioPan( mpiID_, pan );
}

//==============================================================================
//==============================================================================
const IEventListener* CAudioMPI::GetDefaultAudioEventListener( void ) const
{
	if ( !pModule_ )
		return NULL;
	
	return pModule_->GetDefaultAudioEventListener( mpiID_ );
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetDefaultAudioEventListener( IEventListener *pListener ) 
{
	if ( pModule_ )
		pModule_->SetDefaultAudioEventListener( mpiID_, pListener );
}

//==============================================================================
//==============================================================================
Boolean CAudioMPI::IsAudioPlaying( tAudioID id ) 
{
	if ( !pModule_ )
		return false;
	
	return pModule_->IsAudioPlaying( id );
}

//==============================================================================
//==============================================================================
Boolean CAudioMPI::IsAudioPlaying( void ) 
{
	if ( !pModule_ )
		return false;
	
	return pModule_->IsAudioPlaying();
}

// ==============================================================================
// ==============================================================================
tErrType CAudioMPI::AcquireMidiPlayer( tAudioPriority priority, IEventListener *pListener, tMidiPlayerID *midiPlayerID ) 
{
	if ( !pModule_ )
		return kNoImplErr;
	
	return pModule_->AcquireMidiPlayer( priority, pListener, midiPlayerID );
}

// ==============================================================================
// ==============================================================================
tErrType CAudioMPI::ReleaseMidiPlayer( tMidiPlayerID midiPlayerID ) 
{
	if ( !pModule_ )
		return kNoImplErr;
	
	return pModule_->ReleaseMidiPlayer( midiPlayerID );
}

// ==============================================================================
// StartMidiFile
// ==============================================================================
tErrType CAudioMPI::StartMidiFile( 	tMidiPlayerID		id,
									const CPath 		&path, 
									U8					volume, 
									tAudioPriority		priority,
									IEventListener*		pListener,
									tAudioPayload		payload,
									tAudioOptionsFlags	flags )
{
	if ( !pModule_ )
		return kNoImplErr;

	return pModule_->StartMidiFile( mpiID_, id, path, volume, priority, pListener, payload, flags );
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::StartMidiFile( tMidiPlayerID		id,
									const CPath 		&path, 
									tAudioPayload		payload,
									tAudioOptionsFlags	flags )
{
	if ( !pModule_ )
		return kNoImplErr;

	return pModule_->StartMidiFile( mpiID_, id, path, payload, flags );
}

//==============================================================================
//==============================================================================
Boolean CAudioMPI::IsMidiFilePlaying( tMidiPlayerID id ) 
{
	if ( !pModule_ )
		return false;
	
	return pModule_->IsMidiFilePlaying( id );
}

//==============================================================================
//==============================================================================
Boolean CAudioMPI::IsMidiFilePlaying() 
{
	if ( !pModule_ )
		return false;
	
	return pModule_->IsMidiFilePlaying();
}

//==============================================================================
//==============================================================================
void CAudioMPI::PauseMidiFile( tMidiPlayerID id ) 
{
	if ( pModule_ )
		pModule_->PauseMidiFile( id );
}

// ==============================================================================
// ResumeMidiFile
// ==============================================================================
void CAudioMPI::ResumeMidiFile( tMidiPlayerID id ) 
{
	if ( pModule_ )
		pModule_->ResumeMidiFile( id );
}

// ==============================================================================
// StopMidiFile
// ==============================================================================
void CAudioMPI::StopMidiFile( tMidiPlayerID id, Boolean suppressDoneMessage ) 
{
	if ( pModule_ )
		pModule_->StopMidiFile( id, suppressDoneMessage );
}

// ==============================================================================
// GetEnabledMidiTracks
// ==============================================================================
tMidiTrackBitMask CAudioMPI::GetEnabledMidiTracks( tMidiPlayerID id ) 
{
	if ( !pModule_ )
		return 0;
	
	return pModule_->GetEnabledMidiTracks( id );
}

// ==============================================================================
// SetEnableMidiTracks
// ==============================================================================
tErrType CAudioMPI::SetEnableMidiTracks( tMidiPlayerID id, tMidiTrackBitMask trackBitMask ) 
{
	if ( !pModule_ )
		return kNoImplErr;
	
	return pModule_->SetEnableMidiTracks( id, trackBitMask );
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::TransposeMidiTracks( tMidiPlayerID id, tMidiTrackBitMask trackBitMask, S8 transposeAmount ) 
{
	if ( !pModule_ )
		return kNoImplErr;
	
	return pModule_->TransposeMidiTracks( id, trackBitMask, transposeAmount );
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::ChangeMidiInstrument( tMidiPlayerID id, tMidiTrackBitMask channel, tMidiPlayerInstrument programNumber ) 
{
	if ( !pModule_)
		return kNoImplErr;
	
	return pModule_->ChangeMidiInstrument( id, channel, programNumber );
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::ChangeMidiTempo( tMidiPlayerID id, S8 tempo ) 
{
	if ( !pModule_ )
		return kNoImplErr;
	
	return pModule_->ChangeMidiTempo( id, tempo );
}

// ==============================================================================
// SendMidiCommand
// ==============================================================================
tErrType CAudioMPI::SendMidiCommand( tMidiPlayerID id, U8 cmd, U8 data1, U8 data2 ) 
{
	if ( !pModule_ )
		return kNoImplErr;
	
	return pModule_->SendMidiCommand( id, cmd, data1, data2 );
}

// ==============================================================================
// MidiNoteOn
// ==============================================================================
tErrType CAudioMPI::MidiNoteOn( tMidiPlayerID	id,
									U8 			channel, 
									U8			noteNum, 
									U8			velocity, 
									tAudioOptionsFlags	flags )
{
	if ( !pModule_ )
		return kNoImplErr;
	
	return pModule_->MidiNoteOn( id, channel, noteNum, velocity, flags );
}

// ==============================================================================
// MidiNoteOn
// ==============================================================================
tErrType CAudioMPI::MidiNoteOn( tMidiPlayerID	id,
									U8 			channel, 
									U8			noteNum, 
									U8			velocity )
{
	if ( !pModule_ )
		return kNoImplErr;
	
	return pModule_->MidiNoteOn( id, channel, noteNum, velocity, 0 );
}

// ==============================================================================
// MidiNoteOff
// ==============================================================================
tErrType CAudioMPI::MidiNoteOff( tMidiPlayerID	id,
									U8 			channel, 
									U8			noteNum, 
									U8			velocity, 
									tAudioOptionsFlags	flags )
{
	if ( !pModule_ )
		return kNoImplErr;
	
	return pModule_->MidiNoteOff( id, channel, noteNum, velocity, flags );
}

// ==============================================================================
// MidiNoteOff
// ==============================================================================
tErrType CAudioMPI::MidiNoteOff( tMidiPlayerID	id,
									U8 			channel, 
									U8			noteNum)
{
	if ( !pModule_ )
		return kNoImplErr;
	
	return pModule_->MidiNoteOff( id, channel, noteNum, 0, 0 );
}

LF_END_BRIO_NAMESPACE()	
// EOF	
