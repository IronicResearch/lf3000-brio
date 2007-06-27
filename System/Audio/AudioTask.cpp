//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		AudioTask.cpp
//
// Description:
//		Audio Manager task.
//
//==============================================================================
//fixme/dg: remove c std headers
#include <stdio.h>
#include <errno.h>

#include <KernelMPI.h>
#include <DebugMPI.h>
#include <SystemResourceMPI.h>
#include <AudioTypes.h>
#include <AudioTask.h>
#include <AudioOutput.h>
#include <AudioMsg.h>
#include <Mixer.h>

#include <AudioPlayer.h>
#include <RawPlayer.h>
#include <VorbisPlayer.h>
#include <MidiPlayer.h>
#include <spmidi.h>


//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Global variables
//==============================================================================
namespace
{
struct tAudioContext {
	
public:	
	CKernelMPI* 		kernelMPI;
	CDebugMPI* 			debugMPI;
	CSystemResourceMPI*	resourceMPI;
	Boolean				threadRun;			// set to false to exit main thread loop
	CAudioMixer*		pAudioMixer;		// Pointer to the global audio mixer
	CMidiPlayer*		pMidiPlayer;			// temp global midi player object for testing	
//	CAudioEffectsProcessor	*pAudioEffects;	// Pointer to the global audio effects processor
	U16					outSizeInBytes;		// Size of one audio out buffer in bytes
	U16					outSizeInWords;		// Size of one audio out buffer in words
	Boolean				audioOutputOn;		// Flag for whether audio out driver is on
	Boolean				audioOutputPaused;	// Flag for whether audio out driver is paused
	volatile Boolean	audioMgrTaskReady;	// Flag for whether audio manager task is ready
	tAudioID			nextAudioID;		// Next audioID
	U8					masterVolume;		// Master volume
	U8					numMixerChannels;
	U32					sampleRate;
	IEventListener*		pDefaultListener;
	
	tMessageQueueHndl 	hSendMsgQueue;
	tMessageQueueHndl 	hRecvMsgQueue;	
};
}

// Global Audio Context
tAudioContext gContext;

// Globals that must be accessable to other modules for operation.
extern tMutex		gAudioConditionMutex;
extern tMutexAttr 	gAudioConditionMutexAttributes;
extern tCond		gAudioCondition;
extern tCondAttr 	gAudioConditionAttributes;
extern bool			gAudioTaskRunning;

typedef struct{
        int numTest;
        char *testDescription;
}thread_arg_t;


// Prototypes
void* AudioTaskMain( void* arg );
static tErrType DoStopAudioSystem( void );

//==============================================================================
//==============================================================================
tErrType InitAudioTask( void )
{
	tErrType err;
	Boolean	ret;
	
	tTaskHndl pHndl;
	tTaskProperties properties;
	thread_arg_t threadArg;
	
	// Get Debug MPI
	gContext.debugMPI = new CDebugMPI( kGroupAudio );
	ret = gContext.debugMPI->IsValid();
	if (ret != true)
		printf("AudioTask ctor -- Couldn't create DebugMPI!\n");
	
	// Get Kernel MPI
	gContext.kernelMPI = new CKernelMPI;
	ret = gContext.kernelMPI->IsValid();
	if (ret != true)
		printf("AudioTask ctor-- Couldn't create KernelMPI!\n");

	// Get Resource MPI
	gContext.resourceMPI = new CSystemResourceMPI;
	ret = gContext.resourceMPI->IsValid();
	if (ret != true)
		printf("AudioTask ctor-- Couldn't create ResourceMPI!\n");

	gContext.debugMPI->DebugOut( kDbgLvlValuable, 
		(const char *)"\nDebug, Kernel, and Resource MPIs created by AudioTask ctor\n");	

	// only want important stuff.
	gContext.debugMPI->SetDebugLevel( kDbgLvlVerbose );

	// Hard code the configuration resource
	gContext.numMixerChannels = 	kAudioNumMixerChannels;
	gContext.sampleRate =			kAudioSampleRate;

	// Set the output buffer sizes.  These values are based on
	// 20ms buffer of stereo samples: see AudioConfig.h
	gContext.outSizeInWords = kAudioOutBufSizeInWords;
	gContext.outSizeInBytes = kAudioOutBufSizeInBytes;

	// Allocate the global audio mixer
	gContext.pAudioMixer = new CAudioMixer( kAudioNumMixerChannels );
	
	// Get a ptr to the midi player so we can send it commands.
	gContext.pMidiPlayer = gContext.pAudioMixer->GetMidiPlayer();
	
	// Initialize the audio out driver
	gContext.audioOutputOn = false;
	gContext.audioOutputPaused = false;
	
	// Init output driver and register callback.  We have to pass in a pointer
	// to the mixer object as a bit of "user data" so that when the callback happens,
	// the C call can get to the mixer's C++ member function for rendering.  
	// Too complicated!
	err = InitAudioOutput( &CAudioMixer::WrapperToCallRenderBuffer, (void *)gContext.pAudioMixer );
	gContext.debugMPI->Assert( kNoErr == err, "AudioTask -- Failed to initalize audio output!\n" );

/*
  	// Set the global audio effects processor to kNull
	//pAudioEffects = kNull;
 */

	// Initialize the next audioID
	gContext.nextAudioID = 0;

	// Get set up to create and run the task thread.
// 	priority;				// 1	
//	properties.priority = 1;
  
 //	stackAddr;				// 2
//	const unsigned PTHREAD_STACK_ADDRESS = 0xABC;
//	properties.stackAddr = (tAddr ) malloc(PTHREAD_STACK_ADDRESS + 0x4000);

//	stackSize;				// 3	
//    const unsigned PTHREAD_STACK_MINIM = 0x5000;
//	properties.stackSize = PTHREAD_STACK_MIN + 0x0000;

//	TaskMainFcn;			// 4
//	properties.TaskMainFcn = (void* (*)(void*))myTask;
	properties.TaskMainFcn = (void* (*)(void*))AudioTaskMain;

//	taskMainArgCount;		// 5
//    threadArg = (thread_arg_t *)malloc(sizeof(thread_arg_t));
//	threadArg->value = 1;

//  pTaskMainArgValues;		// 6						
//    threadArg->stringArg[0] = (char *)malloc(10 * sizeof(char));
//    strcpy(threadArg->stringArg[0], "20000");  
//	properties.pTaskMainArgValues = (void *)&threadArg;
	properties.pTaskMainArgValues = &threadArg;
	
//	startupMode;			// 7
//	pProperties.startupMode 

//	schedulingPolicy;		// 8
//   properties.schedulingPolicy = SCHED_FIFO;

	// Set thread's main loop run flag and create thread.
	gContext.threadRun = true;
	err = gContext.kernelMPI->CreateTask( pHndl, properties, NULL );

	gContext.debugMPI->Assert( kNoErr == err, "AudioTask -- Failed to create AudioTask!\n" );

return err;
}

//==============================================================================
//==============================================================================
void DeInitAudioTask( void )
{
	gContext.debugMPI->DebugOut( kDbgLvlVerbose, (const char *)"DeInitAudioTask called...\n");	

	// Exit nicely
	if ( gContext.audioOutputOn )
		DoStopAudioSystem();
		
	DeInitAudioOutput();

	// allow thread to exit main while loop
	gContext.threadRun = false;
	
	delete gContext.kernelMPI;
	delete gContext.debugMPI;

}

// FIXME - MG - Temporary event handler
//tErrType CSendMessageEventHandler::Notify(tEventType event, const IEventMessage *pMsg,
//									tEventContext callerContext)
//{
//	CAudioEventMessage	*pAudioMsg = (CAudioEventMessage*)pMsg;
//	printf("event:x%x, audioID:%d\n", event, pAud &audioPlayInfo, 0ioMsg->audioMsgData.audioCompleted.audioID); 
//	return kNoErr;
//}

//==============================================================================
//==============================================================================
static void SendMsgToAudioModule( CAudioReturnMessage msg ) 
{
	tErrType err;
	
 	gContext.debugMPI->DebugOut( kDbgLvlVerbose, (const char *)"AudioTask:Sending msg back to module...\n");	
//  	gContext.debugMPI->DebugOut( kDbgLvlVerbose, (const char *)"AudioTask:Return msg size = %d.\n", msg.GetMessageSize());	
//  	gContext.debugMPI->DebugOut( kDbgLvlVerbose, (const char *)"AudioTask:size of CAudioReturnMesage = %d.\n", sizeof(CAudioReturnMessage));	
 	
    err = gContext.kernelMPI->SendMessage( gContext.hSendMsgQueue, msg );
    gContext.debugMPI->Assert((kNoErr == err), "After call SendMessage err = %d \n", err );

}

//==============================================================================
//==============================================================================
static void DoSetMasterVolume( CAudioMsgSetMasterVolume* pMsg ) 
{
	tAudioMasterVolume*	pVolume = pMsg->GetData();
	gContext.masterVolume = pVolume->volume;
	gContext.pAudioMixer->SetMasterVolume( gContext.masterVolume );
}

//==============================================================================
//==============================================================================
static void DoStartAudio( CAudioMsgStartAudio* pMsg ) 
{
	tErrType			err;
	tRsrcType			rsrcType;
	CAudioReturnMessage	msg;
	tAudioID			newID = kNoAudioID;
	CChannel*			pChannel = kNull;
	CAudioPlayer*		pPlayer = kNull;

	// Retrieve the StartAudio message's data.
	tAudioStartAudioInfo*	pAudioInfo = pMsg->GetData();

	//		printf("Start Audio Msg: vol:%d, pri:%d, pan:%d, rsrc:0x%x, listen:0x%x, payload:%d, flags:%d \n", pAudioInfo->volume, pAudioInfo->priority, pAudioInfo->pan, pAudioInfo->hRsrc,
	//				pAudioInfo->pListener, pAudioInfo->payload, pAudioInfo->flags);

	// Find the best channel for the specified priority
	pChannel = gContext.pAudioMixer->FindChannelUsing( pAudioInfo->priority );
//	printf("Find Best Channel returned:%d\n", pChannel);

	// If we have a good channel, play the audio, if not return error to caller.
	if (pChannel != kNull) {
								   	
		// Generate the audio ID, accounting for wrap around
		newID = gContext.nextAudioID++;
		if (gContext.nextAudioID == kNoAudioID)
			gContext.nextAudioID = 0;
	
		// Branch on the type of audio resource to create a new audio player
		rsrcType = gContext.resourceMPI->GetType( pAudioInfo->hRsrc );  

		switch ( rsrcType )
		{
		case kAudioRsrcRaw:
			printf("Create RawPlayer\n");
			pPlayer = new CRawPlayer( pAudioInfo, newID );
			break;
	
		case kAudioRsrcOggVorbis:
			printf("Create VorbisPlayer\n");
			pPlayer = new CVorbisPlayer( pAudioInfo, newID );
			break;
		
		case kAudioRsrcMIDI:
			printf("Faking Create MidiPlayer... DON'T DO THIS! USE MIDI API\n");
	//		pPlayer = new CMidiPlayer( pAudioInfo->hRsrc, newID );
			break;
	
		default:
			printf("Create *NO* Player... bad news, unhandled audio type!\n");
			break;
	
		} 
		
		// Start channel found above using newly created player.
		// When the audio is done playing, the player will be destroyed automatically.
		pChannel->InitChanWithPlayer( pPlayer );
			 
		// Send the audioID back to the caller
		msg.SetAudioErr( kNoErr );
		msg.SetAudioID( newID );
	} else {
		printf("Couldn't Find A Channel!\n");
		// Send the audioID back to the caller
		msg.SetAudioErr( kAudioNoChannelAvailErr );
	}

	SendMsgToAudioModule( msg );
}

//==============================================================================
//==============================================================================
static void DoPauseAudio( CAudioMsgPauseAudio* pMsg ) 
{
	CChannel*				pChannel = kNull;
	tAudioPauseAudioInfo*	pAudioInfo = pMsg->GetData();

	// Find the best channel for the specified priority
	pChannel = gContext.pAudioMixer->FindChannelUsing( pAudioInfo->id );

	if (pChannel != kNull)
		pChannel->Pause();
}

//==============================================================================
//==============================================================================
static void DoResumeAudio( CAudioMsgResumeAudio* pMsg ) 
{
	CChannel*				pChannel = kNull;
	tAudioResumeAudioInfo*	pAudioInfo = pMsg->GetData();

	// Find the best channel for the specified priority
	pChannel = gContext.pAudioMixer->FindChannelUsing( pAudioInfo->id );

	if (pChannel != kNull)
		pChannel->Resume();
}

//==============================================================================
//==============================================================================
static void DoStopAudio( CAudioMsgStopAudio* pMsg ) 
{
	CChannel*				pChannel = kNull;
	tAudioStopAudioInfo*	pAudioInfo = pMsg->GetData();

	// Find the best channel for the specified priority
	pChannel = gContext.pAudioMixer->FindChannelUsing( pAudioInfo->id );

	if (pChannel != kNull)
		pChannel->Release( pAudioInfo->suppressDoneMsg);
}

//==============================================================================
// 		Overall Audio System Control
//==============================================================================
static tErrType DoStartAudioSystem( void )
{
	tErrType err = kNoErr;
	CAudioReturnMessage	msg;
	
	gContext.debugMPI->DebugOut( kDbgLvlVerbose, (const char *)"Starting audio output driver...\n");	

	if (!gContext.audioOutputOn && !gContext.audioOutputPaused) {
		gContext.audioOutputOn = true;
		err = StartAudioOutput();
	}
	
	// Send the status back to the caller
	msg.SetAudioErr( err );
	SendMsgToAudioModule( msg );

	return err;
}

static tErrType DoPauseAudioSystem( void )
{
	tErrType err = kNoErr;
	CAudioReturnMessage	msg;
	
	gContext.debugMPI->DebugOut( kDbgLvlVerbose, (const char *)"Pausing all audio output...\n");	

	if (gContext.audioOutputOn && !gContext.audioOutputPaused) {
		gContext.audioOutputPaused = true;
		err = StopAudioOutput();
	}
	
	// Send the status back to the caller
	msg.SetAudioErr( err );
	SendMsgToAudioModule( msg );

	return err;
}

static tErrType DoResumeAudioSystem( void )
{
	tErrType err = kNoErr;
	CAudioReturnMessage	msg;
	
	gContext.debugMPI->DebugOut( kDbgLvlVerbose, (const char *)"Resuming all audio output...\n");	

	if (gContext.audioOutputOn && gContext.audioOutputPaused) {
		gContext.audioOutputPaused = false;
		err = StartAudioOutput();
	}
	
	// Send the status back to the caller
	msg.SetAudioErr( err );
	SendMsgToAudioModule( msg );

	return err;
}

//----------------------------------------------------------------------------
static tErrType DoStopAudioSystem( void )
{
	tErrType err = kNoErr;
	CAudioReturnMessage	msg;
	
	gContext.debugMPI->DebugOut( kDbgLvlVerbose, (const char *)"Stopping audio output driver...\n");	

	if (gContext.audioOutputOn) {
		gContext.audioOutputOn = false;
		err = StopAudioOutput();
	}

/*	
	const tEventPriority	kPriorityTBD = 0;
	tAudioMsgDataCompleted	data;
	data.audioID = 99;	// dummy
	data.payload = 101;	// dummy
	data.count = 1;

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, mpDefaultListener);
*/
	// Send the status back to the caller
	msg.SetAudioErr( err );
	SendMsgToAudioModule( msg );

	return err;
}

//==============================================================================
//==============================================================================
void DoAcquireMidiPlayer( void ) {
	CAudioReturnMessage	msg;
	
	gContext.debugMPI->DebugOut( kDbgLvlVerbose, (const char *)"Acquiring MIDI player...\n");	
	
	// Send the status back to the caller
	msg.SetAudioErr( kNoErr );
	msg.SetMidiID( 1 );
	SendMsgToAudioModule( msg );
}

//==============================================================================
//==============================================================================
void DoMidiNoteOn( CAudioMsgMidiNoteOn* msg ) {
	tAudioMidiNoteInfo* info = msg->GetData();
	
	gContext.pMidiPlayer->NoteOn( 
			info->channel,
			info->noteNum,
			info->velocity,
			info->flags );

	//	info.priority = kAudioDefaultPriority;

}

//==============================================================================
//==============================================================================
void DoMidiNoteOff( CAudioMsgMidiNoteOff* msg ) {
	tAudioMidiNoteInfo* info = msg->GetData();
	
	gContext.pMidiPlayer->NoteOff( 
			info->channel,
			info->noteNum,
			info->velocity,
			info->flags );
	
	//	info.priority = kAudioDefaultPriority;

}

void DoStartMidiFile( CAudioMsgStartMidiFile* msg ) {
	tErrType					err;
	tAudioStartMidiFileInfo* 	pInfo = msg->GetData();
	
	// Load the midi file using the resource manager.
	err = gContext.resourceMPI->LoadRsrc( pInfo->hRsrc );  
	
	// Store pointer in the struct so that the Player object can access it.
	pInfo->pMidiFileImage = (U8*)gContext.resourceMPI->GetPtr( pInfo->hRsrc );
	pInfo->imageSize = gContext.resourceMPI->GetUnpackedSize( pInfo->hRsrc );

//	printf("Msg: vol:%d, pri:%d, rsrc:0x%x, listen:0x%x, payload:%d, flags:%d \n", pInfo->volume, pInfo->priority, (int)pInfo->hRsrc,
//			(unsigned int)pInfo->pListener, (int)pInfo->payload, (int)pInfo->flags);

	gContext.pMidiPlayer->StartMidiFile( pInfo );
	
	//	info.priority = kAudioDefaultPriority;
}

void DoPauseMidiFile( CAudioMsgPauseMidiFile* msg ) {
	tAudioPauseMidiFileInfo* 	pInfo = msg->GetData();

	gContext.pMidiPlayer->PauseMidiFile( pInfo );
}

void DoResumeMidiFile( CAudioMsgResumeMidiFile* msg ) {
	tAudioResumeMidiFileInfo* 	pInfo = msg->GetData();

	gContext.pMidiPlayer->ResumeMidiFile( pInfo );
}

void DoStopMidiFile( CAudioMsgStopMidiFile* msg ) {
	tAudioStopMidiFileInfo* 	pInfo = msg->GetData();

	gContext.pMidiPlayer->StopMidiFile( pInfo );
}

//==============================================================================
//==============================================================================
void* AudioTaskMain( void* arg )
{
	tErrType err;
	U32 i = 0;
	
	// First, create a msg queue that allows the Audio Task to RECEIVE msgs from the audio module.
	tMessageQueuePropertiesPosix msgQueueProperties = 
	{
	    0,                          // msgProperties.blockingPolicy;  
	    "/audioTaskIncomingQ",      // msgProperties.nameQueue
	    S_IRWXU,                    // msgProperties.mode 
	    O_RDWR|O_CREAT|O_TRUNC,     // msgProperties.oflag  
	    0,                          // msgProperties.priority
	    0,                          // msgProperties.mq_flags
	    8,                          // msgProperties.mq_maxmsg
	    kMAX_AUDIO_MSG_SIZE,   		// msgProperties.mq_msgsize
	    0                           // msgProperties.mq_curmsgs
	};
	
	// I want to see everything...
	gContext.debugMPI->SetDebugLevel( kDbgLvlVerbose );

	gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
		(const char *)"Audio Task creating task incoming Q. size = %d\n", kMAX_AUDIO_MSG_SIZE );	
	
	err = gContext.kernelMPI->OpenMessageQueue( gContext.hRecvMsgQueue, msgQueueProperties, NULL );

    gContext.debugMPI->Assert((kNoErr == err), "Trying to create incoming audio task msg queue. err = %d \n", err );

	// Now create a msg queue that allows the Audio Task to send messages back to us.
	msgQueueProperties.nameQueue = "/audioTaskOutgoingQ";
	msgQueueProperties.mq_msgsize = sizeof(CAudioReturnMessage);

	gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
		(const char *)"Audio Task Creating task outgoing Q. size = %d\n", msgQueueProperties.mq_msgsize );	

	err = gContext.kernelMPI->OpenMessageQueue( gContext.hSendMsgQueue, msgQueueProperties, NULL );

    gContext.debugMPI->Assert((kNoErr == err), "Trying to create outgoing audio task msg queue. Err = %d \n", err );

// Set the task to ready
	gAudioTaskRunning = true;
	
	char 				msgBuf[kMAX_AUDIO_MSG_SIZE];
	tAudioCmdMsgType 	cmdType;
	U32					msgSize;
	CAudioCmdMsg*		pAudioMsg;
	
	while ( gContext.threadRun ) {
		gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
			(const char *)"MyAudioTask top of loop, cnt: %u.\n", i++);	
   
	    err = gContext.kernelMPI->ReceiveMessage( gContext.hRecvMsgQueue, 
								   (CMessage*)msgBuf, kMAX_AUDIO_MSG_SIZE );
								   
	    gContext.debugMPI->DebugOut(kDbgLvlVerbose, "Audio Task: ReceivedMessage err = % d\n", err);

		pAudioMsg = reinterpret_cast<CAudioCmdMsg*>(msgBuf);
		msgSize = pAudioMsg->GetMessageSize();
		cmdType = pAudioMsg->GetCmdType();
	    gContext.debugMPI->DebugOut(kDbgLvlVerbose, "Audio Task: Got Audio Command, Type= %d; Msg Size = %d \n", cmdType, msgSize );  

		switch ( cmdType )
		{
		//*********************************
		case kAudioCmdMsgTypeSetMasterVolume:
			gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
				(const char *)"\nAudio Task: setting master volume.\n" );	

			DoSetMasterVolume( (CAudioMsgSetMasterVolume*)pAudioMsg );
										   
			break;

		//*********************************
		case kAudioCmdMsgTypeStartAllAudio:
			gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
				(const char *)"\nAudio Task: starting audio system.\n" );	
	
			DoStartAudioSystem();
			break;
			
		//*********************************
		case kAudioCmdMsgTypeStopAllAudio:
			gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
				(const char *)"\nAudio Task: stopping audio system\n" );	
	
			DoStopAudioSystem();
			break;

		//*********************************
		case kAudioCmdMsgTypePauseAllAudio:
			gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
				(const char *)"\nAudio Task: pause all audio.\n" );	
	
			DoPauseAudioSystem();
			break;

		//*********************************
		case kAudioCmdMsgTypeResumeAllAudio:
			gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
				(const char *)"\nAudio Task: resume all audio.\n" );	

			DoResumeAudioSystem();
			break;

		//*********************************
		case kAudioCmdMsgTypeStartAudio:
			gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
				(const char *)"\nAudio Task: kAudioCmdMsgTypeStartAudio : calling DoStartAudio().\n" );	
	
			DoStartAudio( (CAudioMsgStartAudio*)pAudioMsg );
			break;

		//*********************************
		case kAudioCmdMsgTypeStopAudio:
			gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
				(const char *)"\nAudio Task: stop audio.\n" );	

			DoStopAudio( (CAudioMsgStopAudio*)pAudioMsg );
			break;

		//*********************************
		case kAudioCmdMsgTypePauseAudio:
			gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
				(const char *)"\nAudio Task: pause audio.\n" );	

			DoPauseAudio( (CAudioMsgPauseAudio*)pAudioMsg );
			break;

		//*********************************
		case kAudioCmdMsgTypeResumeAudio:
			gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
				(const char *)"\nAudio Task: resume audio.\n" );	

			DoResumeAudio( (CAudioMsgResumeAudio*)pAudioMsg );
			break;

		
		//*********************************
		case kAudioCmdMsgTypeAcquireMidiPlayer:
			gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
				(const char *)"\nAudio Task: Acquire Midi Player.\n" );	

			DoAcquireMidiPlayer();
			break;

			//*********************************
		case kAudioCmdMsgTypeMidiNoteOn:
			gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
				(const char *)"\nAudio Task: midi note On.\n" );	

			DoMidiNoteOn( (CAudioMsgMidiNoteOn*)pAudioMsg );
			break;

		//*********************************
		case kAudioCmdMsgTypeMidiNoteOff:
			gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
				(const char *)"\nAudio Task: midi note Off.\n" );	
			
			DoMidiNoteOff( (CAudioMsgMidiNoteOff*)pAudioMsg );
			break;

		//*********************************
		case kAudioCmdMsgTypeStartMidiFile:
			gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
				(const char *)"\nAudio Task: Start midi file.\n" );	
			
			DoStartMidiFile( (CAudioMsgStartMidiFile*)pAudioMsg );
			break;

		//*********************************
		case kAudioCmdMsgTypePauseMidiFile:
			gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
				(const char *)"\nAudio Task: pause midi file.\n" );	
			
			DoPauseMidiFile( (CAudioMsgPauseMidiFile*)pAudioMsg );
			break;
			
		//*********************************
		case kAudioCmdMsgTypeResumeMidiFile:
			gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
				(const char *)"\nAudio Task: resume midi file.\n" );	
			
			DoResumeMidiFile( (CAudioMsgResumeMidiFile*)pAudioMsg );
			break;

		//*********************************
		case kAudioCmdMsgTypeStopMidiFile:
			gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
				(const char *)"\nAudio Task: stop midi file.\n" );	
			
			DoStopMidiFile( (CAudioMsgStopMidiFile*)pAudioMsg );
			break;

		default:
		gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
			(const char *)"\nAudio Task: unhandled audio command!!!.\n" );	

		break;
		}

	}

	gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
		(const char *)"MyAudioTask is exiting, loop cnt: %u.\n", i);	
		
	// Set the task to byebye
	gAudioTaskRunning = false;

	return (void *)kNull;
}

// EOF

