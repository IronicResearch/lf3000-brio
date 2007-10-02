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
#include <pthread.h>
#include <errno.h>
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

CRawPlayer::CRawPlayer( tAudioStartAudioInfo* pInfo, tAudioID id  ) : CAudioPlayer( pInfo, id  )
{
	tErrType			result;
	int					bytesRead;
	const tMutexAttr 	attr = {0};
	
	pDebugMPI_->DebugOut( kDbgLvlVerbose, "CRawPlayer::ctor -- Entering ctor...\n");

	// Get Kernel MPI
	pKernelMPI_ =  new CKernelMPI();
	result = pKernelMPI_->IsValid();
	pDebugMPI_->Assert((true == result), "CRawPlayer::ctor -- Couldn't create KernelMPI.\n");

	// Setup Mutex object for protecting render calls
	result = pKernelMPI_->InitMutex( render_mutex_, attr );
	pDebugMPI_->Assert((kNoErr == result), "CRawPlayer::ctor -- Couldn't init mutex.\n");

	// Allocate the player's sample buffer
	pPcmBuffer_ = new S16[ kAudioOutBufSizeInWords ];

    // Find out if the caller has requested looping.
	if (optionsFlags_ & 1)
		shouldLoop_ = true;
	else 
		shouldLoop_ = false;

	// Open the  file.
	file_ = fopen( pInfo->path->c_str(), "r" );
	pDebugMPI_->Assert( file_ > 0, 
		"CRawPlayer::ctor -- Could not open RAW audio file: %s.\n", pInfo->path->c_str() );

	// Get the  header.
	bytesRead = fread( &rawHeader, 1, sizeof(tAudioHeader), file_);
	
	pDebugMPI_->DebugOut( kDbgLvlVerbose, "Header: dataOffset:%d, flags:%d, rate:%u, size:%u\n", 
		(int)rawHeader.offsetToData, rawHeader.flags, (unsigned int)rawHeader.sampleRate, 
		(unsigned int)rawHeader.dataSize);

	dataSampleRate_ = rawHeader.sampleRate;
	audioDataSize_ = rawHeader.dataSize;			
	if (rawHeader.flags & 0x1)
		hasStereoData_ = true;
	else
		hasStereoData_ = false;
	
	// Most of the member vars set by superclass; these are RAW specific.
	if (!hasStereoData_)
		numFrames_ = (audioDataSize_ / kAudioBytesPerMonoFrame);
	else
		numFrames_ = (audioDataSize_ / kAudioBytesPerStereoFrame);
	
	framesLeft_ = numFrames_;
	
	pDebugMPI_->DebugOut( kDbgLvlVerbose, "CRawPlayer::ctor Number of Frames:%d\n", (int)numFrames_);
	pDebugMPI_->DebugOut( kDbgLvlVerbose, "CRawPlayer::ctor Header flags:%d\n", (int)optionsFlags_);

}

//==============================================================================
//==============================================================================
CRawPlayer::~CRawPlayer()
{
	tErrType result;
	
	result = pKernelMPI_->LockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CRawPlayer::dtor -- Couldn't lock mutex.\n");

	// If there's anyone listening, let them know we're done.
	if ((pListener_ != kNull) && bDoneMessage_)
		SendDoneMsg();

	// Free the sample buffer
	if (pPcmBuffer_)
		delete pPcmBuffer_;

	// Close the file
	fclose( file_ );
	
	result = pKernelMPI_->UnlockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CRawPlayer::dtor: Couldn't unlock mutex.\n");
	result = pKernelMPI_->DeInitMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CRawPlayer::dtor -- Couldn't deinit mutex.\n");

	pDebugMPI_->DebugOut( kDbgLvlVerbose,
		" CRawPlayer::dtor -- vaporizing...\n");

	// Free debug MPI
	if (pDebugMPI_)
		delete pDebugMPI_;

	// Free kernel MPI
	if (pKernelMPI_)
		delete pKernelMPI_;
}


//==============================================================================
//==============================================================================
void CRawPlayer::Rewind()
{
	// Point back to start of audio data and reset total.
	fseek( file_, sizeof(tAudioHeader), SEEK_SET);
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
	tAudioMsgAudioCompleted	data;
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
	tErrType result;
	U32		index;
	U32		framesRead = 0, framesToProcess = 0;
	U32		totalBytesRead = 0, bytesRead = 0, bytesToRead = 0;
	S16*	pCurSample;
	char* 	bufferPtr = kNull;

//	printf("Raw Player::RenderBuffer: entered.  ");

	// Don't want to try to render if stop() or dtor() have been entered.
	result = pKernelMPI_->TryLockMutex( render_mutex_ );
	
	// TODO/dg: this is a really ugly hack.  need to figure out what to return
	// in the case of render being called while stopping/dtor is running.
	if (result == EBUSY)
		return numStereoFrames;
	else
		pDebugMPI_->Assert((kNoErr == result), "CRawPlayer::RenderBuffer -- Couldn't lock mutex.\n");

	// Copy the requested frames from our audioDatPtr to the output buffer.
	if (hasStereoData_)
		bytesToRead = numStereoFrames * 2 * 2;	// 16bit stereo
	else
		bytesToRead = numStereoFrames * 2;		// 16bit mono

	// We may have to read multiple times because vorbis doesn't always
	// give you what you ask for.
	bufferPtr = (char*)pPcmBuffer_;
	while ( bytesToRead > 0 ) {			
		bytesRead = fread( bufferPtr, 1, bytesToRead, file_ );
	 				
		// at EOF
		if ( bytesRead < bytesToRead ) {
			if (shouldLoop_)
				Rewind();
			else
				break;
		}
		
		// Keep track of where we are...
		bytesToRead -= bytesRead;
		totalBytesRead += bytesRead;
	 	bufferPtr += bytesRead;
	}
		

	// Convert bytes back into sample frames
	if (hasStereoData_)
		framesRead = totalBytesRead / 2 / 2;
	else
		framesRead = totalBytesRead / 2;

	// Copy the requested frames from our local buffer to the output buffer.
	framesToProcess = framesRead;
	pCurSample = pPcmBuffer_;

	if (hasStereoData_) {
		for (index = 0; index < framesToProcess; index++) {			
			// Copy Left sample
//				printf("left: %d ", *pCurSample);
			*pOutBuff++ = *pCurSample++; // fixme/!!
			// Copy Right sample
//				printf("right: %d ", *pCurSample);
			*pOutBuff++ = *pCurSample++;
		}
		// Now update the current frame pointer.
//			printf("stereo buffer: pCurSample = 0x%x; frames = %d; next pCurSample Should be: 0x%x\n", pCupCurSamplerFrame_, framesToProcess, (pCurSample + framesToProcess));
	} else {
		for (index = 0; index < framesToProcess; index++) {			
			// Copy mono sample twice
//				printf("mono(doubled): %d ", *pCurSample);
			*pOutBuff++ = *pCurSample;
			*pOutBuff++ = *pCurSample++;
		}
	}
			
	result = pKernelMPI_->UnlockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CRawPlayer::RenderBuffer -- Couldn't unlock mutex.\n");

	// Return the number of frames rendered.
	return framesRead;
}
LF_END_BRIO_NAMESPACE()

// EOF	
