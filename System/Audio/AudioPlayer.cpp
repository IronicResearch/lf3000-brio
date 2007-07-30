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
	bPaused_ = 0;
	bComplete_ = 0;
	bStopping_ = 0;
	bHasCodec_ = 0;

	// Save the resource handle
	hRsrc_ = pAudioInfo->hRsrc;

	// Get Debug MPI
	pDebugMPI_ = new CDebugMPI( kGroupAudio );
	ret = pDebugMPI_->IsValid();
	if (ret != true)
		printf("AudioPlayer ctor -- Couldn't create DebugMPI!\n");
	
	// Get Resource MPI
	pRsrcMPI_ = new CResourceMPI;
	ret = pRsrcMPI_->IsValid();
	if (ret != true)
		printf("AudioPlayer ctor-- Couldn't create ResourceMPI!\n");

	pDebugMPI_->DebugOut( kDbgLvlValuable, 
		(const char *)"\nDebug and Resource MPIs created by AudioPlayer ctor\n");	

	
	// Set all of the audio player class variables from the message
	volume_ = pAudioInfo->volume;
	priority_ = pAudioInfo->priority;
	pan_ = pAudioInfo->pan;
	if (pAudioInfo->pListener != kNull) {
		pListener_ = pAudioInfo->pListener;
		bDoneMessage_ = true;
	} else {
		pListener_ = kNull;
		bDoneMessage_ = false;
	}
	payload_ = pAudioInfo->payload;
	optionsFlags_ = pAudioInfo->flags;
}

//==============================================================================
//==============================================================================
CAudioPlayer::~CAudioPlayer()
{
}


LF_END_BRIO_NAMESPACE()
// EOF	
