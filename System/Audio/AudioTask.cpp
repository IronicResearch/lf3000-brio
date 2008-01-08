//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// AudioTask.cpp
//
//==============================================================================
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

#include <AudioTypes.h>
#include <AudioTask.h>
#include <AudioOutput.h>
#include <AudioMsg.h>
#include <AudioPriv.h>

#include <KernelMPI.h>
#include <DebugMPI.h>

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
	Boolean				audioTaskRunning;	
	CAudioMixer*		pAudioMixer;		
	CMidiPlayer*		pMidiPlayer;		// ptr to Mixer's single MIDI player 	
//	CAudioEffectsProcessor	*pAudioEffects;	// Pointer to the global audio effects processor

	U16					outBufferSizeInBytes;		
	U16					outBufferSizeInWords;	
	Boolean				audioOutputDriverOn;	
	Boolean				audioOutputDriverPaused;
	volatile Boolean	audioTaskReady;	
	tAudioID			nextAudioID;	

	U8					masterVolume;		// GK NOTE: free of the burden of "units"
	U8					outputEqualizerEnabled;		
	U8					numMixerChannels;
	U32					samplingFrequency; // Hertz

	IEventListener*		pDefaultListener;
	
	tMessageQueueHndl 	hSendMsgQueue;
	tMessageQueueHndl 	hRecvMsgQueue;	
};
}

tAudioContext gAudioContext;

// Globals 
typedef struct{
        int numTest;
        char *testDescription;
}thread_arg_t;


// Prototypes
void* AudioTaskMain( void* arg );

//==============================================================================
// InitAudioTask
//==============================================================================
tErrType InitAudioTask( void )
{
	tErrType err;
	Boolean	ret;
	
	tTaskHndl pHndl;
	tTaskProperties properties;
	thread_arg_t threadArg;
	
	// Get Debug MPI
	gAudioContext.pDebugMPI = new CDebugMPI( kGroupAudio );
	ret = gAudioContext.pDebugMPI->IsValid();
	if (ret != true)
		printf("InitAudioTask() -- Couldn't create DebugMPI!\n");
	
	// Get Kernel MPI
	gAudioContext.pKernelMPI = new CKernelMPI;
	ret = gAudioContext.pKernelMPI->IsValid();
	if (ret != true)
		printf("InitAudioTask() -- Couldn't create KernelMPI!\n");

// gAudioContext.pDebugMPI->DebugOut( kDbgLvlValuable, "InitAudioTask() -Debug and Kernel MPIs created.\n");	

	// Setup debug level.
	gAudioContext.pDebugMPI->SetDebugLevel( kAudioDebugLevel );

	// Hard code configuration resource
	gAudioContext.numMixerChannels  = kAudioNumMixerChannels;
	gAudioContext.samplingFrequency = kAudioSampleRate;

	// Set output buffer sizes.  These values are based on
	// 20ms buffer of stereo samples: see AudioConfig.h
	gAudioContext.outBufferSizeInWords = kAudioOutBufSizeInWords;
	gAudioContext.outBufferSizeInBytes = kAudioOutBufSizeInBytes;

	// Allocate global audio mixer
	gAudioContext.pAudioMixer = new CAudioMixer( kAudioNumMixerChannels );
	
	gAudioContext.pMidiPlayer = gAudioContext.pAudioMixer->GetMidiPlayerPtr();
	
	// Initialize audio out driver
	gAudioContext.audioOutputDriverOn     = false;
	gAudioContext.audioOutputDriverPaused = false;
	
	// Init output driver and register callback.  We have to pass in a pointer
	// to the mixer object as a bit of "user data" so that when the callback happens,
	// the C call can get to the mixer's C++ member function for rendering.  
	err = InitAudioOutput( &CAudioMixer::WrapperToCallRender, (void *)gAudioContext.pAudioMixer );
	gAudioContext.pDebugMPI->Assert( kNoErr == err, "InitAudioTask() Failed to initalize audio output\n" );

	//pAudioEffects = kNull;
 
	gAudioContext.nextAudioID = 0;

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
	gAudioContext.threadRun = true;
	err = gAudioContext.pKernelMPI->CreateTask( pHndl, properties, NULL );

	gAudioContext.pDebugMPI->Assert( kNoErr == err, "InitAudioTask() -- Failed to create AudioTask!\n" );

	// Wait for the Audio task to be ready
	while (!gAudioContext.audioTaskRunning)
	{
		gAudioContext.pKernelMPI->TaskSleep(10);
		gAudioContext.pDebugMPI->DebugOut(kDbgLvlVerbose, 
			"AudioTask::InitAudioTask -- waiting for the audio task to start up...\n");	
	}

	gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTask Started...\n" );
	return err;
}

// ==============================================================================
// DeInitAudioTask
// ==============================================================================
void DeInitAudioTask( void )
{
	gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "DeInitAudioTask called...\n");	

	// Wait for task to die
	while (gAudioContext.audioTaskRunning)
	{
		gAudioContext.pKernelMPI->TaskSleep(10);
		gAudioContext.pDebugMPI->DebugOut(kDbgLvlVerbose, 
			"AudioTask::DeInitAudioTask: Waiting for audio task to die...\n");	
	}

	DeInitAudioOutput();
	
	delete gAudioContext.pKernelMPI;
	delete gAudioContext.pDebugMPI;
}   // ---- end DeInitAudioTask() ----

// ==============================================================================
// SendMsgToAudioModule
// ==============================================================================
static void SendMsgToAudioModule( CAudioReturnMessage msg ) 
{
// 	gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTask:Sending msg back to module...\n");	
// 	gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTask:Return msg size = %d.\n", msg.GetMessageSize());	
  	
    tErrType err = gAudioContext.pKernelMPI->SendMessage( gAudioContext.hSendMsgQueue, msg );
 //   gAudioContext.pDebugMPI->AssertNoErr(err, "SendMsgToAudioModule() -- After call SendMessage().\n" );
}   // ---- end SendMsgToAudioModule() ----

// ==============================================================================
// DoSetMasterVolume
// ==============================================================================
static void DoSetMasterVolume( CAudioMsgSetMasterVolume* pMsg ) 
{
	U8 x = pMsg->GetData();
	gAudioContext.masterVolume = x;
	gAudioContext.pAudioMixer->SetMasterVolume( gAudioContext.masterVolume );
}   // ---- end DoSetMasterVolume() ----

// ==============================================================================
// DoSetOutputEqualizer:   Well, it does more than just set output equalizer
//                          for Didj
// ==============================================================================
static void DoSetOutputEqualizer( CAudioMsgSetOutputEqualizer* pMsg ) 
{
	U8 x = pMsg->GetData();
	gAudioContext.outputEqualizerEnabled = x;
	gAudioContext.pAudioMixer->EnableOutputSpeakerDSP( gAudioContext.outputEqualizerEnabled );
}   // ---- end DoSetOutputEqualizer() ----

// ==============================================================================
// DoStartAudio
// ==============================================================================
static void DoStartAudio( CAudioMsgStartAudio *pMsg ) 
{
	CAudioReturnMessage	msg;
	CPath				filename;
	CPath				fileExt;

	tAudioStartAudioInfo *pAi = pMsg->GetData();
	int strIndex = pAi->path->size(); 

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
	tAudioID newID = gAudioContext.nextAudioID++;
	if (kNoAudioID == gAudioContext.nextAudioID)
		gAudioContext.nextAudioID = 0;

//printf("---- AUDIO_PLAYER_ADDITION\n");
	// Create player based on file extension
    if (kNoErr == gAudioContext.pAudioMixer->AddPlayer( pAi, sExt, newID))
        {
		msg.SetAudioErr( kNoErr );
		msg.SetAudioID( newID );
        }

// Send audioID and error code back to caller
	SendMsgToAudioModule( msg );
}   // ---- end DoStartAudio() ----

// ==============================================================================
// DoGetAudioTime   Get current time from audio player
//
// ==============================================================================
static void DoGetAudioTime( CAudioMsgGetAudioTime* msg ) 
{
tAudioID  			id = msg->GetData();
CAudioReturnMessage	retMsg;
U32					time = 0;
 
CChannel *pChannel = gAudioContext.pAudioMixer->FindChannel( id );
if (pChannel) 
    {
	CAudioPlayer *pPlayer = pChannel->GetPlayer();
    if (pPlayer)
	    time = pPlayer->GetAudioTime_mSec();
//    else
//        printf("DoGetAudioTime: Unable to find player for id=%ld\n", id);
    }
//else
//   printf("DoGetAudioTime: Unable to find channel for id=%ld\n", id);

//printf("AudioTask::DoGetAudioTime(): GetAudioTime_mSec() ID=%ld time=%ld\n", id, time);	

// Send time in message back to caller
retMsg.SetU32Result( time );
SendMsgToAudioModule( retMsg );
}   // ---- end DoGetAudioTime() ----

// ==============================================================================
// DoGetAudioState: 
// ==============================================================================
static void DoGetAudioState( CAudioMsgGetAudioState *msg ) 
{
tAudioState 	d = msg->GetData();
CAudioReturnMessage	retMsg;

gAudioContext.pAudioMixer->GetAudioState( &d );

retMsg.SetAudioStateResult( d );
SendMsgToAudioModule( retMsg );
}  // ---- end DoGetAudioState() ----

// ==============================================================================
// DoSetAudioState: 
// ==============================================================================
    static void 
DoSetAudioState( CAudioMsgSetAudioState *msg ) 
{
tAudioState 	d = msg->GetData();
//printf("DoSetAudioState: d.srcType=%d\n", d.srcType);
gAudioContext.pAudioMixer->SetAudioState( &d );
}  // ---- end DoSetAudioVolume() ----

// ==============================================================================
// DoGetAudioVolume: 
// ==============================================================================
static void DoGetAudioVolume( CAudioMsgGetAudioVolume* msg ) 
{
tAudioVolumeInfo 	info = msg->GetData();
CAudioReturnMessage	retMsg;
U32					volume = 0;
CChannel*			pChannel;

//gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
//	"AudioTask::DoGetAudioVolume() -- Get Volume for audioID = %d...\n", (int)info.id);	

// Find current volume for channel using audioID
pChannel = gAudioContext.pAudioMixer->FindChannel( info.id );
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

//gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
//	"AudioTask::DoSetAudioVolume() Set Volume for audioID=%d\n", (int)info.id);	

//	Set requested property
pChannel = gAudioContext.pAudioMixer->FindChannel( info.id );
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
	
//	gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
//		"AudioTask::DoGetAudioPriority() Get Priority for audioID = %d\n", (int)info.id);	

	//	Find current Priority for channel using audioID
	pChannel = gAudioContext.pAudioMixer->FindChannel( info.id );
	if (pChannel) 
		priority = pChannel->GetPriority();
	
	// Send the Priority back to the caller
	retMsg.SetU32Result( (U32)priority );
	SendMsgToAudioModule( retMsg );
}  // ---- end DoGetAudioPriority() ----

// ==============================================================================
// DoSetAudioPriority: 
// ==============================================================================
static void DoSetAudioPriority( CAudioMsgSetAudioPriority* msg ) 
{
	tAudioPriorityInfo 	info = msg->GetData();
	CAudioReturnMessage	retMsg;
	CChannel*			pChannel;
	
//	gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
//		"AudioTask::DoSetAudioPriority() Set Priority for audioID = %d\n", (int)info.id);	

	//	Set requested property
	pChannel = gAudioContext.pAudioMixer->FindChannel( info.id );
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

//gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTask::DoGetAudioPan() for audioID=%d\n", static_cast<int>(info.id));	

// Find current Pan for channel using audioID
CChannel *pChannel = gAudioContext.pAudioMixer->FindChannel( info.id );
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

//printf("AudioTask::DoSetAudioPan() ID=%d pan=%d\n", (int)info.id, (int) info.pan);	

//	Set requested property
CChannel *pChannel = gAudioContext.pAudioMixer->FindChannel( info.id );
if (pChannel) 
	pChannel->SetPan( info.pan );
//else
//    printf("DoSetAudioPan: unable to find channel with ID=%3ld\n", info.id);
}  // ---- end DoSetAudioPan() ----

// ==============================================================================
// DoGetAudioListener: 
// ==============================================================================
static void DoGetAudioListener( CAudioMsgGetAudioListener* msg ) 
{
	tAudioListenerInfo 		info = msg->GetData();
	CAudioReturnMessage		retMsg;
	const IEventListener*	pListener = NULL;
//	CAudioPlayer*			pPlayer;
	
//printf("AudioTask::DoGetAudioListener() Get Listener for audioID = %d\n", (int)info.id);	

// Find listener ptr from channel with specified id
CChannel*pChannel = gAudioContext.pAudioMixer->FindChannel( info.id );
if (pChannel) 
    {
	pListener = pChannel->GetPlayer()->GetEventListener();
	}
	
// Send listener back to caller
retMsg.SetU32Result( (U32)pListener );
SendMsgToAudioModule( retMsg );
}  // ---- end DoGetAudioListener() ----

// ==============================================================================
// DoSetAudioListener:   
// ==============================================================================
static void DoSetAudioListener( CAudioMsgSetAudioListener* msg ) 
{
tAudioListenerInfo 	info = msg->GetData();

//printf("AudioTask::DoSetAudioListener() Set Listener for ID = %d\n", (int)info.id);	

//	Set requested property
CChannel *pChannel = gAudioContext.pAudioMixer->FindChannel( info.id );
if (pChannel) 
    {
	CAudioPlayer *pPlayer = pChannel->GetPlayer();
	pPlayer->SetEventListener( info.pListener );
	}
}  // ---- end DoSetAudioListener() ----

// ==============================================================================
// DoIsAudioPlaying:    GKFIXX NOTE:  why are both of these functions here ?
// ==============================================================================
    static void 
DoIsAudioPlaying( CAudioMsgIsAudioPlaying* msg ) 
{
tAudioID  			id = msg->GetData();
CAudioReturnMessage	retMsg;
CChannel*			pChannel = NULL;
Boolean				isAudioActive = false;

//printf("AudioTask::DoIsAudioPlaying() Is ID = %d Playing?\n",  (int)id);	

//	Is audio file is still playing
isAudioActive = (kNull != gAudioContext.pAudioMixer->FindFreeChannel( id ));
	
// Send result back to caller
retMsg.SetBooleanResult( isAudioActive );
	
SendMsgToAudioModule( retMsg );
}  // ---- end DoIsAudioPlaying() ----

// ==============================================================================
// DoIsAnyAudioPlaying:    GKFIXX NOTE:  why are both of these functions here ?
// ==============================================================================
    static void 
DoIsAnyAudioPlaying( CAudioMsgIsAudioPlaying* /* msg */) 
{
CAudioReturnMessage	retMsg;

//printf("AudioTask::DoIsAnyAudioPlaying() Audio playing?\n");	

// Is audio is playing
retMsg.SetBooleanResult(  gAudioContext.pAudioMixer->IsAnyAudioActive() );
SendMsgToAudioModule( retMsg );
}  // ---- end DoIsAnyAudioPlaying() ----

// ==============================================================================
// DoPauseAudio
// ==============================================================================
    static void 
DoPauseAudio( CAudioMsgPauseAudio* pMsg ) 
{
// Find channel with specified ID
CChannel *pChannel = gAudioContext.pAudioMixer->FindChannel( (tAudioID) pMsg->GetData() );

if (pChannel)
	pChannel->Pause();
}  // ---- end DoPauseAudio() ----

// ==============================================================================
// DoResumeAudio
// ==============================================================================
    static void 
DoResumeAudio( CAudioMsgResumeAudio* pMsg ) 
{
// Find channel with specified ID
CChannel *pChannel = gAudioContext.pAudioMixer->FindChannel( (tAudioID) pMsg->GetData() );

if (pChannel && pChannel->IsPaused())
	pChannel->Resume();
}  // ---- end DoResumeAudio() ----

// ==============================================================================
// DoStopAudio
// ==============================================================================
    static void 
DoStopAudio( CAudioMsgStopAudio* pMsg ) 
{
tAudioStopAudioInfo*	pAudioInfo = pMsg->GetData();

//printf( "AudioTask::DoStopAudio() Stopping ID = %d\n", (int)pAudioInfo->id);	

// Find channel with specified ID
CChannel *pCh = gAudioContext.pAudioMixer->FindChannel( pAudioInfo->id );
//printf("AudioTask::DoStopAudio() AudioID %d on channel $%p\n", (int)pAudioInfo->id, (void*)pCh);	

if (pCh && pCh->IsInUse())
	pCh->Release( true); //pAudioInfo->suppressDoneMsg );
}  // ---- end DoStopAudio() ----

//==============================================================================
// 		Overall Audio System Control
//==============================================================================


// ==============================================================================
// StartAudioSystem
// ==============================================================================
static tErrType StartAudioSystem( void )
{
	tErrType err = kNoErr;
	CAudioReturnMessage	msg;
	
	if (!gAudioContext.audioOutputDriverOn && !gAudioContext.audioOutputDriverPaused) 
    {
		gAudioContext.audioOutputDriverOn = true;
		err = StartAudioOutput();
	}
	
	return err;
}  // ---- end StartAudioSystem() ----

// ==============================================================================
// StopAudioSystem
// ==============================================================================
static void StopAudioSystem( void )
{
tErrType err = kNoErr;
CAudioReturnMessage	msg;

if (gAudioContext.audioOutputDriverOn) 
    {
	gAudioContext.audioOutputDriverOn = false;
	err = StopAudioOutput();
	}
}  // ---- end StopAudioSystem() ----

// ==============================================================================
// DoPauseAudioSystem
// ==============================================================================
static tErrType DoPauseAudioSystem( void )
{
tErrType err = kNoErr;
CAudioReturnMessage	msg;

if (gAudioContext.audioOutputDriverOn && !gAudioContext.audioOutputDriverPaused) 
    {
		gAudioContext.audioOutputDriverPaused = true;
		err = StopAudioOutput();
	}
	
// Send status back to caller
	msg.SetAudioErr( err );
	SendMsgToAudioModule( msg );

	return err;
}  // ---- end DoPauseAudioSystem() ----

// ==============================================================================
// DoResumeAudioSystem
// ==============================================================================
static tErrType DoResumeAudioSystem( void )
{
	tErrType err = kNoErr;
	CAudioReturnMessage	msg;
	
if (gAudioContext.audioOutputDriverOn && gAudioContext.audioOutputDriverPaused) 
    {
		gAudioContext.audioOutputDriverPaused = false;
		err = StartAudioOutput();
	}
	
	// Send status back to caller
	msg.SetAudioErr( err );
	SendMsgToAudioModule( msg );

	return err;
}  // ---- end DoResumeAudioSystem() ----

// ==============================================================================
// DoAcquireMidiPlayer
// ==============================================================================
static void DoAcquireMidiPlayer( void ) 
{
	CAudioReturnMessage	msg;
	
	gAudioContext.pMidiPlayer->Activate();
	
	// Send status back to caller
	msg.SetAudioErr( kNoErr );
	msg.SetMidiID( gAudioContext.pMidiPlayer->GetID() );
	SendMsgToAudioModule( msg );
}   // ---- end DoAcquireMidiPlayer() ----

// ==============================================================================
// DoReleaseMidiPlayer
// ==============================================================================
static void DoReleaseMidiPlayer( void ) 
{
CAudioReturnMessage	msg;
		
gAudioContext.pMidiPlayer->DeActivate();

// Send status back to caller
msg.SetAudioErr( kNoErr );
msg.SetMidiID( 1 );
SendMsgToAudioModule( msg );
}   // ---- end DoReleaseMidiPlayer() ----

// ==============================================================================
// DoMidiNoteOn
// ==============================================================================
static void DoMidiNoteOn( CAudioMsgMidiNoteOn* msg ) 
{
tAudioMidiNoteInfo* d = msg->GetData();
	
gAudioContext.pMidiPlayer->NoteOn( d->channel, d->note, d->velocity, d->flags );
	//	d.priority = kAudioDefaultPriority;
}   // ---- end DoMidiNoteOn() ----

// ==============================================================================
// DoMidiNoteOff
// ==============================================================================
static void DoMidiNoteOff( CAudioMsgMidiNoteOff* msg ) 
{
tAudioMidiNoteInfo* d = msg->GetData();

gAudioContext.pMidiPlayer->NoteOff( d->channel, d->note, d->velocity, d->flags );
	//	d.priority = kAudioDefaultPriority;
}   // ---- end DoMidiNoteOff() ----

// ==============================================================================
// DoMidiCommand
// ==============================================================================
static void DoMidiCommand( CAudioMsgMidiCommand *msg ) 
{
tAudioMidiCommandInfo *d = msg->GetData();
gAudioContext.pMidiPlayer->SendCommand( d->cmd, d->data1, d->data2 );
	//	d->priority = kAudioDefaultPriority;
}   // ---- end DoMidiNoteOff() ----

// ==============================================================================
// DoStartMidiFile:
// ==============================================================================
static void DoStartMidiFile( CAudioMsgStartMidiFile* msg ) {
tErrType					result;
int							err;
int							bytesRead;
tAudioStartMidiFileInfo* 	pInfo = msg->GetData();
CAudioReturnMessage			retMsg;
struct stat					fileStat;
FILE*						file;

//gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTask::DoStartMidiFile vol=%d pri=%d path='%s' listener=%p payload=%d flags=%X \n", pInfo->volume, pInfo->priority, pInfo->path->c_str(),
//			(void *)pInfo->pListener, (int)pInfo->payload, (int)pInfo->flags);

// Determine size of MIDI file
err = stat( pInfo->path->c_str(), &fileStat );
gAudioContext.pDebugMPI->Assert((kNoErr == err), 
	"AudioTask::DoStartMidiFile()  file doesn't exist '%s' \n", pInfo->path->c_str() );

// Allocate buffer for file image
pInfo->pMidiFileImage = (U8*)gAudioContext.pKernelMPI->Malloc( fileStat.st_size );
pInfo->imageSize      = fileStat.st_size;

// Load image
file = fopen( pInfo->path->c_str(), "r" );
bytesRead = fread( pInfo->pMidiFileImage, sizeof(char), fileStat.st_size, file );
err = fclose( file );
	
result = gAudioContext.pMidiPlayer->StartMidiFile( pInfo );
gAudioContext.pDebugMPI->Assert((kNoErr == result), 
	"AudioTask::DoStartMidiFile(): Failed to start MIDI file. result=%d \n", (int)result );

// Send status back to caller
retMsg.SetAudioErr( result );
SendMsgToAudioModule( retMsg );
}   // ---- end DoStartMidiFile() ----

// ==============================================================================
// DoIsMidiFilePlaying:
// If we have more than one MIDI player in the future, this needs to be changed.
// ==============================================================================
static void DoIsMidiFilePlaying( CAudioMsgIsMidiFilePlaying* /* msg */) 
{
//	tAudioID  			id = msg->GetData();
	CAudioReturnMessage	retMsg;

	gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTask::DoIsMidiFilePlaying() start\n");	

	// figure out if MIDI file is playing and send answer back to Module
	retMsg.SetBooleanResult( gAudioContext.pMidiPlayer->IsFileActive() );
	SendMsgToAudioModule( retMsg );

//	return err;
}   // ---- end DoIsMidiFilePlaying() ----

// ==============================================================================
// DoIsAnyMidiFilePlaying:
// If we have more than one MIDI player in the future, this needs to be changed.
// ==============================================================================
static void DoIsAnyMidiFilePlaying( CAudioMsgIsMidiFilePlaying* /* msg */) 
{
	CAudioReturnMessage	retMsg;

	gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTask::DoIsAnyMidiFilePlaying() start\n");	

	// figure out if MIDI file is playing and send answer back to Module
	retMsg.SetBooleanResult( gAudioContext.pMidiPlayer->IsFileActive() );
	SendMsgToAudioModule( retMsg );

}   // ---- end DoIsMidiFilePlaying() ----

// ==============================================================================
// DoPauseMidiFile
// ==============================================================================
static void DoPauseMidiFile( CAudioMsgPauseMidiFile* /* msg */) 
{
//	tMidiPlayerID  	id = msg->GetData();

	gAudioContext.pMidiPlayer->PauseMidiFile();
}   // ---- end DoPauseMidiFile() ----

// ==============================================================================
// DoResumeMidiFile
// ==============================================================================
static void DoResumeMidiFile( CAudioMsgResumeMidiFile* /* msg */) {
//	tMidiPlayerID  	id = msg->GetData();

	gAudioContext.pMidiPlayer->ResumeMidiFile();
}   // ---- end DoResumeMidiFile() ----

// ==============================================================================
// DoStopMidiFile
// ==============================================================================
static void DoStopMidiFile( CAudioMsgStopMidiFile* msg ) 
{
	tAudioStopMidiFileInfo* 	pInfo = msg->GetData();

	gAudioContext.pMidiPlayer->StopMidiFile( pInfo );
}   // ---- end DoStopMidiFile() ----

// ==============================================================================
// DoGetEnabledMidiTracks
// ==============================================================================
static void DoGetEnabledMidiTracks( CAudioMsgMidiFilePlaybackParams* /* msg */) 
{

	tErrType				result;
	CAudioReturnMessage		retMsg;
	tMidiTrackBitMask 		trackBitMask;

//	tAudioMidiFilePlaybackParams* pParams = msg->GetData();
	// pParams->id;  for the future

	result = gAudioContext.pMidiPlayer->GetEnableTracks( &trackBitMask );
	
	// Send status back to caller
	if (result != kNoErr)
		retMsg.SetAudioErr( result );
	else
		retMsg.SetU32Result( trackBitMask );

	SendMsgToAudioModule( retMsg );
}   // ---- end DoGetEnabledMidiTracks() ----

// ==============================================================================
// DoSetEnableMidiTracks
// ==============================================================================
static void DoSetEnableMidiTracks( CAudioMsgMidiFilePlaybackParams* msg ) 
{
	tErrType				result;
	CAudioReturnMessage		retMsg;
	tAudioMidiFilePlaybackParams* pParams = msg->GetData();
	
	// pParams->id; for the future
	
	result = gAudioContext.pMidiPlayer->SetEnableTracks( pParams->trackBitMask );
	
	// Sendstatus back to caller
	retMsg.SetAudioErr( result );

	SendMsgToAudioModule( retMsg );
}   // ---- end DoSetEnableMidiTracks() ----

// ==============================================================================
// DoTransposeMidiTracks
// ==============================================================================
static void DoTransposeMidiTracks( CAudioMsgMidiFilePlaybackParams* msg ) 
{
	tErrType				result;
	CAudioReturnMessage		retMsg;
	tAudioMidiFilePlaybackParams* pParams = msg->GetData();

	// pParams->id; for the future
	
	result = gAudioContext.pMidiPlayer->TransposeTracks( pParams->trackBitMask, pParams->transposeAmount );
	
	// Send the status back to the caller
	retMsg.SetAudioErr( result );

	SendMsgToAudioModule( retMsg );
}   // ---- end DoTransposeMidiTracks() ----

// ==============================================================================
// DoChangeMidiInstrument
// ==============================================================================
static void DoChangeMidiInstrument( CAudioMsgMidiFilePlaybackParams* msg ) 
{
	tErrType				result;
	CAudioReturnMessage		retMsg;
	tAudioMidiFilePlaybackParams* pParams = msg->GetData();

	// pParams->id; for the future
	
	result = gAudioContext.pMidiPlayer->ChangeProgram( pParams->trackBitMask, pParams->instrument );
	
	// Send the status back to the caller
	retMsg.SetAudioErr( result );

	SendMsgToAudioModule( retMsg );
}   // ---- end DoChangeMidiInstrument() ----

// ==============================================================================
// DoChangeMidiTempo
// ==============================================================================
static void DoChangeMidiTempo( CAudioMsgMidiFilePlaybackParams* msg ) 
{
	tErrType				result;
	CAudioReturnMessage		retMsg;
	tAudioMidiFilePlaybackParams* pParams = msg->GetData();

	// pParams->id; for the future
	
	result = gAudioContext.pMidiPlayer->ChangeTempo( pParams->tempo );
	
	// Send the status back to the caller
	retMsg.SetAudioErr( result );

	SendMsgToAudioModule( retMsg );
}   // ---- end DoChangeMidiTempo() ----

// ==============================================================================
// AudioTaskMain
// ==============================================================================
void* AudioTaskMain( void* /*arg*/ )
{
	tErrType 			err = kNoErr;
	U32 				i = 0;
	tMessageQueueHndl 	hQueue;
	
	// Set appropriate debug level
	gAudioContext.pDebugMPI->SetDebugLevel( kAudioDebugLevel );

// gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Audio task starting to run\n" );	

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
	
    gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
		"AudioTaskMain() -- Attempting to create incoming msg queue.\n" );	

    err = gAudioContext.pKernelMPI->OpenMessageQueue( gAudioContext.hRecvMsgQueue, msgQueueProperties, NULL );
    
    if (err != kNoErr) {
		if (err == EEXIST) {
			// Apparently the queue already exists, so lets delete it and create 
			// a new one in case properties/size have changed since the last time 
			// a queue was created.
			gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
				"AudioTaskMain() -- Attempting to delete orphaned incoming msg queue.\n" );
			
			msgQueueProperties.oflag = B_O_RDONLY;
		   	err = gAudioContext.pKernelMPI->OpenMessageQueue( hQueue, msgQueueProperties, NULL );
		    gAudioContext.pDebugMPI->Assert((kNoErr == err), 
		 "AudioTaskMain() Failed to open incoming msg queue in preparation for deletion. err=%d\n", (int) err);
	
		    err = gAudioContext.pKernelMPI->CloseMessageQueue( hQueue, msgQueueProperties );
		    gAudioContext.pDebugMPI->Assert((kNoErr == err), 
		    	"AudioTaskMain() Trying to close/delete incoming msg queue. err=%d\n", (int)err );
		    
		    hQueue = 0;

		    // Now that the old one is deleted, create a new one.
		    gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
				"Audio Task creating task incoming Q. size = %d\n", 
				static_cast<int>(kAUDIO_MAX_MSG_SIZE) );	

		    msgQueueProperties.oflag = B_O_RDONLY|B_O_CREAT;
		   	err = gAudioContext.pKernelMPI->OpenMessageQueue( gAudioContext.hRecvMsgQueue, msgQueueProperties, NULL );
		    gAudioContext.pDebugMPI->Assert((kNoErr == err), 
		    	"AudioTaskMain() Trying to create incoming msg queue after deletion of orphan. err=%d \n", 
		    	static_cast<int>(err) );

		} else {
			// Unknown error, bail out.
			gAudioContext.pDebugMPI->Assert((kNoErr == err), 
		    	"AudioTaskMain() -- Trying to create incoming audio task msg queue. err = %d \n", 
		    	static_cast<int>(err) );			
		}
	}
 
    // Now create a msg queue that allows the Audio Task to send msgs back to Audio Module
   gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
		"AudioTaskMain() -- Attempting to create outgoing msg queue.\n" );	

	msgQueueProperties.nameQueue = "/audioTaskOutgoingQ";
    msgQueueProperties.oflag = B_O_WRONLY|B_O_CREAT|B_O_EXCL;

 	err = gAudioContext.pKernelMPI->OpenMessageQueue( gAudioContext.hSendMsgQueue, msgQueueProperties, NULL );
	if (err != kNoErr) {
		if (err == EEXIST) {
			// Apparently the queue already exists, so lets delete it and create 
			// a new one in case properties/size have changed since the last time 
			// a queue was created.
			gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
				"AudioTaskMain() -- Attempting to delete orphaned outgoing msg queue.\n" );
			
			msgQueueProperties.oflag = B_O_WRONLY;
		   	err = gAudioContext.pKernelMPI->OpenMessageQueue( hQueue, msgQueueProperties, NULL );
		    gAudioContext.pDebugMPI->Assert((kNoErr == err), 
		    	"AudioTaskMain() -- Trying to open outgoing msg queue in preparation for deletion. err = %d \n", 
		    	static_cast<int>(err) );
	
		    err = gAudioContext.pKernelMPI->CloseMessageQueue( hQueue, msgQueueProperties );
		    gAudioContext.pDebugMPI->Assert((kNoErr == err), 
		    	"AudioTaskMain() -- Trying to close/delete outgoing msg queue. err = %d \n", 
		    	static_cast<int>(err) );
		    
		    hQueue = 0;

		    // Now that the old one is deleted, create a new one.
		    gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
				"AudioTaskMain() -- creating task outgoing Q. size = %d\n", 
				static_cast<int>(kAUDIO_MAX_MSG_SIZE) );	

		    msgQueueProperties.oflag = B_O_WRONLY|B_O_CREAT;
		   	err = gAudioContext.pKernelMPI->OpenMessageQueue( gAudioContext.hSendMsgQueue, msgQueueProperties, NULL );
		    gAudioContext.pDebugMPI->Assert((kNoErr == err), 
		    	"AudioTaskMain() v Trying to create outgoing msg queue after deletion of orphan. err = %d \n", 
		    	static_cast<int>(err) );

		} else {
			// Unknown error, bail out.
			gAudioContext.pDebugMPI->Assert((kNoErr == err), 
		    	"AudioTaskMain() Trying to create outgoing audio task msg queue. err = %d \n", 
		    	static_cast<int>(err) );			
		}
	}
    
	// Startup Audio Output
	StartAudioSystem();	

    // Set the task to ready
	gAudioContext.audioTaskRunning = true;
	
	char 				msgBuf[kAUDIO_MAX_MSG_SIZE];
	tAudioCmdMsgType 	cmdType;
	U32					msgSize;
	CAudioCmdMsg*		pAudioMsg;
	
	while ( gAudioContext.threadRun ) {
//		gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "\n---------- AudioTaskMain() -- top of loop, cnt: %u.\n", static_cast<unsigned int>(i++));	
   
		// Wait for message.
	    err = gAudioContext.pKernelMPI->ReceiveMessage( gAudioContext.hRecvMsgQueue, 
								   (CMessage*)msgBuf, kAUDIO_MAX_MSG_SIZE );
								   
		    gAudioContext.pDebugMPI->AssertNoErr( err, "AudioTask::MainLoop; Could not get cmd message from AudioModule.\n" );
	
	    pAudioMsg = reinterpret_cast<CAudioCmdMsg*>(msgBuf);
		msgSize = pAudioMsg->GetMessageSize();
		cmdType = pAudioMsg->GetCmdType();
//	    gAudioContext.pDebugMPI->DebugOut(kDbgLvlVerbose, 
//	    	"AudioTaskMain() -- Got Audio Command, Type= %d; Msg Size = %d \n", 
//	    	cmdType, static_cast<int>(msgSize) );  

	    // Process incoming audio msgs based on command type.
		switch ( cmdType ) {
			case kAudioCmdMsgTypeSetMasterVolume:
gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Set master volume.\n" );		
				DoSetMasterVolume( (CAudioMsgSetMasterVolume*)pAudioMsg );				   
				break;
	
			case kAudioCmdMsgTypeSetOutputEqualizer:
gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain: Enable output equalizer.\n" );	
				DoSetOutputEqualizer( (CAudioMsgSetOutputEqualizer*)pAudioMsg );				   
				break;

			case kAudioCmdMsgTypePauseAllAudio:
//gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose,  "AudioTaskMain() Pause all audio.\n" );		
				DoPauseAudioSystem();
				break;
	
			case kAudioCmdMsgTypeResumeAllAudio:
//gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Resume all audio.\n" );	
				DoResumeAudioSystem();
				break;
	
			case kAudioCmdMsgTypeStartAudio:
//				gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
//					"AudioTaskMain() -- kAudioCmdMsgTypeStartAudio : Calling DoStartAudio().\n" );	
				DoStartAudio( (CAudioMsgStartAudio*)pAudioMsg );
				break;
	
			case kAudioCmdMsgTypeGetAudioTime:
// gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Get Audio Time.\n" );		
				DoGetAudioTime( (CAudioMsgGetAudioTime*)pAudioMsg );
				break;

			case kAudioCmdMsgTypeGetAudioState:
//		gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() GetAudioState.\n" );	
//printf("AudioTaskMain() GetAudioState.\n" );	
				DoGetAudioState( (CAudioMsgGetAudioState *)pAudioMsg );
				break;
						
			case kAudioCmdMsgTypeSetAudioState:
//		gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Set AudioState.\n" );	
//printf("AudioTaskMain() SetAudioState.\n" );	
				DoSetAudioState( (CAudioMsgSetAudioState*)pAudioMsg);
				break;

			case kAudioCmdMsgTypeGetAudioVolume:
//		gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Get Volume.\n" );	
				DoGetAudioVolume( (CAudioMsgGetAudioVolume*)pAudioMsg );
				break;
						
			case kAudioCmdMsgTypeSetAudioVolume:
//		gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Set Volume.\n" );	
				DoSetAudioVolume( (CAudioMsgSetAudioVolume*)pAudioMsg);
				break;
				
			case kAudioCmdMsgTypeGetAudioPriority:
//		gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Get Priority.\n" );	
				DoGetAudioPriority( (CAudioMsgGetAudioPriority*)pAudioMsg );
				break;
						
			case kAudioCmdMsgTypeSetAudioPriority:
		gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Set Priority.\n" );		
				DoSetAudioPriority( (CAudioMsgSetAudioPriority*)pAudioMsg);
				break;

			case kAudioCmdMsgTypeGetAudioPan:
//		gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Get Pan.\n" );		
				DoGetAudioPan( (CAudioMsgGetAudioPan*)pAudioMsg );
				break;
						
			case kAudioCmdMsgTypeSetAudioPan:
//		gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Set Pan.\n" );			
				DoSetAudioPan( (CAudioMsgSetAudioPan*)pAudioMsg);
				break;

			case kAudioCmdMsgTypeGetAudioListener:
		gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Get Listener.\n");		
				DoGetAudioListener( (CAudioMsgGetAudioListener*)pAudioMsg );
				break;
						
			case kAudioCmdMsgTypeSetAudioListener:
		gAudioContext.pDebugMPI->DebugOut(kDbgLvlVerbose, "AudioTaskMain() Set Listener.\n");		
				DoSetAudioListener( (CAudioMsgSetAudioListener*)pAudioMsg);
				break;

			case kAudioCmdMsgTypeIsAnyAudioPlaying:
	gAudioContext.pDebugMPI->DebugOut(kDbgLvlVerbose, "AudioTaskMain() Is ANY audio playing?\n");	
				DoIsAnyAudioPlaying( (CAudioMsgIsAudioPlaying*)pAudioMsg);
				break;

			case kAudioCmdMsgTypeIsAudioPlaying:
	gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Is ANY audio playing?\n" );	
					DoIsAudioPlaying( (CAudioMsgIsAudioPlaying*)pAudioMsg);
				break;

			case kAudioCmdMsgTypePauseAudio:
	gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Pause audio.\n" );	
				DoPauseAudio( (CAudioMsgPauseAudio*)pAudioMsg );
				break;

			case kAudioCmdMsgTypeResumeAudio:
	gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose,  "AudioTaskMain() Resume audio.\n" );	
				DoResumeAudio( (CAudioMsgResumeAudio*)pAudioMsg );
				break;
	
			case kAudioCmdMsgTypeStopAudio:
//		gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Stop audio.\n" );	
				DoStopAudio( (CAudioMsgStopAudio*)pAudioMsg );
				break;
				
			case kAudioCmdMsgTypeAcquireMidiPlayer:
//	gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Acquire MIDI Player.\n" );	
				DoAcquireMidiPlayer();
				break;
	
			case kAudioCmdMsgTypeReleaseMidiPlayer:
//	gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Release MIDI Player.\n" );	
				DoReleaseMidiPlayer();
				break;
	
			case kAudioCmdMsgTypeMidiNoteOn:
//	gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose,  "AudioTaskMain() MIDI note On.\n" );		
				DoMidiNoteOn( (CAudioMsgMidiNoteOn*)pAudioMsg );
				break;
	
			case kAudioCmdMsgTypeMidiNoteOff:
//		gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose,  "AudioTaskMain() MIDI note Off.\n" );	
				DoMidiNoteOff( (CAudioMsgMidiNoteOff*)pAudioMsg );
				break;

			case kAudioCmdMsgTypeMidiCommand:
//		printf("AudioTaskMain() MIDI Command .\n" );	
				DoMidiCommand( (CAudioMsgMidiCommand *) pAudioMsg );
				break;
	
			case kAudioCmdMsgTypeStartMidiFile:
// gAudioContext.pDebugMPI->DebugOut(kDbgLvlVerbose, "AudioTaskMain: Start MIDI file.\n");			
				DoStartMidiFile( (CAudioMsgStartMidiFile*)pAudioMsg);
				break;
	
			case kAudioCmdMsgTypeIsMidiFilePlaying:
	gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain: Is MIDI file playing?\n" );	
				DoIsMidiFilePlaying( (CAudioMsgIsMidiFilePlaying*)pAudioMsg);
				break;
				
			case kAudioCmdMsgTypeIsAnyMidiFilePlaying:
gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Is ANY MIDI file playing?\n" );	
				DoIsAnyMidiFilePlaying( (CAudioMsgIsMidiFilePlaying*)pAudioMsg);
				break;

			case kAudioCmdMsgTypePauseMidiFile:
gAudioContext.pDebugMPI->DebugOut(kDbgLvlVerbose, "AudioTaskMain() Pause MIDI file\n");				
				DoPauseMidiFile( (CAudioMsgPauseMidiFile*)pAudioMsg );
				break;
				
			case kAudioCmdMsgTypeResumeMidiFile:
gAudioContext.pDebugMPI->DebugOut(kDbgLvlVerbose, "AudioTaskMain: Resume MIDI file\n");					
				DoResumeMidiFile( (CAudioMsgResumeMidiFile*)pAudioMsg );
				break;
	
			case kAudioCmdMsgTypeStopMidiFile:
//gAudioContext.pDebugMPI->DebugOut(kDbgLvlVerbose, "AudioTaskMain() Stop MIDI file.\n");					
				DoStopMidiFile( (CAudioMsgStopMidiFile*)pAudioMsg );
				break;
	
			case kAudioCmdMsgTypeGetEnabledMidiTracks:
gAudioContext.pDebugMPI->DebugOut(kDbgLvlVerbose, "AudioTaskMain() GetEnabled MIDI tracks\n");	
				DoGetEnabledMidiTracks( (CAudioMsgMidiFilePlaybackParams*)pAudioMsg );
				break;

			case kAudioCmdMsgTypeSetEnableMidiTracks:
gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Enable MIDI tracks.\n" );	
				DoSetEnableMidiTracks( (CAudioMsgMidiFilePlaybackParams*)pAudioMsg );
				break;

			case kAudioCmdMsgTypeTransposeMidiTracks:
gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Transpose MIDI tracks.\n" );	
				DoTransposeMidiTracks( (CAudioMsgMidiFilePlaybackParams*)pAudioMsg );
				break;

			case kAudioCmdMsgTypeChangeMidiInstrument:
gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Change MIDI instrument.\n" );	
				DoChangeMidiInstrument( (CAudioMsgMidiFilePlaybackParams*)pAudioMsg );
				break;

			case kAudioCmdMsgTypeChangeMidiTempo:
//	gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Change MIDI tempo.\n" );	
				DoChangeMidiTempo( (CAudioMsgMidiFilePlaybackParams*)pAudioMsg );
				break;

				case kAudioCmdMsgExitThread:
	gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, "AudioTaskMain() Exiting Audio Thread.\n" );	
				gAudioContext.threadRun = false;
				break;
							
			default:
gAudioContext.pDebugMPI->DebugOut(kDbgLvlCritical, "AudioTaskMain() unhandled audio command.\n");	
			break;
		}
	}

	gAudioContext.pDebugMPI->DebugOut( kDbgLvlVerbose, 
		"AudioTaskMain() -- exiting, loop cnt: %u.\n", static_cast<unsigned int>(i));	
		
	// Exit nicely
	StopAudioSystem();	

	// Set the task to byebye
	gAudioContext.audioTaskRunning = false;

	return (void *)kNull;
}

LF_END_BRIO_NAMESPACE()
// EOF

