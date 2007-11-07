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

CAudioPlayer::CAudioPlayer( tAudioStartAudioInfo* pAudioInfo, tAudioID id  )
{
	tErrType ret;
	
	// Initialize instance variables
	id_ = id;

	bPaused_        = 0;
	bComplete_      = 0;
	bStopping_      = 0;
	bHasAudioCodec_ = 0;

	// Get Debug MPI
	pDebugMPI_ = new CDebugMPI( kGroupAudio );
	ret = pDebugMPI_->IsValid();
	if (ret != true)
		printf("AudioPlayer ctor -- Couldn't create DebugMPI!\n");
	
#if !defined SET_DEBUG_LEVEL_DISABLE
	pDebugMPI_->SetDebugLevel( kAudioDebugLevel );
#endif

	pDebugMPI_->DebugOut( kDbgLvlValuable, 
		(const char *)"\nDebug and Resource MPIs created by AudioPlayer ctor\n");	

	
	// Set all of the audio player class variables from the message
	pan_    = pAudioInfo->pan;
	volume_ = pAudioInfo->volume;

	priority_ = pAudioInfo->priority;
	if (pAudioInfo->pListener) 
        {
		pListener_    = pAudioInfo->pListener;
		bDoneMessage_ = true;
	    } 
    else 
        {
		pListener_    = kNull;
		bDoneMessage_ = false;
	    }
	payload_      = pAudioInfo->payload;
	optionsFlags_ = pAudioInfo->flags;
}

//==============================================================================
//==============================================================================
CAudioPlayer::~CAudioPlayer()
{
}


LF_END_BRIO_NAMESPACE()
// EOF	
