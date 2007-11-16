//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//send
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
//	ogg_int64_t			lengthInSeconds, length;
	const tMutexAttr 	attr = {0};
	
	pDebugMPI_->SetDebugLevel( kAudioDebugLevel );

	// Allocate sample buffer
	pPcmBuffer_ = new S16[ kAudioOutBufSizeInWords ];

	// Get Kernel MPI
	pKernelMPI_ =  new CKernelMPI();
	ret = pKernelMPI_->IsValid();
	pDebugMPI_->Assert((true == ret), "CVorbisPlayer::ctor: Couldn't create KernelMPI.\n");

	// Setup Mutex object for protecting render calls
  	ret = pKernelMPI_->InitMutex( render_mutex_, attr );
	pDebugMPI_->Assert((kNoErr == ret), "CVorbisPlayer::ctor: Couldn't init mutex.\n");

	// Open file
	fileH_ = fopen( pInfo->path->c_str(), "r" );
	pDebugMPI_->Assert( fileH_ > 0, 
		"VorbisPlayer::ctor : Unable to open file: '%s'\n", pInfo->path->c_str() );
 
    // Set up looping
	shouldLoop_  = (0 < payload_) && (0 != (optionsFlags_ & kAudioOptionsLooped));
    loopCount_   = payload_;
    loopCounter_ = 0;
//#define DEBUG_VORBISPLAYER_OPTIONS
#ifdef DEBUG_VORBISPLAYER_OPTIONS
{
char s[80];
s[0] = '\0';
if (optionsFlags_ & kAudioOptionsLooped)
    strcat(s, "Loop=On ");
else
    strcat(s, "Loop=Off ");
if (optionsFlags_ & kAudioOptionsDoneMsgAfterComplete)
    strcat(s, "SendDone=On ");
else
    strcat(s, "SendDone=Off ");

printf("VorbisPlayer::ctor: listener=%d flags=$%X '%s'\n", (kNull != pListener_), (unsigned int)optionsFlags_, s);
printf("VorbisPlayer::ctor bDoneMessage_=%d shouldLoop_=%d loopCount=%ld\n", bDoneMessage_, shouldLoop_, loopCount_);
}
#endif // DEBUG_VORBISPLAYER_OPTIONS

	// Open Ogg-compressed file
	int ov_ret = ov_open( fileH_, &vorbisFile_, NULL, 0 );
	pDebugMPI_->DebugOut( kDbgLvlVerbose,
		"VorbisPlayer::ctor: ov_open() returned: %d.\n", static_cast<int>(ov_ret) );
	pDebugMPI_->AssertNoErr( ov_ret, 
		"VorbisPlayer::ctor: Is Ogg file? '%s'\n", pInfo->path->c_str());
	
	pVorbisInfo     = ov_info( &vorbisFile_, -1 );
	channels_       = pVorbisInfo->channels;
	dataSampleRate_ = pVorbisInfo->rate;
	
	// Figure out how big the vorbis bitstream actually is.
//	pDebugMPI_->DebugOut( kDbgLvlVerbose,
//		"VorbisPlayer::ctor fs=%ld channels=%d samples=%ld (%g Seconds)\n", 
//           pVorbisInfo->rate, pVorbisInfo->channels,
//            (long)ov_pcm_total( &vorbisFile_, -1 ),
//            (float)ov_time_total( &vorbisFile_, -1 ));

//	printf("VorbisPlayer::ctor bitstream length=%ld\n", (long)ov_raw_total( &vorbisFile_, -1 ));

#if PROFILE_DECODE_LOOP
	totalUsecs_ = 0;
	minUsecs_ = 1000000;
	maxUsecs_ = 0;
	totalBytes_ = 0;
#endif
} // ---- end CVorbisPlayer() ----

//==============================================================================
// ~CVorbisPlayer
//==============================================================================
CVorbisPlayer::~CVorbisPlayer()
{
	tErrType result;
	
	result = pKernelMPI_->LockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CVorbisPlayer::dtor -- Couldn't lock mutex.\n");

	// If anyone is listening, let them know we're done.
	if (pListener_ && bDoneMessage_)
		SendDoneMsg();
	
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

	pDebugMPI_->DebugOut( kDbgLvlVerbose, " CVorbisPlayer::dtor -- vaporizing...\n");
	
	// Free MPIs
	if (pDebugMPI_)
		delete pDebugMPI_;
	if (pKernelMPI_)
		delete pKernelMPI_;
} // ---- end ~CVorbisPlayer() ----

// ==============================================================================
// Rewind: Seek to beginning of file
// ==============================================================================
void CVorbisPlayer::Rewind()
{
pDebugMPI_->DebugOut( kDbgLvlVerbose, "Vorbis Player::Rewind.\n "); 
	
ov_time_seek( &vorbisFile_, 0 );
} // ---- end Rewind() ----

// ==============================================================================
// GetAudioTime
// ==============================================================================
U32 CVorbisPlayer::GetAudioTime_mSec( void )
{
	ogg_int64_t vorbisTime = ov_time_tell( &vorbisFile_ );
	U32 timeInMS           = (U32)vorbisTime;

	pDebugMPI_->DebugOut( kDbgLvlVerbose,
		"Vorbis Player::GetAudioTime() -- vorbisTime = %ld; time in ms = %d.\n ",
		static_cast<long>(vorbisTime), static_cast<int>(timeInMS) ); 

	return timeInMS;
} // ---- end GetAudioTime() ----

// ==============================================================================
// SendDoneMsg
// ==============================================================================
void CVorbisPlayer::SendDoneMsg( void ) 
{
	const tEventPriority	kPriorityTBD = 0; // lower priority for async post
	tAudioMsgAudioCompleted	data;

	data.audioID = id_;
	data.payload = loopCount_;
	data.count   = 1;

//printf("CVorbisPlayer::SendDoneMsg audioID=%d\n", id_);

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pListener_);
} // ---- end SendDoneMsg() ----

// ==============================================================================
// RenderBuffer:        Convert Ogg stream to linear PCM data
// ==============================================================================
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
	
		// TODO/dg: this is a really ugly hack.  need to figure out what to return
		// in the case of render being called while stopping/dtor is running.
	if (EBUSY == result)
		return numStereoFrames;
	else
		pDebugMPI_->Assert((kNoErr == result), "CVorbisPlayer::RenderBuffer -- Couldn't lock mutex.\n");

		bytesToRead = numStereoFrames * sizeof(S16) * channels_;
	
#if	PROFILE_DECODE_LOOP
	pKernelMPI_->GetHRTAsUsec( startTime );
#endif	
	// We may have to read multiple times because vorbis doesn't always
	// give you what you ask for.
	bufferPtr = (char*)pPcmBuffer_;
long fileEndReached = false;
	while ( !fileEndReached && bytesToRead > 0 ) {
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
//		if ( 0 == bytesRead) 
//        {
//			if (pListener_ && bDoneMessage_)
//				SendDoneMsg();
//		}
        fileEndReached = (0 == bytesRead);
		if ( fileEndReached && shouldLoop_)
            {
//{static long c=0; printf("CVorbisPlayer::RenderBuffer: Rewind %ld loop%ld/%ld\n", c++, loopCounter_+1, loopCount_);}
            if (++loopCounter_ < loopCount_)
                {
			    Rewind();
                fileEndReached = false;
                }
		    }
		
		bytesToRead    -= bytesRead;
		totalBytesRead += bytesRead;
	 	bufferPtr      += bytesRead;
	}

//#define VORBISPLAYER_SENDDONE_IN_RENDERBUFFER
#ifdef VORBISPLAYER_SENDDONE_IN_RENDERBUFFER
	if (pListener_ && bDoneMessage_ && fileEndReached)
        {
//{static long c=0; printf("CVorbisPlayer::RenderBuffer%ld: bDoneMessage_=%d fileEndReached=%ld\n", c++, bDoneMessage_, fileEndReached);}
		SendDoneMsg();
        }
#endif // VORBISPLAYER_SENDDONE_IN_RENDERBUFFER

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
		framesToProcess = totalBytesRead / sizeof(S16) / channels_;
			
	// Save total number of frames decoded
	framesRead = framesToProcess;

	// Copy requested frames from our audioDatPtr to the output buffer.
	pCurSample = pPcmBuffer_;

	if (2 == channels_) 
        {
		for (index = 0; index < framesToProcess; index++) 
            {			
			*pOutBuff++ = *pCurSample++;
			*pOutBuff++ = *pCurSample++;
		    }
	// Now update the current frame pointer.
//	printf("stereo buffer: pCurSample = 0x%x; frames = %d; next pCurSample Should be: 0x%x\n", pCupCurSamplerFrame_, framesToProcess, (pCurSample + framesToProcess));
	    } 
    else 
        {
	// Copy mono sample twice
		for (index = 0; index < framesToProcess; index++) 
            {		
            S16 x = *pCurSample++;
			*pOutBuff++ = x;
			*pOutBuff++ = x;
    		}
    	}
	
	result = pKernelMPI_->UnlockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CVorbisPlayer::RenderBuffer -- Couldn't unlock mutex.\n");

	// Return # of frames rendered.
	return framesRead;
} // ---- end RenderBuffer() ----

LF_END_BRIO_NAMESPACE()
// EOF	
