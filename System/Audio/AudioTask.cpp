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
#include <sys/stat.h>

#include <KernelMPI.h>
#include <DebugMPI.h>
#include <AudioTypes.h>
#include <AudioTask.h>
#include <AudioOutput.h>
#include <AudioMsg.h>
#include <AudioPriv.h>

#include <Mixer.h>
#include <AudioPlayer.h>
#include <Channel.h>
#include <RawPlayer.h>
#include <VorbisPlayer.h>
#include <MidiPlayer.h>

#include <spmidi.h>
LF_BEGIN_BRIO_NAMESPACE()

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
	CKernelMPI* 		pKernelMPI;
	CDebugMPI* 			pDebugMPI;
	Boolean				threadRun;			// set to false to exit main thread loop; used to control task
	Boolean				audioTaskRunning;	// Flag audio task uses so we know when it is running/stoppped.
	CAudioMixer*		pAudioMixer;		// Pointer to the global audio mixer
	CMidiPlayer*		pMidiPlayer;		// temp global MIDI player object for testing	
//	CAudioEffectsProcessor	*pAudioEffects;	// Pointer to the global audio effects processor
	U16					outSizeInBytes;		// Size of one audio out buffer in bytes
	U16					outSizeInWords;		// Size of one audio out buffer in words
	Boolean				audioOutputOn;		// Flag for whether audio out driver is on
	Boolean				audioOutputPaused;	// Flag for whether audio out driver is paused
	volatile Boolean	audioMgrTaskReady;	// Flag for whether audio manager task is ready
	tAudioID			nextAudioID;		// Next audioID

	U8					masterVolume;		// NOTE: free of the burden "units"
	U8					outputEqualizerEnabled;		
	U8					numMixerChannels;
	U32					sampleRate;

	IEventListener*		pDefaultListener;
	
	tMessageQueueHndl 	hSendMsgQueue;
	tMessageQueueHndl 	hRecvMsgQueue;	
};
}

// Global Audio Context
tAudioContext gContext;

// Globals 
typedef struct{
        int numTest;
        char *testDescription;
}thread_arg_t;


// Prototypes
void* AudioTaskMain( void* arg );

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
	gContext.pDebugMPI = new CDebugMPI( kGroupAudio );
	ret = gContext.pDebugMPI->IsValid();
	if (ret != true)
		printf("InitAudioTask() -- Couldn't create DebugMPI!\n");
	
	// Get Kernel MPI
	gContext.pKernelMPI = new CKernelMPI;
	ret = gContext.pKernelMPI->IsValid();
	if (ret != true)
		printf("InitAudioTask() -- Couldn't create KernelMPI!\n");

// gContext.pDebugMPI->DebugOut( kDbgLvlValuable, "InitAudioTask() -Debug and Kernel MPIs created.\n");	

	// Setup debug level.
	gContext.pDebugMPI->SetDebugLevel( kAudioDebugLevel );

	// Hard code configuration resource
	gContext.numMixerChannels = 	kAudioNumMixerChannels;
	gContext.sampleRate =			kAudioSampleRate;

	// Set output buffer sizes.  These values are based on
	// 20ms buffer of stereo samples: see AudioConfig.h
	gContext.outSizeInWords = kAudioOutBufSizeInWords;
	gContext.outSizeInBytes = kAudioOutBufSizeInBytes;

	// Allocate global audio mixer
	gContext.pAudioMixer = new CAudioMixer( kAudioNumMixerChannels );
	
	gContext.pMidiPlayer = gContext.pAudioMixer->GetMidiPlayerPtr();
	
	// Initialize audio out driver
	gContext.audioOutputOn     = false;
	gContext.audioOutputPaused = false;
	
	// Init output driver and register callback.  We have to pass in a pointer
	// to the mixer object as a bit of "user data" so that when the callback happens,
	// the C call can get to the mixer's C++ member function for rendering.  
	// Too complicated!
	err = InitAudioOutput( &CAudioMixer::WrapperToCallRenderBuffer, (void *)gContext.pAudioMixer );
	gContext.pDebugMPI->Assert( kNoErr == err, "InitAudioTask() -- Failed to initalize audio output!\n" );

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
//    properties.schedulingPolicy = SCHED_OTHER; //SCHED_FIFO;

	// Set thread's main loop run flag and create thread.
	gContext.threadRun = true;
	err = gContext.pKernelMPI->CreateTask( pHndl, properties, NULL );

	gContext.pDebugMPI->Assert( kNoErr == err, "InitAudioTask() -- Failed to create AudioTask!\n" );

	// Wait for the AudioMgr task to be ready
	while (!gContext.audioTaskRunning)
	{
		gContext.pKernelMPI->TaskSleep(10);
		gContext.pDebugMPI->DebugOut(kDbgLvlVerbose, 
			"AudioTask::InitAudioTask -- waiting for the audio task to start up...\n");	
	}

	gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTask Started...\n" );
	return err;
}

//==============================================================================
//==============================================================================
void DeInitAudioTask( void )
{
	gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "DeInitAudioTask called...\n");	

	// Wait for the task to die
	while (gContext.audioTaskRunning)
	{
		gContext.pKernelMPI->TaskSleep(10);
		gContext.pDebugMPI->DebugOut(kDbgLvlVerbose, 
			"AudioTask::DeInitAudioTask: Waiting for the audio task to die...\n");	
	}

	DeInitAudioOutput();
	
	delete gContext.pKernelMPI;
	delete gContext.pDebugMPI;
}

//==============================================================================
//==============================================================================
static void SendMsgToAudioModule( CAudioReturnMessage msg ) 
{
	tErrType err;
	
// 	gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTask:Sending msg back to module...\n");	
// 	gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTask:Return msg size = %d.\n", msg.GetMessageSize());	
  	
    err = gContext.pKernelMPI->SendMessage( gContext.hSendMsgQueue, msg );
 //   gContext.pDebugMPI->AssertNoErr(err, "SendMsgToAudioModule() -- After call SendMessage().\n" );

}

//==============================================================================
//==============================================================================
static void DoSetMasterVolume( CAudioMsgSetMasterVolume* pMsg ) 
{
	U8 x = pMsg->GetData();
	gContext.masterVolume = x;
	gContext.pAudioMixer->SetMasterVolume( gContext.masterVolume );
}

//==============================================================================
//==============================================================================
static void DoSetOutputEqualizer( CAudioMsgSetOutputEqualizer* pMsg ) 
{
	U8 x = pMsg->GetData();
	gContext.outputEqualizerEnabled = x;
	gContext.pAudioMixer->SetOutputEqualizer( gContext.outputEqualizerEnabled );
}

// ==============================================================================
// DoStartAudio
// ==============================================================================
static void DoStartAudio( CAudioMsgStartAudio *pMsg ) 
{
//	tAudioTypeInt		rsrcType = kAudioRsrcUndefined;
	CAudioReturnMessage	msg;
	CPath				filename;
	CPath				fileExt;

	tAudioStartAudioInfo*	pAudioInfo = pMsg->GetData();
	tAudioStartAudioInfo*	pAi = pAudioInfo;
	int strIndex = pAudioInfo->path->size(); 

//printf("AudioTask::DoStartAudio: StartAudioMsg: vol=%d pri=%d pan=%d path='%s' listen=%p payload=%d flags=$%X \n",
//			(int)pAi->volume, (int)pAudioInfo->priority, (int)pAi->pan, 
//			pAi->path->c_str(), (void *)pAi->pListener, (int)pAi->payload, (int)pAi->flags );

// Extract filename (including extension).
	strIndex = pAi->path->rfind('/', pAi->path->size());
	filename = pAi->path->substr(strIndex + 1, pAi->path->size());
	strIndex = pAi->path->rfind('.', pAi->path->size());
	fileExt  = pAi->path->substr(strIndex + 1, strIndex+3);
    char *sExt = (char *) fileExt.c_str();
//	printf("AudioTask::DoStartAudio Filename: '%s' Ext='%s'\n", filename.c_str(), fileExtension.c_str());

	msg.SetAudioErr( kAudioNoChannelAvailErr );       
	msg.SetAudioID( kNoAudioID );

	// Generate audio ID
	tAudioID newID = gContext.nextAudioID++;
	if (kNoAudioID == gContext.nextAudioID)
		gContext.nextAudioID = 0;

//printf("---- AUDIO_PLAYER_ADDITION_V1\n");
	// Create player based on file extension
    if (kNoErr == gContext.pAudioMixer->AddPlayer( pAudioInfo, sExt, newID))
        {
		msg.SetAudioErr( kNoErr );
		msg.SetAudioID( newID );
        }

// Send audioID and error codes back to caller
	SendMsgToAudioModule( msg );
}   // ---- end DoStartAudio() ----

// ==============================================================================
// DoGetAudioTime
// ==============================================================================
static void DoGetAudioTime( CAudioMsgGetAudioTime* msg ) {
	tAudioID  			id = msg->GetData();
	CAudioReturnMessage	retMsg;
	U32					time = 0;
	CChannel*			pChannel;
	CAudioPlayer*		pPlayer;
	
	gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
		"AudioTask::DoGetAudioTime() -- Stream time for audioID = %d...\n", 
		static_cast<int>(id));	

	//	find the current time from the vorbis player
	pChannel = gContext.pAudioMixer->FindChannelUsing( id );
	if (pChannel != NULL) {
		pPlayer = pChannel->GetPlayer();
		time = pPlayer->GetAudioTime_mSec();
	}
	
	gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
		"AudioTask::DoGetAudioTime() -- Stream time = %d...\n", 
		static_cast<int>(time));	

	// Send the time back to the caller
	retMsg.SetU32Result( time );
	SendMsgToAudioModule( retMsg );
}

// ==============================================================================
// DoGetAudioVolume: 
// ==============================================================================
static void DoGetAudioVolume( CAudioMsgGetAudioVolume* msg ) 
{
tAudioVolumeInfo 	info = msg->GetData();
CAudioReturnMessage	retMsg;
U32					volume = 0;
CChannel*			pChannel;

gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
	"AudioTask::DoGetAudioVolume() -- Get Volume for audioID = %d...\n", 
	static_cast<int>(info.id));	

// Find current volume for channel using audioID
pChannel = gContext.pAudioMixer->FindChannelUsing( info.id );
if (pChannel) 
	volume = pChannel->GetVolume();

retMsg.SetU32Result( (U32)volume );
SendMsgToAudioModule( retMsg );
}  // ---- end DoGetAudioVolume() ----

// ==============================================================================
// DoSetAudioVolume: 
// ==============================================================================
static void DoSetAudioVolume( CAudioMsgSetAudioVolume* msg ) 
{
tAudioVolumeInfo 	info = msg->GetData();
CAudioReturnMessage	retMsg;
CChannel*			pChannel;

gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
	"AudioTask::DoSetAudioVolume() -- Set Volume for audioID = %d...\n", 
	static_cast<int>(info.id));	

//	Set requested property
pChannel = gContext.pAudioMixer->FindChannelUsing( info.id );
if (pChannel) 
	pChannel->SetVolume( info.volume );
}  // ---- end DoSetAudioVolume() ----

// ==============================================================================
// DoGetAudioPriority: 
// ==============================================================================
static void DoGetAudioPriority( CAudioMsgGetAudioPriority* msg ) {
	tAudioPriorityInfo 	info = msg->GetData();
	CAudioReturnMessage	retMsg;
	U32					priority = 0;
	CChannel*			pChannel;
	
	gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
		"AudioTask::DoGetAudioPriority() -- Get Priority for audioID = %d...\n", 
		static_cast<int>(info.id));	

	//	find the current Priority for the channel using audioID
	pChannel = gContext.pAudioMixer->FindChannelUsing( info.id );
	if (pChannel) 
		priority = pChannel->GetPriority();
	
	// Send the Priority back to the caller
	retMsg.SetU32Result( (U32)priority );
	SendMsgToAudioModule( retMsg );
}  // ---- end DoGetAudioPriority() ----

// ==============================================================================
// DoSetAudioPriority: 
// ==============================================================================
static void DoSetAudioPriority( CAudioMsgSetAudioPriority* msg ) {
	tAudioPriorityInfo 	info = msg->GetData();
	CAudioReturnMessage	retMsg;
	CChannel*			pChannel;
	
	gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
		"AudioTask::DoSetAudioPriority() -- Set Priority for audioID = %d...\n", 
		static_cast<int>(info.id));	

	//	Set requested property
	pChannel = gContext.pAudioMixer->FindChannelUsing( info.id );
	if (pChannel) 
		pChannel->SetPriority( info.priority );
}  // ---- end DoSetAudioPriority() ----

// ==============================================================================
// DoGetAudioPan: 
// ==============================================================================
static void DoGetAudioPan( CAudioMsgGetAudioPan* msg ) 
{
tAudioPanInfo 	info = msg->GetData();
CAudioReturnMessage	retMsg;
U32					pan = 0;

//gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTask::DoGetAudioPan() for audioID=%d\n", static_cast<int>(info.id));	

// Find current Pan for channel using audioID
CChannel *pChannel = gContext.pAudioMixer->FindChannelUsing( info.id );
if (pChannel) 
	pan = pChannel->GetPan();

retMsg.SetU32Result( (U32)pan );
SendMsgToAudioModule( retMsg );
}  // ---- end DoGetAudioPan() ----

// ==============================================================================
// DoSetAudioPan: 
// ==============================================================================
static void DoSetAudioPan( CAudioMsgSetAudioPan* msg ) 
{
tAudioPanInfo 	info = msg->GetData();
CAudioReturnMessage	retMsg;
CChannel*			pChannel;

gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
	"AudioTask::DoSetAudioPan() -- Set Pan for audioID = %d...\n", 
	static_cast<int>(info.id));	

//	Set requested property
pChannel = gContext.pAudioMixer->FindChannelUsing( info.id );
if (pChannel) 
	pChannel->SetPan( info.pan );
}  // ---- end DoSetAudioPan() ----

// ==============================================================================
// DoGetAudioListener: 
// ==============================================================================
static void DoGetAudioListener( CAudioMsgGetAudioListener* msg ) 
{
	tAudioListenerInfo 		info = msg->GetData();
	CAudioReturnMessage		retMsg;
	const IEventListener*	pListener = NULL;
	CChannel*				pChannel;
	CAudioPlayer*			pPlayer;
	
	gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
		"AudioTask::DoGetAudioListener() -- Get Listener for audioID = %d...\n", 
		static_cast<int>(info.id));	

	//	find the current time from the vorbis player
	pChannel = gContext.pAudioMixer->FindChannelUsing( info.id );
	if (pChannel) {
		pPlayer   = pChannel->GetPlayer();
		pListener = pPlayer->GetEventListener();
	}
	
	// Send the listener back to the caller
	retMsg.SetU32Result( (U32)pListener );
	SendMsgToAudioModule( retMsg );
}  // ---- end DoGetAudioListener() ----

static void DoSetAudioListener( CAudioMsgSetAudioListener* msg ) {
	tAudioListenerInfo 	info = msg->GetData();
	CAudioReturnMessage	retMsg;
	CChannel*			pChannel;
	CAudioPlayer*		pPlayer;
	
	gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
		"AudioTask::DoSetAudioListener() -- Set Listener for audioID = %d...\n", 
		static_cast<int>(info.id));	

	//	Set requested property
	pChannel = gContext.pAudioMixer->FindChannelUsing( info.id );
	if (pChannel) {
		pPlayer = pChannel->GetPlayer();
		pPlayer->SetEventListener( info.pListener );
	}
}

static void DoIsAudioPlaying( CAudioMsgIsAudioPlaying* msg ) {
	tAudioID  			id = msg->GetData();
	CAudioReturnMessage	retMsg;
	CChannel*			pChannel = NULL;
	Boolean				isAudioActive = false;

	gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
		"AudioTask::DoIsAudioPlaying() -- Is audioID = %d Playing?\n", 
		static_cast<int>(id));	

	//	figure out if this audio file is still playing
	pChannel = gContext.pAudioMixer->FindChannelUsing( id );
	
	if (pChannel)
		isAudioActive = true;
	
	// Send the result back to the caller
	retMsg.SetBooleanResult( isAudioActive );
	
	SendMsgToAudioModule( retMsg );
}

static void DoIsAnyAudioPlaying( CAudioMsgIsAudioPlaying* /* msg */) {
	CAudioReturnMessage	retMsg;
	Boolean				isAudioActive;

	gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
		"AudioTask::DoIsAnyAudioPlaying() -- Is ANY audio playing?\n");	

	// figure out if any audio is playing
	isAudioActive = gContext.pAudioMixer->IsAnyAudioActive();
	
	retMsg.SetBooleanResult( isAudioActive );
	SendMsgToAudioModule( retMsg );
}

//==============================================================================
//==============================================================================
static void DoPauseAudio( CAudioMsgPauseAudio* pMsg ) 
{
	CChannel*	pChannel = kNull;
	tAudioID	id = pMsg->GetData();

	// Find the best channel for the specified priority
	pChannel = gContext.pAudioMixer->FindChannelUsing( id );

	if (pChannel)
		pChannel->Pause();
}

// ==============================================================================
// DoResumeAudio
// ==============================================================================
static void DoResumeAudio( CAudioMsgResumeAudio* pMsg ) 
{
tAudioID	id = pMsg->GetData();

// Find channel with specified ID
CChannel *pChannel = gContext.pAudioMixer->FindChannelUsing( id );

if (pChannel && pChannel->IsPaused())
	pChannel->Resume();
}

// ==============================================================================
// DoStopAudio
// ==============================================================================
static void DoStopAudio( CAudioMsgStopAudio* pMsg ) 
{
tAudioStopAudioInfo*	pAudioInfo = pMsg->GetData();

//printf( "AudioTask::DoStopAudio() Stopping ID = %d\n", (int)pAudioInfo->id);	

// Find channel with specified ID
CChannel *pCh = gContext.pAudioMixer->FindChannelUsing( pAudioInfo->id );
//printf("AudioTask::DoStopAudio() AudioID %d on channel $%p\n", (int)pAudioInfo->id, (void*)pCh);	

	if (pCh && pCh->IsInUse())
		pCh->Release( true); //pAudioInfo->suppressDoneMsg );
}

//==============================================================================
// 		Overall Audio System Control
//==============================================================================
static tErrType StartAudioSystem( void )
{
	tErrType err = kNoErr;
	CAudioReturnMessage	msg;
	
	if (!gContext.audioOutputOn && !gContext.audioOutputPaused) {
		gContext.audioOutputOn = true;
		err = StartAudioOutput();
	}
	
	return err;
}

//----------------------------------------------------------------------------
static void StopAudioSystem( void )
{
	tErrType err = kNoErr;
	CAudioReturnMessage	msg;
	
	if (gContext.audioOutputOn) {
		gContext.audioOutputOn = false;
		err = StopAudioOutput();
	}
}
//==============================================================================
//==============================================================================

static tErrType DoPauseAudioSystem( void )
{
	tErrType err = kNoErr;
	CAudioReturnMessage	msg;
	
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
	
	if (gContext.audioOutputOn && gContext.audioOutputPaused) {
		gContext.audioOutputPaused = false;
		err = StartAudioOutput();
	}
	
	// Send the status back to the caller
	msg.SetAudioErr( err );
	SendMsgToAudioModule( msg );

	return err;
}


//==============================================================================
//==============================================================================
static void DoAcquireMidiPlayer( void ) {
	CAudioReturnMessage	msg;
	
	gContext.pMidiPlayer->Activate();
	
	// Send the status back to the caller
	msg.SetAudioErr( kNoErr );
	msg.SetMidiID( gContext.pMidiPlayer->GetID() );
	SendMsgToAudioModule( msg );
}

//==============================================================================
//==============================================================================
static void DoReleaseMidiPlayer( void ) {
	CAudioReturnMessage	msg;
		
	gContext.pMidiPlayer->DeActivate();

	// Send the status back to the caller
	msg.SetAudioErr( kNoErr );
	msg.SetMidiID( 1 );
	SendMsgToAudioModule( msg );
}

//==============================================================================
//==============================================================================
static void DoMidiNoteOn( CAudioMsgMidiNoteOn* msg ) {
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
static void DoMidiNoteOff( CAudioMsgMidiNoteOff* msg ) {
	tAudioMidiNoteInfo* info = msg->GetData();
	
	gContext.pMidiPlayer->NoteOff( 
			info->channel,
			info->noteNum,
			info->velocity,
			info->flags );
	
	//	info.priority = kAudioDefaultPriority;

}

static void DoStartMidiFile( CAudioMsgStartMidiFile* msg ) {
	tErrType					result;
	int							err;
	int							bytesRead;
	tAudioStartMidiFileInfo* 	pInfo = msg->GetData();
	CAudioReturnMessage			retMsg;
	struct stat					fileStat;
	FILE*						file;
	
	gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTask::DoStartMidiFile -- Info: vol:%d, pri:%d, path:%s, listen:0x%x, payload:%d, flags:%d \n", 
				pInfo->volume, pInfo->priority, pInfo->path->c_str(),
				reinterpret_cast<unsigned int>(pInfo->pListener), static_cast<int>(pInfo->payload), static_cast<int>(pInfo->flags) );

	// Find out how big the MIDI file is.
	err = stat( pInfo->path->c_str(), &fileStat );
	gContext.pDebugMPI->Assert((kNoErr == err), 
		"AudioTask::DoStartMidiFile() -- MIDI file does not appear to exist. path = %s \n", 
		pInfo->path->c_str() );
	
	// Malloc the buffer for the file image and store the size.
	pInfo->pMidiFileImage = (U8*)gContext.pKernelMPI->Malloc( fileStat.st_size );
	pInfo->imageSize = fileStat.st_size;
	
	// Open the file and load the image.
	file = fopen( pInfo->path->c_str(), "r" );
	bytesRead = fread( pInfo->pMidiFileImage, 1, fileStat.st_size, file );
	err = fclose( file );
		
	result = gContext.pMidiPlayer->StartMidiFile( pInfo );
	gContext.pDebugMPI->Assert((kNoErr == result), 
		"AudioTask::DoStartMidiFile() -- Failed to start MIDI file. result = %d \n", 
		static_cast<int>(result) );

	// Send the status back to the caller
//	gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTask::DoStartMidiFile -- Sending result back to Module...\n" );
	retMsg.SetAudioErr( result );
	SendMsgToAudioModule( retMsg );
}

static void DoIsMidiFilePlaying( CAudioMsgIsMidiFilePlaying* /* msg */) {
//	tAudioID  			id = msg->GetData();
	CAudioReturnMessage	retMsg;

	gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
		"AudioTask::DoIsMidiFilePlaying() -- Is ANY audio playing?\n");	

	// figure out if MIDI file is playing and send answer back to Module
	retMsg.SetBooleanResult( gContext.pMidiPlayer->IsFileActive() );
	SendMsgToAudioModule( retMsg );

//	return err;
}

// If we have more than one MIDI player in the future, this needs to be changed.
static void DoIsAnyMidiFilePlaying( CAudioMsgIsMidiFilePlaying* /* msg */) {
	CAudioReturnMessage	retMsg;

	gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
		"AudioTask::DoIsAnyMidiFilePlaying() -- Is ANY audio playing?\n");	

	// figure out if MIDI file is playing and send answer back to Module
	retMsg.SetBooleanResult( gContext.pMidiPlayer->IsFileActive() );
	SendMsgToAudioModule( retMsg );

//	return err;
}

static void DoPauseMidiFile( CAudioMsgPauseMidiFile* /* msg */) {
//	tMidiPlayerID  	id = msg->GetData();

	gContext.pMidiPlayer->PauseMidiFile();
}

static void DoResumeMidiFile( CAudioMsgResumeMidiFile* /* msg */) {
//	tMidiPlayerID  	id = msg->GetData();

	gContext.pMidiPlayer->ResumeMidiFile();
}

static void DoStopMidiFile( CAudioMsgStopMidiFile* msg ) {
	tAudioStopMidiFileInfo* 	pInfo = msg->GetData();

	gContext.pMidiPlayer->StopMidiFile( pInfo );
}

static void DoGetEnabledMidiTracks( CAudioMsgMidiFilePlaybackParams* /* msg */) {

	tErrType				result;
	CAudioReturnMessage		retMsg;
	tMidiTrackBitMask 		trackBitMask;

//	tAudioMidiFilePlaybackParams* pParams = msg->GetData();
	// pParams->id;  for the future

	result = gContext.pMidiPlayer->GetEnableTracks( &trackBitMask );
	
	// Send the status back to the caller
	if (result != kNoErr)
		retMsg.SetAudioErr( result );
	else
		retMsg.SetU32Result( trackBitMask );

	SendMsgToAudioModule( retMsg );
}

static void DoSetEnableMidiTracks( CAudioMsgMidiFilePlaybackParams* msg ) {
	tErrType				result;
	CAudioReturnMessage		retMsg;
	tAudioMidiFilePlaybackParams* pParams = msg->GetData();
	
	// pParams->id; for the future
	
	result = gContext.pMidiPlayer->SetEnableTracks( pParams->trackBitMask );
	
	// Send the status back to the caller
	retMsg.SetAudioErr( result );

	SendMsgToAudioModule( retMsg );
}
static void DoTransposeMidiTracks( CAudioMsgMidiFilePlaybackParams* msg ) {
	tErrType				result;
	CAudioReturnMessage		retMsg;
	tAudioMidiFilePlaybackParams* pParams = msg->GetData();

	// pParams->id; for the future
	
	result = gContext.pMidiPlayer->TransposeTracks( pParams->trackBitMask, pParams->transposeAmount );
	
	// Send the status back to the caller
	retMsg.SetAudioErr( result );

	SendMsgToAudioModule( retMsg );
}
 
static void DoChangeMidiInstrument( CAudioMsgMidiFilePlaybackParams* msg ) {
	tErrType				result;
	CAudioReturnMessage		retMsg;
	tAudioMidiFilePlaybackParams* pParams = msg->GetData();

	// pParams->id; for the future
	
	result = gContext.pMidiPlayer->ChangeProgram( pParams->trackBitMask, pParams->instrument );
	
	// Send the status back to the caller
	retMsg.SetAudioErr( result );

	SendMsgToAudioModule( retMsg );
}

static void DoChangeMidiTempo( CAudioMsgMidiFilePlaybackParams* msg ) {
	tErrType				result;
	CAudioReturnMessage		retMsg;
	tAudioMidiFilePlaybackParams* pParams = msg->GetData();

	// pParams->id; for the future
	
	result = gContext.pMidiPlayer->ChangeTempo( pParams->tempo );
	
	// Send the status back to the caller
	retMsg.SetAudioErr( result );

	SendMsgToAudioModule( retMsg );
}

//==============================================================================
//==============================================================================
void* AudioTaskMain( void* /*arg*/ )
{
	tErrType 			err = kNoErr;
	U32 				i = 0;
	tMessageQueueHndl 	hQueue;
	
	// Set appropriate debug level
	gContext.pDebugMPI->SetDebugLevel( kAudioDebugLevel );

    gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
		"AudioTaskMain() -- Audio task starting to run...\n" );	

    // Create a msg queue that allows the Audio Task to RECEIVE msgs from the audio module.

	// Begin by setting up properties with EXCL so we can detect an existing
	// Queue and delete it if it was orphaned.
	tMessageQueuePropertiesPosix msgQueueProperties = 
	{
	    0,								// msgProperties.blockingPolicy;  
	    "/audioTaskIncomingQ",			// msgProperties.nameQueue
	    B_S_IRUSR|B_S_IWUSR,			// msgProperties.mode, file permission bits
	    B_O_RDONLY|B_O_CREAT|B_O_EXCL,	// msgProperties.oflag, queue creation props
	    0,								// msgProperties.priority
	    0,								// msgProperties.mq_flags
	    kAUDIO_MAX_NUM_MSGS,			// msgProperties.mq_maxmsg
	    kAUDIO_MAX_MSG_SIZE,			// msgProperties.mq_msgsize
	    0								// msgProperties.mq_curmsgs
	};
	
    gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
		"AudioTaskMain() -- Attempting to create incoming msg queue.\n" );	

    err = gContext.pKernelMPI->OpenMessageQueue( gContext.hRecvMsgQueue, msgQueueProperties, NULL );
    
    if (err != kNoErr) {
		if (err == EEXIST) {
			// Apparently the queue already exists, so lets delete it and create 
			// a new one in case properties/size have changed since the last time 
			// a queue was created.
			gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
				"AudioTaskMain() -- Attempting to delete orphaned incoming msg queue.\n" );
			
			msgQueueProperties.oflag = B_O_RDONLY;
		   	err = gContext.pKernelMPI->OpenMessageQueue( hQueue, msgQueueProperties, NULL );
		    gContext.pDebugMPI->Assert((kNoErr == err), 
		    	"AudioTaskMain() -- Trying to open incoming msg queue in preparation for deletion. err = %d \n", 
		    	static_cast<int>(err) );
	
		    err = gContext.pKernelMPI->CloseMessageQueue( hQueue, msgQueueProperties );
		    gContext.pDebugMPI->Assert((kNoErr == err), 
		    	"AudioTaskMain() -- Trying to close/delete incoming msg queue. err = %d \n", 
		    	static_cast<int>(err) );
		    
		    hQueue = 0;

		    // Now that the old one is deleted, create a new one.
		    gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
				"Audio Task creating task incoming Q. size = %d\n", 
				static_cast<int>(kAUDIO_MAX_MSG_SIZE) );	

		    msgQueueProperties.oflag = B_O_RDONLY|B_O_CREAT;
		   	err = gContext.pKernelMPI->OpenMessageQueue( gContext.hRecvMsgQueue, msgQueueProperties, NULL );
		    gContext.pDebugMPI->Assert((kNoErr == err), 
		    	"AudioTaskMain() -- Trying to create incoming msg queue after deletion of orphan. err = %d \n", 
		    	static_cast<int>(err) );

		} else {
			// Unknown error, bail out.
			gContext.pDebugMPI->Assert((kNoErr == err), 
		    	"AudioTaskMain() -- Trying to create incoming audio task msg queue. err = %d \n", 
		    	static_cast<int>(err) );			
		}
	}
 
    // Now create a msg queue that allows the Audio Task to send msgs back to Audio Module
   gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
		"AudioTaskMain() -- Attempting to create outgoing msg queue.\n" );	

	msgQueueProperties.nameQueue = "/audioTaskOutgoingQ";
    msgQueueProperties.oflag = B_O_WRONLY|B_O_CREAT|B_O_EXCL;

 	err = gContext.pKernelMPI->OpenMessageQueue( gContext.hSendMsgQueue, msgQueueProperties, NULL );
	if (err != kNoErr) {
		if (err == EEXIST) {
			// Apparently the queue already exists, so lets delete it and create 
			// a new one in case properties/size have changed since the last time 
			// a queue was created.
			gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
				"AudioTaskMain() -- Attempting to delete orphaned outgoing msg queue.\n" );
			
			msgQueueProperties.oflag = B_O_WRONLY;
		   	err = gContext.pKernelMPI->OpenMessageQueue( hQueue, msgQueueProperties, NULL );
		    gContext.pDebugMPI->Assert((kNoErr == err), 
		    	"AudioTaskMain() -- Trying to open outgoing msg queue in preparation for deletion. err = %d \n", 
		    	static_cast<int>(err) );
	
		    err = gContext.pKernelMPI->CloseMessageQueue( hQueue, msgQueueProperties );
		    gContext.pDebugMPI->Assert((kNoErr == err), 
		    	"AudioTaskMain() -- Trying to close/delete outgoing msg queue. err = %d \n", 
		    	static_cast<int>(err) );
		    
		    hQueue = 0;

		    // Now that the old one is deleted, create a new one.
		    gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
				"AudioTaskMain() -- creating task outgoing Q. size = %d\n", 
				static_cast<int>(kAUDIO_MAX_MSG_SIZE) );	

		    msgQueueProperties.oflag = B_O_WRONLY|B_O_CREAT;
		   	err = gContext.pKernelMPI->OpenMessageQueue( gContext.hSendMsgQueue, msgQueueProperties, NULL );
		    gContext.pDebugMPI->Assert((kNoErr == err), 
		    	"AudioTaskMain() v Trying to create outgoing msg queue after deletion of orphan. err = %d \n", 
		    	static_cast<int>(err) );

		} else {
			// Unknown error, bail out.
			gContext.pDebugMPI->Assert((kNoErr == err), 
		    	"AudioTaskMain() Trying to create outgoing audio task msg queue. err = %d \n", 
		    	static_cast<int>(err) );			
		}
	}
    
	// Startup Audio Output
	StartAudioSystem();	

    // Set the task to ready
	gContext.audioTaskRunning = true;
	
	char 				msgBuf[kAUDIO_MAX_MSG_SIZE];
	tAudioCmdMsgType 	cmdType;
	U32					msgSize;
	CAudioCmdMsg*		pAudioMsg;
	
	while ( gContext.threadRun ) {
//		gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "\n---------- AudioTaskMain() -- top of loop, cnt: %u.\n", static_cast<unsigned int>(i++));	
   
		// Wait for message.
	    err = gContext.pKernelMPI->ReceiveMessage( gContext.hRecvMsgQueue, 
								   (CMessage*)msgBuf, kAUDIO_MAX_MSG_SIZE );
								   
		    gContext.pDebugMPI->AssertNoErr( err, "AudioTask::MainLoop; Could not get cmd message from AudioModule.\n" );
	
	    pAudioMsg = reinterpret_cast<CAudioCmdMsg*>(msgBuf);
		msgSize = pAudioMsg->GetMessageSize();
		cmdType = pAudioMsg->GetCmdType();
//	    gContext.pDebugMPI->DebugOut(kDbgLvlVerbose, 
//	    	"AudioTaskMain() -- Got Audio Command, Type= %d; Msg Size = %d \n", 
//	    	cmdType, static_cast<int>(msgSize) );  

	    // Process incoming audio msgs based on command type.
		switch ( cmdType ) {
			//*********************************
			case kAudioCmdMsgTypeSetMasterVolume:
gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Setting master volume.\n" );	
	
				DoSetMasterVolume( (CAudioMsgSetMasterVolume*)pAudioMsg );				   
				break;
	
			//*********************************
			case kAudioCmdMsgTypeSetOutputEqualizer:
gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Adjust output equalizer enable.\n" );	
	
				DoSetOutputEqualizer( (CAudioMsgSetOutputEqualizer*)pAudioMsg );				   
				break;

			//*********************************
			case kAudioCmdMsgTypePauseAllAudio:
//gContext.pDebugMPI->DebugOut( kDbgLvlVerbose,  "AudioTaskMain() Pause all audio.\n" );	
		
				DoPauseAudioSystem();
				break;
	
			//*********************************
			case kAudioCmdMsgTypeResumeAllAudio:
//gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Resume all audio.\n" );	
	
				DoResumeAudioSystem();
				break;
	
			//*********************************
			case kAudioCmdMsgTypeStartAudio:
//				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
//					"AudioTaskMain() -- kAudioCmdMsgTypeStartAudio : Calling DoStartAudio().\n" );	
		
				DoStartAudio( (CAudioMsgStartAudio*)pAudioMsg );
				break;
	
			//*********************************
			case kAudioCmdMsgTypeGetAudioTime:
// gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Get Audio Time.\n" );	
	
				DoGetAudioTime( (CAudioMsgGetAudioTime*)pAudioMsg );
				break;

			//*********************************
			case kAudioCmdMsgTypeGetAudioVolume:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Get Volume.\n" );	
	
				DoGetAudioVolume( (CAudioMsgGetAudioVolume*)pAudioMsg );
				break;
						
			//*********************************
			case kAudioCmdMsgTypeSetAudioVolume:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Set Volume.\n" );	
		
				DoSetAudioVolume( (CAudioMsgSetAudioVolume*)pAudioMsg);
				break;
				
			//*********************************
			case kAudioCmdMsgTypeGetAudioPriority:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Get Priority.\n" );	
	
				DoGetAudioPriority( (CAudioMsgGetAudioPriority*)pAudioMsg );
				break;
						
			//*********************************
			case kAudioCmdMsgTypeSetAudioPriority:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Set Priority.\n" );	
		
				DoSetAudioPriority( (CAudioMsgSetAudioPriority*)pAudioMsg);
				break;

			//*********************************
			case kAudioCmdMsgTypeGetAudioPan:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Get Pan.\n" );	
	
				DoGetAudioPan( (CAudioMsgGetAudioPan*)pAudioMsg );
				break;
						
			//*********************************
			case kAudioCmdMsgTypeSetAudioPan:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Set Pan.\n" );	
		
				DoSetAudioPan( (CAudioMsgSetAudioPan*)pAudioMsg);
				break;

			//*********************************
			case kAudioCmdMsgTypeGetAudioListener:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Get Listener.\n" );	
	
				DoGetAudioListener( (CAudioMsgGetAudioListener*)pAudioMsg );
				break;
						
			//*********************************
			case kAudioCmdMsgTypeSetAudioListener:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Set Listener.\n" );	
		
				DoSetAudioListener( (CAudioMsgSetAudioListener*)pAudioMsg);
				break;

			//*********************************
			case kAudioCmdMsgTypeIsAnyAudioPlaying:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Is ANY audio playing?\n" );	
		
				DoIsAnyAudioPlaying( (CAudioMsgIsAudioPlaying*)pAudioMsg);
				break;

				//*********************************
				case kAudioCmdMsgTypeIsAudioPlaying:
					gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Is ANY audio playing?\n" );	
			
					DoIsAudioPlaying( (CAudioMsgIsAudioPlaying*)pAudioMsg);
					break;

			//*********************************
			case kAudioCmdMsgTypePauseAudio:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Pause audio.\n" );	
	
				DoPauseAudio( (CAudioMsgPauseAudio*)pAudioMsg );
				break;
	
			//*********************************
			case kAudioCmdMsgTypeResumeAudio:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose,  "AudioTaskMain() Resume audio.\n" );	
	
				DoResumeAudio( (CAudioMsgResumeAudio*)pAudioMsg );
				break;
	
			//*********************************
			case kAudioCmdMsgTypeStopAudio:
//				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Stop audio.\n" );	
	
				DoStopAudio( (CAudioMsgStopAudio*)pAudioMsg );
				break;
				
			//*********************************
			case kAudioCmdMsgTypeAcquireMidiPlayer:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Acquire MIDI Player.\n" );	
	
				DoAcquireMidiPlayer();
				break;
	
			//*********************************
			case kAudioCmdMsgTypeReleaseMidiPlayer:
//				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Release MIDI Player.\n" );	
	
				DoReleaseMidiPlayer();
				break;
	
					//*********************************
			case kAudioCmdMsgTypeMidiNoteOn:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose,  "AudioTaskMain() MIDI note On.\n" );	
	
				DoMidiNoteOn( (CAudioMsgMidiNoteOn*)pAudioMsg );
				break;
	
			//*********************************
			case kAudioCmdMsgTypeMidiNoteOff:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose,  "AudioTaskMain() MIDI note Off.\n" );	
				
				DoMidiNoteOff( (CAudioMsgMidiNoteOff*)pAudioMsg );
				break;
	
			//*********************************
			case kAudioCmdMsgTypeStartMidiFile:
//				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Start MIDI file.\n" );	
				
				DoStartMidiFile( (CAudioMsgStartMidiFile*)pAudioMsg );
				break;
	
			//*********************************
			case kAudioCmdMsgTypeIsMidiFilePlaying:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain(): Is MIDI file playing?\n" );	
		
				DoIsMidiFilePlaying( (CAudioMsgIsMidiFilePlaying*)pAudioMsg);
				break;
				
			//*********************************
			case kAudioCmdMsgTypeIsAnyMidiFilePlaying:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Is ANY MIDI file playing?\n" );	
		
				DoIsAnyMidiFilePlaying( (CAudioMsgIsMidiFilePlaying*)pAudioMsg);
				break;

			//*********************************
			case kAudioCmdMsgTypePauseMidiFile:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Pause MIDI file.\n" );	
				
				DoPauseMidiFile( (CAudioMsgPauseMidiFile*)pAudioMsg );
				break;
				
			//*********************************
			case kAudioCmdMsgTypeResumeMidiFile:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Resume MIDI file.\n" );	
				
				DoResumeMidiFile( (CAudioMsgResumeMidiFile*)pAudioMsg );
				break;
	
			//*********************************
			case kAudioCmdMsgTypeStopMidiFile:
//				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Stop MIDI file.\n" );	
				
				DoStopMidiFile( (CAudioMsgStopMidiFile*)pAudioMsg );
				break;
	
			//*********************************
			case kAudioCmdMsgTypeGetEnabledMidiTracks:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() GetEnabled MIDI tracks.\n" );	
				
				DoGetEnabledMidiTracks( (CAudioMsgMidiFilePlaybackParams*)pAudioMsg );
				break;

			//*********************************
			case kAudioCmdMsgTypeSetEnableMidiTracks:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Enable MIDI tracks.\n" );	
				
				DoSetEnableMidiTracks( (CAudioMsgMidiFilePlaybackParams*)pAudioMsg );
				break;

			//*********************************
			case kAudioCmdMsgTypeTransposeMidiTracks:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Transpose MIDI tracks.\n" );	
				
				DoTransposeMidiTracks( (CAudioMsgMidiFilePlaybackParams*)pAudioMsg );
				break;

			//*********************************
			case kAudioCmdMsgTypeChangeMidiInstrument:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Change MIDI instrument.\n" );	
				
				DoChangeMidiInstrument( (CAudioMsgMidiFilePlaybackParams*)pAudioMsg );
				break;

			//*********************************
			case kAudioCmdMsgTypeChangeMidiTempo:
				gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Change MIDI tempo.\n" );	
				
				DoChangeMidiTempo( (CAudioMsgMidiFilePlaybackParams*)pAudioMsg );
				break;

				//*********************************
				case kAudioCmdMsgExitThread:
					gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Exiting Audio Thread.\n" );	
					
				gContext.threadRun = false;
				break;
				
				
			default:
			gContext.pDebugMPI->DebugOut( kDbgLvlCritical, 
				"AudioTaskMain() -- unhandled audio command!!!.\n" );	
	
			break;
		}
	}

	gContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
		"AudioTaskMain() -- exiting, loop cnt: %u.\n", static_cast<unsigned int>(i));	
		
	// Exit nicely
	StopAudioSystem();	

	// Set the task to byebye
	gContext.audioTaskRunning = false;

	return (void *)kNull;
}

LF_END_BRIO_NAMESPACE()
// EOF

