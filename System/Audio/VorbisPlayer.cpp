//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// VorbisPlayer.cpp
//
// The class to manage the playing of Vorbis audio.
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
#include <VorbisPlayer.h>

#include <Dsputil.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================
#define VORBIS_LITTLE_ENDIAN	0
#define VORBIS_BIG_ENDIAN		1
#define VORBIS_16BIT_WORD		2
#define VORBIS_SIGNED_DATA		1

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CVorbisPlayer implementation
//==============================================================================

CVorbisPlayer::CVorbisPlayer( tAudioStartAudioInfo* pInfo, tAudioID id	) :
	CAudioPlayer( pInfo, id	 )
{
	tErrType			ret = kNoErr;
	vorbis_info*		pVorbisInfo;
	
	pDebugMPI_->SetDebugLevel( kAudioDebugLevel );

	// Open file
	fileH_ = fopen( pInfo->path->c_str(), "r" );
	pDebugMPI_->Assert( fileH_ > 0, 
						"VorbisPlayer::ctor : Unable to open file: '%s'\n",
						pInfo->path->c_str() );

	// Open Ogg-compressed file
	int ov_ret = ov_open( fileH_, &vorbisFile_, NULL, 0 );
	pDebugMPI_->AssertNoErr( ov_ret, 
							 "VorbisPlayer::ctor: Is Ogg file? '%s'\n",
							 pInfo->path->c_str());

	pVorbisInfo		   = ov_info( &vorbisFile_, -1 );
	channels_		   = pVorbisInfo->channels;
	samplingFrequency_ = pVorbisInfo->rate;
	
//#define PRINT_FILE_INFO
#ifdef PRINT_FILE_INFO
	printf( "VorbisPlayer::ctor fs=%ld channels=%ld samples=%ld (%g Seconds)\n", 
			samplingFrequency_, channels_,
			(long)ov_pcm_total( &vorbisFile_, -1 ),
			0.001f*(float)ov_time_total( &vorbisFile_, -1 ));
	printf("VorbisPlayer::ctor bitstream length=%ld\n",
		   (long)ov_raw_total( &vorbisFile_, -1 ));
	printf("VorbisPlayer::ctor bitrate lower=%ld upper=%ld nominal=%ld\n",
		   pVorbisInfo->bitrate_lower, pVorbisInfo->bitrate_upper,
		   pVorbisInfo->bitrate_nominal);
#endif

}

//==============================================================================
// ~CVorbisPlayer
//==============================================================================
CVorbisPlayer::~CVorbisPlayer()
{
	tErrType result;
	
#ifdef USE_AUDIO_PLAYER_MUTEX
	result = pKernelMPI_->LockMutex( render_Mtex_ );
	pDebugMPI_->Assert((kNoErr == result), "CVorbisPlayer::dtor -- Couldn't lock mutex.\n");
#endif
	// If anyone is listening, let them know we're done.
	if (pListener_ && bSendDoneMessage_)
	{
		SendDoneMsg();
	}
	
	if (pReadBuf_)
		delete pReadBuf_;
	
	// Close vorbis file
	S32 ret = ov_clear( &vorbisFile_ );
	if ( ret < 0)
		printf("Could not close OggVorbis file.\n");

#ifdef USE_AUDIO_PLAYER_MUTEX
	result = pKernelMPI_->UnlockMutex( renderMutex_ );
	pDebugMPI_->Assert((kNoErr == result),
					   "CVorbisPlayer::dtor: Couldn't unlock mutex.\n");
	result = pKernelMPI_->DeInitMutex( renderMutex_ );
	pDebugMPI_->Assert((kNoErr == result),
					   "CVorbisPlayer::dtor: Couldn't deinit mutex.\n");
	if (pKernelMPI_)
		delete (pKernelMPI_);
#endif
	pDebugMPI_->DebugOut( kDbgLvlVerbose, " CVorbisPlayer::dtor -- vaporizing...\n");
	
	// Free MPIs
	if (pDebugMPI_)
		delete (pDebugMPI_);
}

// ==============================================================================
// RewindFile: Seek to beginning of file
// ==============================================================================
void CVorbisPlayer::RewindFile()
{
	ov_time_seek( &vorbisFile_, 0 );
}

// ==============================================================================
// GetAudioTime_mSec:	Return file position in time (milliSeconds)
// ==============================================================================
U32 CVorbisPlayer::GetAudioTime_mSec( void )
{
	ogg_int64_t vorbisTime = ov_time_tell( &vorbisFile_ );
	U32 timeInMS		   = (U32) vorbisTime;

	// GK FIXXX: should probably protect this with mutex
	return (timeInMS);
}

// ==============================================================================
// Render		 Convert Ogg stream to linear PCM data
// ==============================================================================
U32 CVorbisPlayer::Render( S16* pOut, U32 numStereoFrames )
{	
	tErrType result;
	U32		index;
	int		dummy;
	U32		framesToProcess = 0;
	U32		totalBytesRead	= 0;
	U32		bytesRead	= 0;
	U32		bytesToRead = 0;
	char*	bufPtr;
	U32		framesRead = 0;

#if PROFILE_DECODE_LOOP
	U32		startTime, endTime, elapsedTime;
#endif
	
#ifdef USE_AUDIO_PLAYER_MUTEX
	result = pKernelMPI_->TryLockMutex( renderMutex_ );
	// TODO/dg: this is a really ugly hack.  need to figure out what to return
	// in the case of render being called while stopping/dtor is running.
	if (EBUSY == result)
		return numStereoFrames;
	pDebugMPI_->Assert((kNoErr == result), "CVorbisPlayer::Render: Couldn't lock mutex.\n");
#endif

	bytesToRead = numStereoFrames * sizeof(S16) * channels_;
	
#if	PROFILE_DECODE_LOOP
	pKernelMPI_->GetHRTAsUsec( startTime );
#endif	

	// GK FIXX: pad short reads with zeros unless there is looping
	// GK FIXX: looping is not seamless
	// Read multiple times because vorbis doesn't always return requested # of
	// bytes Notes that BytesToRead is decremented
	bufPtr = (char *) pReadBuf_;
	long fileEndReached = false;
	long bytesReadThisRender   = 0;
	long bytesToReadThisRender = bytesToRead;
	while ( !fileEndReached && bytesToRead > 0 ) 
	{
		bytesRead = ov_read( &vorbisFile_, bufPtr, bytesToRead, 
							 &dummy );
		fileEndReached = (0 == bytesRead);
		bytesReadThisRender += bytesRead;
		if ( fileEndReached && shouldLoop_)
		{
			if (loopCounter_++ < loopCount_)
			{
				RewindFile();
				fileEndReached = false;
				if (bSendLoopEndMessage_)
				{
					SendLoopEndMsg();
				}
			}
			else
			{
				// Pad with zeros after last legitimate sample
				ClearBytes(&bufPtr[bytesReadThisRender],
						   bytesToReadThisRender-bytesReadThisRender);
			}
		}
		
		bytesToRead	   -= bytesRead;
		totalBytesRead += bytesRead;
		bufPtr		   += bytesRead;
	}
	
//#define VORBISPLAYER_SENDDONE_IN_RENDER
#ifdef VORBISPLAYER_SENDDONE_IN_RENDER
	if (pListener_ && bDoneMessage_ && fileEndReached)
	{
		SendDoneMsg();
	}
#endif // VORBISPLAYER_SENDDONE_IN_RENDER

#if	PROFILE_DECODE_LOOP
	pKernelMPI_->GetHRTAsUsec( endTime );

	if (endTime > startTime) 
	{
		elapsedTime = endTime - startTime;
		if (elapsedTime < minUsecs_)
			minUsecs_ = elapsedTime;
		if (elapsedTime > maxUsecs_)
			maxUsecs_ = elapsedTime;
		
		totalUsecs_ += elapsedTime;
		totalBytes_ += totalBytesRead;
	}
	
	// 1KB of mono 16KHz = 32ms of data
	if ( totalBytes_ == 1024 ) 
	{
		printf("Vorbis Player::Render: Accumulated %lu total bytes.\n\n ",
			   (long unsigned int)totalBytes_);
		printf("Vorbis Player::Render: this took %lu usecs.\n\n ",
			   (long unsigned int)totalUsecs_ );
		printf("Utilization (assuming 16KHz mono data): %f\n",
			   (float)(totalUsecs_/(float)32000));	
		totalUsecs_ = 0;
		totalBytes_ = 0;
	}
#endif

	// Convert bytes back into sample frames
	framesToProcess = totalBytesRead / sizeof(S16) / channels_;
			
	// Save total number of frames decoded
	framesRead = framesToProcess;

	// Copy to output buffer
	if (2 == channels_) 
	{
		U32 samplesToProcess = framesToProcess*2;
		for (index = 0; index < samplesToProcess; index++)			
			pOut[index] = pReadBuf_[index];
	} 
	else 
	{
		// Copy mono sample to stereo output
		for (index = 0; index < framesToProcess; index++, pOut += 2) 
		{	
			S16 x = pReadBuf_[index];
			pOut[0] = x;
			pOut[1] = x;
		}
	}
	
#ifdef USE_AUDIO_PLAYER_MUTEX
	result = pKernelMPI_->UnlockMutex( renderMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CVorbisPlayer::Render -- Couldn't unlock mutex.\n");
#endif

	return (framesRead);  
}

LF_END_BRIO_NAMESPACE()

