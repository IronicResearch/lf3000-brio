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
		pModule_->SetDefaultListener(pListener);
	}
}

//----------------------------------------------------------------------------
CAudioMPI::~CAudioMPI()
{
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
tErrType CAudioMPI::StartAudio() 
{
	if ( pModule_ != kNull )
	{
		return pModule_->StartAudio();
	}
}

//----------------------------------------------------------------------------
tErrType CAudioMPI::StopAudio() 
{
	if ( pModule_ != kNull )
	{
		return pModule_->StopAudio();
	}
}

//----------------------------------------------------------------------------
tErrType CAudioMPI::PauseAudio() 
{
	if ( pModule_ != kNull )
	{
		return pModule_->PauseAudio();
	}
}

//----------------------------------------------------------------------------
tErrType CAudioMPI::ResumeAudio() 
{
	if ( pModule_ != kNull )
	{
		return pModule_->ResumeAudio();
	}
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
tErrType CAudioMPI::ChangeAudioEffectsProcessor(tAudioID audioID, CAudioEffectsProcessor *pChain) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->pChangeAudioEffectsProcessor(audioID, pChain);
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetMasterVolume(U8 volume) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pSetMasterVolume(volume);
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
	return pModule_->pGetMasterVolume();
}

//==============================================================================
//==============================================================================
tAudioID CAudioMPI::PlayAudio(tRsrcHndl			hRsrc, 
								U8					volume, 
								tAudioPriority		priority,
								S8					pan, 
								IEventListener		*pHandler,
								tAudioPayload		payload,
								tAudioOptionsFlags	flags)
{
	if ( pModule_ == kNull )
	{
		return kNoAudioID;
	}
	return pModule_->pPlayAudio(pModule_, hRsrc, volume, priority, pan, pHandler, payload, flags);
}

//==============================================================================
//==============================================================================
tAudioID CAudioMPI::PlayAudio(tRsrcHndl			hRsrc, 
								tAudioPayload		payload,
								tAudioOptionsFlags	flags)
{
	if ( pModule_ == kNull )
	{
		return kNoAudioID;
	}
	return pModule_->pPlayAudioDefault(pModule_, hRsrc, payload, flags);
}

//==============================================================================
//==============================================================================
tAudioID CAudioMPI::PlayAudioArray(const tRsrcHndl	*pArray, 
									U8					arrayItemCount, 
									U8					volume,
									tAudioPriority		priority,
									S8					pan, 
									IEventListener		*pHandler,
									tAudioPayload		payload,
									tAudioOptionsFlags	flags)
{
	if ( pModule_ == kNull )
	{
		return kNoAudioID;
	}
	return pModule_->pPlayAudioArray(pModule_, pArray, arrayItemCount, volume, priority, pan, pHandler, payload, flags);
}

//==============================================================================
//==============================================================================
tAudioID CAudioMPI::PlayAudioArray(const tRsrcHndl	*pArray, 
									U8					arrayItemCount, 
									tAudioPayload		payload,
									tAudioOptionsFlags	flags)
{
	if ( pModule_ == kNull )
	{
		return kNoAudioID;
	}
	return pModule_->pPlayAudioArrayDefault(pModule_, pArray, arrayItemCount, payload, flags);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::AppendAudioArray(tAudioID		audioID,
									const tRsrcHndl		*pArray, 
									U8					arrayItemCount, 
									U8					volume,
									tAudioPriority		priority,
									S8					pan, 
									IEventListener		*pHandler,
									tAudioPayload		payload,
									tAudioOptionsFlags	flags)
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->pAppendAudioArray(audioID, pArray, arrayItemCount, volume, priority, pan, pHandler, payload, flags);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::AppendAudioArray(tAudioID		audioID,
									const tRsrcHndl		*pArray, 
									U8					arrayItemCount, 
									tAudioPayload		payload,
									tAudioOptionsFlags	flags)
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->pAppendAudioArrayDefault(audioID, pArray, arrayItemCount, payload, flags);
}

//==============================================================================
//==============================================================================
void CAudioMPI::StopAudio(tAudioID audioID, Boolean surpressDoneMessage) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pStopAudio(audioID, surpressDoneMessage);
	}
}

//==============================================================================
//==============================================================================
void CAudioMPI::PauseAudio(tAudioID audioID) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pPauseAudio(audioID);
	}
}

//==============================================================================
//==============================================================================
void CAudioMPI::ResumeAudio(tAudioID audioID) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pResumeAudio(audioID);
	}
}

//==============================================================================
//==============================================================================
U8 CAudioMPI::GetAudioVolume(tAudioID audioID) 
{
	if ( pModule_ == kNull )
	{
		return 0;
	}
	return pModule_->pGetAudioVolume(audioID);
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetAudioVolume(tAudioID audioID, U8 volume) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pSetAudioVolume(audioID, volume);
	}
}

//==============================================================================
//==============================================================================
tAudioPriority CAudioMPI::GetAudioPriority(tAudioID audioID) 
{
	if ( pModule_ == kNull )
	{
		return 0;
	}
	return pModule_->pGetAudioPriority(audioID);
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetAudioPriority(tAudioID audioID, tAudioPriority priority) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pSetAudioPriority(audioID, priority);
	}
}

//==============================================================================
//==============================================================================
S8 CAudioMPI::GetAudioPan(tAudioID audioID) 
{
	if ( pModule_ == kNull )
	{
		return 0;
	}
	return pModule_->pGetAudioPan(audioID);
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetAudioPan(tAudioID audioID, S8 pan) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pSetAudioPan(audioID, pan);
	}
}

//==============================================================================
//==============================================================================
IEventListener* CAudioMPI::GetAudioEventHandler(tAudioID audioID) 
{
	if ( pModule_ == kNull )
	{
		return NULL;
	}
	return pModule_->pGetAudioEventHandler(audioID);
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetAudioEventHandler(tAudioID audioID, IEventListener *pHandler) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pSetAudioEventHandler(audioID, pHandler);
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

//==============================================================================
//==============================================================================
Boolean CAudioMPI::IsAudioBusy(tAudioID audioID) 
{
	if ( pModule_ == kNull )
	{
		return false;
	}
	return pModule_->pIsAudioBusy(audioID);
}

//==============================================================================
//==============================================================================
Boolean CAudioMPI::IsAnyAudioBusy() 
{
	if ( pModule_ == kNull )
	{
		return false;
	}
	return pModule_->pIsAnyAudioBusy();
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::AcquireMidiPlayer(tAudioPriority priority, IEventListener *pHandler, tMidiID *midiID) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->pAcquireMidiPlayer(priority, pHandler, midiID);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::ReleaseMidiPlayer(tMidiID midiID) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->pReleaseMidiPlayer(midiID);
}

//==============================================================================
//==============================================================================
tMidiID CAudioMPI::GetMidiIDForAudioID(tAudioID audioID) 
{
	if ( pModule_ == kNull )
	{
		return kNoAudioID;
	}
	return pModule_->pGetMidiIDForAudioID(audioID);
}

//==============================================================================
//==============================================================================
tAudioID CAudioMPI::GetAudioIDForMidiID(tMidiID midiID) 
{
	if ( pModule_ == kNull )
	{
		return kNoAudioID;
	}
	return pModule_->pGetAudioIDForMidiID(midiID);
}

//==============================================================================
//==============================================================================
void CAudioMPI::StopMidiPlayer(tMidiID midiID, Boolean surpressDoneMessage) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pStopMidiPlayer(midiID, surpressDoneMessage);
	}
}

//==============================================================================
//==============================================================================
void CAudioMPI::PauseMidiPlayer(tMidiID midiID) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pPauseMidiPlayer(midiID);
	}
}

//==============================================================================
//==============================================================================
void CAudioMPI::ResumeMidiPlayer(tMidiID midiID) 
{
	if ( pModule_ != kNull )
	{
		pModule_->pResumeMidiPlayer(midiID);
	}
}

//==============================================================================
//==============================================================================
tMidiTrackBitMask CAudioMPI::GetEnabledMidiTracks(tMidiID midiID) 
{
	if ( pModule_ == kNull )
	{
		return 0;
	}
	return pModule_->pGetEnabledMidiTracks(midiID);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::EnableMidiTracks(tMidiID midiID, tMidiTrackBitMask trackBitMask) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->pEnableMidiTracks(midiID, trackBitMask);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::TransposeMidiTracks(tMidiID midiID, tMidiTrackBitMask trackBitMask, S8 transposeAmount) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->pTransposeMidiTracks(midiID, trackBitMask, transposeAmount);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::ChangeMidiInstrument(tMidiID midiID, tMidiTrackBitMask trackBitMask, tMidiInstr instr) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->pChangeMidiInstrument(midiID, trackBitMask, instr);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::ChangeMidiTempo(tMidiID midiID, S8 tempo) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->pChangeMidiTempo(midiID, tempo);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::SendMidiCommand(tMidiID midiID, U8 cmd, U8 data1, U8 data2) 
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->pSendMidiCommand(midiID, cmd, data1, data2);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::PlayMidiNote(tMidiID	midiID,
									U8 			track, 
									U8			pitch, 
									U8			velocity, 
									U16			noteCount,
									tAudioOptionsFlags	flags)
{
	if ( pModule_ == kNull )
	{
		return kNoImplErr;
	}
	return pModule_->pPlayMidiNote(midiID, track, pitch, velocity, noteCount, flags);
}
*/
// EOF	
