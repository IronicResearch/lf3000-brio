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
CAudioMsgStartAudio::CAudioMsgStartAudio( const tAudioStartAudioInfo& data )
	: data_(data)
{
	type_ = kAudioCmdMsgTypeStartAudio;
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
	type_ = kAudioCmdMsgTypePauseAudio;
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

CAudioMsgAcquireMidiPlayer::CAudioMsgAcquireMidiPlayer( void ) {
	type_ = kAudioCmdMsgTypeAcquireMidiPlayer;
	messageSize = sizeof(CAudioMsgAcquireMidiPlayer);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgReleaseMidiPlayer::CAudioMsgReleaseMidiPlayer( void ) {
	type_ = kAudioCmdMsgTypeReleaseMidiPlayer;
	messageSize = sizeof(CAudioMsgReleaseMidiPlayer);
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

CAudioMsgStartMidiFile::CAudioMsgStartMidiFile( const tAudioStartMidiFileInfo& data ) {
	type_ = kAudioCmdMsgTypeStartMidiFile;
	messageSize = sizeof(CAudioMsgStartMidiFile);
	messagePriority = kAudioMsgDefaultPriority;
	data_ = data;
}

CAudioMsgPauseMidiFile::CAudioMsgPauseMidiFile( const tAudioPauseMidiFileInfo& data ) {
	type_ = kAudioCmdMsgTypePauseMidiFile;
	messageSize = sizeof(CAudioMsgPauseMidiFile);
	messagePriority = kAudioMsgDefaultPriority;
	data_ = data;
}

CAudioMsgResumeMidiFile::CAudioMsgResumeMidiFile( const tAudioResumeMidiFileInfo& data ) {
	type_ = kAudioCmdMsgTypeResumeMidiFile;
	messageSize = sizeof(CAudioMsgResumeMidiFile);
	messagePriority = kAudioMsgDefaultPriority;
	data_ = data;
}

CAudioMsgStopMidiFile::CAudioMsgStopMidiFile( const tAudioStopMidiFileInfo& data ) {
	type_ = kAudioCmdMsgTypeStopMidiFile;
	messageSize = sizeof(CAudioMsgStopMidiFile);
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

	// Init listener.
	pDefaultListener_ = kNull;
	
	// Get Kernel MPI
	pKernelMPI_ =  new CKernelMPI();
	ret = pKernelMPI_->IsValid();
//	if (ret != true)
//		printf("AudioModule -- Couldn't create KernelMPI!\n");

	// Get Debug MPI
	pDebugMPI_ =  new CDebugMPI( kGroupAudio );
	ret = pDebugMPI_->IsValid();
//	if (ret != true)
//		printf("AudioModule -- Couldn't create DebugMPI!\n");

	// I want to see everything...
	//	pDebugMPI_->SetDebugLevel( kDbgLvlVerbose );
	pDebugMPI_->SetDebugLevel( kDbgLvlImportant );
		
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"Kernel and Debug modules created by AudioModule\n");	
	
/*	
	err = pKernelMPI_->InitCondAttr( gAudioConditionAttributes );

	err = pKernelMPI_->InitMutex( gAudioConditionMutex, gAudioConditionMutexAttributes );
	pDebugMPI_.Assert((kNoErr == err), "Couldn't init audio condition variable mutex.\n");

	err = pKernelMPI_->InitCond( gAudioCondition, gAudioConditionAttributes );
	pDebugMPI_.Assert((kNoErr == err), "Couldn't init audio condition variable.\n");
*/

	// Create the Audio Task...
	err = InitAudioTask();
	pDebugMPI_->Assert((kNoErr == err), "CAudioModule::ctor: Audio task create failed.\n");

	// Wait for the AudioMgr task to be ready
	while (!gAudioTaskRunning)
	{
		pKernelMPI_->TaskSleep(5);
	}

	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CAudioModule::ctor: AudioModule thinks the audio task is running now...\n");	
	
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
	
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::ctor: creating task incoming Q. size = %d\n", 
				static_cast<int>(kMAX_AUDIO_MSG_SIZE) );	
	
	err = pKernelMPI_->OpenMessageQueue( hSendMsgQueue_, msgQueueProperties, NULL );

    pDebugMPI_->AssertNoErr(err, "CAudioModule::ctor:Trying to create incoming audio task msg queue.\n");

	// Now create a msg queue that allows the Audio Task to send messages back to us.
	msgQueueProperties.nameQueue = "/audioTaskOutgoingQ";
	msgQueueProperties.mq_msgsize = sizeof(CAudioReturnMessage);

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::ctor: creating task outgoing Q. size = %u\n", 
				static_cast<unsigned int>(msgQueueProperties.mq_msgsize) );	

	err = pKernelMPI_->OpenMessageQueue( hRecvMsgQueue_,  msgQueueProperties, NULL );

    pDebugMPI_->AssertNoErr(err, "CAudioModule::ctor: Trying to create outgoing audio task msg queue.\n" );
}

//==============================================================================
//==============================================================================
CAudioModule::~CAudioModule(void)
{
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::dtor: dtor called\n" );	

	DeInitAudioTask();
	
	// Wait for the AudioMgr task to die
	while (gAudioTaskRunning)
	{
		pKernelMPI_->TaskSleep(5);
	}
	
//	pKernelMPI_->DestroyMessageQueue( hRecvMsgQueue_ )
//	pKernelMPI_->DestroyMessageQueue( hSendMsgQueue_ )
	
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
#ifndef LF_MONOLITHIC_DEBUG
extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion /*version*/)
	{
		if( sinst == NULL )
			sinst = new CAudioModule;
		return sinst;
	}
	
	//------------------------------------------------------------------------
	void DestroyInstance(ICoreModule* /*ptr*/)
	{
//		assert(ptr == sinst);
		delete sinst;
		sinst = NULL;
	}
}
#endif	// LF_MONOLITHIC_DEBUG



//==============================================================================
// 		Overall Audio System Control
//==============================================================================
tErrType CAudioModule::StartAudio( void )
{
    CAudioMsgStartAudio msg;

   	SendCmdMessage( msg ); 

	return WaitForStatus();
}

//----------------------------------------------------------------------------
tErrType CAudioModule::StopAudio( void )
{
    CAudioMsgStopAudio msg;

  	SendCmdMessage( msg ); 
	
	return WaitForStatus();
}

//----------------------------------------------------------------------------
tErrType CAudioModule::PauseAudio( void )
{
    CAudioMsgPauseAudio msg;

	SendCmdMessage( msg ); 
	
	return WaitForStatus();
}

//----------------------------------------------------------------------------
tErrType CAudioModule::ResumeAudio( void )
{
    CAudioMsgResumeAudio msg;

 	SendCmdMessage( msg ); 
	
	return WaitForStatus();
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
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"AudioMgr:SetMasterVolume; volume = %d\n", volume );	

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
tAudioID CAudioModule::StartAudio( tRsrcHndl hRsrc, U8 volume,  
	tAudioPriority priority, S8 pan, IEventListener *pListener, tAudioPayload payload, 
	tAudioOptionsFlags flags )
{
	// Generate the command message to send to the audio Mgr task
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"AudioMgr:StartAudio; hRsrc = 0x%x\n", static_cast<unsigned int>(hRsrc) );	

	tAudioStartAudioInfo msgData(hRsrc, volume, priority, pan, 
								pListener, payload, flags);

	CAudioMsgStartAudio	msg( msgData );
	
	// Send the message and wait to get the audioID back from the audio Mgr task
 	SendCmdMessage( msg ); 
	
 	return WaitForAudioID();
}
/*
//==============================================================================
//==============================================================================
static tAudioID CAudioModule::StartAudioDefault(CAudioMgrMPIImpl *pImpl, tRsrcHndl hRsrc, 
	tAudioPayload payload, tAudioOptionsFlags flags)
{
	return 0;
};

*/

//==============================================================================
//==============================================================================
void CAudioModule::StopAudio( tAudioID audioID, Boolean surpressDoneMessage )
{
	// Generate the command message to send to the audio Mgr task
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"AudioMgr:StopAudio; ID = %d\n", static_cast<int>(audioID) );	

	tAudioStopAudioInfo msgData;
	
	msgData.id = audioID;
	msgData.suppressDoneMsg = surpressDoneMessage;

	
	CAudioMsgStopAudio	msg( msgData );
	SendCmdMessage( msg ); 
}

//==============================================================================
//==============================================================================
void CAudioModule::PauseAudio( tAudioID audioID )
{
	tAudioPauseAudioInfo	msgData;

	msgData.id = audioID;

	CAudioMsgPauseAudio	msg( msgData );
	SendCmdMessage( msg ); 
}

//==============================================================================
//==============================================================================
void CAudioModule::ResumeAudio( tAudioID audioID )
{
	tAudioResumeAudioInfo	msgData;

	msgData.id = audioID;

	CAudioMsgResumeAudio	msg( msgData );
	SendCmdMessage( msg ); 
}

/*
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
tErrType CAudioModule::AcquireMidiPlayer( tAudioPriority priority, IEventListener *pHandler, tMidiID *midiID )
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
	CAudioMsgReleaseMidiPlayer msg;

	// Send the message.
 	SendCmdMessage( msg );
 	
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

//==============================================================================
//==============================================================================
tMidiID CAudioModule::StartMidiFile( tMidiID	midiID,
						tRsrcHndl			hRsrc, 
						U8					volume, 
						tAudioPriority		priority,
						IEventListener*		pListener,
						tAudioPayload		payload,
						tAudioOptionsFlags	flags )
{
	tAudioStartMidiFileInfo info;
	
	info.id = midiID;
	info.hRsrc = hRsrc;
	info.volume = volume;
	info.priority = priority;
	info.pListener = pListener;
	info.priority = priority;
	info.payload = payload;
	info.flags = flags;
	
	CAudioMsgStartMidiFile msg( info );
	
	SendCmdMessage( msg );
	
	return kNoErr;
}

//==============================================================================
//==============================================================================
void CAudioModule::PauseMidiFile( tMidiID midiID )
{
	tAudioPauseMidiFileInfo info;

	info.id = midiID;
	
	CAudioMsgPauseMidiFile msg( info );
	
	SendCmdMessage( msg );
}

//==============================================================================
//==============================================================================
void CAudioModule::ResumeMidiFile( tMidiID midiID )
{
	tAudioResumeMidiFileInfo info;

	info.id = midiID;
	
	CAudioMsgResumeMidiFile msg( info );
	
	SendCmdMessage( msg );
}

//==============================================================================
//==============================================================================
void CAudioModule::StopMidiFile( tMidiID midiID, Boolean surpressDoneMessage )
{
	tAudioStopMidiFileInfo info;

	info.id = midiID;
	info.suppressDoneMsg = surpressDoneMessage;
	
	CAudioMsgStopMidiFile msg( info );
	
	SendCmdMessage( msg );
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
void CAudioModule::SendCmdMessage( CAudioCmdMsg& msg ) 
{
	tErrType err;
	
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::SendCmdMessage -- Sending message to audio task. size = %d; type = %d\n", 
							msg.GetMessageSize(), msg.GetCmdType());	
	
    err = pKernelMPI_->SendMessage( hSendMsgQueue_, msg );
    pDebugMPI_->AssertNoErr(err, "CAudioModule::SendCmdMessage -- After call SendMessage().\n" );
}

//==============================================================================
//==============================================================================
tAudioID CAudioModule::WaitForAudioID( void ) 
{
	tErrType 				err;
	char 					msgBuf[sizeof(CAudioReturnMessage)];
	CAudioReturnMessage* 	msg;

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
			"CAudioModule::WaitForAudioID -- Waiting for message from audio task.\n" );	

	err = pKernelMPI_->ReceiveMessage( hRecvMsgQueue_,  (CMessage*)msgBuf, sizeof(CAudioReturnMessage) );
	pDebugMPI_->DebugOutErr(kDbgLvlVerbose, err, "Audio Task: ReceivedMessage err\n");
	
	msg = reinterpret_cast<CAudioReturnMessage*>(msgBuf);

	pDebugMPI_->DebugOut(kDbgLvlVerbose, "CAudioModule::WaitForAudioID -- Got ID = %d \n",
				static_cast<int>(msg->GetAudioID()));
	    	  
	return msg->GetAudioID();
}

//==============================================================================
//==============================================================================
tMidiID CAudioModule::WaitForMidiID( void ) 
{
	tErrType 				err;
	char 					msgBuf[sizeof(CAudioReturnMessage)];
	CAudioReturnMessage* 	msg;

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
			"CAudioModule::WaitForAudioID -- Waiting for message from audio task.\n" );	

	err = pKernelMPI_->ReceiveMessage( hRecvMsgQueue_,  (CMessage*)msgBuf, sizeof(CAudioReturnMessage) );
	pDebugMPI_->DebugOutErr(kDbgLvlVerbose, err, "Audio Task: ReceivedMessage err\n");
	
	msg = reinterpret_cast<CAudioReturnMessage*>(msgBuf);

	pDebugMPI_->DebugOut(kDbgLvlVerbose, "CAudioModule::WaitForMidiID -- Got ID = %d \n",
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

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
			"CAudioModule::WaitForStatus -- Waiting for message from audio task.\n" );	

	err = pKernelMPI_->ReceiveMessage( hRecvMsgQueue_,  (CMessage*)msgBuf, sizeof(CAudioReturnMessage) );
	pDebugMPI_->DebugOutErr(kDbgLvlVerbose, err, "CAudioModule::WaitForStatus: ReceivedMessage err\n");
	
	msg = reinterpret_cast<CAudioReturnMessage*>(msgBuf);

	pDebugMPI_->DebugOutErr(kDbgLvlVerbose, msg->GetAudioErr(),
			"CAudioModule::WaitForStatus -- Got bad status\n");
		    	  
	return msg->GetAudioErr();
}




// EOF

