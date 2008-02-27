//==============================================================================
// Copyright (c) 2002-2007 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// RawPlayer.cpp
//
// Description:
//		The class to manage the playing of audio files (currently uncompressed)
//				Supports AIFF, WAV and the "RAW" Brio file, the latter of which
//				is technically not a RAW file.
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

#include <Dsputil.h>

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

CRawPlayer::CRawPlayer( tAudioStartAudioInfo* pInfo, tAudioID id  ) :
	CAudioPlayer( pInfo, id  )
{
	tErrType result;
	
	// ---- Open audio file
	fileType_ = kRawPlayer_FileType_Unknown;
	fileH_	  = NULL;

	// Try to open WAV or AIFF File
	SF_INFO	 inFileInfo;
	inFile_ = OpenSoundFile( (char *) pInfo->path->c_str(), &inFileInfo,
							 SFM_READ);
	if (inFile_)
	{
		long fileFormatType = (inFileInfo.format & SF_FORMAT_TYPEMASK);
		if		(SF_FORMAT_AIFF == fileFormatType)
			fileType_ = kRawPlayer_FileType_AIFF; 
		else if (SF_FORMAT_WAV == fileFormatType)
			fileType_ = kRawPlayer_FileType_WAV;
		else
			printf("CRawPlayer::ctor: unsupported file type=%ld\n",
				   fileFormatType);

		if (kRawPlayer_FileType_Unknown != fileType_)
		{
			samplingFrequency_ = inFileInfo.samplerate;
			audioDataBytes_	   = inFileInfo.frames*inFileInfo.channels*sizeof(S16);		
			channels_		   = inFileInfo.channels;	
		}
		else
		{
			// For unsupported file type, just zero everything
			samplingFrequency_ = 0;
			audioDataBytes_	   = 0;		
			channels_		   = 0;	  
		}
	}
	else
	{
		// If WAV/AIFF opens failed, try to open as Brio "RAW" file
		fileH_ = fopen(	 pInfo->path->c_str(), "r" );
		if (!fileH_)
			printf("CRawPlayer::ctor : Unable to open '%s'\n",
				   pInfo->path->c_str() );

		// Read Brio "Raw" audio header
		fileType_ = kRawPlayer_FileType_Brio;
		tAudioHeader	brioHeader;		
		tAudioHeader *bH = &brioHeader;
		int bytesRead = fread( bH, 1, sizeof(tAudioHeader), fileH_);
		pDebugMPI_->Assert((bytesRead == sizeof(tAudioHeader)),
						   "CRawPlayer::ctor: Unable to read "
						   "RAW audio header from '%s'\n",
						   (char *) pInfo->path->c_str());
		samplingFrequency_ = bH->sampleRate;
		audioDataBytes_	   = bH->dataSize;		
		channels_		   = 1 + (0 != (bH->flags & kAudioHeader_StereoBit));

		pDebugMPI_->Assert( (sizeof(tAudioHeader) == bH->offsetToData),
							"CRawPlayer::ctor: offsetToData=%ld, but should be %d.  "
							"Is this Brio Raw Audio file ? '%s'\n",
							bH->offsetToData , sizeof(tAudioHeader),
							(char *) pInfo->path->c_str());
	}
	
	totalBytesRead_ = 0;

}

// ==============================================================================
// ~CRawPlayer
// ==============================================================================
CRawPlayer::~CRawPlayer()
{

	if (pReadBuf_)
		delete pReadBuf_;

	// Close file
	if (fileH_)
	{
		fclose( fileH_ );
		fileH_ = NULL;
	}
	if (inFile_)
	{
		CloseSoundFile(&inFile_);
	}

	// Free MPIs
	if (pDebugMPI_)
	{
		delete pDebugMPI_;	
		pDebugMPI_ = NULL;
	}
}

// ==============================================================================
// ReadBytesFromFile :	 
//						Return specified bytes read.
// ==============================================================================
U32 CRawPlayer::ReadBytesFromFile( void *d, U32 bytesToRead)
{
	U32 bytesRead = 0;

	if (kRawPlayer_FileType_Brio == fileType_)
		bytesRead = fread(d, sizeof(char), bytesToRead, fileH_ );
	else if (kRawPlayer_FileType_WAV  == fileType_ ||
			 kRawPlayer_FileType_AIFF == fileType_)
	{		
		U32 framesToRead = bytesToRead/( sizeof(S16) * channels_);
		U32 framesRead = 0;
		if (inFile_)
			framesRead = sf_readf_short(inFile_, (short*) d, framesToRead);
		bytesRead = framesRead * sizeof(S16) * channels_;
	}
	return (bytesRead);
}

// ==============================================================================
// Render:		  Return framesRead
// ==============================================================================
U32 CRawPlayer::Render( S16 *pOut, U32 framesToRender )
{	
	tErrType result;
	U32		index;
	U32		framesRead = 0, framesToProcess = 0;
	U32		bytesRead = 0;
	U32		bytesToRead = framesToRender * sizeof(S16) * channels_;
	S16		*bufPtr = pReadBuf_;
	long	fileEndReached = false;

	if (bIsDone_)
		return (0);

	// Read data from file to output buffer
	while ( !fileEndReached && bytesToRead > 0) 
	{
		bytesRead = ReadBytesFromFile(bufPtr, bytesToRead);
		fileEndReached = ( bytesRead < bytesToRead );
		if ( fileEndReached)
		{
			// Pad with zeros after last legitimate sample
			ClearBytes(&bufPtr[bytesRead], bytesToRead-bytesRead);
		}
		
		bytesToRead		-= bytesRead;
		bufPtr			+= bytesRead;
		totalBytesRead_ += bytesRead;
	}
		
	// Copy to output buffer
	bIsDone_ = fileEndReached;
		
	framesRead		= bytesRead / (sizeof(S16) * channels_);
	framesToProcess = framesRead;
	U32 samplesToProcess = channels_*framesToProcess;

	// Copy Stereo data to stereo output buffer
	if (2 == channels_) 
	{
		for (index = 0; index < samplesToProcess; index++)			
			pOut[index] = pReadBuf_[index];
	} 
	else 
	{
		// Fan out mono data to stereo output buffer
		for (index = 0; index < samplesToProcess; index++, pOut += 2) 
		{	
			S16 x = pReadBuf_[index];
			pOut[0] = x;
			pOut[1] = x;
		}
	}

	return (framesRead);
}

// ==============================================================================
// RewindFile:	  Set file ptr to start of file
// ==============================================================================
void CRawPlayer::RewindFile()
{
	if (fileH_)
		fseek( fileH_, sizeof(tAudioHeader), SEEK_SET);
	if (inFile_)
		RewindSoundFile( inFile_);
	bIsDone_ = false;
}

// ==============================================================================
// GetAudioTime_mSec :	 Return current position in audio file in milliSeconds
// ==============================================================================
U32 CRawPlayer::GetAudioTime_mSec( void ) 
{
	U32 totalFramesRead = totalBytesRead_ / (sizeof(S16)*channels_);
	U32 milliSeconds = (1000 * totalFramesRead)/samplingFrequency_;

	return (milliSeconds);
}

LF_END_BRIO_NAMESPACE()
