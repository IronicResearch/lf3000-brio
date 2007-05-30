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

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CAudioPlayer implementation
//==============================================================================

CAudioPlayer::CAudioPlayer( tAudioPlayAudioInfo* pData, tAudioID id  )
{
	tErrType 		err = kNoErr;
	
	id_ = id;
	
// FIXME MG want to put in RsrcMgr code if we're going to stream data from storage.
#if 0
	CRsrcMgrMPI 	*pRsrcMgrMPI;

	// Get the pointer to the resource
	// which is the pointer to the audio header
	if ((pRsrcMgrMPI = new CRsrcMgrMPI) == kNull)
		return kAllocMPIErr;

	if ((err = pRsrcMgrMPI->GetRsrcPtr(pData->hRsrc, (tPtr*)&mpHeader)) != kNoErr)
		goto ReturnErr;									   
#else
	// for now, the Audio task is loading the whole resource into RAM so 
	// this header allows the player to play it directly.  Later we'll add
	// support for streaming.
	pHeader_ = pData->pAudioHeader;
#endif

	// Initialize class variables
	bPaused_ = 0;
	bComplete_ = 0;
	bDoneMessage_ = 0;
	bStopping_ = 0;
	bHasCodec_ = 0;
	rsrc_ = pData->hRsrc;

	// Set all of the audio player class variables from the message
	volume_ = pData->volume;
	priority_ = pData->priority;
	pan_ = pData->pan;
	if (pData->pListener != kNull)
		pListener_ = pData->pListener;
	payload_ = pData->payload;
	optionsFlags_ = pData->flags;
	
	// Get data from audio header associated with this player.
	// fixme/dg: do it for real.
	pAudioData_ = (void*)(pHeader_ + pHeader_->offsetToData);
	printf("AudioPlayer::ctor -- Audio Header @ 0x%x, Audio Data at 0x%x.\n", pHeader_, pAudioData_ );
	dataSampleRate_ = pHeader_->sampleRate;
	audioDataSize_ = pHeader_->dataSize;			
	if (pHeader_->flags & 0x1)
		hasStereoData_ = true;
	else
		hasStereoData_ = false;
}

//==============================================================================
//==============================================================================
CAudioPlayer::~CAudioPlayer()
{
}


// EOF	
