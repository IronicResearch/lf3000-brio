//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		RawPlayer.cpp
//
// Description:
//		The class to manage the playing of raw audio.
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <EventMPI.h>
#include <AudioTypes.h>
#include <AudioPriv.h>
#include <AudioTypesPriv.h>
#include <RawPlayer.h>

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CRawPlayer implementation
//==============================================================================

CRawPlayer::CRawPlayer( tAudioPlayAudioInfo* pData, tAudioID id  ) : CAudioPlayer( pData, id  )
{
	// Most of the member vars set by superclass; these are RAW specific.
	pCurFrame_ = (S16*)pAudioData_;
	
	if (!hasStereoData_)
		numFrames_ = (audioDataSize_ / kAudioBytesPerMonoFrame);
	else
		numFrames_ = (audioDataSize_ / kAudioBytesPerStereoFrame);
	
	framesLeft_ = numFrames_;
	
	printf("CRawPlayer::ctor Number of Frames:%d\n", numFrames_);
	printf("CRawPlayer::ctor Header flags:%d\n", optionsFlags_);
}

//==============================================================================
//==============================================================================
CRawPlayer::~CRawPlayer()
{
	// If there's anyone listening, let them know we're done.
	// fixme/dg: check for suppress done msg flag too!
	if (pListener_ != kNull)
		SendDoneMsg();

	printf(" CRawPlayer::dtor -- I'm HERE!!!\n\n\n\n");
}

/*
 * I think these will go away and be handled in the channel...
//==============================================================================
//==============================================================================
void CRawPlayer::Stop()
{
	bStopping_ = 1;
	bComplete_ = 1;
}

//==============================================================================
//==============================================================================
void CRawPlayer::Pause()
{
	// Set the player and its assigned channel to the paused state
	bPaused_ = 1;
//	mpChannel->SetPauseState(true);
}

//==============================================================================
//==============================================================================
void CRawPlayer::Resume()
{
	// Set the player and its assigned channel to the un-paused state
	bPaused_ = 0;
//	mpChannel->SetPauseState(false);
}


//==============================================================================
//==============================================================================
void CRawPlayer::Restart()
{
	// Point curSample_ back to start and reset total.
	pCurFrame_ = (S16 *)pAudioData_;
	framesLeft_ = numFrames_;
}
*/

void CRawPlayer::SendDoneMsg( void ) {
	const tEventPriority	kPriorityTBD = 0;
	tAudioMsgDataCompleted	data;
	data.audioID = id_;	// dummy
	data.payload = 101;	// dummy
	data.count = 1;

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pListener_);
}

//==============================================================================
//==============================================================================
U32 CRawPlayer::RenderBuffer( S16* pOutBuff, U32 numStereoFrames )
{	
	U32		index;
	U32		framesToProcess = 0;
	S16*	pCurSample;
	
//	printf("Raw Player::RenderBuffer: entered.  ");

	// Check if there are any more samples to process 
	if (framesLeft_ > 0) {
//		printf("curFramePtr:0x%x; framesLeft: %d\n", pCurFrame_, framesLeft_); 

		// Fill the buffer with the requested number of sample frames.
		// (Or as many as we have left).
		if (framesLeft_ >= numStereoFrames)
			framesToProcess = numStereoFrames;
		else
			framesToProcess = framesLeft_;

		// Copy the requested frames from our audioDatPtr to the output buffer.
		if (hasStereoData_) {
			pCurSample = pCurFrame_;
			for (index = 0; index < framesToProcess; index++) {			
				// Copy Left sample
//				printf("left: %d ", *pCurFrame_);
				*pOutBuff++ = *pCurSample++; // fixme/!!
				// Copy Right sample
//				printf("right: %d ", *pCurFrame_);
				*pOutBuff++ = *pCurSample++;
			}
			// Now update the current frame pointer.
//			printf("stereo buffer: pCurFrame = 0x%x; frames = %d; next pCurFrame Should be: 0x%x\n", pCurFrame_, framesToProcess, (pCurFrame_ + framesToProcess));
			pCurFrame_ += framesToProcess;
		} else {
			for (index = 0; index < framesToProcess; index++) {			
				// Copy mono sample twice
//				printf("mono(doubled): %d ", *pCurFrame_);
				*pOutBuff++ = *pCurFrame_;
				*pOutBuff++ = *pCurFrame_++;
			}
		}
	
	// Account for the frames we've rendered.
	framesLeft_ -= framesToProcess;	
	}
			
	// Return the number of frames rendered.
	return framesToProcess;
}

// EOF	
