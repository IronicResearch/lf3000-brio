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
#include <string.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Global variables
//==============================================================================
static U32 numRawPlayers = 0;
static U32 maxNumRawPlayers = kAudioMaxRawStreams;

//==============================================================================
// CRawPlayer implementation
//==============================================================================

CRawPlayer::CRawPlayer( tAudioStartAudioInfo* pInfo, tAudioID id  ) :
	CAudioPlayer( pInfo, id  )
{
	tErrType result;
	
	TimeStampOn(3);
	
	// ---- Open audio file
	fileType_ = kRawPlayer_FileType_Unknown;
	fileH_	  = NULL;

	// Try to open WAV or AIFF File
	SF_INFO	 inFileInfo;

	memset (&inFileInfo, 0, sizeof(SF_INFO));
	inFile_ = sf_open((char *)pInfo->path->c_str(), SFM_READ, &inFileInfo);

	if (inFile_)
	{
		int fileFormatType = (inFileInfo.format & SF_FORMAT_TYPEMASK);
		if		(SF_FORMAT_AIFF == fileFormatType)
			fileType_ = kRawPlayer_FileType_AIFF; 
		else if (SF_FORMAT_WAV == fileFormatType)
			fileType_ = kRawPlayer_FileType_WAV;
		else {
			pDebugMPI_->DebugOut(kDbgLvlCritical,
								 "%s.%d: Unable to open '%s'\n",
				   				 __FUNCTION__, __LINE__, pInfo->path->c_str() );
			pDebugMPI_->DebugOut(kDbgLvlCritical,
								 "%s.%d: unsupported file type=0x%08X\n",
				   				 __FUNCTION__, __LINE__, fileFormatType);
		}

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
		pDebugMPI_->Assert((fileH_ != NULL),
						"%s.%d: Unable to open '%s'\n",
				   		__FUNCTION__, __LINE__, pInfo->path->c_str() );

		// Read Brio "Raw" audio header
		fileType_ = kRawPlayer_FileType_Brio;
		tAudioHeader	brioHeader;		
		tAudioHeader *bH = &brioHeader;
		int bytesRead = fread( bH, 1, sizeof(tAudioHeader), fileH_);
		pDebugMPI_->Assert((bytesRead == sizeof(tAudioHeader)),
						   "%s.%d: Unable to read RAW audio header from '%s'\n",
						   __FUNCTION__, __LINE__, (char *) pInfo->path->c_str());
		samplingFrequency_ = bH->sampleRate;
		audioDataBytes_	   = bH->dataSize;		
		channels_		   = 1 + (0 != (bH->flags & kAudioHeader_StereoBit));
		// FIXME: sf2brio bug calculates dataSize incorrectly!
		// Ignored by reading up to EOF in ReadBytesFromFile().
		
		pDebugMPI_->Assert( (sizeof(tAudioHeader) == bH->offsetToData),
							"%s.%d: offsetToData=%ld, but should be %d.  "
							"Is this Brio Raw Audio file ? '%s'\n",
							__FUNCTION__, __LINE__,
							bH->offsetToData , sizeof(tAudioHeader),
							(char *) pInfo->path->c_str());
	}
	totalBytesRead_ = 0;
	
	// Time lapse delta needs to be multiple of playback quantum (16 msec)
	if (optionsFlags_ & kAudioOptionsTimeEvent) {
		bIsTimeEvent_ = true;
		timeDelta_	= payload_;
		timeDelta_ 	/= kAudioFramesPerMS;
		timeDelta_	*= kAudioFramesPerMS;
	}

	numRawPlayers++;

	TimeStampOff(3);
}

// =============================================================================
// ~CRawPlayer
// =============================================================================
CRawPlayer::~CRawPlayer()
{

	if (pReadBuf_)
		delete[] pReadBuf_;

	// Close file
	if (fileH_)
	{
		fclose( fileH_ );
		fileH_ = NULL;
	}
	if (inFile_)
		sf_close(inFile_);
		//CloseSoundFile(&inFile_);

	// Free MPIs
	if (pDebugMPI_)
	{
		delete pDebugMPI_;	
		pDebugMPI_ = NULL;
	}
	numRawPlayers--;
}

U32 CRawPlayer::GetNumPlayers(void)
{
	return numRawPlayers;
}

U32 CRawPlayer::GetMaxPlayers(void)
{
	return maxNumRawPlayers;
}

Boolean CRawPlayer::IsRawPlayer(CAudioPlayer *pPlayer)
{
	CRawPlayer *p;
	if (p = dynamic_cast<CRawPlayer *>(pPlayer))
		return true;
	else
		return false;
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

// =============================================================================
// Render:		  Return framesRead
// =============================================================================
U32 CRawPlayer::Render( S16 *pOut, U32 numStereoFrames)
{	
	tErrType result;
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
	
	// Track elapsed playback intervals for time events
	if (bIsTimeEvent_) {
		timeLapsed_		+= framesRead / kAudioFramesPerMS;
		bIsTimeElapsed_	= (timeLapsed_ % timeDelta_ == 0) ? true : false;
	}
	
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
void CRawPlayer::RewindFile()
{
	if (fileH_)
		fseek( fileH_, sizeof(tAudioHeader), SEEK_SET);
	if (inFile_)
		sf_seek(inFile_, 0, SEEK_SET);
		//RewindSoundFile( inFile_);
	bIsDone_ = false;
}

// =============================================================================
// GetAudioTime_mSec :	 Return current position in audio file in milliSeconds
// =============================================================================
U32 CRawPlayer::GetAudioTime_mSec( void ) 
{
	U32 totalFramesRead = totalBytesRead_ / (sizeof(S16)*channels_);
	U32 milliSeconds = (1000 * totalFramesRead)/samplingFrequency_;

	return (milliSeconds);
}

// =============================================================================
// Seek :	
// =============================================================================
Boolean CRawPlayer::SeekAudioTime(U32 timeMilliSeconds)
{
	U32 target_frame = timeMilliSeconds * samplingFrequency_ / 1000;
	totalBytesRead_ = target_frame * sizeof(S16) * channels_;
	
	if (fileH_)
		fseek( fileH_, sizeof(tAudioHeader) + totalBytesRead_, SEEK_SET);
	if (inFile_)
		sf_seek(inFile_, totalBytesRead_, SEEK_SET);
	
	bSeeked = true;
	return true;
}

LF_END_BRIO_NAMESPACE()
