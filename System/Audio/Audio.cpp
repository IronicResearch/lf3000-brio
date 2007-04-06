//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		Audio.cpp
//
// Description:
//		Underlying implementation code for the Audio Manager module.
//
//==============================================================================
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <EventMPI.h>
#include <AudioTypes.h>
#include <AudioPriv.h>

#include <AudioOutput.h>
#include <EventListener.h>

/*
#include <AudioMsg.h>
#include <AudioPlayer.h>

extern void AudioMgrTask(UNSIGNED argc, VOID *argv);
extern void AudioCodecTask(UNSIGNED argc, VOID *argv);

static void * GetMessageBlock(tAudioCmdMsgType cmdMsgType); 
static void SendCmdMessage(void* pMsg);
static tAudioID WaitForAudioID(CAudioMgrMPIImpl *pImpl); 

*/
//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Global variables
//==============================================================================

const CURI	kModuleURI	= "/Somewhere/AudioModule";

// single instance of module object.
static CAudioModule*	sinst = NULL;

//==============================================================================
//==============================================================================
CAudioModule::CAudioModule( )
	: mpDefaultListener(NULL)
{

	tErrType	err = kNoErr;
	Boolean		ret = false;
	U32			msgSize;
	void		*mgrTaskMemory, *codecTaskMemory, *queMemory;

	// Get Kernel MPI
	KernelMPI =  new CKernelMPI();
	ret = KernelMPI->IsValid();
//	if (ret != true)
//		printf("AudioModule -- Couldn't create KernelMPI!\n");

	// Get Debug MPI
	DebugMPI =  new CDebugMPI();
	ret = DebugMPI->IsValid();
//	if (ret != true)
//		printf("AudioModule -- Couldn't create DebugMPI!\n");
	
	DebugMPI->DebugOut((tDebugSignature)1, kDbgLvlValuable, 
		(const char *)"\nKernel and Debug modules created by AudioMPI\n");	
	
	// Hard code the configuration resource
	numMixerChannels = 	kAudioNumMixerChannels;
	sampleRate =			kAudioSampleRate;

	// Set the output buffer sizes
	outSizeInWords = kAudioOutBufSizeInWords;
	outSizeInBytes = kAudioOutBufSizeInBytes;

	// Allocate the global audio mixer
//	if ((pAudioMixer = new CAudioMixer()) == kNull)
//	{
//		err = kAllocMPIErr;
//		goto ReturnErr;
//	}

	// Initialize the audio out driver
	audioOutIndex = 0;
	audioOutputOn = false;
	InitAudioOutput();

/*

	// Set the global audio effects processor to kNull
	//pAudioEffects = kNull;

	// Initialize the linked list of audio players
	pNode = kNull;

	// Initialize the next audioID
	nextAudioID = 0;

	// Create the audio event group
	if (NU_Create_Event_Group(&audioEvents, "AudioEv") != NU_SUCCESS)
	{
		err = kAudioCreateEventErr;
		goto ReturnErr;
	}

	// Create the main audio manager task
	audioMgrTaskReady = 0;
	NU_Allocate_Memory(&System_Memory, &mgrTaskMemory, kAudioMgrTaskSize, NU_NO_SUSPEND);
	if (NU_Create_Task(&audioMgrTask, "AudMgr", AudioMgrTask, 0, NU_NULL,
		mgrTaskMemory, kAudioMgrTaskSize, kAudioMgrTaskPriority, 10, NU_PREEMPT, NU_START) != NU_SUCCESS)
	{
		err = kAudioCreateTaskErr;
		goto ReturnErr;
	}

	// Create the audio codec task
	NU_Allocate_Memory(&System_Memory, &codecTaskMemory, kAudioCodecTaskSize, NU_NO_SUSPEND);
	if (NU_Create_Task(&audioCodecTask, "AudCod", AudioCodecTask, 0, NU_NULL,
		codecTaskMemory, kAudioCodecTaskSize, kAudioCodecTaskPriority, 10, NU_PREEMPT, NU_START) != NU_SUCCESS)
	{
		err = kAudioCreateTaskErr;
		goto ReturnErr;
	}

	// Wait for the AudioMgr task to be ready
	while (!audioMgrTaskReady)
	{
		NU_Sleep(5);
	}

	// Create the return Queue (for sending the audioID)
	msgSize = sizeof(tAudioRtnMsg)/sizeof(UNSIGNED);
	NU_Allocate_Memory(&System_Memory, &queMemory, sizeof(tAudioRtnMsg), NU_NO_SUSPEND);
	if (NU_Create_Queue(&pImpl->returnQueue, "AudRQue", queMemory, msgSize,
		NU_FIXED_SIZE, msgSize, NU_FIFO) != NU_SUCCESS)
	{
		err = kAudioCreateQueueErr;
		goto ReturnErr;
	}
*/	
}

//==============================================================================
//==============================================================================
CAudioModule::~CAudioModule(void)
{
	// Exit nicely
	if ( audioOutputOn )
		StopAudio();
		
	DeInitAudioOutput();
/*
	// Return error if global context isn't set
	if (gAudioContext == kNull)
	{
		return kAudioNullContextErr;
	}
	// Don't bother with deleting global variables

	// Delete the return Queue
	NU_Delete_Queue(&pImpl->returnQueue);
	return kNoErr;
*/
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::SetDefaultListener( const IEventListener* pListener )
{
	mpDefaultListener = pListener;
	return kNoErr;
}


//============================================================================
// Informational functions
//============================================================================
Boolean CAudioModule::IsValid() const
{
	return (sinst != NULL) ? true : false; 
}

//----------------------------------------------------------------------------
tErrType CAudioModule::GetModuleVersion(tVersion &version) const
{
	version = kAudioModuleVersion;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CAudioModule::GetModuleName(ConstPtrCString &pName) const
{
	pName = &kAudioModuleName;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CAudioModule::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	pURI = &kModuleURI;
	return kNoErr;
}

//==============================================================================
// 		Overall Audio System Control
//==============================================================================
tErrType CAudioModule::StartAudio( void )
{
	tErrType err = kNoErr;
	
	if (!audioOutputOn) {
		audioOutputOn = true;
		err = StartAudioOutput();
	}
	
	return err;
	
//	tAudioCmdMsgHeader*	pMsg;
//	pMsg = (tAudioCmdMsgHeader*)GetMessageBlock(kAudioCmdMsgTypeStopAllAudio);
//	SendCmdMessage(pMsg); 
}

//----------------------------------------------------------------------------
tErrType CAudioModule::StopAudio( void )
{
	tErrType err = kNoErr;
	
	if (audioOutputOn) {
		audioOutputOn = false;
		err = StopAudioOutput();
	}
	
	const tEventPriority	kPriorityTBD = 0;
	tAudioMsgDataCompleted	data;
	data.audioID = 99;	// dummy
	data.payload = 101;	// dummy
	data.count = 1;

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, mpDefaultListener);
	
	return err;
	
	
//	tAudioCmdMsgHeader*	pMsg;
//	pMsg = (tAudioCmdMsgHeader*)GetMessageBlock(kAudioCmdMsgTypeStopAllAudio);
//	SendCmdMessage(pMsg); 
}

//----------------------------------------------------------------------------
tErrType CAudioModule::PauseAudio( void )
{
	return kNoErr;
//	tAudioCmdMsgHeader*	pMsg;
//	pMsg = (tAudioCmdMsgHeader*)GetMessageBlock(kAudioCmdMsgTypePauseAllAudio);
//	SendCmdMessage(pMsg); 
}

//----------------------------------------------------------------------------
tErrType CAudioModule::ResumeAudio( void )
{
	return kNoErr;
//	tAudioCmdMsgHeader*	pMsg;
//	pMsg = (tAudioCmdMsgHeader*)GetMessageBlock(kAudioCmdMsgTypeResumeAllAudio);
//	SendCmdMessage(pMsg); 
}


//============================================================================
// Instance management interface for the Module Manager
//============================================================================
extern "C"
{
	//------------------------------------------------------------------------
	tVersion ReportVersion()
	{
		// TBD: ask modules for this or incorporate verion into so file name
		return kAudioModuleVersion;
	}
	
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion version)
	{
		if( sinst == NULL )
			sinst = new CAudioModule;
		return sinst;
	}
	
	//------------------------------------------------------------------------
	void DestroyInstance(ICoreModule* ptr)
	{
//		assert(ptr == sinst);
		delete sinst;
		sinst = NULL;
	}
}


/*
//==============================================================================
//==============================================================================
static tErrType RegisterGetStereoAudioStreamFcn(tRsrcType type, tGetStereoAudioStreamFcn pFcn)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tErrType RegisterAudioEffectsProcessor(tRsrcType type, CAudioEffectsProcessor *pChain)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tErrType RegisterGlobalAudioEffectsProcessor(CAudioEffectsProcessor *pChain)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tErrType ChangeAudioEffectsProcessor(tAudioID audioID, CAudioEffectsProcessor *pChain)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static void SetMasterVolume(U8 volume)
{
	// Set the master volume
	masterVolume = volume;

	// Need to inform the audio mixer that the master volume has changed
	tAudioCmdMsgSetMasterVolume*	pMsg;
	pMsg = (tAudioCmdMsgSetMasterVolume*)GetMessageBlock(kAudioCmdMsgTypeSetMasterVolume);
	pMsg->masterVolume = volume;
	SendCmdMessage(pMsg); 
};

//==============================================================================
//==============================================================================
static U8 GetMasterVolume()
{
	return masterVolume;
};

//==============================================================================
//==============================================================================
static tAudioID PlayAudio(CAudioMgrMPIImpl *pImpl, tRsrcHndl hRsrc, U8 volume,  
	tAudioPriority priority, S8 pan, IEventHandler *pHandler, tAudioPayload payload, 
	tAudioOptionsFlags flags)
{
	// Generate the command message to send to the audio Mgr task
	printf("AudioMgr:PlayAudio\n");
	tAudioCmdMsgPlayAudio*	pMsg;
	pMsg = (tAudioCmdMsgPlayAudio*)GetMessageBlock(kAudioCmdMsgTypePlayAudio);
	pMsg->hRsrc = hRsrc;
	pMsg->volume = volume;
	pMsg->priority = priority;
	pMsg->pan = pan;
	pMsg->pHandler = pHandler;
	pMsg->payload = payload;
	pMsg->flags = flags;
	pMsg->pReturnQueue = &pImpl->returnQueue;
	// Send the message and wait to get the audioID back from the audio Mgr task
	SendCmdMessage(pMsg); 
	return WaitForAudioID(pImpl);
};

//==============================================================================
//==============================================================================
static tAudioID PlayAudioDefault(CAudioMgrMPIImpl *pImpl, tRsrcHndl hRsrc, 
	tAudioPayload payload, tAudioOptionsFlags flags)
{
	return 0;
};

//==============================================================================
//==============================================================================
static void StopAudio(tAudioID audioID, Boolean surpressDoneMessage)
{
	tAudioCmdMsgStopAudio*	pMsg;
	pMsg = (tAudioCmdMsgStopAudio*)GetMessageBlock(kAudioCmdMsgTypeStopAudio);
	pMsg->audioID = audioID;
	pMsg->suppressDoneMsg = surpressDoneMessage;
	SendCmdMessage(pMsg); 
};

//==============================================================================
//==============================================================================
static void PauseAudio(tAudioID audioID)
{
	tAudioCmdMsgPauseAudio*	pMsg;
	pMsg = (tAudioCmdMsgPauseAudio*)GetMessageBlock(kAudioCmdMsgTypePauseAudio);
	pMsg->audioID = audioID;
	SendCmdMessage(pMsg); 
};

//==============================================================================
//==============================================================================
static void ResumeAudio(tAudioID audioID)
{
	tAudioCmdMsgResumeAudio*	pMsg;
	pMsg = (tAudioCmdMsgResumeAudio*)GetMessageBlock(kAudioCmdMsgTypeResumeAudio);
	pMsg->audioID = audioID;
	SendCmdMessage(pMsg); 
};

//==============================================================================
//==============================================================================
static U8 GetAudioVolume(tAudioID audioID) 
{
	return 0;
};

//==============================================================================
//==============================================================================
static void SetAudioVolume(tAudioID audioID, U8 volume) 
{
};

//==============================================================================
//==============================================================================
static tAudioPriority GetAudioPriority(tAudioID audioID) 
{
	return 0;
};

//==============================================================================
//==============================================================================
static void SetAudioPriority(tAudioID audioID, tAudioPriority priority) 
{
};

//==============================================================================
//==============================================================================
static S8 GetAudioPan(tAudioID audioID) 
{
	return 0;
};

//==============================================================================
//==============================================================================
static void SetAudioPan(tAudioID audioID, S8 pan) 
{
};

//==============================================================================
//==============================================================================
static IEventHandler* GetAudioEventHandler(tAudioID audioID) 
{
	return NULL;
};

//==============================================================================
//==============================================================================
static void SetAudioEventHandler(tAudioID audioID, IEventHandler *pHandler) 
{
};

//==============================================================================
//==============================================================================
static U8 GetDefaultAudioVolume() 
{
	return 0;
};

//==============================================================================
//==============================================================================
static void SetDefaultAudioVolume(U8 volume) 
{
};

//==============================================================================
//==============================================================================
static tAudioPriority GetDefaultAudioPriority() 
{
	return 0;
};

//==============================================================================
//==============================================================================
static void SetDefaultAudioPriority(tAudioPriority priority) 
{
};

//==============================================================================
//==============================================================================
static S8 GetDefaultAudioPan() 
{
	return 0;
};

//==============================================================================
//==============================================================================
static void SetDefaultAudioPan(S8 pan) 
{
};

//==============================================================================
//==============================================================================
static IEventHandler* GetDefaultAudioEventHandler() 
{
	return NULL;
};

//==============================================================================
//==============================================================================
static void SetDefaultAudioEventHandler(IEventHandler *pHandler) 
{
};

//==============================================================================
//==============================================================================
static Boolean IsAudioBusy(tAudioID audioID)
{
	return false;
};

//==============================================================================
//==============================================================================
static Boolean IsAnyAudioBusy()
{
	return false;
};

//==============================================================================
//==============================================================================
static tErrType AcquireMidiPlayer(tAudioPriority priority, IEventHandler *pHandler, tMidiID *midiID)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tErrType ReleaseMidiPlayer(tMidiID midiID)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tMidiID GetMidiIDForAudioID(tAudioID audioID)
{
	return 0;
};

//==============================================================================
//==============================================================================
static tAudioID GetAudioIDForMidiID(tMidiID midiID)
{
	return 0;
};

//==============================================================================
//==============================================================================
static void StopMidiPlayer(tMidiID midiID, Boolean surpressDoneMessage)
{
};

//==============================================================================
//==============================================================================
static void PauseMidiPlayer(tMidiID midiID)
{
};

//==============================================================================
//==============================================================================
static void ResumeMidiPlayer(tMidiID midiID)
{
};

//==============================================================================
//==============================================================================
static tMidiTrackBitMask GetEnabledMidiTracks(tMidiID midiID )
{
	return 0;
};

//==============================================================================
//==============================================================================
static tErrType EnableMidiTracks(tMidiID midiID, tMidiTrackBitMask trackBitMask)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tErrType TransposeMidiTracks(tMidiID midiID, tMidiTrackBitMask tracktBitMask, S8 transposeAmount)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tErrType ChangeMidiInstrument(tMidiID midiID, tMidiTrackBitMask trackBitMask, tMidiInstr instr)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tErrType ChangeMidiTempo(tMidiID midiID, S8 tempo) 
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tErrType SendMidiCommand(tMidiID midiID, U8 cmd, U8 data1, U8 data2)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tErrType PlayMidiNote(tMidiID	midiID, U8 track, U8 pitch, U8 velocity, 
	U16 noteCount, tAudioOptionsFlags flags)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static void* GetMessageBlock(tAudioCmdMsgType cmdMsgType) 
{
	// Eventually we'll allocate the message from a special message pool
	// For now, we'll just use the non-cached memory pool
	void* 	pMsg;

	NU_Allocate_Memory(&System_Memory, &pMsg, sizeof(tAudioCmdMsg), NU_NO_SUSPEND);

	// Set the message type
	((tAudioCmdMsgHeader *)pMsg)->audioCmdMsgType = cmdMsgType;
	return pMsg;
};

//==============================================================================
//==============================================================================
static void SendCmdMessage(void* pMsg) 
{
	STATUS	status;

	printf("SendCmdMessage:%d, %d\n", sizeof(tAudioCmdMsg), sizeof(tAudioCmdMsg)/sizeof(UNSIGNED));
	status = NU_Send_To_Queue(&audioCmdQueue, pMsg, 
		sizeof(tAudioCmdMsg)/sizeof(UNSIGNED), NU_NO_SUSPEND);

	// Need to deallocate the message
	NU_Deallocate_Memory(pMsg);
};

//==============================================================================
//==============================================================================
static tAudioID WaitForAudioID(CAudioMgrMPIImpl *pImpl) 
{
	tAudioRtnMsg	msg;
	U32				msgSize;

	// Wait for the audio manager task to send back the audioID
	NU_printf("Wait for AudioID\n");
	NU_Receive_From_Queue(&pImpl->returnQueue, &msg, 
		sizeof(tAudioRtnMsg)/sizeof(UNSIGNED), &msgSize, NU_SUSPEND); 
	NU_printf("AudioID:%d\n",msg.audioID);
	return msg.audioID; 
};

*/



// EOF

