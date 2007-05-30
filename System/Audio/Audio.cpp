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
#include <AudioMsg.h>
#include <AudioTypesPriv.h>

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
	type_ = kAudioCmdMsgTypeSetMasterVolume;
	messageSize = sizeof(CAudioMsgSetMasterVolume);
	messagePriority = kAudioMsgDefaultPriority;
	data_ = data;
}

///------------------------------------------------------------------------------
CAudioMsgStartAudio::CAudioMsgStartAudio( void ) {
	type_ = kAudioCmdMsgTypeStartAllAudio;
	messageSize = sizeof(CAudioMsgStartAudio);
	messagePriority = kAudioMsgDefaultPriority;
}

//------------------------------------------------------------------------------
CAudioMsgStopAudio::CAudioMsgStopAudio( void ) {
	type_ = kAudioCmdMsgTypeStopAllAudio;
	messageSize = sizeof(CAudioMsgStopAudio);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgStopAudio::CAudioMsgStopAudio( const tAudioStopAudioInfo& data ) {
	type_ = kAudioCmdMsgTypeStopAudio;
	messageSize = sizeof(CAudioMsgStopAudio);
	messagePriority = kAudioMsgDefaultPriority;
	data_ = data;
}

//------------------------------------------------------------------------------
CAudioMsgPauseAudio::CAudioMsgPauseAudio( void ) {
	type_ = kAudioCmdMsgTypePauseAllAudio;
	messageSize = sizeof(CAudioMsgPauseAudio);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgPauseAudio::CAudioMsgPauseAudio(  const tAudioPauseAudioInfo& data ) {
	type_ = kAudioCmdMsgTypePauseAllAudio;
	messageSize = sizeof(CAudioMsgPauseAudio);
	messagePriority = kAudioMsgDefaultPriority;
	data_ = data;
}

//------------------------------------------------------------------------------
CAudioMsgResumeAudio::CAudioMsgResumeAudio( void ) {
	type_ = kAudioCmdMsgTypeResumeAllAudio;
	messageSize = sizeof(CAudioMsgResumeAudio);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgResumeAudio::CAudioMsgResumeAudio( const tAudioResumeAudioInfo& data ) {
	type_ = kAudioCmdMsgTypeResumeAudio;
	messageSize = sizeof(CAudioMsgResumeAudio);
	messagePriority = kAudioMsgDefaultPriority;
	data_ = data;
}

//------------------------------------------------------------------------------
CAudioMsgPlayAudio::CAudioMsgPlayAudio( const tAudioPlayAudioInfo& data ) {
	type_ = kAudioCmdMsgTypePlayAudio;
	messageSize = sizeof(CAudioMsgPlayAudio);
	messagePriority = kAudioMsgDefaultPriority;
	data_ = data;
}

//------------------------------------------------------------------------------

CAudioMsgAcquireMidiPlayer::CAudioMsgAcquireMidiPlayer( void ) {
	type_ = kAudioCmdMsgTypeAcquireMidiPlayer;
	messageSize = sizeof(CAudioMsgAcquireMidiPlayer);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgMidiNoteOn::CAudioMsgMidiNoteOn( const tAudioMidiNoteInfo& data ) {
	type_ = kAudioCmdMsgTypeMidiNoteOn;
	messageSize = sizeof(CAudioMsgMidiNoteOn);
	messagePriority = kAudioMsgDefaultPriority;
	data_ = data;
}

CAudioMsgMidiNoteOff::CAudioMsgMidiNoteOff( const tAudioMidiNoteInfo& data ) {
	type_ = kAudioCmdMsgTypeMidiNoteOff;
	messageSize = sizeof(CAudioMsgMidiNoteOff);
	messagePriority = kAudioMsgDefaultPriority;
	data_ = data;
}

CAudioMsgPlayMidiFile::CAudioMsgPlayMidiFile( const tAudioMidiFileInfo& data ) {
	type_ = kAudioCmdMsgTypePlayMidiFile;
	messageSize = sizeof(CAudioMsgPlayMidiFile);
	messagePriority = kAudioMsgDefaultPriority;
	data_ = data;
}

//------------------------------------------------------------------------------
CAudioReturnMessage::CAudioReturnMessage( ) {
	messageSize = sizeof(CAudioReturnMessage);
	messagePriority = kAudioMsgDefaultPriority;
}

//==============================================================================
//==============================================================================
CAudioModule::CAudioModule( )
{

	tErrType	err = kNoErr;
	Boolean		ret = false;
	U32			msgSize;

	// Init listener.
	pDefaultListener_ = kNull;
	
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
		(const char *)"Kernel and Debug modules created by AudioModule\n");	
	
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
		(const char *)"Audio Module creating task incoming Q. size = %d\n", kMAX_AUDIO_MSG_SIZE );	
	
	err = KernelMPI->OpenMessageQueue( hSendMsgQueue_, msgQueueProperties, NULL );

    DebugMPI->Assert((kNoErr == err), "Trying to create incoming audio task msg queue. err = %d \n", err );

	// Now create a msg queue that allows the Audio Task to send messages back to us.
	msgQueueProperties.nameQueue = "/audioTaskOutgoingQ";
	msgQueueProperties.mq_msgsize = sizeof(CAudioReturnMessage);

	DebugMPI->DebugOut( kDbgLvlVerbose, 
		(const char *)"Audio Module creating task outgoing Q. size = %d\n", msgQueueProperties.mq_msgsize );	

	err = KernelMPI->OpenMessageQueue( hRecvMsgQueue_,  msgQueueProperties, NULL );

    DebugMPI->Assert((kNoErr == err), "Trying to create outgoing audio task msg queue. Err = %d \n", err );
}

//==============================================================================
//==============================================================================
CAudioModule::~CAudioModule(void)
{
	DebugMPI->DebugOut( kDbgLvlVerbose, 
		(const char *)"Audio Module dtor called\n" );	

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


//==============================================================================
//==============================================================================
tErrType CAudioModule::SetDefaultListener( const IEventListener* pListener )
{
	pDefaultListener_ = pListener;
	return kNoErr;
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


//==============================================================================
// 		Overall Audio System Control
//==============================================================================
tErrType CAudioModule::StartAudio( void )
{
	tErrType 			err = kNoErr;
    CAudioMsgStartAudio msg;

   	SendCmdMessage( msg ); 

	return WaitForStatus();
}

//----------------------------------------------------------------------------
tErrType CAudioModule::StopAudio( void )
{
	tErrType err = kNoErr;

    CAudioMsgStopAudio msg;

  	SendCmdMessage( msg ); 
	
	return WaitForStatus();
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
*/
//==============================================================================
//==============================================================================
void CAudioModule::SetMasterVolume( U8 volume )
{
	// Need to inform the audio mixer that the master volume has changed
	// Generate the command message to send to the audio Mgr task
	printf("AudioMgr:SetMasterVolume; volume = %d\n", volume);

	tAudioMasterVolume msgData;

	msgData.volume = volume;

	CAudioMsgSetMasterVolume	msg( msgData );
	
	// Send the message and wait to get the audioID back from the audio Mgr task
 	SendCmdMessage( msg ); 
}

/*
//==============================================================================
//==============================================================================
U8 CAudioModule::GetMasterVolume()
{
	return masterVolume;
}
*/
//==============================================================================
//==============================================================================
tAudioID CAudioModule::PlayAudio( tRsrcHndl hRsrc, U8 volume,  
	tAudioPriority priority, S8 pan, IEventListener *pListener, tAudioPayload payload, 
	tAudioOptionsFlags flags )
{
	// Generate the command message to send to the audio Mgr task
	printf("AudioMgr:PlayAudio; hRsrc = 0x%x\n", hRsrc);

	tAudioPlayAudioInfo msgData;

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
}
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
static IEventListener* CAudioModule::GetAudioEventHandler(tAudioID audioID) 
{
	return NULL;
};

//==============================================================================
//==============================================================================
static void CAudioModule::SetAudioEventHandler(tAudioID audioID, IEventListener *pHandler) 
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
static IEventListener* CAudioModule::GetDefaultAudioEventHandler() 
{
	return NULL;
};

//==============================================================================
//==============================================================================
static void CAudioModule::SetDefaultAudioEventHandler(IEventListener *pHandler) 
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
}
*/
//==============================================================================
//==============================================================================
tErrType CAudioModule::AcquireMidiPlayer(tAudioPriority priority, IEventListener *pHandler, tMidiID *midiID)
{
	CAudioMsgAcquireMidiPlayer msg;

	// Send the message and wait to get the midiID back from the audio Mgr task
 	SendCmdMessage( msg ); 
	
 	*midiID = WaitForMidiID();
 	return kNoErr;
}
//==============================================================================
//==============================================================================
tErrType CAudioModule::ReleaseMidiPlayer(tMidiID midiID)
{
	return kNoErr;
}

//==============================================================================
//==============================================================================
tMidiID CAudioModule::GetMidiIDForAudioID(tAudioID audioID)
{
	return 0;
}

//==============================================================================
//==============================================================================
tAudioID CAudioModule::GetAudioIDForMidiID(tMidiID midiID)
{
	return 0;
}

//==============================================================================
//==============================================================================
void CAudioModule::StopMidiPlayer(tMidiID midiID, Boolean surpressDoneMessage)
{
}

//==============================================================================
//==============================================================================
void CAudioModule::PauseMidiPlayer(tMidiID midiID)
{
}

//==============================================================================
//==============================================================================
void CAudioModule::ResumeMidiPlayer(tMidiID midiID)
{
}

//==============================================================================
//==============================================================================
tMidiTrackBitMask CAudioModule::GetEnabledMidiTracks(tMidiID midiID )
{
	return 0;
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::EnableMidiTracks(tMidiID midiID, tMidiTrackBitMask trackBitMask)
{
	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::TransposeMidiTracks(tMidiID midiID, tMidiTrackBitMask tracktBitMask, S8 transposeAmount)
{
	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::ChangeMidiInstrument(tMidiID midiID, tMidiTrackBitMask trackBitMask, tMidiInstr instr)
{
	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::ChangeMidiTempo(tMidiID midiID, S8 tempo) 
{
	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::SendMidiCommand(tMidiID midiID, U8 cmd, U8 data1, U8 data2)
{
	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::MidiNoteOn( tMidiID midiID, U8 channel, U8 noteNum, U8 velocity, 
										tAudioOptionsFlags flags )
{
	tAudioMidiNoteInfo info;
	
	info.channel = channel;
	info.noteNum = noteNum;
	info.velocity = velocity;
//	info.priority = kAudioDefaultPriority;
	info.flags = flags;
	
	CAudioMsgMidiNoteOn	msg( info );
	
	SendCmdMessage( msg );
	
	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::MidiNoteOff( tMidiID midiID, U8 channel, U8 noteNum, U8 velocity, 
										tAudioOptionsFlags flags )
{
	tAudioMidiNoteInfo info;
	
	info.channel = channel;
	info.noteNum = noteNum;
	info.velocity = velocity;
//	info.priority = kAudioDefaultPriority;
	info.flags = flags;
	
	CAudioMsgMidiNoteOff msg( info );
	
	SendCmdMessage( msg );
	
	return kNoErr;
}

tAudioID CAudioModule::PlayMidiFile( tMidiID	midiID,
						tRsrcHndl			hRsrc, 
						U8					volume, 
						tAudioPriority		priority,
						IEventListener*		pListener,
						tAudioPayload		payload,
						tAudioOptionsFlags	flags )
{
	tAudioMidiFileInfo info;
	
	info.midiID = midiID;
	info.hRsrc = hRsrc;
	info.volume = volume;
	info.priority = priority;
	info.pListener = pListener;
	info.priority = priority;
	info.payload = payload;
	info.flags = flags;
	
	CAudioMsgPlayMidiFile msg( info );
	
	SendCmdMessage( msg );
	
	return kNoErr;
}

//==============================================================================
//==============================================================================
void CAudioModule::SendCmdMessage( CAudioCmdMsg& msg ) 
{
	tErrType err;
	
	DebugMPI->DebugOut( kDbgLvlVerbose, 
		(const char *)"CAudioModule::SendCmdMessage -- Sending message to audio task. size = %d; type = %d\n", 
							msg.GetMessageSize(), msg.GetCmdType());	
	
    err = KernelMPI->SendMessage( hSendMsgQueue_, msg );
    DebugMPI->Assert((kNoErr == err), "CAudioModule::SendCmdMessage -- After call SendMessage err = %d \n", err );
}

//==============================================================================
//==============================================================================
tAudioID CAudioModule::WaitForAudioID( void ) 
{
	tErrType 				err;
	char 					msgBuf[sizeof(CAudioReturnMessage)];
	CAudioReturnMessage* 	msg;

	DebugMPI->DebugOut( kDbgLvlVerbose, 
			(const char *)"CAudioModule::WaitForAudioID -- Waiting for message from audio task.\n" );	

	err = KernelMPI->ReceiveMessage( hRecvMsgQueue_,  (CMessage*)msgBuf, sizeof(CAudioReturnMessage) );
	DebugMPI->DebugOut(kDbgLvlVerbose, "Audio Task: ReceivedMessage err = % d\n", err);
	
	msg = reinterpret_cast<CAudioReturnMessage*>(msgBuf);

	DebugMPI->DebugOut(kDbgLvlVerbose, "CAudioModule::WaitForAudioID -- Got ID = %d \n",
	    	  msg->GetAudioID() );  
	    	  
	return msg->GetAudioID();
}

//==============================================================================
//==============================================================================
tMidiID CAudioModule::WaitForMidiID( void ) 
{
	tErrType 				err;
	char 					msgBuf[sizeof(CAudioReturnMessage)];
	CAudioReturnMessage* 	msg;

	DebugMPI->DebugOut( kDbgLvlVerbose, 
			(const char *)"CAudioModule::WaitForAudioID -- Waiting for message from audio task.\n" );	

	err = KernelMPI->ReceiveMessage( hRecvMsgQueue_,  (CMessage*)msgBuf, sizeof(CAudioReturnMessage) );
	DebugMPI->DebugOut(kDbgLvlVerbose, "Audio Task: ReceivedMessage err = % d\n", err);
	
	msg = reinterpret_cast<CAudioReturnMessage*>(msgBuf);

	DebugMPI->DebugOut(kDbgLvlVerbose, "CAudioModule::WaitForMidiID -- Got ID = %d \n",
	    	  msg->GetMidiID() );  
	    	  
	return msg->GetMidiID();
}

//==============================================================================
//==============================================================================
tAudioID CAudioModule::WaitForStatus( void ) 
{
	tErrType 				err;
	char 					msgBuf[sizeof(CAudioReturnMessage)];
	CAudioReturnMessage* 	msg;

	DebugMPI->DebugOut( kDbgLvlVerbose, 
			(const char *)"CAudioModule::WaitForStatus -- Waiting for message from audio task.\n" );	

	err = KernelMPI->ReceiveMessage( hRecvMsgQueue_,  (CMessage*)msgBuf, sizeof(CAudioReturnMessage) );
	DebugMPI->DebugOut(kDbgLvlVerbose, "CAudioModule::WaitForStatus -- ReceivedMessage err = % d\n", err);
	
	msg = reinterpret_cast<CAudioReturnMessage*>(msgBuf);

	DebugMPI->DebugOut(kDbgLvlVerbose, "CAudioModule::WaitForStatus -- Got status = %d \n",
	    	  msg->GetAudioErr() );  
		    	  
	return msg->GetAudioErr();
}




// EOF

