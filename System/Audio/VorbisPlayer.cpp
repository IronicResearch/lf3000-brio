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

CVorbisPlayer::CVorbisPlayer( tAudioStartAudioInfo* pInfo, tAudioID id  ) : CAudioPlayer( pInfo, id  )
{
	tErrType			ret = kNoErr;
	vorbis_info*		pVorbisInfo;
//	ogg_int64_t			lengthInSeconds, length;
	
	pDebugMPI_->SetDebugLevel( kAudioDebugLevel );

	pReadBuf_ = new S16[ kAudioOutBufSizeInWords ];

#ifdef USE_VORBIS_PLAYER_MUTEX
	pKernelMPI_ =  new CKernelMPI();
	ret = pKernelMPI_->IsValid();
	pDebugMPI_->Assert((true == ret), "CVorbisPlayer::ctor: Couldn't create KernelMPI.\n");

	// Setup Mutex object for protecting render calls
	const tMutexAttr 	attr = {0};
  	ret = pKernelMPI_->InitMutex( render_mutex_, attr );
	pDebugMPI_->Assert((kNoErr == ret), "CVorbisPlayer::ctor: Couldn't init mutex.\n");
#endif 

	// Open file
	fileH_ = fopen( pInfo->path->c_str(), "r" );
	pDebugMPI_->Assert( fileH_ > 0, 
		"VorbisPlayer::ctor : Unable to open file: '%s'\n", pInfo->path->c_str() );
 
// Set up looping
	shouldLoopFile_ = (0 < payload_) && (0 != (optionsFlags_ & kAudioOptionsLooped));
    loopCount_      = payload_;
    loopCounter_    = 0;

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
printf("kAudioOptionsDoneMsgAfterComplete=%d\n", kAudioOptionsDoneMsgAfterComplete);
printf("kAudioOptionsNoDoneMsg           =%d\n", kAudioOptionsNoDoneMsg);

printf("VorbisPlayer:ctor: payload=%d optionsFlags=$%X -> shouldLoopFile=%d\n", 
        (int)payload_, (unsigned int) optionsFlags_, shouldLoopFile_);
printf("VorbisPlayer:: listener=%p DoneMessage=%d flags=$%X '%s' loopCount=%ld\n", 
        (void *)pListener_, bDoneMessage_, (unsigned int)optionsFlags_, s, loopCount_);
}
#endif // DEBUG_VORBISPLAYER_OPTIONS

	// Open Ogg-compressed file
	int ov_ret = ov_open( fileH_, &vorbisFile_, NULL, 0 );
//	printf("VorbisPlayer::ctor: ov_open() returned: %d.\n", (int)ov_ret);
	pDebugMPI_->AssertNoErr( ov_ret, 
		"VorbisPlayer::ctor: Is Ogg file? '%s'\n", pInfo->path->c_str());
	  
	pVorbisInfo        = ov_info( &vorbisFile_, -1 );
	channels_          = pVorbisInfo->channels;
	samplingFrequency_ = pVorbisInfo->rate;
	
// Printf file info 
//printf( "VorbisPlayer::ctor fs=%ld channels=%d samples=%ld (%g Seconds)\n", 
//           samplingFrequency_, channels_,
//           (long)ov_pcm_total( &vorbisFile_, -1 ), 0.001f*(float)ov_time_total( &vorbisFile_, -1 ));
//printf("VorbisPlayer::ctor bitstream length=%ld\n", (long)ov_raw_total( &vorbisFile_, -1 ));

#if PROFILE_DECODE_LOOP
	totalUsecs_ = 0;
	minUsecs_   = 1000000;
	maxUsecs_   = 0;
	totalBytes_ = 0;
#endif
} // ---- end CVorbisPlayer() ----

//==============================================================================
// ~CVorbisPlayer
//==============================================================================
CVorbisPlayer::~CVorbisPlayer()
{
	tErrType result;
	
#ifdef USE_VORBIS_PLAYER_MUTEX
	result = pKernelMPI_->LockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CVorbisPlayer::dtor -- Couldn't lock mutex.\n");
#endif
	// If anyone is listening, let them know we're done.
if (pListener_ && bDoneMessage_)
    {
//printf("~CVorbisPlayer: SSSSSSSSS befo SendDoneMsg()\n");
	SendDoneMsg();
    }
	
	if (pReadBuf_)
		delete pReadBuf_;
	
	// Close vorbis file
	S32 ret = ov_clear( &vorbisFile_ );
	if ( ret < 0)
		printf("Could not close OggVorbis file.\n");

#ifdef USE_VORBIS_PLAYER_MUTEX
	result = pKernelMPI_->UnlockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CVorbisPlayer::dtor: Couldn't unlock mutex.\n");
	result = pKernelMPI_->DeInitMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CVorbisPlayer::dtor: Couldn't deinit mutex.\n");
	if (pKernelMPI_)
		delete pKernelMPI_;
#endif
	pDebugMPI_->DebugOut( kDbgLvlVerbose, " CVorbisPlayer::dtor -- vaporizing...\n");
	
// Free MPIs
if (pDebugMPI_)
	delete pDebugMPI_;
} // ---- end ~CVorbisPlayer() ----

// ==============================================================================
// RewindFile: Seek to beginning of file
// ==============================================================================
    void 
CVorbisPlayer::RewindFile()
{
//printf("Vorbis Player::RewindFile.\n "); 
ov_time_seek( &vorbisFile_, 0 );
} // ---- end RewindFile() ----

// ==============================================================================
// GetAudioTime_mSec:   Return file position in time (milliSeconds)
// ==============================================================================
    U32 
CVorbisPlayer::GetAudioTime_mSec( void )
{
ogg_int64_t vorbisTime = ov_time_tell( &vorbisFile_ );
U32 timeInMS           = (U32) vorbisTime;
//printf("Vorbis Player::GetAudioTime: vorbisTime=%ld time(mSec)=%d\n", (long)vorbisTime, (int) timeInMS ); 

// GK FIXXX: should probably protect this with a mutex
return (timeInMS);
} // ---- end GetAudioTime_mSec() ----

// ==============================================================================
// SendDoneMsg
// ==============================================================================
    void 
CVorbisPlayer::SendDoneMsg( void ) 
{
	const tEventPriority	kPriorityTBD = 0; // lower priority for async post
	tAudioMsgAudioCompleted	data;

//printf("CVorbisPlayer::SendDoneMsg audioID=%d\n", id_);
	data.audioID = id_;
	data.payload = loopCount_;
	data.count   = 1;

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pListener_);
} // ---- end SendDoneMsg() ----

// ==============================================================================
// Render        Convert Ogg stream to linear PCM data
// ==============================================================================
    U32 
CVorbisPlayer::Render( S16* pOut, U32 numStereoFrames )
{	
	tErrType result;
	U32		index;
	int 	dummy;
	U32		framesToProcess = 0;
	U32		totalBytesRead  = 0;
	U32		bytesRead   = 0;
	U32 	bytesToRead = 0;
	char* 	bufPtr;
	U32		framesRead = 0;

#if PROFILE_DECODE_LOOP
	U32		startTime, endTime, elapsedTime;
#endif
	
#ifdef USE_VORBIS_PLAYER_MUTEX
	result = pKernelMPI_->TryLockMutex( render_mutex_ );
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
// Read multiple times because vorbis doesn't always return requested # of bytes
// Notes that BytesToRead is decremented
bufPtr = (char *) pReadBuf_;
long fileEndReached = false;
long bytesReadThisRender   = 0;
long bytesToReadThisRender = bytesToRead;
while ( !fileEndReached && bytesToRead > 0 ) 
    {
//printf("Vorbis Player::Render: about to ov_read() %u bytes.\n ", bytesToRead);
		bytesRead = ov_read( &vorbisFile_, bufPtr, bytesToRead, 
	 		//VORBIS_LITTLE_ENDIAN, 
	 		//VORBIS_16BIT_WORD, 
	 		//VORBIS_SIGNED_DATA, 
	 		&dummy );
	 	// Tremor ov_read() = 16-bit signed native format 		
// printf("VorbisPlayer::Render: ov_read() %u bytes\n", bytesRead);
    // NOTE: this is different from other files in that we 
	    fileEndReached = (0 == bytesRead);
        bytesReadThisRender += bytesRead;
		if ( fileEndReached && shouldLoopFile_)
            {
            if (loopCounter_++ < loopCount_)
                {
//{static long c=0; printf("CVorbisPlayer::Render: Rewind %ld loop%ld/%ld\n", c++, loopCounter_, loopCount_);}
			    RewindFile();
                fileEndReached = false;
                }
        // Pad with zeros after last legitimate sample
            else
                {
                ClearBytes(&bufPtr[bytesReadThisRender], bytesToReadThisRender-bytesReadThisRender);
                }
		    }
		
		bytesToRead    -= bytesRead;
		totalBytesRead += bytesRead;
	 	bufPtr         += bytesRead;
	}

//#define VORBISPLAYER_SENDDONE_IN_RENDER
#ifdef VORBISPLAYER_SENDDONE_IN_RENDER
	if (pListener_ && bDoneMessage_ && fileEndReached)
        {
//{static long c=0; printf("CVorbisPlayer::Render%ld: bDoneMessage_=%d fileEndReached=%ld\n", c++, bDoneMessage_, fileEndReached);}
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
		printf("Vorbis Player::Render: Accumulated %lu total bytes.\n\n ", (long unsigned int)totalBytes_);
		printf("Vorbis Player::Render: this took %lu usecs.\n\n ", (long unsigned int)totalUsecs_ );
		printf("Utilization (assuming 16KHz mono data): %f\n", (float)(totalUsecs_/(float)32000));  
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
//	printf("stereo buffer: pCurSample = 0x%x; frames = %d; next pCurSample Should be: 0x%x\n", pCupCurSamplerFrame_, framesToProcess, (pCurSample + framesToProcess));
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
	
#ifdef USE_VORBIS_PLAYER_MUTEX
	result = pKernelMPI_->UnlockMutex( renderMutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CVorbisPlayer::Render -- Couldn't unlock mutex.\n");
#endif

//{static long c=0; printf("CVorbisPlayer::Render%ld: END framesRead=%ld\n", c++, framesRead);}
return (framesRead);  
} // ---- end Render() ----

LF_END_BRIO_NAMESPACE()

