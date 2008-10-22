#ifndef LF_BRIO_MIDILOADER_H
#define LF_BRIO_MIDILOADER_H

//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// MidiLoader.h
//
// Module to dynamically load SPMIDI functions from Mobileer shared library.
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <AudioTypes.h>
#include <AudioPlayer.h>

// Mobileer MIDI Engine
#include <spmidi.h>
#include <spmidi_util.h>
#include <midifile_player.h>
#include "program_list.h"
#include "spmidi_load.h"
#include "spmidi_print.h"
#include "spmidi/engine/spmidi_host.h"

//==============================================================================
// Function Typedefs
//==============================================================================

typedef int (*pFnInitialize)( void );
typedef int (*pFnTerminate)( void );
typedef int (*pFnCreateContext)( SPMIDI_Context **, int	 );
typedef int (*pFnDeleteContext)( SPMIDI_Context * );
typedef int (*pFnSetMaxVoices)( SPMIDI_Context *, int );
typedef int (*pFnGetMaxVoices)( SPMIDI_Context * );
typedef void (*pFnWriteCommand)( SPMIDI_Context *, int, int, int );
typedef int (*pFnReadFrames)( SPMIDI_Context *, void *, int, int, int );
typedef int (*pFnGetFramesPerBuffer)( void );
typedef void (*pFnSetMasterVolume)( SPMIDI_Context *, int );
typedef int (*pFnGetChannelEnable)( SPMIDI_Context *, int );
typedef int (*pFnSetChannelEnable)( SPMIDI_Context *, int, int );
typedef int (*pFnCreateProgramList)( SPMIDI_ProgramList ** );
typedef void (*pFnDeleteProgramList)( SPMIDI_ProgramList * );
typedef int (*pFnCreateOrchestra)( SPMIDI_Orchestra **, spmSInt32 );
typedef void (*pFnDeleteOrchestra)( SPMIDI_Orchestra * );
typedef SPMIDI_Error (*pFnLoadOrchestra)( StreamIO *, SPMIDI_ProgramList *, SPMIDI_Orchestra ** );
typedef int (*pFnGetMemoryAllocationCount)( void );

typedef void (*pFnUtilReset)( SPMIDI_Context * );
typedef void (*pFnUtilNoteOn)( SPMIDI_Context *, int, int, int );
typedef void (*pFnUtilNoteOff)( SPMIDI_Context *, int, int, int );
typedef void (*pFnUtilControlChange)( SPMIDI_Context *, int, int, int );
typedef int (*pFnFileScanForPrograms)( SPMIDI_ProgramList *, unsigned char *, int );
typedef StreamIO* (*pFnStreamOpenFile)( char *, char * );
typedef void (*pFnStreamClose)( StreamIO * );
typedef SPMIDI_Error (*pFnFilePlayerCreate)( MIDIFilePlayer **, int, const unsigned char *, int );
typedef SPMIDI_Error (*pFnFilePlayerRewind)( MIDIFilePlayer * );
typedef SPMIDI_Error (*pFnFilePlayerSetTempoScaler)( MIDIFilePlayer *, int );
typedef int (*pFnFilePlayerPlayFrames)( MIDIFilePlayer *, SPMIDI_Context *, int );
typedef void (*pFnFilePlayerDelete)( MIDIFilePlayer * );
typedef void (*pFnFilePlayerSetTextCallback)( MIDIFilePlayer *, MIDIFilePlayer_TextCallback *, void * );
typedef int (*pFnFilePlayerGetFrameTime)( MIDIFilePlayer * );


//==============================================================================
// Export Function Pointers
//==============================================================================

extern pFnInitialize				pSPMIDI_Initialize;
extern pFnTerminate					pSPMIDI_Terminate;
extern pFnCreateContext				pSPMIDI_CreateContext;
extern pFnDeleteContext				pSPMIDI_DeleteContext;
extern pFnSetMaxVoices				pSPMIDI_SetMaxVoices;
extern pFnGetMaxVoices				pSPMIDI_GetMaxVoices;
extern pFnWriteCommand				pSPMIDI_WriteCommand;
extern pFnReadFrames				pSPMIDI_ReadFrames;
extern pFnGetFramesPerBuffer		pSPMIDI_GetFramesPerBuffer;
extern pFnSetMasterVolume			pSPMIDI_SetMasterVolume;
extern pFnGetChannelEnable			pSPMIDI_GetChannelEnable;
extern pFnSetChannelEnable			pSPMIDI_SetChannelEnable;
extern pFnCreateProgramList			pSPMIDI_CreateProgramList;
extern pFnDeleteProgramList			pSPMIDI_DeleteProgramList;
extern pFnCreateOrchestra			pSPMIDI_CreateOrchestra;
extern pFnDeleteOrchestra			pSPMIDI_DeleteOrchestra;
extern pFnLoadOrchestra				pSPMIDI_LoadOrchestra;
extern pFnGetMemoryAllocationCount	pSPMIDI_GetMemoryAllocationCount;
extern pFnUtilReset					pSPMUtil_Reset;
extern pFnUtilNoteOn				pSPMUtil_NoteOn;
extern pFnUtilNoteOff				pSPMUtil_NoteOff;
extern pFnUtilControlChange			pSPMUtil_ControlChange;
extern pFnFileScanForPrograms		pMIDIFile_ScanForPrograms;
extern pFnStreamOpenFile			pStream_OpenFile;
extern pFnStreamClose				pStream_Close;
extern pFnFilePlayerCreate			pMIDIFilePlayer_Create;
extern pFnFilePlayerRewind			pMIDIFilePlayer_Rewind;
extern pFnFilePlayerSetTempoScaler	pMIDIFilePlayer_SetTempoScaler;
extern pFnFilePlayerPlayFrames		pMIDIFilePlayer_PlayFrames;
extern pFnFilePlayerDelete			pMIDIFilePlayer_Delete;
extern pFnFilePlayerSetTextCallback	pMIDIFilePlayer_SetTextCallback;
extern pFnFilePlayerGetFrameTime	pMIDIFilePlayer_GetFrameTime;

#ifndef USE_SPMIDI_EXPORTS
#define SPMIDI_Initialize			pSPMIDI_Initialize
#define SPMIDI_Terminate			pSPMIDI_Terminate				
#define SPMIDI_CreateContext		pSPMIDI_CreateContext			
#define SPMIDI_DeleteContext		pSPMIDI_DeleteContext			
#define SPMIDI_SetMaxVoices			pSPMIDI_SetMaxVoices				
#define SPMIDI_GetMaxVoices			pSPMIDI_GetMaxVoices			
#define SPMIDI_WriteCommand			pSPMIDI_WriteCommand				
#define SPMIDI_ReadFrames			pSPMIDI_ReadFrames				
#define SPMIDI_GetFramesPerBuffer	pSPMIDI_GetFramesPerBuffer		
#define SPMIDI_SetMasterVolume		pSPMIDI_SetMasterVolume			
#define SPMIDI_GetChannelEnable		pSPMIDI_GetChannelEnable			
#define SPMIDI_SetChannelEnable		pSPMIDI_SetChannelEnable			
#define SPMIDI_CreateProgramList	pSPMIDI_CreateProgramList		
#define SPMIDI_DeleteProgramList	pSPMIDI_DeleteProgramList		
#define SPMIDI_CreateOrchestra		pSPMIDI_CreateOrchestra			
#define SPMIDI_DeleteOrchestra		pSPMIDI_DeleteOrchestra			
#define SPMIDI_LoadOrchestra		pSPMIDI_LoadOrchestra			
#define SPMIDI_GetMemoryAllocationCount	pSPMIDI_GetMemoryAllocationCount 
#define SPMUtil_Reset				pSPMUtil_Reset
#define SPMUtil_NoteOn				pSPMUtil_NoteOn
#define SPMUtil_NoteOff				pSPMUtil_NoteOff
#define SPMUtil_ControlChange		pSPMUtil_ControlChange
#define MIDIFile_ScanForPrograms	pMIDIFile_ScanForPrograms
#define Stream_OpenFile				pStream_OpenFile
#define Stream_Close				pStream_Close
#define MIDIFilePlayer_Create		pMIDIFilePlayer_Create
#define MIDIFilePlayer_Rewind		pMIDIFilePlayer_Rewind			
#define MIDIFilePlayer_SetTempoScaler	pMIDIFilePlayer_SetTempoScaler	
#define MIDIFilePlayer_PlayFrames	pMIDIFilePlayer_PlayFrames		
#define MIDIFilePlayer_Delete		pMIDIFilePlayer_Delete		
#define MIDIFilePlayer_SetTextCallback	pMIDIFilePlayer_SetTextCallback
#define MIDIFilePlayer_GetFrameTime		pMIDIFilePlayer_GetFrameTime
#endif

//==============================================================================
// Functions
//==============================================================================

LF_BEGIN_BRIO_NAMESPACE()

bool LoadMidiLibrary(void);
void UnloadMidiLibrary(void);

LF_END_BRIO_NAMESPACE()

#endif		// LF_BRIO_MIDILOADER_H
	
// EOF
