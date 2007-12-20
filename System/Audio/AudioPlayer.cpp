//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		AudioPlayer.cpp
//
// Description:
//		The base class for all Audio Players.
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
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
	// Initialize instance variables
	id_ = id;

	bPaused_        = 0;
	bComplete_      = 0;
	bStopping_      = 0;
	bHasAudioCodec_ = 0;
    waitForRender_  = 0;

	// Get Debug MPI
	pDebugMPI_ = new CDebugMPI( kGroupAudio );
	if (!pDebugMPI_->IsValid())
		printf("AudioPlayer ctor: Couldn't create DebugMPI!\n");
	
	pDebugMPI_->SetDebugLevel( kAudioDebugLevel );
	
// Set class variables from message
	pan_    = pInfo->pan;
	volume_ = pInfo->volume;

	priority_     = pInfo->priority;
	pListener_    = pInfo->pListener;
	payload_      = pInfo->payload;
	optionsFlags_ = pInfo->flags;
	bDoneMessage_ = (pInfo->flags & kAudioOptionsDoneMsgAfterComplete);

//#define DEBUG_AUDIOPLAYER_OPTIONS
#ifdef DEBUG_AUDIOPLAYER_OPTIONS
{
char s[80];
s[0] = '\0';
if (optionsFlags_ & kAudioOptionsLooped)
    strcat(s, "Loop=On");
else
    strcat(s, "Loop=Off");
if (optionsFlags_ & kAudioOptionsDoneMsgAfterComplete)
    strcat(s, "-SendDone=On");
else
    strcat(s, "-SendDone=Off");

printf("CAudioPlayer::ctor: listener=%d bDoneMessage_=%d flags=$%X '%s'\n", (kNull != pListener_), bDoneMessage_, (unsigned int)optionsFlags_, s);
}
#endif // DEBUG_AUDIOPLAYER_OPTIONS

}

//==============================================================================
//==============================================================================
CAudioPlayer::~CAudioPlayer()
{
}


LF_END_BRIO_NAMESPACE()
// EOF	
