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
#include <stdio.h>
#include <errno.h>
#include <AudioTask.h>
#include <KernelMPI.h>
#include <AudioOutput.h>
#include <AudioMsg.h>

//#include <AudioPlayer.h>
//#include <AudioMsg.h>
//#include <CodecPlayer.h>
//#include <MidiPlayer.h>
//#include <RawPlayer.h>
//#include <AudioOutDriver.h>
//#include <SystemEvents.h>

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
	
	Boolean				threadRun;			// set to false to exit main thread loop
//	tAudioRsrcConfig	config;				// Audio rsrc config
//	CAudioMixer			*pAudioMixer;		// Pointer to the global audio mixer
//	CAudioEffectsProcessor	*pAudioEffects;	// Pointer to the global audio effects processor
//	tAudioPlayerNode	*pNode;				// Linked list of audio players
//	S16					*pAudioOutBuffer;	// Pointer to the audio out buffers
	U16					outSizeInBytes;		// Size of one audio out buffer in bytes
	U16					outSizeInWords;		// Size of one audio out buffer in words
	volatile U8			audioOutIndex;		// Index of the current audio out buffer
	Boolean				audioOutputOn;		// Flag for whether audio out driver is on
	Boolean				audioCodecActive;	// Flag for whether an audio codec is active
	volatile Boolean	audioMgrTaskReady;	// Flag for whether audio manager task is ready
//	tAudioID			nextAudioID;		// Next audioID
	U8					masterVolume;		// Master volume
	U8					numMixerChannels;
	U32					sampleRate;
	const IEventListener*	mpDefaultListener;
	
	tMessageQueueHndl 	hSendMsgQueue;
	tMessageQueueHndl 	hRecvMsgQueue;

//	NU_EVENT_GROUP		audioEvents;		// Audio event group
//	NU_QUEUE			audioCmdQueue;		// Audio command message queue
//	NU_TASK             audioCodecTask;		// Audio Codec Task
	
	
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

// Function Prototypes
tErrType StopAudio( void );
void* AudioTaskMain( void* arg );
tErrType StopAudioSystem( void );
tErrType StartAudioSystem( void );


//==============================================================================
//==============================================================================
tErrType InitAudioTask( void )
{
	tErrType err;
	Boolean	ret;
	
	tTaskHndl pHndl;
	tTaskProperties properties;
	thread_arg_t threadArg;
	
	// Get Kernel MPI
	gContext.kernelMPI = new CKernelMPI;
	ret = gContext.kernelMPI->IsValid();
	if (ret != true)
		printf("AudioTask ctor-- Couldn't create KernelMPI!\n");

	// Get Debug MPI
	gContext.debugMPI = new CDebugMPI( kGroupAudio );
	ret = gContext.debugMPI->IsValid();
	if (ret != true)
		printf("AudioTask ctor -- Couldn't create DebugMPI!\n");
	
	gContext.debugMPI->DebugOut( kDbgLvlValuable, 
		(const char *)"\nKernel and Debug modules created by AudioTask ctor\n");	


	// Hard code the configuration resource
	gContext.numMixerChannels = 	kAudioNumMixerChannels;
	gContext.sampleRate =			kAudioSampleRate;

	// Set the output buffer sizes
	gContext.outSizeInWords = kAudioOutBufSizeInWords;
	gContext.outSizeInBytes = kAudioOutBufSizeInBytes;

	// Allocate the global audio mixer
//	if ((pAudioMixer = new CAudioMixer()) == kNull)
//	{
//		err = kAllocMPIErr;
//		goto ReturnErr;
//	}

	// Initialize the audio out driver
	gContext.audioOutIndex = 0;
	gContext.audioOutputOn = false;
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
 */
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

/*

	// Create the audio codec task
	NU_Allocate_Memory(&System_Memory, &codecTaskMemory, kAudioCodecTaskSize, NU_NO_SUSPEND);
	if (NU_Create_Task(&audioCodecTask, "AudCod", AudioCodecTask, 0, NU_NULL,
		codecTaskMemory, kAudioCodecTaskSize, kAudioCodecTaskPriority, 10, NU_PREEMPT, NU_START) != NU_SUCCESS)
	{
		err = kAudioCreateTaskErr;
		goto ReturnErr;
	}

*/
return err;
}

//==============================================================================
//==============================================================================
void DeInitAudioTask( void )
{
	gContext.debugMPI->DebugOut( kDbgLvlVerbose, (const char *)"DeInitAudioTask called...\n");	

	// Exit nicely
	if ( gContext.audioOutputOn )
		StopAudioSystem();
		
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
//	printf("event:x%x, audioID:%d\n", event, pAudioMsg->audioMsgData.audioCompleted.audioID); 
//	return kNoErr;
//}

//==============================================================================
//==============================================================================
static void SendMsgToAudioModule( tErrType taskErr, tAudioID audioID ) 
{
	tErrType err;
	CAudioReturnMessage	msg(taskErr, audioID);

  	gContext.debugMPI->DebugOut( kDbgLvlVerbose, (const char *)"Sending msg back to module...\n");	
 	
    err = gContext.kernelMPI->SendMessage( gContext.hSendMsgQueue, msg );
    gContext.debugMPI->Assert((kNoErr == err), "After call SendMessage err = %d \n", err );

}

//==============================================================================
//==============================================================================
// FIXME - MG Decide if this should be moved to the AudioPlayer
static void SendAudioCompletedMsg(/*CAudioPlayer *player*/) 
{
/*
	// First check if done message is needed
	printf("SendAudioCompletedMsg\n");
	if (player->SendDoneMessage())
	{
		IEventHandler		*pHandler;
		CAudioEventMessage	*pMsg;

		// Make sure the caller passed in a valid handler
		pHandler = player->GetEventHandler();
		if (pHandler != kNull)
		{
			// Create the message and then send it via the Notify function
			if (player->CreateAudioCompletedMsg(0, &pMsg) == kNoErr)
				pHandler->Notify(kAudioMgrEventTypeAudioCompleted, pMsg, 0);
		}
	}
*/
}

//==============================================================================
//==============================================================================
static void StartPlayAudio(/*tAudioCmdMsgPlayAudio *pMsg*/) 
{
/*
	U8					iChan;
	tAudioID			audioID = kNoAudioID;
	tAudioPlayerNode 	*pNode, *pLast;
	tAudioHeader		*pHeader;
	CRsrcMgrMPI 		*pRsrcMgrMPI = kNull;
	CChannel			*pChannel;

	// Find the best channel for the specified priority
	iChan = gAudioContext->pAudioMixer->FindBestChannel(pMsg->priority);
	printf("Best Channel:%d\n", iChan);
	// If couldn't find a channel, inform the caller
	if (iChan == kNoChannelAvail)
		goto ReturnErr;									   

// FIXME MG want to put in RsrcMgr when it becomes available
#if 1
	// Get the pointer to the audio resource
	// which is the pointer to the audio header
	if ((pRsrcMgrMPI = new CRsrcMgrMPI) == kNull)
		goto ReturnErr;									   

	pRsrcMgrMPI->Init();
	if (pRsrcMgrMPI->GetRsrcPtr(pMsg->hRsrc, (tPtr*)&pHeader) != kNoErr)
		goto ReturnErr;
#else
	pHeader = (tAudioHeader *)pMsg->hRsrc;
#endif
	
	printf("Msg:%d, %d, %d, x%x, x%x, %d, %d \n", pMsg->volume, pMsg->priority, pMsg->pan, pMsg->hRsrc,
		pMsg->pHandler, pMsg->payload, pMsg->flags);

	printf("Header:x%x, %d, %d, %d, %d\n", pHeader->type, pHeader->offsetToData, pHeader->flags, pHeader->sampleRateInHz, pHeader->dataSize);
	// Create a new node for the audio player linked list	
	pNode = new tAudioPlayerNode(kNull, kNull);
	//printf("Node:x%x, x%x\n",pNode, gAudioContext->pNode);

	// Branch on the type of audio resource to create a new audio player
	switch (pHeader->type)
	{
	case kAudioRsrcRaw:
		printf("Create RawPlayer\n");
		pNode->pPlayer = new CRawPlayer();
		pNode->pPlayer->Init(pMsg->hRsrc);
		break;

	case kAudioRsrcAdpcm:
	case kAudioRsrcGen2:
		printf("CreateCodecPlayer\n");
		pNode->pPlayer = new CCodecPlayer();
		pNode->pPlayer->Init(pMsg->hRsrc);
		break;

	case kAudioRsrcMIDI:
		printf("Create MidiPlayer\n");
		pNode->pPlayer = new CMidiPlayer();
		pNode->pPlayer->Init(pMsg->hRsrc);
		break;

	default:
		printf("Create *NO* Player\n");
		break;

	} 
	
	// Set all of the audio player class variables from the message
	pNode->pPlayer->SetVolume(pMsg->volume);
	pNode->pPlayer->SetPriority(pMsg->priority);
	pNode->pPlayer->SetPan(pMsg->pan);
	pNode->pPlayer->SetEventHandler(pMsg->pHandler);
	pNode->pPlayer->SetPayload(pMsg->payload);
	pNode->pPlayer->SetOptionsFlags(pMsg->flags);
	
	// Acquire the channel found above to play this resource
	pChannel = &gAudioContext->pAudioMixer->mpChannel[iChan];
	pNode->pPlayer->SetChannel(pChannel);
	pNode->pPlayer->SetParentPlayer(kNull);
	pChannel->Acquire(pHeader);
	pChannel->mpPlayer = pNode->pPlayer;

	// Get the audioID
	audioID = pNode->pPlayer->GetAudioID();

	// We're now ready to add the node to the linked list
	// If the list is empty, assign it to the head node
	pLast = gAudioContext->pNode;
	if (pLast == kNull)
	{
		gAudioContext->pNode = pNode;
	}
	// Otherwise assign it to the last node in linked list
	else
	{
		while (pLast->pNext != kNull)
			pLast = pLast->pNext;
		pLast->pNext = pNode;
	}
	
ReturnErr:
#if 1
	// Cleanup
	if (pRsrcMgrMPI != kNull)
	{
		pRsrcMgrMPI->DeInit();
		delete pRsrcMgrMPI;
	}
#endif
	// Send the audioID back to the caller
	SendBackAudioID(pMsg->pReturnQueue, audioID);
*/
}

//==============================================================================
//==============================================================================
void StopAllAudioPlayers()
{
/*
	// Loop through the linked list of audio players
	// and call the player's stop function
	// We don't want to delete the nodes yet as some of the players 
	// (ie CodecPlayer) may take a while to stop processing
	tAudioPlayerNode 	*pNode = gAudioContext->pNode;
	while (pNode != kNull)
	{
		pNode->pPlayer->Stop();
		pNode = pNode->pNext;
	}
*/
}

//==============================================================================
//==============================================================================
void ProcessAudioPlayers()
{
/*	tAudioPlayerNode 	*pNode = gAudioContext->pNode;
	tAudioPlayerNode 	*pPrev = kNull;

	// Reset the audio codec active flag
	gAudioContext->audioCodecActive = 0;

	// Loop through the linked list of audio players
	while (pNode != kNull)
	{
		// Call the player's ProcessTick function
		pNode->pPlayer->ProcessTick();

		// If the player is now complete
		if (pNode->pPlayer->IsComplete())
		{
			// Release the channel, send done message, and delete the Player
			CChannel	*pChannel;
			pChannel = pNode->pPlayer->GetChannel();
			pChannel->Release();
			SendAudioCompletedMsg(pNode->pPlayer);
			pNode->pPlayer->DeInit();
			delete pNode->pPlayer;

			// Remove the player from the list
			// if it's at the top of the list
			if (pNode == gAudioContext->pNode)
			{
				gAudioContext->pNode = gAudioContext->pNode->pNext;
				delete pNode;
				pNode = gAudioContext->pNode;
			}
			// otherwise it's in the middle of the list
			else
			{
				pPrev->pNext = pNode->pNext;
				delete pNode;
				pNode = pPrev->pNext;
			}
		}
		// Otherwise update to the next node in the list
		else
		{
			pPrev = pNode;
			pNode = pNode->pNext;
		}
	}
	// At least one player has an audio codec,
	// then set the audio decode go bit
	if (gAudioContext->audioCodecActive)
	{
//		NU_Set_Events(&gAudioContext->audioEvents, kAudioDecodeGoBit, NU_OR);
		printf("send audioCodecActive event\n");
	}
	*/
}

//==============================================================================
//==============================================================================
//static CAudioPlayer* FindPlayer(/*tAudioID audioID */) 
static void FindPlayer(/*tAudioID audioID */) 
{
/*
	// Find the audio player with the given audioID
	CAudioPlayer* 		pPlayer = kNull;
	tAudioPlayerNode*	pNode = gAudioContext->pNode;

	// Traverse the linked list of audio players
	while (pNode != kNull)
	{
		// If the node's player has the desired audioID
		// return the pointer to the audio player  
		if (pNode->pPlayer->GetAudioID() == audioID)
		{
			pPlayer = pNode->pPlayer;
			break;
		}
		pNode = pNode->pNext;
	}
	return pPlayer;
	
*/
}

//==============================================================================
// 		Overall Audio System Control
//==============================================================================
tErrType StartAudioSystem( void )
{
	tErrType err = kNoErr;
	
	gContext.debugMPI->DebugOut( kDbgLvlVerbose, (const char *)"Starting audio output driver...\n");	

	if (!gContext.audioOutputOn) {
		gContext.audioOutputOn = true;
		err = StartAudioOutput();
	}
	
	return err;
	
//	tAudioCmdMsgHeader*	pMsg;
//	pMsg = (tAudioCmdMsgHeader*)GetMessageBlock(kAudioCmdMsgTypeStopAllAudio);
//	SendCmdMessage(pMsg); 
}

//----------------------------------------------------------------------------
tErrType StopAudioSystem( void )
{
	tErrType err = kNoErr;
	
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
	return err;
	
	
//	tAudioCmdMsgHeader*	pMsg;
//	pMsg = (tAudioCmdMsgHeader*)GetMessageBlock(kAudioCmdMsgTypeStopAllAudio);
//	SendCmdMessage(pMsg); 
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
		(const char *)"\nAudio Task creating task incoming Q. size = %d\n", kMAX_AUDIO_MSG_SIZE );	
	
	err = gContext.kernelMPI->OpenMessageQueue( gContext.hRecvMsgQueue, msgQueueProperties, NULL );

    gContext.debugMPI->Assert((kNoErr == err), "Trying to create incoming audio task msg queue. err = %d \n", err );

	// Now create a msg queue that allows the Audio Task to send messages back to us.
	msgQueueProperties.nameQueue = "/audioTaskOutgoingQ";
	msgQueueProperties.mq_msgsize = sizeof(CAudioReturnMessage);

	gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
		(const char *)"\nAudio Task Creating task outgoing Q. size = %d\n", msgQueueProperties.mq_msgsize );	

	err = gContext.kernelMPI->OpenMessageQueue( gContext.hSendMsgQueue, msgQueueProperties, NULL );

    gContext.debugMPI->Assert((kNoErr == err), "Trying to create outgoing audio task msg queue. Err = %d \n", err );

	// Set the task to ready
	gAudioTaskRunning = true;
	
	char 				msgBuf[kMAX_AUDIO_MSG_SIZE];
	tAudioCmdMsgType 	cmdType;
	U32					msgSize;
	CAudioMsg*			pAudioMsg;
	
	while ( gContext.threadRun ) {
		gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
			(const char *)"MyAudioTask top of loop, cnt: %u.\n", i++);	
   
	    err = gContext.kernelMPI->ReceiveMessage( gContext.hRecvMsgQueue, 
								   (CMessage*)msgBuf, kMAX_AUDIO_MSG_SIZE );
								   
	    gContext.debugMPI->DebugOut(kDbgLvlVerbose, "ReceivedMessage err = % d\n", err);

		pAudioMsg = reinterpret_cast<CAudioMsg*>(msgBuf);
		msgSize = pAudioMsg->GetMessageSize();
		cmdType = pAudioMsg->GetMessageCmdType();
	    gContext.debugMPI->DebugOut(kDbgLvlVerbose, "Got Audio Command, Type= %d; Msg Size = %d \n", cmdType, msgSize );  

		switch ( cmdType )
		{
		//*********************************
		case kAudioCmdMsgTypeSetMasterVolume:
			break;

		//*********************************
		case kAudioCmdMsgTypeStartAllAudio:
			gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
				(const char *)"\nAudio Task: starting audio system\n" );	
	
			StartAudioSystem();
			break;
			
		//*********************************
		case kAudioCmdMsgTypeStopAllAudio:
			gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
				(const char *)"\nAudio Task: stopping audio system\n" );	
	
			StopAudioSystem();
			break;

		//*********************************
		case kAudioCmdMsgTypePauseAllAudio:
			// Loop through the linked list of audio players
			// and call the player's pause function
//				pNode = gAudioContext->pNode;
//				while (pNode != kNull)
//				{
//					pNode->pPlayer->Pause();
//					pNode = pNode->pNext;
//				}
			break;

		//*********************************
		case kAudioCmdMsgTypeResumeAllAudio:
			// Loop through the linked list of audio players
			// and call the player's resume function
//				pNode = gAudioContext->pNode;
//				while (pNode != kNull)
//				{
//					pNode->pPlayer->Resume();
//					pNode = pNode->pNext;
//				}
			break;

		//*********************************
		case kAudioCmdMsgTypePlayAudio:
//				StartPlayAudio((tAudioCmdMsgPlayAudio*)pMsg);
//				audioToStart = 1;
			break;

		//*********************************
		case kAudioCmdMsgTypeStopAudio:
			// Find the audio player with the given audioID
//				if ((pPlayer = FindPlayer(((tAudioCmdMsgStopAudio*)pMsg)->audioID)) != kNull)
//				{
//					// Stop the player and if requested, suppress the done message
//					pPlayer->Stop();
//					if (((tAudioCmdMsgStopAudio*)pMsg)->suppressDoneMsg)  
//						pPlayer->SetSendDoneMessage(false);
//				} 
			break;

		//*********************************
		case kAudioCmdMsgTypePauseAudio:
			// Find the audio player with the given audioID,
			// and if one is found, pause it
//				if ((pPlayer = FindPlayer(((tAudioCmdMsgPauseAudio*)pMsg)->audioID)) != kNull)
//					pPlayer->Pause();
			break;

		//*********************************
		case kAudioCmdMsgTypeResumeAudio:
			// Find the audio player with the given audioID,
			// and if one is found, resume it
//				if ((pPlayer = FindPlayer(((tAudioCmdMsgResumeAudio*)pMsg)->audioID)) != kNull)
//					pPlayer->Resume();
				break;

		}

	}

/*
	int					status;
	U8					audioToStart = 0;
	tErrType			err;
//	tAudioCmdMsg		*pMsg;
//	CAudioPlayer		*pPlayer;
//	tAudioPlayerNode	*pNode;


	// Use command type to cast msg buffer to proper object type.
	pMsg = new tAudioCmdMsg;

	// Main Task Loop
	printf("\nAudio Mgr Task\n");
	while (1)
	{
		// If the audio out driver is on, wait for the audio done bit
		if (gAudioContext->audioOutDriverOn)
		{
//			NU_Retrieve_Events(&gAudioContext->audioEvents, kAudioDoneBit, 
//				NU_OR_CONSUME, &events, NU_SUSPEND);
		}

		// Main loop to process commands for the current tick
		while (1)
		{
			// If the audio out driver is on, poll for a message
			// and break if the queue is empty
			if (gAudioContext->audioOutDriverOn || audioToStart)
			{
//				if ((status = NU_Receive_From_Queue(&gAudioContext->audioCmdQueue, 
//					pMsg, sizeof(tAudioCmdMsg)/sizeof(UNSIGNED), &msgSize, NU_NO_SUSPEND)) == NU_QUEUE_EMPTY) 
				printf("Poll for audio message\n");
				break;
			}
			// Otherwise wait for an audio command message
			else
			{
//				NU_Receive_From_Queue(&gAudioContext->audioCmdQueue, pMsg, sizeof(tAudioCmdMsg)/sizeof(UNSIGNED), &msgSize, NU_SUSPEND); 
				printf("Try to get audio cmd msg from queue.\n");
			}

			// Branch on the command

			printf("AudioCmd:%d\n",pMsg->cmdHeader.audioCmdMsgType);
		}
		// Process the channels for the current tick
		ProcessAudioPlayers();
		err = gAudioContext->pAudioMixer->ProcessAudioMix();
		// If needed, stop the audio out from going
		if ((err == kAudioNoMoreDataErr) && gAudioContext->audioOutDriverOn)
		{
			StopAllAudioPlayers();
			// Turn off the audio out driver
			gAudioContext->audioOutDriverOn = 0;
			StopAudioOut();
		}
		// If needed, start the audio out going
		if (audioToStart && (gAudioContext->audioOutDriverOn == 0))
		{
			printf("StartAudio\n");
			gAudioContext->audioOutDriverOn = 1;
			StartAudioOut();
		}
		audioToStart = 0;
	}
*/

	gContext.debugMPI->DebugOut( kDbgLvlVerbose, 
		(const char *)"MyAudioTask is exiting, loop cnt: %u.\n", i);	
		
	// Set the task to byebye
	gAudioTaskRunning = false;
}

// EOF

