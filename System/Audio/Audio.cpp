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
LF_BEGIN_BRIO_NAMESPACE()

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
Boolean		gAudioTaskRunning = false;

//============================================================================
// CAudioCmdMessages
//============================================================================
//------------------------------------------------------------------------------
CAudioMsgSetMasterVolume::CAudioMsgSetMasterVolume( const U8 masterVolume ) {
	type_ = kAudioCmdMsgTypeSetMasterVolume;
	messageSize = sizeof(CAudioMsgSetMasterVolume);
	messagePriority = kAudioMsgDefaultPriority;
	masterVolume_ = masterVolume;
}

///------------------------------------------------------------------------------
CAudioMsgStartAudio::CAudioMsgStartAudio( void ) {
	type_ = kAudioCmdMsgTypeStartAllAudio;
	messageSize = sizeof(CAudioMsgStartAudio);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgStartAudio::CAudioMsgStartAudio( const tAudioStartAudioInfo& data )
	: data_(data)
{
	type_ = kAudioCmdMsgTypeStartAudio;
	messageSize = sizeof(CAudioMsgStartAudio);
	messagePriority = kAudioMsgDefaultPriority;
}
//------------------------------------------------------------------------------
CAudioMsgIsAudioPlaying::CAudioMsgIsAudioPlaying( void ) {
	type_ = kAudioCmdMsgTypeIsAnyAudioPlaying;
	messageSize = sizeof(CAudioMsgIsAudioPlaying);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgIsAudioPlaying::CAudioMsgIsAudioPlaying( const tAudioID id ) {
	type_ = kAudioCmdMsgTypeIsAudioPlaying;
	messageSize = sizeof(CAudioMsgIsAudioPlaying);
	messagePriority = kAudioMsgDefaultPriority;
	id_ = id;
}

//------------------------------------------------------------------------------
CAudioMsgGetAudioTime::CAudioMsgGetAudioTime( const tAudioID id ) {
	type_ = kAudioCmdMsgTypeGetAudioTime;
	messageSize = sizeof(CAudioMsgGetAudioTime);
	messagePriority = kAudioMsgDefaultPriority;
	id_ = id;
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

CAudioMsgPauseAudio::CAudioMsgPauseAudio(  const tAudioID id ) {
	type_ = kAudioCmdMsgTypePauseAudio;
	messageSize = sizeof(CAudioMsgPauseAudio);
	messagePriority = kAudioMsgDefaultPriority;
	id_ = id;
}

//------------------------------------------------------------------------------
CAudioMsgResumeAudio::CAudioMsgResumeAudio( void ) {
	type_ = kAudioCmdMsgTypeResumeAllAudio;
	messageSize = sizeof(CAudioMsgResumeAudio);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgResumeAudio::CAudioMsgResumeAudio( const tAudioID id ) {
	type_ = kAudioCmdMsgTypeResumeAudio;
	messageSize = sizeof(CAudioMsgResumeAudio);
	messagePriority = kAudioMsgDefaultPriority;
	id_ = id;
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

CAudioMsgPauseMidiFile::CAudioMsgPauseMidiFile( const tMidiPlayerID id ) {
	type_ = kAudioCmdMsgTypePauseMidiFile;
	messageSize = sizeof(CAudioMsgPauseMidiFile);
	messagePriority = kAudioMsgDefaultPriority;
	id_ = id;
}

CAudioMsgResumeMidiFile::CAudioMsgResumeMidiFile( const tMidiPlayerID id ) {
	type_ = kAudioCmdMsgTypeResumeMidiFile;
	messageSize = sizeof(CAudioMsgResumeMidiFile);
	messagePriority = kAudioMsgDefaultPriority;
	id_ = id;
}

CAudioMsgStopMidiFile::CAudioMsgStopMidiFile( const tAudioStopMidiFileInfo& data ) {
	type_ = kAudioCmdMsgTypeStopMidiFile;
	messageSize = sizeof(CAudioMsgStopMidiFile);
	messagePriority = kAudioMsgDefaultPriority;
	data_ = data;
}

//------------------------------------------------------------------------------
CAudioMsgIsMidiFilePlaying::CAudioMsgIsMidiFilePlaying( void ) {
	type_ = kAudioCmdMsgTypeIsAnyMidiFilePlaying;
	messageSize = sizeof(CAudioMsgIsMidiFilePlaying);
	messagePriority = kAudioMsgDefaultPriority;
}

CAudioMsgIsMidiFilePlaying::CAudioMsgIsMidiFilePlaying( const tMidiPlayerID id ) {
	type_ = kAudioCmdMsgTypeIsMidiFilePlaying;
	messageSize = sizeof(CAudioMsgIsMidiFilePlaying);
	messagePriority = kAudioMsgDefaultPriority;
	id_ = id;
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

	// Set debug level from a constant
	pDebugMPI_->SetDebugLevel( kAudioDebugLevel );
		
	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"Kernel and Debug modules created by AudioModule\n");	
	
	// Create the Audio Task...
	err = InitAudioTask();
	pDebugMPI_->Assert((kNoErr == err), "CAudioModule::ctor: Audio task create failed.\n");

	// Wait for the AudioMgr task to be ready
	while (!gAudioTaskRunning)
	{
		pKernelMPI_->TaskSleep(10);
		pDebugMPI_->DebugOut(kDbgLvlVerbose, 
			"CAudioModule::ctor: AudioModule is waiting for the audio task to start up...\n");	
	}

	pDebugMPI_->DebugOut(kDbgLvlVerbose, 
		"CAudioModule::ctor: AudioModule thinks the audio task is running now...\n");	
	
	// First, Open the msg queue that allows the Audio Task to RECEIVE msgs from us.
	tMessageQueuePropertiesPosix msgQueueProperties = 
	{
	    0,							// msgProperties.blockingPolicy;  
	    "/audioTaskIncomingQ",		// msgProperties.nameQueue
	    0,							// msgProperties.mode 
	    O_WRONLY,					// msgProperties.oflag  
	    0,							// msgProperties.priority
	    0,							// msgProperties.mq_flags
	    kAUDIO_MAX_NUM_MSGS,		// msgProperties.mq_maxmsg
	    kAUDIO_MAX_MSG_SIZE,		// msgProperties.mq_msgsize
	    0							// msgProperties.mq_curmsgs
	};
	
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"CAudioModule::ctor: creating task incoming Q. size = %d\n", 
				static_cast<int>(kAUDIO_MAX_MSG_SIZE) );	
	
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"Audio Task: verifing incoming Q size = %d\n", 
		static_cast<int>(msgQueueProperties.mq_msgsize) );	

	err = pKernelMPI_->OpenMessageQueue( hSendMsgQueue_, msgQueueProperties, NULL );

    pDebugMPI_->Assert((kNoErr == err), "CAudioModule::ctor:Trying to create incoming audio task msg queue. err = %d \n", 
    		static_cast<int>(err) );

	// Now create a msg queue that allows the Audio Task to send messages back to us.
	msgQueueProperties.nameQueue = "/audioTaskOutgoingQ";
	msgQueueProperties.oflag = O_RDONLY;

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"Audio Task: verifing outgoing Q size = %d\n", 
		static_cast<int>(msgQueueProperties.mq_msgsize) );	

	err = pKernelMPI_->OpenMessageQueue( hRecvMsgQueue_,  msgQueueProperties, NULL );

    pDebugMPI_->Assert((kNoErr == err), "CAudioModule::ctor: Trying to create outgoing audio task msg queue. Err = %d \n", 
    		static_cast<int>(err) );
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
static tErrType CAudioModule::ChangeAudioEffectsProcessor(tAudioID id, CAudioEffectsProcessor *pChain)
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

	CAudioMsgSetMasterVolume	msg( volume );
	
	// Send the message and wait to get the audioID back from the audio Mgr task
 	SendCmdMessage( msg ); 
}

//==============================================================================
//==============================================================================
U8 CAudioModule::GetMasterVolume( void )
{
//	return masterVolume;
	return 0;
}

//==============================================================================
//==============================================================================
tAudioID CAudioModule::StartAudio( tRsrcHndl hRsrc, U8 volume,  
	tAudioPriority priority, S8 pan, IEventListener *pListener, 
	tAudioPayload payload, tAudioOptionsFlags flags )
{
	// Generate the command message to send to the audio Mgr task
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"AudioMgr:StartAudio; hRsrc = 0x%x\n", static_cast<unsigned int>(hRsrc) );	

	tAudioStartAudioInfo msgData( hRsrc, volume, priority, pan, 
								pListener, payload, flags );

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
}

*/

//==============================================================================
//==============================================================================
U32 CAudioModule::GetAudioTime( tAudioID id )
{
	CAudioMsgGetAudioTime 	msg( id );
	
	SendCmdMessage( msg );

	return WaitForU32Result();
}

//==============================================================================
//==============================================================================
Boolean CAudioModule::IsAudioPlaying( tAudioID id )
{
	CAudioMsgIsAudioPlaying 	msg( id );
	
	SendCmdMessage( msg );

	return WaitForBooleanResult();
}

//==============================================================================
//==============================================================================
Boolean CAudioModule::IsAudioPlaying()
{
	CAudioMsgIsAudioPlaying 	msg;
	
	SendCmdMessage( msg );

	return WaitForBooleanResult();
}

//==============================================================================
//==============================================================================
void CAudioModule::StopAudio( tAudioID id, Boolean surpressDoneMessage )
{
	// Generate the command message to send to the audio Mgr task
	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
		"AudioMgr:StopAudio; ID = %d\n", static_cast<int>(id) );	

	tAudioStopAudioInfo msgData;
	
	msgData.id = id;
	msgData.suppressDoneMsg = surpressDoneMessage;

	
	CAudioMsgStopAudio	msg( msgData );
	SendCmdMessage( msg ); 
}

//==============================================================================
//==============================================================================
void CAudioModule::PauseAudio( tAudioID id )
{
	CAudioMsgPauseAudio	msg( id );
	SendCmdMessage( msg ); 
}

//==============================================================================
//==============================================================================
void CAudioModule::ResumeAudio( tAudioID id )
{
	CAudioMsgResumeAudio	msg( id );
	SendCmdMessage( msg ); 
}

/*
//==============================================================================
//==============================================================================
static U8 CAudioModule::GetAudioVolume(tAudioID id) 
{
	return 0;
};

//==============================================================================
//==============================================================================
static void CAudioModule::SetAudioVolume(tAudioID id, U8 volume) 
{
};

//==============================================================================
//==============================================================================
static tAudioPriority CAudioModule::GetAudioPriority(tAudioID id) 
{
	return 0;
};

//==============================================================================
//==============================================================================
static void CAudioModule::SetAudioPriority(tAudioID id, tAudioPriority priority) 
{
};

//==============================================================================
//==============================================================================
static S8 CAudioModule::GetAudioPan(tAudioID id) 
{
	return 0;
};

//==============================================================================
//==============================================================================
static void CAudioModule::SetAudioPan(tAudioID id, S8 pan) 
{
};

//==============================================================================
//==============================================================================
static IEventListener* CAudioModule::GetAudioEventHandler(tAudioID id) 
{
	return NULL;
};

//==============================================================================
//==============================================================================
static void CAudioModule::SetAudioEventHandler(tAudioID id, IEventListener *pHandler) 
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
*/

//==============================================================================
//==============================================================================
tErrType CAudioModule::AcquireMidiPlayer( tAudioPriority priority, IEventListener *pHandler, tMidiPlayerID *id )
{
	CAudioMsgAcquireMidiPlayer msg;

	// Send the message and wait to get the id back from the audio Mgr task
 	SendCmdMessage( msg ); 
	
 	*id = WaitForMidiID();
 	return kNoErr;
}
//==============================================================================
//==============================================================================
tErrType CAudioModule::ReleaseMidiPlayer(tMidiPlayerID id)
{
	CAudioMsgReleaseMidiPlayer msg;

	// Send the message.
 	SendCmdMessage( msg );
 	
	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::MidiNoteOn( tMidiPlayerID id, U8 channel, U8 noteNum, U8 velocity, 
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
tErrType CAudioModule::MidiNoteOff( tMidiPlayerID id, U8 channel, U8 noteNum, U8 velocity, 
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
tMidiPlayerID CAudioModule::StartMidiFile( tMidiPlayerID	id,
						tRsrcHndl			hRsrc, 
						U8					volume, 
						tAudioPriority		priority,
						IEventListener*		pListener,
						tAudioPayload		payload,
						tAudioOptionsFlags	flags )
{
	tAudioStartMidiFileInfo info;
	
	info.id = id;
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
Boolean CAudioModule::IsMidiFilePlaying( tMidiPlayerID id )
{
	CAudioMsgIsMidiFilePlaying 	msg( id );
	
	SendCmdMessage( msg );

	return WaitForBooleanResult();
}

//==============================================================================
//==============================================================================
Boolean CAudioModule::IsMidiFilePlaying( void )
{
	CAudioMsgIsMidiFilePlaying 	msg;
	
	SendCmdMessage( msg );

	return WaitForBooleanResult();
}

//==============================================================================
//==============================================================================
void CAudioModule::PauseMidiFile( tMidiPlayerID id )
{
	CAudioMsgPauseMidiFile msg( id );
	
	SendCmdMessage( msg );
}

//==============================================================================
//==============================================================================
void CAudioModule::ResumeMidiFile( tMidiPlayerID id )
{
	CAudioMsgResumeMidiFile msg( id );
	
	SendCmdMessage( msg );
}

//==============================================================================
//==============================================================================
void CAudioModule::StopMidiFile( tMidiPlayerID id, Boolean surpressDoneMessage )
{
	tAudioStopMidiFileInfo info;

	info.id = id;
	info.suppressDoneMsg = surpressDoneMessage;
	
	CAudioMsgStopMidiFile msg( info );
	
	SendCmdMessage( msg );
}

//==============================================================================
//==============================================================================
tMidiTrackBitMask CAudioModule::GetEnabledMidiTracks( tMidiPlayerID id )
{
	return 0;
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::EnableMidiTracks( tMidiPlayerID id, tMidiTrackBitMask trackBitMask )
{
	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::TransposeMidiTracks( tMidiPlayerID id, tMidiTrackBitMask tracktBitMask, S8 transposeAmount )
{
	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::ChangeMidiInstrument(tMidiPlayerID id, tMidiTrackBitMask trackBitMask, tMidiInstr instr)
{
	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::ChangeMidiTempo(tMidiPlayerID id, S8 tempo) 
{
	return kNoErr;
}

//==============================================================================
//==============================================================================
tErrType CAudioModule::SendMidiCommand(tMidiPlayerID id, U8 cmd, U8 data1, U8 data2)
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
    pDebugMPI_->Assert((kNoErr == err), "CAudioModule::SendCmdMessage -- After call SendMessage err = %d \n", 
    		static_cast<int>(err) );
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

	err = pKernelMPI_->ReceiveMessage( hRecvMsgQueue_,  (CMessage*)msgBuf, kAUDIO_MAX_MSG_SIZE );
	pDebugMPI_->AssertNoErr( err, "CAudioModule::WaitForAudioID -- Could not get audio ID from Audio Task.\n" );
	
	msg = reinterpret_cast<CAudioReturnMessage*>(msgBuf);

	pDebugMPI_->DebugOut(kDbgLvlVerbose, "CAudioModule::WaitForAudioID -- Got ID = %d \n",
				static_cast<int>(msg->GetAudioID()));
	    	  
	return msg->GetAudioID();
}

//==============================================================================
//==============================================================================
tMidiPlayerID CAudioModule::WaitForMidiID( void ) 
{
	tErrType 				err;
	char 					msgBuf[sizeof(CAudioReturnMessage)];
	CAudioReturnMessage* 	msg;

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
			"CAudioModule::WaitForMidiID -- Waiting for message from audio task.\n" );	

	err = pKernelMPI_->ReceiveMessage( hRecvMsgQueue_,  (CMessage*)msgBuf, kAUDIO_MAX_MSG_SIZE );
	pDebugMPI_->AssertNoErr( err, "CAudioModule::WaitForMidiID -- Could not get MIDI Player ID from Audio Task.\n" );
	
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
			"CAudioModule::WaitForStatus -- Waiting for message from audio task. MsgSize = %d.\n", sizeof(CAudioReturnMessage) );fflush(stdout);

	pKernelMPI_->TaskSleep(2000);
			
	err = pKernelMPI_->ReceiveMessage( hRecvMsgQueue_,  (CMessage*)msgBuf, kAUDIO_MAX_MSG_SIZE );
	pDebugMPI_->AssertNoErr( err, "CAudioModule::WaitForStatus -- Could not get status from Audio Task.\n" );
	
	msg = reinterpret_cast<CAudioReturnMessage*>(msgBuf);

	err = msg->GetAudioErr();
	pDebugMPI_->DebugOut(kDbgLvlVerbose, "CAudioModule::WaitForStatus -- Got status = %d \n",
	    	   static_cast<int>(err) );  
		    	  
	return err;
}

//==============================================================================
//==============================================================================
Boolean CAudioModule::WaitForBooleanResult( void ) 
{
	tErrType 				err;
	char 					msgBuf[sizeof(CAudioReturnMessage)];
	CAudioReturnMessage* 	msg;
	Boolean					result;

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
			"CAudioModule::WaitForBooleanResult -- Waiting for message from audio task.\n" );	

	err = pKernelMPI_->ReceiveMessage( hRecvMsgQueue_,  (CMessage*)msgBuf, kAUDIO_MAX_MSG_SIZE );
	pDebugMPI_->AssertNoErr( err, "CAudioModule::WaitForBooleanResult -- Could not get result from Audio Task.\n" );
	
	msg = reinterpret_cast<CAudioReturnMessage*>(msgBuf);

	result = msg->GetBooleanResult();
	pDebugMPI_->DebugOut(kDbgLvlVerbose, "CAudioModule::WaitForBooleanResult -- Boolean = %d \n",
	    	   static_cast<int>(result) );  
		    	  
	return result;
}

//==============================================================================
//==============================================================================
U32 CAudioModule::WaitForU32Result( void ) 
{
	tErrType 				err;
	char 					msgBuf[sizeof(CAudioReturnMessage)];
	CAudioReturnMessage* 	msg;
	U32						result;

	pDebugMPI_->DebugOut( kDbgLvlVerbose, 
			"CAudioModule::WaitForU32Result -- Waiting for message from audio task.\n" );	

	err = pKernelMPI_->ReceiveMessage( hRecvMsgQueue_,  (CMessage*)msgBuf, kAUDIO_MAX_MSG_SIZE );
	pDebugMPI_->AssertNoErr( err, "CAudioModule::WaitForU32Result -- Could not get U32 value from Audio Task.\n" );
	
	msg = reinterpret_cast<CAudioReturnMessage*>(msgBuf);

	result = msg->GetU32Result();
	pDebugMPI_->DebugOut(kDbgLvlVerbose, "CAudioModule::WaitForU32Result -- Got U32 = %d \n",
	    	   static_cast<int>(result) );  
		    	  
	return result;
}


LF_END_BRIO_NAMESPACE()


//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()
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

// EOF

