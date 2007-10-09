//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		VorbisPlayer.cpp
//
// Description:
//		The class to manage the playing of Vorbis audio.
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

CVorbisPlayer::CVorbisPlayer( tAudioStartAudioInfo* pInfo, tAudioID id  ) : CAudioPlayer( pInfo, id  )
{
	tErrType			ret = kNoErr;
	vorbis_info*		pVorbisInfo;
	ogg_int64_t			lengthInSeconds;
	ogg_int64_t			length;
	const tMutexAttr 	attr = {0};
	
	pDebugMPI_->SetDebugLevel( kAudioDebugLevel );

	// Allocate the player's sample buffer
	pPcmBuffer_ = new S16[ kAudioOutBufSizeInWords ];

	// Get Kernel MPI
	pKernelMPI_ =  new CKernelMPI();
	ret = pKernelMPI_->IsValid();
	pDebugMPI_->Assert((true == ret), "CVorbisPlayer::ctor: Couldn't create KernelMPI.\n");

	// Setup Mutex object for protecting render calls
  	ret = pKernelMPI_->InitMutex( render_mutex_, attr );
	pDebugMPI_->Assert((kNoErr == ret), "CVorbisPlayer::ctor: Couldn't init mutex.\n");

	// Open the ogg file.
	file_ = fopen( pInfo->path->c_str(), "r" );
	pDebugMPI_->Assert( file_ > 0, 
		"VorbisPlayer::ctor -- Could not open oggvorbis file: %s.\n", pInfo->path->c_str() );
 
    // Find out if the caller has requested looping.
	if (optionsFlags_ & kAudioOptionsLooped)
		shouldLoop_ = true;
	else 
		shouldLoop_ = false;
	loopCounter_ = 0;

	// open the file
	int ov_ret = ov_open( file_, &vorbisFile_, NULL, 0 );
	pDebugMPI_->DebugOut( kDbgLvlVerbose,
		"VorbisPlayer::ctor -- ov_open() returned: %d.\n", static_cast<int>(ov_ret) );
	pDebugMPI_->AssertNoErr( ov_ret, 
		"VorbisPlayer::ctor -- Input resource failed Vorbis open test. Is this an OggVorbis file?!\n");
	
	// Figure out how big the vorbis bitstream actually is.
	pVorbisInfo = ov_info( &vorbisFile_, -1 );
	if (pVorbisInfo->channels == 2)
		hasStereoData_ = true;
	else
		hasStereoData_ = false;
	
	dataSampleRate_ = pVorbisInfo->rate;
	
	pDebugMPI_->DebugOut( kDbgLvlVerbose,
		"VorbisPlayer::ctor -- OggVorbis file's num channels is %d.\n", pVorbisInfo->channels);
	pDebugMPI_->DebugOut( kDbgLvlVerbose,
		"VorbisPlayer::ctor -- OggVorbis file's sample rate is %ld.\n", pVorbisInfo->rate);

	lengthInSeconds = ov_time_total( &vorbisFile_, -1 );
//	printf("VorbisPlayer::ctor -- OggVorbis file's length in seconds is %f.\n", (float)lengthInSeconds);

	length = ov_raw_total( &vorbisFile_, -1 );
//	printf("VorbisPlayer::ctor -- OggVorbis file's bitstream length is %ld.\n", (long)length);
	length = ov_pcm_total( &vorbisFile_, -1 );
//	printf("VorbisPlayer::ctor -- OggVorbis file's PCM length is %ld.\n", (long)length );
	
//	printf("VorbisPlayer::ctor -- CVorbisPlayer::ctor Header flags:%d\n", optionsFlags_);

#if PROFILE_DECODE_LOOP
	totalUsecs_ = 0;
	minUsecs_ = 1000000;
	maxUsecs_ = 0;
	totalBytes_ = 0;
#endif
}

//==============================================================================
//==============================================================================
CVorbisPlayer::~CVorbisPlayer()
{
	tErrType result;
	
	result = pKernelMPI_->LockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CVorbisPlayer::dtor -- Couldn't lock mutex.\n");

#if 0	// BUGFIX/dm: Too late here? -- Post done message in RenderBuffer()
	// If there's anyone listening, let them know we're done.
	if ((pListener_ != kNull) && bDoneMessage_)
		SendDoneMsg();
#endif
	
	// Free the sample buffer
	if (pPcmBuffer_)
		delete pPcmBuffer_;
	
	// Close the vorbis file
	S32 ret = ov_clear( &vorbisFile_ );
	if ( ret < 0)
		printf("Could not close OggVorbis file.\n");

	result = pKernelMPI_->UnlockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CVorbisPlayer::dtor: Couldn't unlock mutex.\n");
	result = pKernelMPI_->DeInitMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CVorbisPlayer::dtor: Couldn't deinit mutex.\n");

	pDebugMPI_->DebugOut( kDbgLvlVerbose,
		" CVorbisPlayer::dtor -- vaporizing...\n");
	
	// Free debug MPI
	if (pDebugMPI_)
		delete pDebugMPI_;

	// Free kernel MPI
	if (pKernelMPI_)
		delete pKernelMPI_;

}

//==============================================================================
//==============================================================================
void CVorbisPlayer::Rewind()
{
	// Point curSample_ back to start and reset total.
	pDebugMPI_->DebugOut( kDbgLvlVerbose,
		"Vorbis Player::Rewind.\n "); 
	
	// Seek back to beginning
	ov_time_seek( &vorbisFile_, 0 );
}

//==============================================================================
//==============================================================================
U32 CVorbisPlayer::GetAudioTime( void )
{
	ogg_int64_t vorbisTime;
	U32 timeInMS;
		
	vorbisTime = ov_time_tell( &vorbisFile_ );
	timeInMS = (U32)vorbisTime;

	pDebugMPI_->DebugOut( kDbgLvlVerbose,
		"Vorbis Player::GetAudioTime() -- vorbisTime = %ld; time in ms = %d.\n ",
		static_cast<long>(vorbisTime), static_cast<int>(timeInMS) ); 

	return timeInMS;
}

//==============================================================================
//==============================================================================
void CVorbisPlayer::SendDoneMsg( void ) 
{
	const tEventPriority	kPriorityTBD = 0; // lower priority for async post
	tAudioMsgAudioCompleted	data;

	data.audioID = id_;
	data.payload = payload_;
	data.count = loopCounter_;

	pDebugMPI_->DebugOut( kDbgLvlVerbose,
		"Vorbis Player::SendDoneMsg.\n "); 

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pListener_);
}

//==============================================================================
//==============================================================================
U32 CVorbisPlayer::RenderBuffer( S16* pOutBuff, U32 numStereoFrames )
{	
	tErrType result;
	U32		index;
	int 	dummy;
	U32		framesToProcess = 0;
	U32		totalBytesRead = 0;
	U32		bytesRead = 0;
	U32 	bytesToRead= 0;
	char* 	bufferPtr = kNull;
	S16*	pCurSample;
	U32		framesRead = 0;

#if PROFILE_DECODE_LOOP
	U32		startTime, endTime, elapsedTime;
#endif
	
	result = pKernelMPI_->TryLockMutex( render_mutex_ );
	
	if (result == EBUSY)
		// TODO/dg: this is a really ugly hack.  need to figure out what to return
		// in the case of render being called while stopping/dtor is running.
		return numStereoFrames;
	else
		pDebugMPI_->Assert((kNoErr == result), "CVorbisPlayer::RenderBuffer -- Couldn't lock mutex.\n");

	if (hasStereoData_)
		bytesToRead = numStereoFrames * 2 * 2;	// 16bit stereo
	else
		bytesToRead = numStereoFrames * 2;		// 16bit mono
	
#if	PROFILE_DECODE_LOOP
	pKernelMPI_->GetHRTAsUsec( startTime );
#endif	
	// We may have to read multiple times because vorbis doesn't always
	// give you what you ask for.
	bufferPtr = (char*)pPcmBuffer_;
	while ( bytesToRead > 0 ) {
//		printf("Vorbis Player::RenderBuffer: about to ov_read() %u bytes.\n ", bytesToRead);
		
		bytesRead = ov_read( &vorbisFile_, 
			bufferPtr, 
			bytesToRead, 
	 		//VORBIS_LITTLE_ENDIAN, 
	 		//VORBIS_16BIT_WORD, 
	 		//VORBIS_SIGNED_DATA, 
	 		&dummy );
	 	// Tremor ov_read() = 16-bit signed native format
	 		
//		printf("Vorbis Player::RenderBuffer: ov_read() got %u bytes.\n ", bytesRead);
	
		// at EOF
		if ( bytesRead == 0 ) {
			loopCounter_++;
			if ((pListener_ != kNull) && bDoneMessage_)
				SendDoneMsg();
			if (shouldLoop_ && loopCounter_ != payload_)
				Rewind();
			else
				break;
		}
		
		// Keep track of where we are...
		bytesToRead -= bytesRead;
		totalBytesRead += bytesRead;
	 	bufferPtr += bytesRead;
	}

#if	PROFILE_DECODE_LOOP
	pKernelMPI_->GetHRTAsUsec( endTime );

	if (endTime > startTime) {
		elapsedTime = endTime - startTime;
		if (elapsedTime < minUsecs_)
			minUsecs_ = elapsedTime;
		if (elapsedTime > maxUsecs_)
			maxUsecs_ = elapsedTime;
		
		totalUsecs_ += elapsedTime;
		totalBytes_ += totalBytesRead;
	}
	

	// 1KB of mono 16KHz = 32ms of data
	if ( totalBytes_ == 1024 ) {
		printf("Vorbis Player::RenderBuffer: Accumulated %lu total bytes.\n\n ", (long unsigned int)totalBytes_);
		printf("Vorbis Player::RenderBuffer: this took %lu usecs.\n\n ", (long unsigned int)totalUsecs_ );
		printf("Utilization (assuming 16KHz mono data): %f\n", (float)(totalUsecs_/(float)32000));  
		totalUsecs_ = 0;
		totalBytes_ = 0;
	}
#endif

	// Convert bytes back into sample frames
	if (hasStereoData_)
		framesToProcess = totalBytesRead / 2 / 2;
	else
		framesToProcess = totalBytesRead / 2;
			
	// Save total number of frames decoded
	framesRead = framesToProcess;

	// Copy the requested frames from our audioDatPtr to the output buffer.
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
	pDebugMPI_->Assert((kNoErr == result), "CVorbisPlayer::RenderBuffer -- Couldn't unlock mutex.\n");

	// Return the number of frames rendered.
	return framesRead;
}

LF_END_BRIO_NAMESPACE()
// EOF	
