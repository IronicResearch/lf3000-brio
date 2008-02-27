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
	id_ = id;

	// Allocate player's sample buffer
	pReadBuf_ = new S16[ 2*kAudioOutBufSizeInWords ];  // GK FIXX:	2x needed?

	bPaused_		= 0;
	bStopping_		= 0;
	waitForRender_	= 0;

	// Get Debug MPI
	pDebugMPI_ = new CDebugMPI( kGroupAudio );
	if (!pDebugMPI_->IsValid())
		printf("AudioPlayer ctor: Couldn't create DebugMPI!\n");
	pDebugMPI_->SetDebugLevel( kAudioDebugLevel );
	
	// Set class variables from message
	priority_	  = 0;
	pListener_	  = NULL;
	payload_	  = 0;
	optionsFlags_ = 0;
	bSendLoopEndMessage_ = 0;
	bIsDone_ = false;
	if(pInfo)
	{
		priority_	  = pInfo->priority;
		pListener_	  = pInfo->pListener;
		payload_	  = pInfo->payload;
		optionsFlags_ = pInfo->flags;

		bSendLoopEndMessage_ = (0 != (pInfo->flags & kAudioOptionsLoopEndMsg));
	}

	// Set up looping
	shouldLoop_	 = (0 < payload_) && (optionsFlags_ & kAudioOptionsLooped);
	loopCount_	 = payload_;
	loopCounter_ = 0;

}

//==============================================================================
//==============================================================================
CAudioPlayer::~CAudioPlayer()
{
}

// ==============================================================================
// SendLoopEndMsg:	 Send message to Event listener each time the end of a loop is reached
// ==============================================================================
void CAudioPlayer::SendLoopEndMsg( void )
{
	if (!pListener_)
		return;

	const tEventPriority	kPriorityTBD = 0;
	tAudioMsgLoopEnd		data;

	data.audioID = id_;			
	data.payload = loopCount_;
	data.count	 = 1;

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pListener_);
}	// ---- end SendLoopEndMsg() ----


LF_END_BRIO_NAMESPACE()
// EOF	
