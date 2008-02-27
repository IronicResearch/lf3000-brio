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
	bIsDone_ = false;
	if(pInfo)
	{
		priority_	  = pInfo->priority;
		pListener_	  = pInfo->pListener;
		payload_	  = pInfo->payload;
		optionsFlags_ = pInfo->flags;

	}

	numLoops_	 = payload_;
	loopCounter_ = 0;

}

//==============================================================================
//==============================================================================
CAudioPlayer::~CAudioPlayer()
{
}

LF_END_BRIO_NAMESPACE()
// EOF	
