//==============================================================================
// Copyright (c) 2002-2009 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// MemPlayer.cpp
//
// Description:
//		The class to manage the playing of raw audio samples from memory.
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <AudioTypes.h>
#include <AudioPriv.h>
#include <AudioTypesPriv.h>
#include <MemPlayer.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Global variables
//==============================================================================
static U32 numMemPlayers = 0;
static U32 maxNumMemPlayers = kAudioMaxRawStreams;

//==============================================================================
// CMemPlayer implementation
//==============================================================================

CMemPlayer::CMemPlayer( tAudioStartAudioInfo* pInfo, tAudioID id  ) :
	CAudioPlayer( pInfo, id  )
{
	// Setup memory buffer playback per Brio raw header spec
	if (pInfo->pRawHeader != NULL)
	{
		tAudioHeader* phdr	= pInfo->pRawHeader;		
		samplingFrequency_	= phdr->sampleRate;
		audioDataBytes_		= phdr->dataSize;		
		channels_			= (phdr->flags & kAudioHeader_StereoBit) ? 2 : 1;
		pAudioData_			= phdr + phdr->offsetToData; 
		pDebugMPI_->DebugOut(kDbgLvlVerbose, "%s: %d Hz, %d ch, %d bytes @ %p\n",
			__FUNCTION__,  (int)samplingFrequency_, (int)channels_, 
			(int)audioDataBytes_, pAudioData_ );
	}
	else
	{	
		samplingFrequency_	= 0;
		audioDataBytes_		= 0;		
		channels_			= 0;	 
		pAudioData_			= NULL;	
		pDebugMPI_->DebugOut(kDbgLvlCritical, "%s: Header missing\n", __FUNCTION__ );
	}

	totalBytesRead_ = 0;
	pReadData_		= pAudioData_;
	
	numMemPlayers++;
}

// =============================================================================
// ~CMemPlayer
// =============================================================================
CMemPlayer::~CMemPlayer()
{
	// Release resources created by CAudioPlayer base class
	if (pReadBuf_)
		delete[] pReadBuf_;

	// Free MPIs
	if (pDebugMPI_)
	{
		delete pDebugMPI_;	
		pDebugMPI_ = NULL;
	}
	numMemPlayers--;
}

//==============================================================================
U32 CMemPlayer::GetNumPlayers(void)
{
	return numMemPlayers;
}

//==============================================================================
U32 CMemPlayer::GetMaxPlayers(void)
{
	return maxNumMemPlayers;
}

//==============================================================================
Boolean CMemPlayer::IsMemPlayer(CAudioPlayer *pPlayer)
{
	CMemPlayer *p;
	if (p = dynamic_cast<CMemPlayer *>(pPlayer))
		return true;
	else
		return false;
}

// ==============================================================================
// ReadBytesFromFile :	 
//						Return specified bytes read.
// ==============================================================================
U32 CMemPlayer::ReadBytesFromFile( void *d, U32 bytesToRead)
{
	U32 bytesRead = 0;
	
	// Update memory buffer pointer for next read block
	if (bytesToRead + totalBytesRead_ > audioDataBytes_)
		bytesToRead = audioDataBytes_ - totalBytesRead_;
	if (bytesToRead > 0)
		memcpy(d, pReadData_, bytesToRead);
	bytesRead = bytesToRead;
	pReadData_ = (char*)pReadData_ + bytesRead;
	
	return (bytesRead);
}

// =============================================================================
// Render:		  Return framesRead
// =============================================================================
U32 CMemPlayer::Render( S16 *pOut, U32 numStereoFrames)
{	
	U32		index;
	U32		framesToProcess = 0;
	U32		framesRead = 0;
	U32		bytesRead = 0;
	U32		bytesReadThisRender = 0;
	U32		bytesToRead = numStereoFrames * sizeof(S16) * channels_;
	char*	bufPtr = (char *)pReadBuf_;

	if (bIsDone_)
		return (0);

	// Read data from file to output buffer
	while ( bytesToRead > 0) 
	{
		bytesRead = ReadBytesFromFile(bufPtr, bytesToRead);
		
		if (bytesRead == 0)
			break;

		bytesToRead			-= bytesRead;
		bufPtr				+= bytesRead;
		totalBytesRead_		+= bytesRead;
		bytesReadThisRender += bytesRead;
	}
		
	framesRead		= bytesReadThisRender / (sizeof(S16) * channels_);
	framesToProcess = framesRead;
	
	// Copy Stereo data to stereo output buffer
	if (2 == channels_) 
	{
		U32 samplesToProcess = channels_*framesToProcess;
		for (index = 0; index < samplesToProcess; index++)			
			pOut[index] = pReadBuf_[index];
	} 
	else 
	{
		// Fan out mono data to stereo output buffer
		for (index = 0; index < framesToProcess; index++, pOut += 2) 
		{	
			S16 x = pReadBuf_[index];
			pOut[0] = x;
			pOut[1] = x;
		}
	}

	bIsDone_ = (numStereoFrames > framesRead);

	return (framesRead);
}

// =============================================================================
// RewindFile:	  Set file ptr to start of file
// =============================================================================
void CMemPlayer::RewindFile()
{
	// Reset memory buffer pointer and read-byte counter
	totalBytesRead_ = 0;
	pReadData_		= pAudioData_;

	bIsDone_ = false;
}

// =============================================================================
// GetAudioTime_mSec :	 Return current position in audio file in milliSeconds
// =============================================================================
U32 CMemPlayer::GetAudioTime_mSec( void ) 
{
	U32 totalFramesRead = totalBytesRead_ / (sizeof(S16)*channels_);
	U32 milliSeconds = (1000 * totalFramesRead)/samplingFrequency_;

	return (milliSeconds);
}

LF_END_BRIO_NAMESPACE()
