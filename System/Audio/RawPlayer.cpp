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
LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CRawPlayer implementation
//==============================================================================

CRawPlayer::CRawPlayer( tAudioStartAudioInfo* pAudioInfo, tAudioID id  ) : CAudioPlayer( pAudioInfo, id  )
{
	tErrType		err;
	tAudioHeader*	pHeader;
	
	// Load the audio resource using the resource manager.
	err = pRsrcMPI_->LoadRsrc( pAudioInfo->hRsrc );  
	
	// Get the pointer to the audio header and data.
	pHeader = (tAudioHeader*)pRsrcMPI_->GetPtr( pAudioInfo->hRsrc );

//	printf("Header: type: 0x%x, dataOffset:%d, flags:%d, rate:%u, size:%u\n", (unsigned int)pHeader->type, (int)pHeader->offsetToData, pHeader->flags, (unsigned int)pHeader->sampleRate, (unsigned int)pHeader->dataSize);

	// Get ptr to data
	pAudioData_ = (void*)(pHeader + pHeader->offsetToData);
//	printf("AudioPlayer::ctor -- Audio Header @ 0x%x, Audio Data at 0x%x.\n", (unsigned int)pHeader, (unsigned int)pAudioData_ );

	dataSampleRate_ = pHeader->sampleRate;
	audioDataSize_ = pHeader->dataSize;			
	if (pHeader->flags & 0x1)
		hasStereoData_ = true;
	else
		hasStereoData_ = false;
	
	// Most of the member vars set by superclass; these are RAW specific.
	pCurFrame_ = (S16*)pAudioData_;
	
	if (!hasStereoData_)
		numFrames_ = (audioDataSize_ / kAudioBytesPerMonoFrame);
	else
		numFrames_ = (audioDataSize_ / kAudioBytesPerStereoFrame);
	
	framesLeft_ = numFrames_;
	
//	printf("CRawPlayer::ctor Number of Frames:%d\n", numFrames_);
//	printf("CRawPlayer::ctor Header flags:%d\n", optionsFlags_);
}

//==============================================================================
//==============================================================================
CRawPlayer::~CRawPlayer()
{
	// Unload the audio resource.
	pRsrcMPI_->UnloadRsrc( hRsrc_ );  

	// If there's anyone listening, let them know we're done.
	if ((pListener_ != kNull) && bDoneMessage_)
		SendDoneMsg();

//	printf(" CRawPlayer::dtor -- I'm HERE!!!\n\n\n\n");
}


//==============================================================================
//==============================================================================
void CRawPlayer::Rewind()
{
	// Point curSample_ back to start and reset total.
	pCurFrame_ = (S16 *)pAudioData_;
	framesLeft_ = numFrames_;
}

//==============================================================================
//==============================================================================
U32 CRawPlayer::GetAudioTime( void ) 
{
	return 0;
}

//==============================================================================
//==============================================================================
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
				*pOutBuff++ = *pCurFrame_++; // fixme/!!
				// Copy Right sample
//				printf("right: %d ", *pCurFrame_);
				*pOutBuff++ = *pCurFrame_++;
			}
			// Now update the current frame pointer.
//			printf("stereo buffer: pCurFrame = 0x%x; frames = %d; next pCurFrame Should be: 0x%x\n", pCurFrame_, framesToProcess, (pCurFrame_ + framesToProcess));
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
LF_END_BRIO_NAMESPACE()

// EOF	
