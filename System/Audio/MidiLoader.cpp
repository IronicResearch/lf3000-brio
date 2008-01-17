//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// MidiLoader.cpp
//
// Module to dynamically load SPMIDI functions from Mobileer shared library.
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <AudioTypes.h>
#include <AudioPriv.h>
#include <KernelMPI.h>
#include <EmulationConfig.h>

#define USE_SPMIDI_EXPORTS
#include <MidiLoader.h>

//==============================================================================
// Export function pointers
//==============================================================================

pFnInitialize				pSPMIDI_Initialize = NULL;
pFnTerminate				pSPMIDI_Terminate = NULL;
pFnCreateContext			pSPMIDI_CreateContext = NULL;
pFnDeleteContext			pSPMIDI_DeleteContext = NULL;
pFnSetMaxVoices				pSPMIDI_SetMaxVoices = NULL;
pFnGetMaxVoices 			pSPMIDI_GetMaxVoices = NULL;
pFnWriteCommand				pSPMIDI_WriteCommand = NULL;
pFnReadFrames				pSPMIDI_ReadFrames = NULL;
pFnGetFramesPerBuffer		pSPMIDI_GetFramesPerBuffer = NULL;
pFnSetMasterVolume			pSPMIDI_SetMasterVolume = NULL;
pFnGetChannelEnable			pSPMIDI_GetChannelEnable = NULL;
pFnSetChannelEnable			pSPMIDI_SetChannelEnable = NULL;
pFnCreateProgramList		pSPMIDI_CreateProgramList = NULL;
pFnDeleteProgramList		pSPMIDI_DeleteProgramList = NULL;
pFnCreateOrchestra			pSPMIDI_CreateOrchestra = NULL;
pFnDeleteOrchestra			pSPMIDI_DeleteOrchestra = NULL;
pFnLoadOrchestra			pSPMIDI_LoadOrchestra = NULL;
pFnGetMemoryAllocationCount pSPMIDI_GetMemoryAllocationCount = NULL;
pFnUtilReset				pSPMUtil_Reset = NULL;
pFnUtilNoteOn				pSPMUtil_NoteOn = NULL;
pFnUtilNoteOff				pSPMUtil_NoteOff = NULL;
pFnUtilControlChange		pSPMUtil_ControlChange = NULL;
pFnFileScanForPrograms		pMIDIFile_ScanForPrograms = NULL;
pFnStreamOpenFile			pStream_OpenFile = NULL;
pFnStreamClose				pStream_Close = NULL;
pFnFilePlayerCreate			pMIDIFilePlayer_Create = NULL;
pFnFilePlayerRewind			pMIDIFilePlayer_Rewind = NULL;
pFnFilePlayerSetTempoScaler	pMIDIFilePlayer_SetTempoScaler = NULL;
pFnFilePlayerPlayFrames		pMIDIFilePlayer_PlayFrames = NULL;
pFnFilePlayerDelete			pMIDIFilePlayer_Delete = NULL;

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================

const CPath kMidiLibName = "libme2000.so";

//==============================================================================
// Local variables
//==============================================================================

namespace 
{	
	tHndl						hLibMidi = kInvalidHndl;
	CPath						gLibPath = "";
}

//------------------------------------------------------------------------------
static CPath GetLibPath( void )
{
#ifdef EMULATION
	CPath dir = EmulationConfig::Instance().GetModuleSearchPath();
	int pos = dir.find("Module");
	int len = dir.length() - pos;
	dir.erase(pos, len);
	dir.append("lib/");
	return dir;
#else	
	return "/Didj/Base/Brio/lib/";
#endif	// EMULATION
}

//==============================================================================
// Implementation
//==============================================================================

//------------------------------------------------------------------------------
bool LoadMidiLibrary(void)
{
	CKernelMPI 	kernel;
	gLibPath = GetLibPath() + kMidiLibName;
	hLibMidi = kernel.LoadModule(gLibPath);
	if (hLibMidi == kInvalidHndl)
		return false;
	pSPMIDI_Initialize = (pFnInitialize) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_Initialize"));
	pSPMIDI_Terminate = (pFnTerminate) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_Terminate"));
	pSPMIDI_CreateContext = (pFnCreateContext) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_CreateContext"));
	pSPMIDI_DeleteContext = (pFnDeleteContext) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_DeleteContext"));
	pSPMIDI_SetMaxVoices = (pFnSetMaxVoices) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_SetMaxVoices"));
	pSPMIDI_GetMaxVoices = (pFnGetMaxVoices) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_GetMaxVoices"));
	pSPMIDI_WriteCommand = (pFnWriteCommand) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_WriteCommand"));
	pSPMIDI_ReadFrames = (pFnReadFrames) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_ReadFrames"));
	pSPMIDI_GetFramesPerBuffer = (pFnGetFramesPerBuffer) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_GetFramesPerBuffer"));
	pSPMIDI_SetMasterVolume = (pFnSetMasterVolume) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_SetMasterVolume"));
	pSPMIDI_GetChannelEnable = (pFnGetChannelEnable) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_GetChannelEnable"));
	pSPMIDI_SetChannelEnable = (pFnSetChannelEnable) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_SetChannelEnable"));
	pSPMIDI_CreateProgramList = (pFnCreateProgramList) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_CreateProgramList"));
	pSPMIDI_DeleteProgramList = (pFnDeleteProgramList) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_DeleteProgramList"));
	pSPMIDI_CreateOrchestra = (pFnCreateOrchestra) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_CreateOrchestra"));
	pSPMIDI_DeleteOrchestra = (pFnDeleteOrchestra) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_DeleteOrchestra"));
	pSPMIDI_LoadOrchestra = (pFnLoadOrchestra) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_LoadOrchestra"));
	pSPMIDI_GetMemoryAllocationCount = (pFnGetMemoryAllocationCount) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_GetMemoryAllocationCount"));

	pSPMUtil_Reset = (pFnUtilReset) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMUtil_Reset"));
	pSPMUtil_NoteOn = (pFnUtilNoteOn) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMUtil_NoteOn"));
	pSPMUtil_NoteOff = (pFnUtilNoteOff) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMUtil_NoteOff"));
	pSPMUtil_ControlChange = (pFnUtilControlChange) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMUtil_ControlChange"));

	pMIDIFile_ScanForPrograms = (pFnFileScanForPrograms) (kernel.RetrieveSymbolFromModule(hLibMidi, "MIDIFile_ScanForPrograms"));
	
	pStream_OpenFile = (pFnStreamOpenFile) (kernel.RetrieveSymbolFromModule(hLibMidi, "Stream_OpenFile"));
	pStream_Close = (pFnStreamClose) (kernel.RetrieveSymbolFromModule(hLibMidi, "Stream_Close"));

	pMIDIFilePlayer_Create = (pFnFilePlayerCreate) (kernel.RetrieveSymbolFromModule(hLibMidi, "MIDIFilePlayer_Create"));
	pMIDIFilePlayer_Rewind = (pFnFilePlayerRewind) (kernel.RetrieveSymbolFromModule(hLibMidi, "MIDIFilePlayer_Rewind"));
	pMIDIFilePlayer_SetTempoScaler = (pFnFilePlayerSetTempoScaler) (kernel.RetrieveSymbolFromModule(hLibMidi, "MIDIFilePlayer_SetTempoScaler"));
	pMIDIFilePlayer_PlayFrames = (pFnFilePlayerPlayFrames) (kernel.RetrieveSymbolFromModule(hLibMidi, "MIDIFilePlayer_PlayFrames"));
	pMIDIFilePlayer_Delete = (pFnFilePlayerDelete) (kernel.RetrieveSymbolFromModule(hLibMidi, "MIDIFilePlayer_Delete"));

	//	pSPMIDI_XXXX = (pFnXXXX) (kernel.RetrieveSymbolFromModule(hLibMidi, "SPMIDI_XXXX"));

	return true;
}

//------------------------------------------------------------------------------
void UnloadMidiLibrary(void)
{
	CKernelMPI 	kernel;
	kernel.UnloadModule(hLibMidi);
	hLibMidi = kInvalidHndl;
}

LF_END_BRIO_NAMESPACE()

// EOF
