//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// AudioPlayer.cpp
//
//		The base class for all Audio Players.
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <SystemEvents.h>

#include <EventMPI.h>

#include <AudioTypes.h>
#include <AudioPlayer.h>

#include <AudioPriv.h>
LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CAudioPlayer implementation
//==============================================================================

CAudioPlayer::CAudioPlayer( tAudioStartAudioInfo* pInfo, tAudioID id  )
{	
//{static long c=0; printf("CAudioPlayer::CAudioPlayer: start %ld id=%d\n", c++, (int)id);}
	id_ = id;

#ifdef USE_AUDIO_PLAYER_MUTEX
// Setup Mutex object for protecting render calls
	pKernelMPI_ =  new CKernelMPI();
	result = pKernelMPI_->IsValid();
	pDebugMPI_->Assert((true == result), "CRawPlayer::ctor: Unable to create KernelMPI.\n");

	const tMutexAttr 	attr = {0};
	result = pKernelMPI_->InitMutex( render_mutex_, attr );
	pDebugMPI_->Assert((kNoErr == result), "CRawPlayer::ctor: Unable to init mutex.\n");
#endif

// Allocate player's sample buffer
	pReadBuf_ = new S16[ 2*kAudioOutBufSizeInWords ];  // GK FIXX:  2x needed?

	bPaused_        = 0;
	bComplete_      = 0;
	bStopping_      = 0;
//	bHasAudioCodec_ = 0;
    waitForRender_  = 0;

	// Get Debug MPI
	pDebugMPI_ = new CDebugMPI( kGroupAudio );
	if (!pDebugMPI_->IsValid())
		printf("AudioPlayer ctor: Couldn't create DebugMPI!\n");
	pDebugMPI_->SetDebugLevel( kAudioDebugLevel );
	
// Set class variables from message
	priority_     = pInfo->priority;
	pListener_    = pInfo->pListener;
	payload_      = pInfo->payload;
	optionsFlags_ = pInfo->flags;

	bSendDoneMessage_    = (0 != (pInfo->flags & kAudioOptionsDoneMsgAfterComplete));
	bSendLoopEndMessage_ = (0 != (pInfo->flags & kAudioOptionsLoopEndMsg));

// Set up looping
	shouldLoop_  = (0 < payload_) && (optionsFlags_ & kAudioOptionsLooped);
    loopCount_   = payload_;
    loopCounter_ = 0;

//#define DEBUG_AUDIOPLAYER_OPTIONS
#ifdef DEBUG_AUDIOPLAYER_OPTIONS
{
char sFlags[50];
sFlags[0] = '\0';
if (optionsFlags_ & kAudioOptionsLoopEndMsg)
    strcat(sFlags, "SendLoopEnd=On");
else
    strcat(sFlags, "SendLoopEnd=Off");
if (optionsFlags_ & kAudioOptionsLooped)
    strcat(sFlags, "Loop=On ");
else
    strcat(sFlags, "Loop=Off ");
if (optionsFlags_ & kAudioOptionsDoneMsgAfterComplete)
    strcat(sFlags, "SendDone=On");
else
    strcat(sFlags, "SendDone=Off");

printf("CAudioPlayer::ctor: listener=%d bSendDoneMessage_=%d bSendLoopEndMessage_=%d flags=$%X '%s'\n", (kNull != pListener_), bSendDoneMessage_, bSendLoopEndMessage_, (unsigned int)optionsFlags_, sFlags);

printf("    payload=%d optionsFlags=$%X -> shouldLoop=%d\n", 
        (int)payload_, (unsigned int) optionsFlags_, shouldLoop_);
printf("    listener=%p DoneMessage=%d LoopEndMessage=%d flags=$%X '%s' loopCount=%ld ($%X)\n", 
        (void *)pListener_, bSendDoneMessage_, bSendLoopEndMessage_, (unsigned int)optionsFlags_, sFlags, 
            loopCount_, (unsigned int) loopCount_);
}
#endif // DEBUG_AUDIOPLAYER_OPTIONS

#if PROFILE_DECODE_LOOP
	totalUsecs_ = 0;
	minUsecs_   = 1000000;
	maxUsecs_   = 0;
	totalBytes_ = 0;
#endif
}

//==============================================================================
//==============================================================================
CAudioPlayer::~CAudioPlayer()
{
}

// ==============================================================================
// SendDoneMsg:   Send message to Event listener when audio job completed, which
//                  includes the last iteration of a loop
// ==============================================================================
    void 
CAudioPlayer::SendDoneMsg( void )
{
if (!pListener_)
    return;

	const tEventPriority	kPriorityTBD = 0;
	tAudioMsgAudioCompleted	data;
//printf("CAudioPlayer::SendDoneMsg audioID=%ld\n", id_);

	data.audioID = id_;	        
	data.payload = loopCount_;
	data.count   = 1;

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pListener_);
}   // ---- end SendDoneMsg() ----

// ==============================================================================
// SendLoopEndMsg:   Send message to Event listener each time the end of a loop is reached
// ==============================================================================
    void 
CAudioPlayer::SendLoopEndMsg( void )
{
if (!pListener_)
    return;

	const tEventPriority	kPriorityTBD = 0;
	tAudioMsgLoopEnd	    data;
//printf("CAudioPlayer::SendLoopEndMsg audioID=%ld\n", id_);

	data.audioID = id_;	        
	data.payload = loopCount_;
	data.count   = 1;

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pListener_);
}   // ---- end SendLoopEndMsg() ----


LF_END_BRIO_NAMESPACE()
// EOF	
