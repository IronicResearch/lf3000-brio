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
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <ResourceTypes.h>
#include <SystemErrors.h>
#include <Module.h>
//#include <AudioTypes.h>
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
static const tVersion kMPIVersion = MakeVersion(0, 1);
static const CString kMPIName = "AudioMPI";


//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CAudioMPI implementation
//==============================================================================

CAudioMPI::CAudioMPI() : mpModule(NULL)
{
	tErrType err;
	
	ICoreModule*	pModule;
	err = Module::Connect(pModule, kAudioModuleName, 
									kAudioModuleVersion);
									
	if (kNoErr == err)
		mpModule = reinterpret_cast<CAudioModule*>(pModule);
}

//----------------------------------------------------------------------------
CAudioMPI::~CAudioMPI()
{
	Module::Disconnect( mpModule );
}

//============================================================================
// Informational functions
//============================================================================
Boolean CAudioMPI::IsValid() const
{
	return (mpModule != NULL) ? true : false;
}

//----------------------------------------------------------------------------
tErrType CAudioMPI::GetMPIVersion( tVersion &version ) const
{
	version = kMPIVersion;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CAudioMPI::GetMPIName( ConstPtrCString &pName ) const
{
	pName = &kMPIName;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CAudioMPI::GetModuleVersion( tVersion &version ) const
{
	if(!mpModule)
		return kMpiNotConnectedErr;

	return mpModule->GetModuleVersion( version );
}

//----------------------------------------------------------------------------
tErrType CAudioMPI::GetModuleName( ConstPtrCString &pName ) const
{
	if (!mpModule)
		return kMpiNotConnectedErr;

	return mpModule->GetModuleName( pName );
}

//----------------------------------------------------------------------------
tErrType CAudioMPI::GetModuleOrigin( ConstPtrCURI &pURI ) const
{
	if(!mpModule)
		return kMpiNotConnectedErr;
		
	return mpModule->GetModuleOrigin( pURI );
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::StartAudio() 
{
	if ( mpModule != kNull )
	{
		return mpModule->StartAudio();
	}
}

//----------------------------------------------------------------------------
tErrType CAudioMPI::StopAudio() 
{
	if ( mpModule != kNull )
	{
		return mpModule->StopAudio();
	}
}

//----------------------------------------------------------------------------
tErrType CAudioMPI::PauseAudio() 
{
	if ( mpModule != kNull )
	{
		return mpModule->PauseAudio();
	}
}

//----------------------------------------------------------------------------
tErrType CAudioMPI::ResumeAudio() 
{
	if ( mpModule != kNull )
	{
		return mpModule->ResumeAudio();
	}
}

/*
//==============================================================================
//==============================================================================
tErrType CAudioMPI::RegisterGetStereoAudioStreamFcn(tRsrcType type, tGetStereoAudioStreamFcn pFcn) 
{
	if ( mpModule == kNull )
	{
		return kNoImplErr;
	}
	return mpModule->pRegisterGetStereoAudioStreamFcn(type, pFcn);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::RegisterAudioEffectsProcessor(tRsrcType type, CAudioEffectsProcessor *pChain) 
{
	if ( mpModule == kNull )
	{
		return kNoImplErr;
	}
	return mpModule->pRegisterAudioEffectsProcessor(type, pChain);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::RegisterGlobalAudioEffectsProcessor(CAudioEffectsProcessor *pChain) 
{
	if ( mpModule == kNull )
	{
		return kNoImplErr;
	}
	return mpModule->pRegisterGlobalAudioEffectsProcessor(pChain);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::ChangeAudioEffectsProcessor(tAudioID audioID, CAudioEffectsProcessor *pChain) 
{
	if ( mpModule == kNull )
	{
		return kNoImplErr;
	}
	return mpModule->pChangeAudioEffectsProcessor(audioID, pChain);
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetMasterVolume(U8 volume) 
{
	if ( mpModule != kNull )
	{
		mpModule->pSetMasterVolume(volume);
	}
}

//==============================================================================
//==============================================================================
U8 CAudioMPI::GetMasterVolume() 
{
	if ( mpModule == kNull )
	{
		return 0;
	}
	return mpModule->pGetMasterVolume();
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
	if ( mpModule == kNull )
	{
		return kNoAudioID;
	}
	return mpModule->pPlayAudio(mpModule, hRsrc, volume, priority, pan, pHandler, payload, flags);
}

//==============================================================================
//==============================================================================
tAudioID CAudioMPI::PlayAudio(tRsrcHndl			hRsrc, 
								tAudioPayload		payload,
								tAudioOptionsFlags	flags)
{
	if ( mpModule == kNull )
	{
		return kNoAudioID;
	}
	return mpModule->pPlayAudioDefault(mpModule, hRsrc, payload, flags);
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
	if ( mpModule == kNull )
	{
		return kNoAudioID;
	}
	return mpModule->pPlayAudioArray(mpModule, pArray, arrayItemCount, volume, priority, pan, pHandler, payload, flags);
}

//==============================================================================
//==============================================================================
tAudioID CAudioMPI::PlayAudioArray(const tRsrcHndl	*pArray, 
									U8					arrayItemCount, 
									tAudioPayload		payload,
									tAudioOptionsFlags	flags)
{
	if ( mpModule == kNull )
	{
		return kNoAudioID;
	}
	return mpModule->pPlayAudioArrayDefault(mpModule, pArray, arrayItemCount, payload, flags);
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
	if ( mpModule == kNull )
	{
		return kNoImplErr;
	}
	return mpModule->pAppendAudioArray(audioID, pArray, arrayItemCount, volume, priority, pan, pHandler, payload, flags);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::AppendAudioArray(tAudioID		audioID,
									const tRsrcHndl		*pArray, 
									U8					arrayItemCount, 
									tAudioPayload		payload,
									tAudioOptionsFlags	flags)
{
	if ( mpModule == kNull )
	{
		return kNoImplErr;
	}
	return mpModule->pAppendAudioArrayDefault(audioID, pArray, arrayItemCount, payload, flags);
}

//==============================================================================
//==============================================================================
void CAudioMPI::StopAudio(tAudioID audioID, Boolean surpressDoneMessage) 
{
	if ( mpModule != kNull )
	{
		mpModule->pStopAudio(audioID, surpressDoneMessage);
	}
}

//==============================================================================
//==============================================================================
void CAudioMPI::PauseAudio(tAudioID audioID) 
{
	if ( mpModule != kNull )
	{
		mpModule->pPauseAudio(audioID);
	}
}

//==============================================================================
//==============================================================================
void CAudioMPI::ResumeAudio(tAudioID audioID) 
{
	if ( mpModule != kNull )
	{
		mpModule->pResumeAudio(audioID);
	}
}

//==============================================================================
//==============================================================================
U8 CAudioMPI::GetAudioVolume(tAudioID audioID) 
{
	if ( mpModule == kNull )
	{
		return 0;
	}
	return mpModule->pGetAudioVolume(audioID);
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetAudioVolume(tAudioID audioID, U8 volume) 
{
	if ( mpModule != kNull )
	{
		mpModule->pSetAudioVolume(audioID, volume);
	}
}

//==============================================================================
//==============================================================================
tAudioPriority CAudioMPI::GetAudioPriority(tAudioID audioID) 
{
	if ( mpModule == kNull )
	{
		return 0;
	}
	return mpModule->pGetAudioPriority(audioID);
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetAudioPriority(tAudioID audioID, tAudioPriority priority) 
{
	if ( mpModule != kNull )
	{
		mpModule->pSetAudioPriority(audioID, priority);
	}
}

//==============================================================================
//==============================================================================
S8 CAudioMPI::GetAudioPan(tAudioID audioID) 
{
	if ( mpModule == kNull )
	{
		return 0;
	}
	return mpModule->pGetAudioPan(audioID);
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetAudioPan(tAudioID audioID, S8 pan) 
{
	if ( mpModule != kNull )
	{
		mpModule->pSetAudioPan(audioID, pan);
	}
}

//==============================================================================
//==============================================================================
IEventListener* CAudioMPI::GetAudioEventHandler(tAudioID audioID) 
{
	if ( mpModule == kNull )
	{
		return NULL;
	}
	return mpModule->pGetAudioEventHandler(audioID);
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetAudioEventHandler(tAudioID audioID, IEventListener *pHandler) 
{
	if ( mpModule != kNull )
	{
		mpModule->pSetAudioEventHandler(audioID, pHandler);
	}
}

//==============================================================================
//==============================================================================
U8 CAudioMPI::GetDefaultAudioVolume() 
{
	if ( mpModule == kNull )
	{
		return 0;
	}
	return mpModule->pGetDefaultAudioVolume();
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetDefaultAudioVolume(U8 volume) 
{
	if ( mpModule != kNull )
	{
		mpModule->pSetDefaultAudioVolume(volume);
	}
}

//==============================================================================
//==============================================================================
tAudioPriority CAudioMPI::GetDefaultAudioPriority() 
{
	if ( mpModule == kNull )
	{
		return 0;
	}
	return mpModule->pGetDefaultAudioPriority();
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetDefaultAudioPriority(tAudioPriority priority) 
{
	if ( mpModule != kNull )
	{
		mpModule->pSetDefaultAudioPriority(priority);
	}
}

//==============================================================================
//==============================================================================
S8 CAudioMPI::GetDefaultAudioPan() 
{
	if ( mpModule == kNull )
	{
		return 0;
	}
	return mpModule->pGetDefaultAudioPan();
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetDefaultAudioPan(S8 pan) 
{
	if ( mpModule != kNull )
	{
		mpModule->pSetDefaultAudioPan(pan);
	}
}

//==============================================================================
//==============================================================================
IEventListener* CAudioMPI::GetDefaultAudioEventHandler() 
{
	if ( mpModule == kNull )
	{
		return NULL;
	}
	return mpModule->pGetDefaultAudioEventHandler();
}

//==============================================================================
//==============================================================================
void CAudioMPI::SetDefaultAudioEventHandler(IEventListener *pHandler) 
{
	if ( mpModule != kNull )
	{
		mpModule->pSetDefaultAudioEventHandler(pHandler);
	}
}

//==============================================================================
//==============================================================================
Boolean CAudioMPI::IsAudioBusy(tAudioID audioID) 
{
	if ( mpModule == kNull )
	{
		return false;
	}
	return mpModule->pIsAudioBusy(audioID);
}

//==============================================================================
//==============================================================================
Boolean CAudioMPI::IsAnyAudioBusy() 
{
	if ( mpModule == kNull )
	{
		return false;
	}
	return mpModule->pIsAnyAudioBusy();
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::AcquireMidiPlayer(tAudioPriority priority, IEventListener *pHandler, tMidiID *midiID) 
{
	if ( mpModule == kNull )
	{
		return kNoImplErr;
	}
	return mpModule->pAcquireMidiPlayer(priority, pHandler, midiID);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::ReleaseMidiPlayer(tMidiID midiID) 
{
	if ( mpModule == kNull )
	{
		return kNoImplErr;
	}
	return mpModule->pReleaseMidiPlayer(midiID);
}

//==============================================================================
//==============================================================================
tMidiID CAudioMPI::GetMidiIDForAudioID(tAudioID audioID) 
{
	if ( mpModule == kNull )
	{
		return kNoAudioID;
	}
	return mpModule->pGetMidiIDForAudioID(audioID);
}

//==============================================================================
//==============================================================================
tAudioID CAudioMPI::GetAudioIDForMidiID(tMidiID midiID) 
{
	if ( mpModule == kNull )
	{
		return kNoAudioID;
	}
	return mpModule->pGetAudioIDForMidiID(midiID);
}

//==============================================================================
//==============================================================================
void CAudioMPI::StopMidiPlayer(tMidiID midiID, Boolean surpressDoneMessage) 
{
	if ( mpModule != kNull )
	{
		mpModule->pStopMidiPlayer(midiID, surpressDoneMessage);
	}
}

//==============================================================================
//==============================================================================
void CAudioMPI::PauseMidiPlayer(tMidiID midiID) 
{
	if ( mpModule != kNull )
	{
		mpModule->pPauseMidiPlayer(midiID);
	}
}

//==============================================================================
//==============================================================================
void CAudioMPI::ResumeMidiPlayer(tMidiID midiID) 
{
	if ( mpModule != kNull )
	{
		mpModule->pResumeMidiPlayer(midiID);
	}
}

//==============================================================================
//==============================================================================
tMidiTrackBitMask CAudioMPI::GetEnabledMidiTracks(tMidiID midiID) 
{
	if ( mpModule == kNull )
	{
		return 0;
	}
	return mpModule->pGetEnabledMidiTracks(midiID);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::EnableMidiTracks(tMidiID midiID, tMidiTrackBitMask trackBitMask) 
{
	if ( mpModule == kNull )
	{
		return kNoImplErr;
	}
	return mpModule->pEnableMidiTracks(midiID, trackBitMask);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::TransposeMidiTracks(tMidiID midiID, tMidiTrackBitMask trackBitMask, S8 transposeAmount) 
{
	if ( mpModule == kNull )
	{
		return kNoImplErr;
	}
	return mpModule->pTransposeMidiTracks(midiID, trackBitMask, transposeAmount);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::ChangeMidiInstrument(tMidiID midiID, tMidiTrackBitMask trackBitMask, tMidiInstr instr) 
{
	if ( mpModule == kNull )
	{
		return kNoImplErr;
	}
	return mpModule->pChangeMidiInstrument(midiID, trackBitMask, instr);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::ChangeMidiTempo(tMidiID midiID, S8 tempo) 
{
	if ( mpModule == kNull )
	{
		return kNoImplErr;
	}
	return mpModule->pChangeMidiTempo(midiID, tempo);
}

//==============================================================================
//==============================================================================
tErrType CAudioMPI::SendMidiCommand(tMidiID midiID, U8 cmd, U8 data1, U8 data2) 
{
	if ( mpModule == kNull )
	{
		return kNoImplErr;
	}
	return mpModule->pSendMidiCommand(midiID, cmd, data1, data2);
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
	if ( mpModule == kNull )
	{
		return kNoImplErr;
	}
	return mpModule->pPlayMidiNote(midiID, track, pitch, velocity, noteCount, flags);
}
*/
// EOF	
