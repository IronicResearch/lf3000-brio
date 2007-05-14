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
#include <errno.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <EventMPI.h>
#include <AudioTypes.h>
#include <AudioPriv.h>
#include <AudioTask.h>

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Global variables
//==============================================================================

const CURI	kModuleURI	= "/Somewhere/AudioModule";

// single instance of module object.
static CAudioModule*	sinst = NULL;

// Globals that must be accessable to other modules for operation.
tMutex		gAudioConditionMutex;
tMutexAttr 	gAudioConditionMutexAttributes;
tCond		gAudioCondition;
tCondAttr 	gAudioConditionAttributes;
bool		gAudioTaskRunning = false;

//============================================================================
// CAudioCmdMessages
//============================================================================
//------------------------------------------------------------------------------
CAudioMsgSetMasterVolume::CAudioMsgSetMasterVolume( const tAudioMasterVolume& data ) {
	mType_ = kAudioCmdMsgTypeSetMasterVolume;
	messageSize = sizeof(CAudioMsgSetMasterVolume);
	messagePriority = kAudioMsgDefaultPriority;
	mData_ = data;
}

///------------------------------------------------------------------------------
CAudioMsgStartAudio::CAudioMsgStartAudio( void ) {
	mType_ = kAudioCmdMsgTypeStartAllAudio;
	messageSize = sizeof(CAudioMsgStartAudio);
	messagePriority = kAudioMsgDefaultPriority;
}

//------------------------------------------------------------------------------
CAudioMsgStopAudio::CAudioMsgStopAudio( void ) {
	mType_ = kAudioCmdMsgTypeStopAllAudio;
	messageSize = sizeof(CAudioMsgStopAudio);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgStopAudio::CAudioMsgStopAudio( const tAudioStopAudioData& data ) {
	mType_ = kAudioCmdMsgTypeStopAudio;
	messageSize = sizeof(CAudioMsgStopAudio);
	messagePriority = kAudioMsgDefaultPriority;
	mData_ = data;
}

//------------------------------------------------------------------------------
CAudioMsgPauseAudio::CAudioMsgPauseAudio( void ) {
	mType_ = kAudioCmdMsgTypePauseAllAudio;
	messageSize = sizeof(CAudioMsgPauseAudio);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgPauseAudio::CAudioMsgPauseAudio(  const tAudioPauseAudioData& data ) {
	mType_ = kAudioCmdMsgTypePauseAllAudio;
	messageSize = sizeof(CAudioMsgPauseAudio);
	messagePriority = kAudioMsgDefaultPriority;
	mData_ = data;
}

//------------------------------------------------------------------------------
CAudioMsgResumeAudio::CAudioMsgResumeAudio( void ) {
	mType_ = kAudioCmdMsgTypeResumeAllAudio;
	messageSize = sizeof(CAudioMsgResumeAudio);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgResumeAudio::CAudioMsgResumeAudio( const tAudioResumeAudioData& data ) {
	mType_ = kAudioCmdMsgTypeResumeAudio;
	messageSize = sizeof(CAudioMsgResumeAudio);
	messagePriority = kAudioMsgDefaultPriority;
	mData_ = data;
}

//------------------------------------------------------------------------------
CAudioMsgPlayAudio::CAudioMsgPlayAudio( const tAudioPlayAudioData& data ) {
	mType_ = kAudioCmdMsgTypeSetMasterVolume;
	messageSize = sizeof(CAudioMsgPlayAudio);
	messagePriority = kAudioMsgDefaultPriority;
	mData_ = data;
}

//------------------------------------------------------------------------------
CAudioReturnMessage::CAudioReturnMessage( tErrType err, tAudioID audioID ) {
	messageSize = sizeof(CAudioMsgPlayAudio);
	messagePriority = kAudioMsgDefaultPriority;
	mErr_ = err;
	mAudioID_ = audioID;
}

//==============================================================================
//==============================================================================
CAudioModule::CAudioModule( )
{

	tErrType	err = kNoErr;
	Boolean		ret = false;
	U32			msgSize;

	// Get Kernel MPI
	KernelMPI =  new CKernelMPI();
	ret = KernelMPI->IsValid();
//	if (ret != true)
//		printf("AudioModule -- Couldn't create KernelMPI!\n");

	// Get Debug MPI
	DebugMPI =  new CDebugMPI( kGroupAudio );
	ret = DebugMPI->IsValid();
//	if (ret != true)
//		printf("AudioModule -- Couldn't create DebugMPI!\n");

	// I want to see everything...
	DebugMPI->SetDebugLevel( kDbgLvlVerbose );
	
	DebugMPI->DebugOut(kDbgLvlVerbose, 
		(const char *)"\nKernel and Debug modules created by AudioModule\n");	
	
/*	
	err = KernelMPI->InitCondAttr( gAudioConditionAttributes );

	err = KernelMPI->InitMutex( gAudioConditionMutex, gAudioConditionMutexAttributes );
	DebugMPI.Assert((kNoErr == err), "Couldn't init audio condition variable mutex.\n");

	err = KernelMPI->InitCond( gAudioCondition, gAudioConditionAttributes );
	DebugMPI.Assert((kNoErr == err), "Couldn't init audio condition variable.\n");
*/

	// Create the Audio Task...
	err = InitAudioTask();
	DebugMPI->Assert((kNoErr == err), "Audio task create failed.\n");

	// Wait for the AudioMgr task to be ready
	while (!gAudioTaskRunning)
	{
		KernelMPI->TaskSleep(5);
	}

	DebugMPI->DebugOut(kDbgLvlVerbose, 
		(const char *)"AudioModule thinks the audio task is running now...\n");	
	
	// First, create a msg queue that allows the Audio Task to RECEIVE msgs from us.
	tMessageQueuePropertiesPosix msgQueueProperties = 
	{
	    0,                          // msgProperties.blockingPolicy;  
	    "/audioTaskIncomingQ",      // msgProperties.nameQueue
	    S_IRWXU,                    // msgProperties.mode 
	    O_RDWR|O_CREAT|O_TRUNC,     // msgProperties.oflag  
	    0,                          // msgProperties.priority
	    0,                          // msgProperties.mq_flags
	    8,                          // msgProperties.mq_maxmsg
	    kMAX_AUDIO_MSG_SIZE,  		// msgProperties.mq_msgsize
	    0                           // msgProperties.mq_curmsgs
	};
	
	DebugMPI->DebugOut( kDbgLvlVerbose, 
		(const char *)"\nAudio Module creating task incoming Q. size = %d\n", kMAX_AUDIO_MSG_SIZE );	
	
	err = KernelMPI->OpenMessageQueue( hSendMsgQueue_, msgQueueProperties, NULL );

    DebugMPI->Assert((kNoErr == err), "Trying to create incoming audio task msg queue. err = %d \n", err );

	// Now create a msg queue that allows the Audio Task to send messages back to us.
	msgQueueProperties.nameQueue = "/audioTaskOutgoingQ";
	msgQueueProperties.mq_msgsize = sizeof(CAudioReturnMessage);

	DebugMPI->DebugOut( kDbgLvlVerbose, 
		(const char *)"\nAudio Module creating task outgoing Q. size = %d\n", msgQueueProperties.mq_msgsize );	

	err = KernelMPI->OpenMessageQueue( hRecvMsgQueue_,  msgQueueProperties, NULL );

    DebugMPI->Assert((kNoErr == err), "Trying to create outgoing audio task msg queue. Err = %d \n", err );
}

//==============================================================================
//==============================================================================
CAudioModule::~CAudioModule(void)
{
	DebugMPI->DebugOut( kDbgLvlVerbose, 
		(const char *)"\nAudio Module dtor called\n" );	

	DeInitAudioTask();
	
	// Wait for the AudioMgr task to die
	while (gAudioTaskRunning)
	{
		KernelMPI->TaskSleep(5);
	}
	
//	KernelMPI->DestroyMessageQueue( hRecvMsgQueue_ )
//	KernelMPI->DestroyMessageQueue( hSendMsgQueue_ )
	
}

//============================================================================
// Informational functions
//============================================================================
Boolean CAudioModule::IsValid() const
{
	return (sinst != NULL) ? true : false; 
}

//----------------------------------------------------------------------------
tVersion CAudioModule::GetModuleVersion() const
{
	return kAudioModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CAudioModule::GetModuleName() const
{
	return &kAudioModuleName;
}

//----------------------------------------------------------------------------
const CURI* CAudioModule::GetModuleOrigin() const
{
	return &kModuleURI;
}

/*
//==============================================================================
//==============================================================================
tErrType CAudioModule::SetDefaultListener( const IEventListener* pListener )
{
	mpDefaultListener = pListener;
	return kNoErr;
}
*/

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


//==============================================================================
// 		Overall Audio System Control
//==============================================================================
tErrType CAudioModule::StartAudio( void )
{
	tErrType err = kNoErr;
 
    CAudioMsgStartAudio msg;

   	SendCmdMessage( msg ); 

	return err;
}

//----------------------------------------------------------------------------
tErrType CAudioModule::StopAudio( void )
{
	tErrType err = kNoErr;

    CAudioMsgStopAudio msg;

  	SendCmdMessage( msg ); 
	
	return err;
}

//----------------------------------------------------------------------------
tErrType CAudioModule::PauseAudio( void )
{
	tErrType err = kNoErr;

    CAudioMsgPauseAudio msg;

	SendCmdMessage( msg ); 
	
	return err;
}

//----------------------------------------------------------------------------
tErrType CAudioModule::ResumeAudio( void )
{
	tErrType err = kNoErr;

    CAudioMsgResumeAudio msg;

 	SendCmdMessage( msg ); 
	
	return err;
}

/*
//==============================================================================
//==============================================================================
static tErrType CAudioModule::RegisterGetStereoAudioStreamFcn(tRsrcType type, tGetStereoAudioStreamFcn pFcn)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tErrType CAudioModule::RegisterAudioEffectsProcessor(tRsrcType type, CAudioEffectsProcessor *pChain)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tErrType CAudioModule::RegisterGlobalAudioEffectsProcessor(CAudioEffectsProcessor *pChain)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tErrType CAudioModule::ChangeAudioEffectsProcessor(tAudioID audioID, CAudioEffectsProcessor *pChain)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static void CAudioModule::SetMasterVolume(U8 volume)
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
static U8 CAudioModule::GetMasterVolume()
{
	return masterVolume;
};

*/
//==============================================================================
//==============================================================================
tAudioID CAudioModule::PlayAudio( tRsrcHndl hRsrc, U8 volume,  
	tAudioPriority priority, S8 pan, IEventListener *pListener, tAudioPayload payload, 
	tAudioOptionsFlags flags)
{
	// Generate the command message to send to the audio Mgr task
	printf("AudioMgr:PlayAudio\n");
	tAudioPlayAudioData msgData;

	msgData.hRsrc = hRsrc;
	msgData.volume = volume;
	msgData.priority = priority;
	msgData.pan = pan;
	msgData.pListener = pListener;
	msgData.payload = payload;
	msgData.flags = flags;

	CAudioMsgPlayAudio	msg( msgData );
	
	// Send the message and wait to get the audioID back from the audio Mgr task
 	SendCmdMessage( msg ); 
	
	return WaitForAudioID();
};
/*
//==============================================================================
//==============================================================================
static tAudioID CAudioModule::PlayAudioDefault(CAudioMgrMPIImpl *pImpl, tRsrcHndl hRsrc, 
	tAudioPayload payload, tAudioOptionsFlags flags)
{
	return 0;
};

//==============================================================================
//==============================================================================
static void CAudioModule::StopAudio(tAudioID audioID, Boolean surpressDoneMessage)
{
	tAudioCmdMsgStopAudio*	pMsg;
	pMsg = (tAudioCmdMsgStopAudio*)GetMessageBlock(kAudioCmdMsgTypeStopAudio);
	pMsg->audioID = audioID;
	pMsg->suppressDoneMsg = surpressDoneMessage;
	SendCmdMessage(pMsg); 
};

//==============================================================================
//==============================================================================
static void CAudioModule::PauseAudio(tAudioID audioID)
{
	tAudioCmdMsgPauseAudio*	pMsg;
	pMsg = (tAudioCmdMsgPauseAudio*)GetMessageBlock(kAudioCmdMsgTypePauseAudio);
	pMsg->audioID = audioID;
	SendCmdMessage(pMsg); 
};

//==============================================================================
//==============================================================================
static void CAudioModule::ResumeAudio(tAudioID audioID)
{
	tAudioCmdMsgResumeAudio*	pMsg;
	pMsg = (tAudioCmdMsgResumeAudio*)GetMessageBlock(kAudioCmdMsgTypeResumeAudio);
	pMsg->audioID = audioID;
	SendCmdMessage(pMsg); 
};

//==============================================================================
//==============================================================================
static U8 CAudioModule::GetAudioVolume(tAudioID audioID) 
{
	return 0;
};

//==============================================================================
//==============================================================================
static void CAudioModule::SetAudioVolume(tAudioID audioID, U8 volume) 
{
};

//==============================================================================
//==============================================================================
static tAudioPriority CAudioModule::GetAudioPriority(tAudioID audioID) 
{
	return 0;
};

//==============================================================================
//==============================================================================
static void CAudioModule::SetAudioPriority(tAudioID audioID, tAudioPriority priority) 
{
};

//==============================================================================
//==============================================================================
static S8 CAudioModule::GetAudioPan(tAudioID audioID) 
{
	return 0;
};

//==============================================================================
//==============================================================================
static void CAudioModule::SetAudioPan(tAudioID audioID, S8 pan) 
{
};

//==============================================================================
//==============================================================================
static IEventHandler* CAudioModule::GetAudioEventHandler(tAudioID audioID) 
{
	return NULL;
};

//==============================================================================
//==============================================================================
static void CAudioModule::SetAudioEventHandler(tAudioID audioID, IEventHandler *pHandler) 
{
};

//==============================================================================
//==============================================================================
static U8 CAudioModule::GetDefaultAudioVolume() 
{
	return 0;
};

//==============================================================================
//==============================================================================
static void CAudioModule::SetDefaultAudioVolume(U8 volume) 
{
};

//==============================================================================
//==============================================================================
static tAudioPriority CAudioModule::GetDefaultAudioPriority() 
{
	return 0;
};

//==============================================================================
//==============================================================================
static void CAudioModule::SetDefaultAudioPriority(tAudioPriority priority) 
{
};

//==============================================================================
//==============================================================================
static S8 CAudioModule::GetDefaultAudioPan() 
{
	return 0;
};

//==============================================================================
//==============================================================================
static void CAudioModule::SetDefaultAudioPan(S8 pan) 
{
};

//==============================================================================
//==============================================================================
static IEventHandler* CAudioModule::GetDefaultAudioEventHandler() 
{
	return NULL;
};

//==============================================================================
//==============================================================================
static void CAudioModule::SetDefaultAudioEventHandler(IEventHandler *pHandler) 
{
};

//==============================================================================
//==============================================================================
static Boolean CAudioModule::IsAudioBusy(tAudioID audioID)
{
	return false;
};

//==============================================================================
//==============================================================================
static Boolean CAudioModule::IsAnyAudioBusy()
{
	return false;
};

//==============================================================================
//==============================================================================
static tErrType CAudioModule::AcquireMidiPlayer(tAudioPriority priority, IEventHandler *pHandler, tMidiID *midiID)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tErrType CAudioModule::ReleaseMidiPlayer(tMidiID midiID)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tMidiID CAudioModule::GetMidiIDForAudioID(tAudioID audioID)
{
	return 0;
};

//==============================================================================
//==============================================================================
static tAudioID CAudioModule::GetAudioIDForMidiID(tMidiID midiID)
{
	return 0;
};

//==============================================================================
//==============================================================================
static void CAudioModule::StopMidiPlayer(tMidiID midiID, Boolean surpressDoneMessage)
{
};

//==============================================================================
//==============================================================================
static void CAudioModule::PauseMidiPlayer(tMidiID midiID)
{
};

//==============================================================================
//==============================================================================
static void CAudioModule::ResumeMidiPlayer(tMidiID midiID)
{
};

//==============================================================================
//==============================================================================
static tMidiTrackBitMask CAudioModule::GetEnabledMidiTracks(tMidiID midiID )
{
	return 0;
};

//==============================================================================
//==============================================================================
static tErrType CAudioModule::EnableMidiTracks(tMidiID midiID, tMidiTrackBitMask trackBitMask)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tErrType CAudioModule::TransposeMidiTracks(tMidiID midiID, tMidiTrackBitMask tracktBitMask, S8 transposeAmount)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tErrType CAudioModule::ChangeMidiInstrument(tMidiID midiID, tMidiTrackBitMask trackBitMask, tMidiInstr instr)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tErrType CAudioModule::ChangeMidiTempo(tMidiID midiID, S8 tempo) 
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tErrType CAudioModule::SendMidiCommand(tMidiID midiID, U8 cmd, U8 data1, U8 data2)
{
	return kNoErr;
};

//==============================================================================
//==============================================================================
static tErrType CAudioModule::PlayMidiNote(tMidiID	midiID, U8 track, U8 pitch, U8 velocity, 
	U16 noteCount, tAudioOptionsFlags flags)
{
	return kNoErr;
};

*/

//==============================================================================
//==============================================================================
void CAudioModule::SendCmdMessage( CAudioMsg& msg ) 
{
	tErrType err;
	
	DebugMPI->DebugOut( kDbgLvlVerbose, 
		(const char *)"\nSending message to audio task. size = %d; type = %d\n", 
							msg.GetMessageSize(), msg.GetMessageCmdType());	
	
    err = KernelMPI->SendMessage( hSendMsgQueue_, msg );
    DebugMPI->Assert((kNoErr == err), "After call SendMessage err = %d \n", err );
}

//==============================================================================
//==============================================================================
tAudioID CAudioModule::WaitForAudioID( void ) 
{
	tErrType err;
	
	CAudioReturnMessage* message;

	err = KernelMPI->ReceiveMessage( hRecvMsgQueue_,  message, sizeof(CAudioReturnMessage) );
								   
	DebugMPI->Assert((kNoErr == err), "ReceiveMessage errReceive= % d EventType = %d \n",
	    	  err, message->GetMessageAudioID() );  
	    	  
	return message->GetMessageAudioID();
}





// EOF

