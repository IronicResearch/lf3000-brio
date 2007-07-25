//==============================================================================
// Copyright (c) 2002-2007 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		AudioMgrMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Audio Manager module.
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <SystemTypes.h>
#include <ResourceTypes.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <Module.h>
#include <AudioTypes.h>
//#include <RsrcMgrMPI.h>
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
CAudioEventMessage::CAudioEventMessage( const tAudioMsgDataCompleted& data ) 
	: IEventMessage(kAudioCompletedEvent, 0)
{
	audioMsgData.audioCompleted = data;
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
//		pModule_->SetDefaultListener(pListener);
	}

	pModule_->StartAudioSystem();
}

//----------------------------------------------------------------------------
CAudioMPI::~CAudioMPI()
{
	pModule_->StopAudioSystem();

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
	if(!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::PauseAudioSystem() 
{
	if ( pModule_ != kNull )
	{
		return pModule_->PauseAudioSystem();
	}

	return kMPINotConnectedErr;
}

//----------------------------------------------------------------------------
tErrType CAudioMPI::ResumeAudioSystem() 
{
	if ( pModule_ != kNull )
	{
		return pModule_->ResumeAudioSystem();
	}

	return kMPINotConnectedErr;
}

/*
//==============================================================================
//==============================================================================
tErrType CAudioMPI::RegisterGetStereoAudioStreamFcn(tRsrcType type, tGetStereoAudioStreamFcn pFcn) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->pRegisterGetStereoAudioStreamFcn(type, pFcn);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::RegisterAudioEffectsProcessor(tRsrcType type, CAudioEffectsProcessor *pChain) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->pRegisterAudioEffectsProcessor(type, pChain);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::RegisterGlobalAudioEffectsProcessor(CAudioEffectsProcessor *pChain) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->pRegisterGlobalAudioEffectsProcessor(pChain);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::ChangeAudioEffectsProcessor(tAudioID id, CAudioEffectsProcessor *pChain) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->pChangeAudioEffectsProcessor(id, pChain);
}
*/
//==============================================================================
//==============================================================================
void CAudioMPI::SetMasterVolume( U8 volume ) 
{
	if ( pModule_ != kNull )
	{
		pModule_->SetMasterVolume( volume );
	}
}

//==============================================================================
//==============================================================================
U8 CAudioMPI::GetMasterVolume() 
{
	if ( pModule_ == kNull )
	{
		return 0;
	}
	return pModule_->GetMasterVolume();
}

//==============================================================================
//==============================================================================
tAudioID CAudioMPI::StartAudio( tRsrcHndl			hRsrc, 
								U8					volume, 
								tAudioPriority		priority,
								S8					pan, 
								IEventListener		*pHandler,
								tAudioPayload		payload,
								tAudioOptionsFlags	flags )
{
//	printf("AudioMPI:StartAudio; hRsrc = 0x%x\n", hRsrc);
	
	if ( pModule_ == kNull )
	{
		return kNoAudioID;
	}
	
	// Try to play the resource and return a valid ID if successful.
	return pModule_->StartAudio( hRsrc, volume, priority, pan, pHandler, payload, flags );
}
/*
//==============================================================================
//==============================================================================
tAudioID CAudioMPI::StartAudio(tRsrcHndl			hRsrc, 
								tAudioPayload		payload,
								tAudioOptionsFlags	flags)
{
	if ( pModule_ == kNull )
	{
		return kNoAudioID;
	}
	return pModule_->pStartAudioDefault(pModule_, hRsrc, payload, flags);
}
*/

//==============================================================================
//==============================================================================
U32 CAudioMPI::GetAudioTime( tAudioID id ) 
{
	if ( pModule_ != kNull )
	{
		return pModule_->GetAudioTime( id );
	}
	
	return 0;
}

//==============================================================================
//==============================================================================
void CAudioMPI::StopAudio( tAudioID id, Boolean surpressDoneMessage ) 
{
	if ( pModule_ != kNull )
	{
		pModule_->StopAudio( id, surpressDoneMessage );
	}
}


//==============================================================================
//==============================================================================
void CAudioMPI::PauseAudio( tAudioID id ) 
{
	if ( pModule_ != kNull )
	{
		pModule_->PauseAudio( id );
	}
}

//==============================================================================
//==============================================================================
void CAudioMPI::ResumeAudio( tAudioID id ) 
{
	if ( pModule_ != kNull )
	{
		pModule_->ResumeAudio( id );
	}
}

/*
//==============================================================================
//==============================================================================
U8 CAudioMPI::GetAudioVolume(tAudioID id) 
{
	if ( pModule_ == kNull )
	{
		return 0;
	}
	return pModule_->pGetAudioVolume(id);
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetAudioVolume(tAudioID id, U8 volume) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pSetAudioVolume(id, volume);
	}
}

//==============================================================================
//==============================================================================
tAudioPriority CAudioMPI::GetAudioPriority(tAudioID id) 
{
	if ( pModule_ == kNull )
	{
		return 0;
	}
	return pModule_->pGetAudioPriority(id);
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetAudioPriority(tAudioID id, tAudioPriority priority) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pSetAudioPriority(id, priority);
	}
}

//==============================================================================
//==============================================================================
S8 CAudioMPI::GetAudioPan(tAudioID id) 
{
	if ( pModule_ == kNull )
	{
		return 0;
	}
	return pModule_->pGetAudioPan(id);
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetAudioPan(tAudioID id, S8 pan) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pSetAudioPan(id, pan);
	}
}

//==============================================================================
//==============================================================================
IEventListener* CAudioMPI::GetAudioEventHandler(tAudioID id) 
{
	if ( pModule_ == kNull )
	{
		return NULL;
	}
	return pModule_->pGetAudioEventHandler(id);
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetAudioEventHandler(tAudioID id, IEventListener *pHandler) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pSetAudioEventHandler(id, pHandler);
	}
}

//==============================================================================
//==============================================================================
U8 CAudioMPI::GetDefaultAudioVolume() 
{
	if ( pModule_ == kNull )
	{
		return 0;
	}
	return pModule_->pGetDefaultAudioVolume();
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetDefaultAudioVolume(U8 volume) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pSetDefaultAudioVolume(volume);
	}
}

//==============================================================================
//==============================================================================
tAudioPriority CAudioMPI::GetDefaultAudioPriority() 
{
	if ( pModule_ == kNull )
	{
		return 0;
	}
	return pModule_->pGetDefaultAudioPriority();
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetDefaultAudioPriority(tAudioPriority priority) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pSetDefaultAudioPriority(priority);
	}
}

//==============================================================================
//==============================================================================
S8 CAudioMPI::GetDefaultAudioPan() 
{
	if ( pModule_ == kNull )
	{
		return 0;
	}
	return pModule_->pGetDefaultAudioPan();
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetDefaultAudioPan(S8 pan) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pSetDefaultAudioPan(pan);
	}
}

//==============================================================================
//==============================================================================
IEventListener* CAudioMPI::GetDefaultAudioEventHandler() 
{
	if ( pModule_ == kNull )
	{
		return NULL;
	}
	return pModule_->pGetDefaultAudioEventHandler();
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetDefaultAudioEventHandler(IEventListener *pHandler) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pSetDefaultAudioEventHandler(pHandler);
	}
}
*/
//==============================================================================
//==============================================================================
Boolean CAudioMPI::IsAudioPlaying( tAudioID id ) 
{
	if ( pModule_ == kNull )
	{
		return false;
	}
	return pModule_->IsAudioPlaying( id );
}

//==============================================================================
//==============================================================================
Boolean CAudioMPI::IsAudioPlaying() 
{
	if ( pModule_ == kNull )
	{
		return false;
	}
	return pModule_->IsAudioPlaying();
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::AcquireMidiPlayer( tAudioPriority priority, IEventListener *pHandler, tMidiPlayerID *midiPlayerID ) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->AcquireMidiPlayer( priority, pHandler, midiPlayerID );
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::ReleaseMidiPlayer( tMidiPlayerID midiPlayerID ) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->ReleaseMidiPlayer( midiPlayerID );
}

tErrType CAudioMPI::StartMidiFile( tMidiPlayerID	id,
					tRsrcHndl			hRsrc, 
					U8					volume, 
					tAudioPriority		priority,
					IEventListener*		pHandler,
					tAudioPayload		payload,
					tAudioOptionsFlags	flags )
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}

	return pModule_->StartMidiFile( id, hRsrc, volume, priority, pHandler, payload, flags );
}

//==============================================================================
//==============================================================================
Boolean CAudioMPI::IsMidiFilePlaying( tMidiPlayerID id ) 
{
	if ( pModule_ == kNull )
	{
		return false;
	}
	return pModule_->IsMidiFilePlaying( id );
}

//==============================================================================
//==============================================================================
Boolean CAudioMPI::IsMidiFilePlaying() 
{
	if ( pModule_ == kNull )
	{
		return false;
	}
	return pModule_->IsMidiFilePlaying();
}

//==============================================================================
//==============================================================================
void CAudioMPI::PauseMidiFile( tMidiPlayerID id ) 
{
	if ( pModule_ != kNull )
	{
		pModule_->PauseMidiFile( id );
	}
}

//==============================================================================
//==============================================================================
void CAudioMPI::ResumeMidiFile( tMidiPlayerID id ) 
{
	if ( pModule_ != kNull )
	{
		pModule_->ResumeMidiFile( id );
	}
}

//==============================================================================
//==============================================================================
void CAudioMPI::StopMidiFile( tMidiPlayerID id, Boolean surpressDoneMessage ) 
{
	if ( pModule_ != kNull )
	{
		pModule_->StopMidiFile( id, surpressDoneMessage );
	}
}

//==============================================================================
//==============================================================================
tMidiTrackBitMask CAudioMPI::GetEnabledMidiTracks(tMidiPlayerID id) 
{
	if ( pModule_ == kNull )
	{
		return 0;
	}
	return pModule_->GetEnabledMidiTracks(id);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::EnableMidiTracks(tMidiPlayerID id, tMidiTrackBitMask trackBitMask) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->EnableMidiTracks(id, trackBitMask);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::TransposeMidiTracks(tMidiPlayerID id, tMidiTrackBitMask trackBitMask, S8 transposeAmount) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->TransposeMidiTracks(id, trackBitMask, transposeAmount);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::ChangeMidiInstrument(tMidiPlayerID id, tMidiTrackBitMask trackBitMask, tMidiInstr instr) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->ChangeMidiInstrument(id, trackBitMask, instr);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::ChangeMidiTempo(tMidiPlayerID id, S8 tempo) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->ChangeMidiTempo(id, tempo);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::SendMidiCommand(tMidiPlayerID id, U8 cmd, U8 data1, U8 data2) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->SendMidiCommand(id, cmd, data1, data2);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::MidiNoteOn( tMidiPlayerID	id,
									U8 			channel, 
									U8			noteNum, 
									U8			velocity, 
									tAudioOptionsFlags	flags)
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->MidiNoteOn(id, channel, noteNum, velocity, flags);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::MidiNoteOff( tMidiPlayerID	id,
									U8 			channel, 
									U8			noteNum, 
									U8			velocity, 
									tAudioOptionsFlags	flags)
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->MidiNoteOff( id, channel, noteNum, velocity, flags );
}

LF_END_BRIO_NAMESPACE()	
// EOF	
