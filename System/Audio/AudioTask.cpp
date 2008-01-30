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
#include <AudioTypesPriv.h>
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

	Boolean				threadRun;			// set to false to exit main thread loop; 
	Boolean				audioTaskRunning;	
	CAudioMixer*		pAudioMixer;		
	CMidiPlayer*		pMidiPlayer;		// Mixer's single MIDI player 	

	Boolean				audioOutputDriverOn;	
	Boolean				audioOutputDriverPaused;
	
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

	// Set output buffer sizes.  These values are based on
	// 20ms buffer of stereo samples: see AudioConfig.h
//	gAudioContext.outBufferSizeInWords = kAudioOutBufSizeInWords;
//	gAudioContext.outBufferSizeInBytes = kAudioOutBufSizeInBytes;

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
 
	// Get set up to create and run task thread.
// 	priority;				// 1	
//	properties.priority = 1;
  
 //	stackAddr;				// 2
//	const unsigned PTHREAD_STACK_ADDRESS = 0xABC;
//	properties.stackAddr = (tAddr ) malloc(PTHREAD_STACK_ADDRESS + 0x4000);

//	stackSize;				// 3	
//    const unsigned PTHREAD_STACK_MINIM = 0x5000;
//	properties.stackSize = PTHREAD_STACK_MIN + 0x0000;

//	TaskMainFcn;			// 4
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

	gAudioContext.pDebugMPI->Assert( kNoErr == err, "InitAudioTask() Failed to create AudioTask\n" );

	// Wait for Audio task to be ready
	while (!gAudioContext.audioTaskRunning)
	{
		gAudioContext.pKernelMPI->TaskSleep(10);
//		printf("AudioTask::InitAudioTask: waiting for audio task to start up...\n");	
	}

//	printf("AudioTask Started...\n" );
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
		printf("AudioTask::DeInitAudioTask: Waiting for audio task to die...\n");	
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
// 	printf("AudioTask:Sending msg back to module...\n");	
// 	printf("AudioTask:Return msg size = %d.\n", msg.GetMessageSize());	
  	
    tErrType err = gAudioContext.pKernelMPI->SendMessage( gAudioContext.hSendMsgQueue, msg );
 //   gAudioContext.pDebugMPI->AssertNoErr(err, "SendMsgToAudioModule() After call SendMessage().\n" );
}   // ---- end SendMsgToAudioModule() ----

// ==============================================================================
// DoSetMasterVolume
// ==============================================================================
static void DoSetMasterVolume( CAudioMsgSetMasterVolume *pMsg ) 
{
	gAudioContext.pAudioMixer->SetMasterVolume( (U8) pMsg->GetData() );
}   // ---- end DoSetMasterVolume() ----

// ==============================================================================
// DoEnableSpeakerDSP:   
// ==============================================================================
static void DoEnableSpeakerDSP( CAudioMsgEnableSpeakerDSP *pMsg ) 
{
	gAudioContext.pAudioMixer->EnableSpeakerDSP( (U8) pMsg->GetData() );
}   // ---- end DoEnableSpeakerDSP() ----

// ==============================================================================
// DoStartAudio
// ==============================================================================
static void DoStartAudio( CAudioMsgStartAudio *pMsg ) 
{
CAudioReturnMessage	msg;
CPath				filename;
CPath				fileExt;
tAudioStartAudioInfo *pAi = pMsg->GetData();

msg.SetAudioErr( kNoErr );       
msg.SetAudioID( kNoAudioID );

// Determine whether specified file exists
struct stat	fileStat;
int errStat = stat( pAi->path->c_str(), &fileStat );
//gAudioContext.pDebugMPI->Assert((kNoErr == err), 
//printf("AudioTask::DoStartAudio: vol=%d pri=%d pan=%d path='%s' listen=%p payload=%d flags=$%X \n", (int)pAi->volume, (int)pAi->priority, (int)pAi->pan, 
//			pAi->path->c_str(), (void *)pAi->pListener, (int)pAi->payload, (int)pAi->flags );
if (0 != errStat)   
    {
    printf("AudioTask::DoStartAudio: problem reading '%s\n",  pAi->path->c_str());
    msg.SetAudioErr( kAudioFileReadProblemErr );       
    }
// Create player
else
    {
// Extract filename and extension
    int	strIndex = pAi->path->rfind('/', pAi->path->size());
    	filename = pAi->path->substr(strIndex+1, pAi->path->size());
    	strIndex = pAi->path->rfind('.', pAi->path->size());
    	fileExt  = pAi->path->substr(strIndex+1, strIndex+4);
//	printf("AudioTask::DoStartAudio File='%s' Ext='%s'\n", filename.c_str(), fileExt.c_str());

// Create player based on file extension
    tAudioID audioID = gAudioContext.pAudioMixer->StartChannel( pAi, (char *) fileExt.c_str());
    if (kNoAudioID == audioID)
		msg.SetAudioID( audioID );
    msg.SetAudioErr( kAudioPlayerCreateErr );
    }

// Send audioID and error code back to caller
SendMsgToAudioModule( msg );
}   // ---- end DoStartAudio() ----

// ==============================================================================
// DoGetAudioTime   Get current time from audio player
// ==============================================================================
static void DoGetAudioTime( CAudioMsgGetAudioTime* msg ) 
{
tAudioID  			id = msg->GetData();
CAudioReturnMessage	retMsg;
U32					time = gAudioContext.pAudioMixer->GetChannelTime( id );

// Send time in message back to caller
retMsg.SetU32Result( time );
SendMsgToAudioModule( retMsg );
}   // ---- end DoGetAudioTime() ----

// ==============================================================================
// DoGetAudioState:   This is LF-internal state.  Don't expose to AudioMPI
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
// DoSetAudioState:  This is LF-internal state.  Don't expose to AudioMPI
// ==============================================================================
    static void 
DoSetAudioState( CAudioMsgSetAudioState *msg ) 
{
tAudioState 	d = msg->GetData();
gAudioContext.pAudioMixer->SetAudioState( &d );
}  // ---- end DoSetAudioState() ----

// ==============================================================================
// DoGetAudioVolume: 
// ==============================================================================
    static void 
DoGetAudioVolume( CAudioMsgGetAudioVolume* msg ) 
{
tAudioVolumeInfo 	info = msg->GetData();
// printf("AudioTask::DoGetAudioVolume: audioID = %d\n", (int)info.id);	
CAudioReturnMessage	retMsg;

// Find current volume for channel using audioID
U8 x = gAudioContext.pAudioMixer->GetChannelVolume( info.id );

retMsg.SetU32Result( (U32)x );
SendMsgToAudioModule( retMsg );
}  // ---- end DoGetAudioVolume() ----

// ==============================================================================
// DoSetAudioVolume: 
// ==============================================================================
static void DoSetAudioVolume( CAudioMsgSetAudioVolume* msg ) 
{
tAudioVolumeInfo 	info = msg->GetData();
//CAudioReturnMessage	retMsg;

//printf("AudioTask::DoSetAudioVolume: audioID=%d volume=%d\n", (int)info.id, info.volume);	
gAudioContext.pAudioMixer->SetChannelVolume(info.id, info.volume);
}  // ---- end DoSetAudioVolume() ----

// ==============================================================================
// DoGetAudioPan:   
// ==============================================================================
    static void 
DoGetAudioPan( CAudioMsgGetAudioPan* msg ) 
{
tAudioPanInfo 	info = msg->GetData();
CAudioReturnMessage	retMsg;

//printf("AudioTask::DoGetAudioPan() for audioID=%d\n", (int)info.id);	

S8 x = gAudioContext.pAudioMixer->GetChannelPan( info.id );

retMsg.SetS32Result( (S32)x );
SendMsgToAudioModule( retMsg );
}  // ---- end DoGetAudioPan() ----

// ==============================================================================
// DoSetAudioPan: 
// ==============================================================================
static void DoSetAudioPan( CAudioMsgSetAudioPan* msg ) 
{
tAudioPanInfo 	info = msg->GetData();
CAudioReturnMessage	retMsg;

//printf("AudioTask::DoSetAudioPan: audioID=%d pan=%d\n", (int)info.id, info.pan);	
gAudioContext.pAudioMixer->SetChannelPan(info.id, info.pan);
}  // ---- end DoSetAudioPan() ----

// ==============================================================================
// DoGetAudioPriority: 
// ==============================================================================
static void DoGetAudioPriority( CAudioMsgGetAudioPriority* msg ) 
{
tAudioPriorityInfo 	info = msg->GetData();
CAudioReturnMessage	retMsg;
	
//	printf("AudioTask::DoGetAudioPriority() Get Priority for audioID = %d\n", (int)info.id);	
U32 x =	gAudioContext.pAudioMixer->GetChannelPriority( info.id );
	
// Send value back to caller
retMsg.SetU32Result( (U32)x );
SendMsgToAudioModule( retMsg );
}  // ---- end DoGetAudioPriority() ----

// ==============================================================================
// DoSetAudioPriority: 
// ==============================================================================
static void DoSetAudioPriority( CAudioMsgSetAudioPriority* msg ) 
{
	tAudioPriorityInfo 	info = msg->GetData();
//	CAudioReturnMessage	retMsg;
	
//	printf("AudioTask::DoSetAudioPriority() Set Priority for audioID = %d\n", (int)info.id);	
gAudioContext.pAudioMixer->SetChannelPriority( info.id, info.priority );
}  // ---- end DoSetAudioPriority() ----

// ==============================================================================
// DoGetAudioListener:  Find listener ptr from channel with specified id
// ==============================================================================
static void DoGetAudioListener( CAudioMsgGetAudioListener* msg ) 
{
	tAudioListenerInfo 		info = msg->GetData();
	CAudioReturnMessage		retMsg;
	const IEventListener*	pListener = NULL;
	
//printf("AudioTask::DoGetAudioListener() Get Listener for audioID = %d\n", (int)info.id);	
pListener = gAudioContext.pAudioMixer->GetChannelEventListener(info.id);
	
// Send listener back to caller
retMsg.SetU32Result( (U32)pListener );
SendMsgToAudioModule( retMsg );
}  // ---- end DoGetAudioListener() ----

// ==============================================================================
// DoSetAudioListener:   
// ==============================================================================
static void DoSetAudioListener( CAudioMsgSetAudioListener *msg ) 
{
tAudioListenerInfo 	info = msg->GetData();
//printf("AudioTask::DoSetAudioListener() Set Listener for ID = %d\n", (int)info.id);	

gAudioContext.pAudioMixer->SetChannelEventListener( info.id, (IEventListener*) info.pListener );
}  // ---- end DoSetAudioListener() ----

// ==============================================================================
// DoIsAudioPlaying:    Is audio with specific ID playing ? 
// ==============================================================================
    static void 
DoIsAudioPlaying( CAudioMsgIsAudioPlaying *msg ) 
{
tAudioID  	 id = msg->GetData();
Boolean playing = gAudioContext.pAudioMixer->IsChannelActive( id );
//printf("AudioTask::DoIsAudioPlaying() Is ID = %d Playing=%d\n",  (int)id, playing);	

CAudioReturnMessage	retMsg;
retMsg.SetBooleanResult( playing );
SendMsgToAudioModule( retMsg );
}  // ---- end DoIsAudioPlaying() ----

// ==============================================================================
// DoIsAnyAudioPlaying:    
// ==============================================================================
    static void 
DoIsAnyAudioPlaying() 
{
Boolean playing = gAudioContext.pAudioMixer->IsAnyAudioActive();
//printf("AudioTask::DoIsAnyAudioPlaying() Audio playing=%d\n", playing);	

CAudioReturnMessage	retMsg;
retMsg.SetBooleanResult( playing );
SendMsgToAudioModule( retMsg );
}  // ---- end DoIsAnyAudioPlaying() ----

// ==============================================================================
// DoPauseAudio:   Pause audio channel with specified ID
// ==============================================================================
    static void 
DoPauseAudio( CAudioMsgPauseAudio* pMsg ) 
{
gAudioContext.pAudioMixer->PauseChannel( pMsg->GetData());
}  // ---- end DoPauseAudio() ----

// ==============================================================================
// DoResumeAudio: Resume audio channel with specified ID
// ==============================================================================
    static void 
DoResumeAudio( CAudioMsgResumeAudio* pMsg ) 
{
gAudioContext.pAudioMixer->ResumeChannel( pMsg->GetData() );
}  // ---- end DoResumeAudio() ----

// ==============================================================================
// DoStopAudio: Stop audio channel with specified ID
// ==============================================================================
    static void 
DoStopAudio( CAudioMsgStopAudio* pMsg ) 
{
tAudioStopAudioInfo *pInfo = pMsg->GetData();
gAudioContext.pAudioMixer->StopChannel( pInfo->id );
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
	
	return (err);
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
// Don't Pause PortAudio system
//		err = StopAudioOutput();
    gAudioContext.pAudioMixer->Pause();
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
// Don't Restart PortAudio system
//		err = StartAudioOutput();
    gAudioContext.pAudioMixer->Resume();
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
	
if (!gAudioContext.pMidiPlayer)
    gAudioContext.pMidiPlayer = gAudioContext.pAudioMixer->CreateMIDIPlayer();

if (gAudioContext.pMidiPlayer)
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
		
if (gAudioContext.pMidiPlayer)
    {
    gAudioContext.pMidiPlayer->DeActivate();
    gAudioContext.pAudioMixer->DestroyMIDIPlayer();
    gAudioContext.pMidiPlayer = NULL;
    }

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

// MIDI Player created/destroyed dynamically, so always check ptr	
if (gAudioContext.pMidiPlayer)
    gAudioContext.pMidiPlayer->NoteOn( d->channel, d->note, d->velocity, d->flags );
}   // ---- end DoMidiNoteOn() ----

// ==============================================================================
// DoMidiNoteOff
// ==============================================================================
static void DoMidiNoteOff( CAudioMsgMidiNoteOff* msg ) 
{
tAudioMidiNoteInfo* d = msg->GetData();

// MIDI Player created/destroyed dynamically, so always check ptr	
if (gAudioContext.pMidiPlayer)
    gAudioContext.pMidiPlayer->NoteOff( d->channel, d->note, d->velocity, d->flags );
}   // ---- end DoMidiNoteOff() ----

// ==============================================================================
// DoMidiCommand
// ==============================================================================
static void DoMidiCommand( CAudioMsgMidiCommand *msg ) 
{
// MIDI Player created/destroyed dynamically, so always check ptr	
tAudioMidiCommandInfo *d = msg->GetData();
if (gAudioContext.pMidiPlayer)
    gAudioContext.pMidiPlayer->SendCommand( d->cmd, d->data1, d->data2 );
}   // ---- end DoMidiNoteOff() ----

// ==============================================================================
// DoStartMidiFile:
// ==============================================================================
static void DoStartMidiFile( CAudioMsgStartMidiFile* msg ) {
tAudioStartMidiFileInfo* 	pInfo = msg->GetData();
CAudioReturnMessage			retMsg;
struct stat					fileStat;

//printf("AudioTask::DoStartMidiFile vol=%d pri=%d path='%s' listener=%p payload=%d flags=%X \n", pInfo->volume, pInfo->priority, pInfo->path->c_str(),
//			(void *)pInfo->pListener, (int)pInfo->payload, (int)pInfo->flags);

// Determine size of MIDI file
int err = stat( pInfo->path->c_str(), &fileStat );
gAudioContext.pDebugMPI->Assert((kNoErr == err), 
	"AudioTask::DoStartMidiFile()  file doesn't exist '%s' \n", pInfo->path->c_str() );

// Allocate buffer for file image
pInfo->pMidiFileImage = (U8 *) gAudioContext.pKernelMPI->Malloc( fileStat.st_size );
pInfo->imageSize      = fileStat.st_size;

// Load image
FILE *file = fopen( pInfo->path->c_str(), "r" );
int bytesRead = fread( pInfo->pMidiFileImage, sizeof(char), fileStat.st_size, file );
err = fclose( file );
	
tErrType result = kAudioMidiErr;
if (gAudioContext.pMidiPlayer)
    {
    result = gAudioContext.pMidiPlayer->StartMidiFile( pInfo );
    gAudioContext.pDebugMPI->Assert((kNoErr == result), 
    	"AudioTask::DoStartMidiFile(): Failed to start MIDI file. result=%d \n", (int)result );
    }
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
	CAudioReturnMessage	retMsg;

printf("AudioTask::DoIsMidiFilePlaying() start\n");	

// figure out if MIDI file is playing and send answer back to Module
if (gAudioContext.pMidiPlayer)
	retMsg.SetBooleanResult( gAudioContext.pMidiPlayer->IsFileActive() );
else
	retMsg.SetBooleanResult( false );
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

//	printf("AudioTask::DoIsAnyMidiFilePlaying() start\n");	

	// figure out if MIDI file is playing and send answer back to Module
if (gAudioContext.pMidiPlayer)
	retMsg.SetBooleanResult( gAudioContext.pMidiPlayer->IsFileActive() );
else
	retMsg.SetBooleanResult( false );

SendMsgToAudioModule( retMsg );

}   // ---- end DoIsMidiFilePlaying() ----

// ==============================================================================
// DoPauseMidiFile
// ==============================================================================
static void DoPauseMidiFile( CAudioMsgPauseMidiFile* /* msg */) 
{
//	tMidiPlayerID  	id = msg->GetData();

// MIDI Player created/destroyed dynamically, so always check ptr	
if (gAudioContext.pMidiPlayer)
	gAudioContext.pMidiPlayer->PauseMidiFile();
}   // ---- end DoPauseMidiFile() ----

// ==============================================================================
// DoResumeMidiFile
// ==============================================================================
static void DoResumeMidiFile( CAudioMsgResumeMidiFile* /* msg */) {
//	tMidiPlayerID  	id = msg->GetData();

// MIDI Player created/destroyed dynamically, so always check ptr	
if (gAudioContext.pMidiPlayer)
	gAudioContext.pMidiPlayer->ResumeMidiFile();
}   // ---- end DoResumeMidiFile() ----

// ==============================================================================
// DoStopMidiFile
// ==============================================================================
static void DoStopMidiFile( CAudioMsgStopMidiFile* msg ) 
{
	tAudioStopMidiFileInfo* 	pInfo = msg->GetData();

// MIDI Player created/destroyed dynamically, so always check ptr	
if (gAudioContext.pMidiPlayer)
	gAudioContext.pMidiPlayer->StopMidiFile( pInfo );
}   // ---- end DoStopMidiFile() ----

// ==============================================================================
// DoGetEnabledMidiTracks
// ==============================================================================
static void DoGetEnabledMidiTracks( CAudioMsgMidiFilePlaybackParams* /* msg */) 
{
tErrType				result = kAudioMidiUnavailable;
CAudioReturnMessage		retMsg;
tMidiTrackBitMask 		trackBitMask;

//	tAudioMidiFilePlaybackParams* pParams = msg->GetData();
	// pParams->id;  for the future

if (gAudioContext.pMidiPlayer)
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
tErrType				result = kAudioMidiUnavailable;
CAudioReturnMessage		retMsg;
tAudioMidiFilePlaybackParams* pParams = msg->GetData();
	
	// pParams->id; for the future
	
if (gAudioContext.pMidiPlayer)
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
tErrType				result = kAudioMidiUnavailable;
CAudioReturnMessage		retMsg;
tAudioMidiFilePlaybackParams* pParams = msg->GetData();

	// pParams->id; for the future
	
if (gAudioContext.pMidiPlayer)
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
	tErrType				result = kAudioMidiUnavailable;
	CAudioReturnMessage		retMsg;
	tAudioMidiFilePlaybackParams* pParams = msg->GetData();

	// pParams->id; for the future
	
if (gAudioContext.pMidiPlayer)
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
	tErrType				result = kAudioMidiUnavailable;
	CAudioReturnMessage		retMsg;
	tAudioMidiFilePlaybackParams* pParams = msg->GetData();

	// pParams->id; for the future
	
if (gAudioContext.pMidiPlayer)
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

// printf("AudioTaskMain() Audio task starting to run\n" );	

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
// printf("AudioTaskMain() Got Audio Command, Type= %d; Msg Size = %d \n", cmdType, (int) msgSize );  

	    // Process incoming audio msgs based on command type.
		switch ( cmdType ) {
			case kAudioCmdMsgTypeSetMasterVolume:
// printf("AudioTaskMain() Set master volume.\n" );		
				DoSetMasterVolume( (CAudioMsgSetMasterVolume*) pAudioMsg );				   
				break;
	
			case kAudioCmdMsgTypeEnableSpeakerDSP:
printf("AudioTaskMain: EnableSpeakerDSP.\n" );	
				DoEnableSpeakerDSP( (CAudioMsgEnableSpeakerDSP*) pAudioMsg );				   
				break;

			case kAudioCmdMsgTypePauseAllAudio:
// printf("AudioTaskMain() Pause all audio.\n" );		
				DoPauseAudioSystem();
				break;
	
			case kAudioCmdMsgTypeResumeAllAudio:
// printf("AudioTaskMain() Resume all audio.\n" );	
				DoResumeAudioSystem();
				break;
	
			case kAudioCmdMsgTypeStartAudio:
// printf("AudioTaskMain() kAudioCmdMsgTypeStartAudio : Calling DoStartAudio().\n" );	
				DoStartAudio( (CAudioMsgStartAudio*)pAudioMsg );
				break;
	
			case kAudioCmdMsgTypeGetAudioTime:
// printf("AudioTaskMain() Get Audio Time.\n" );		
				DoGetAudioTime( (CAudioMsgGetAudioTime*)pAudioMsg );
				break;

			case kAudioCmdMsgTypeGetAudioState:
//	printf("AudioTaskMain() GetAudioState.\n" );	
				DoGetAudioState( (CAudioMsgGetAudioState *)pAudioMsg );
				break;
						
			case kAudioCmdMsgTypeSetAudioState:
// printf("AudioTaskMain() Set AudioState.\n" );	
				DoSetAudioState( (CAudioMsgSetAudioState*)pAudioMsg);
				break;

			case kAudioCmdMsgTypeGetAudioVolume:
// printf("AudioTaskMain() Get Volume.\n" );	
				DoGetAudioVolume( (CAudioMsgGetAudioVolume*)pAudioMsg );
				break;
						
			case kAudioCmdMsgTypeSetAudioVolume:
// printf( "AudioTaskMain() Set Volume.\n" );	
				DoSetAudioVolume( (CAudioMsgSetAudioVolume*)pAudioMsg);
				break;
				
			case kAudioCmdMsgTypeGetAudioPriority:
// printf("AudioTaskMain() Get Priority.\n" );	
				DoGetAudioPriority( (CAudioMsgGetAudioPriority*)pAudioMsg );
				break;
						
			case kAudioCmdMsgTypeSetAudioPriority:
// printf("AudioTaskMain() Set Priority.\n" );		
				DoSetAudioPriority( (CAudioMsgSetAudioPriority*)pAudioMsg);
				break;

			case kAudioCmdMsgTypeGetAudioPan:
//printf( "AudioTaskMain() Get Pan.\n" );		
				DoGetAudioPan( (CAudioMsgGetAudioPan*)pAudioMsg );
				break;
						
			case kAudioCmdMsgTypeSetAudioPan:
// printf("AudioTaskMain() Set Pan.\n" );			
				DoSetAudioPan( (CAudioMsgSetAudioPan*)pAudioMsg);
				break;

			case kAudioCmdMsgTypeGetAudioListener:
// printf( "AudioTaskMain() Get Listener.\n");		
				DoGetAudioListener( (CAudioMsgGetAudioListener*)pAudioMsg );
				break;
						
			case kAudioCmdMsgTypeSetAudioListener:
// printf( "AudioTaskMain() Set Listener.\n");		
				DoSetAudioListener( (CAudioMsgSetAudioListener*)pAudioMsg);
				break;

			case kAudioCmdMsgTypeIsAnyAudioPlaying:
// printf("AudioTaskMain() Is ANY audio playing?\n");	
				DoIsAnyAudioPlaying();
				break;

			case kAudioCmdMsgTypeIsAudioPlaying:
// printf( "AudioTaskMain() Is ANY audio playing?\n" );	
					DoIsAudioPlaying( (CAudioMsgIsAudioPlaying*)pAudioMsg);
				break;

			case kAudioCmdMsgTypePauseAudio:
// printf( "AudioTaskMain() Pause audio.\n" );	
				DoPauseAudio( (CAudioMsgPauseAudio*)pAudioMsg );
				break;

			case kAudioCmdMsgTypeResumeAudio:
// printf( "AudioTaskMain() Resume audio.\n" );	
				DoResumeAudio( (CAudioMsgResumeAudio*)pAudioMsg );
				break;
	
			case kAudioCmdMsgTypeStopAudio:
printf("AudioTaskMain() Stop audio.\n" );	
				DoStopAudio( (CAudioMsgStopAudio*)pAudioMsg );
				break;
				
			case kAudioCmdMsgTypeAcquireMidiPlayer:
// printf( "AudioTaskMain() Acquire MIDI Player.\n" );	
				DoAcquireMidiPlayer();
				break;
	
			case kAudioCmdMsgTypeReleaseMidiPlayer:
//printf("AudioTaskMain() Release MIDI Player.\n" );	
				DoReleaseMidiPlayer();
				break;
	
			case kAudioCmdMsgTypeMidiNoteOn:
// printf( "AudioTaskMain() MIDI note On.\n" );		
				DoMidiNoteOn( (CAudioMsgMidiNoteOn*)pAudioMsg );
				break;
	
			case kAudioCmdMsgTypeMidiNoteOff:
// printf("AudioTaskMain() MIDI note Off.\n" );	
				DoMidiNoteOff( (CAudioMsgMidiNoteOff*)pAudioMsg );
				break;

			case kAudioCmdMsgTypeMidiCommand:
// printf("AudioTaskMain() MIDI Command .\n" );	
				DoMidiCommand( (CAudioMsgMidiCommand *) pAudioMsg );
				break;
	
			case kAudioCmdMsgTypeStartMidiFile:
// printf("AudioTaskMain: Start MIDI file.\n");			
				DoStartMidiFile( (CAudioMsgStartMidiFile*)pAudioMsg);
				break;
	
			case kAudioCmdMsgTypeIsMidiFilePlaying:
// printf("AudioTaskMain: Is MIDI file playing?\n" );	
				DoIsMidiFilePlaying( (CAudioMsgIsMidiFilePlaying*)pAudioMsg);
				break;
				
			case kAudioCmdMsgTypeIsAnyMidiFilePlaying:
// printf("AudioTaskMain() Is ANY MIDI file playing?\n" );	
				DoIsAnyMidiFilePlaying( (CAudioMsgIsMidiFilePlaying*)pAudioMsg);
				break;

			case kAudioCmdMsgTypePauseMidiFile:
// printf( "AudioTaskMain() Pause MIDI file\n");				
				DoPauseMidiFile( (CAudioMsgPauseMidiFile*)pAudioMsg );
				break;
				
			case kAudioCmdMsgTypeResumeMidiFile:
// printf("AudioTaskMain: Resume MIDI file\n");					
				DoResumeMidiFile( (CAudioMsgResumeMidiFile*)pAudioMsg );
				break;
	
			case kAudioCmdMsgTypeStopMidiFile:
// printf("AudioTaskMain() Stop MIDI file.\n");					
				DoStopMidiFile( (CAudioMsgStopMidiFile*)pAudioMsg );
				break;
	
			case kAudioCmdMsgTypeGetEnabledMidiTracks:
// printf("AudioTaskMain() GetEnabled MIDI tracks\n");	
				DoGetEnabledMidiTracks( (CAudioMsgMidiFilePlaybackParams*)pAudioMsg );
				break;

			case kAudioCmdMsgTypeSetEnableMidiTracks:
// printf( "AudioTaskMain() Enable MIDI tracks.\n" );	
				DoSetEnableMidiTracks( (CAudioMsgMidiFilePlaybackParams*)pAudioMsg );
				break;

			case kAudioCmdMsgTypeTransposeMidiTracks:
// printf( "AudioTaskMain() Transpose MIDI tracks.\n" );	
				DoTransposeMidiTracks( (CAudioMsgMidiFilePlaybackParams*)pAudioMsg );
				break;

			case kAudioCmdMsgTypeChangeMidiInstrument:
// printf( "AudioTaskMain() Change MIDI instrument.\n" );	
				DoChangeMidiInstrument( (CAudioMsgMidiFilePlaybackParams*)pAudioMsg );
				break;

			case kAudioCmdMsgTypeChangeMidiTempo:
// printf( "AudioTaskMain() Change MIDI tempo.\n" );	
				DoChangeMidiTempo( (CAudioMsgMidiFilePlaybackParams*)pAudioMsg );
				break;

				case kAudioCmdMsgExitThread:
// printf( "AudioTaskMain() Exiting Audio Thread.\n" );	
				gAudioContext.threadRun = false;
				break;
							
			default:
gAudioContext.pDebugMPI->DebugOut(kDbgLvlCritical, "AudioTaskMain() unhandled audio command.\n");	
			break;
		}
	}

//printf("AudioTaskMain() exiting, loop cnt: %u.\n", (int)i);	
		
	// Exit nicely
	StopAudioSystem();	

	// Set task to byebye
	gAudioContext.audioTaskRunning = false;

	return (void *)kNull;
}

LF_END_BRIO_NAMESPACE()
// EOF

