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
//#define VORBIS_LITTLE_ENDIAN	0
//#define VORBIS_BIG_ENDIAN		1
//#define VORBIS_16BIT_WORD		2
//#define VORBIS_SIGNED_DATA		1

//==============================================================================
// Global variables
//==============================================================================

//==============================================================================
// CVorbisPlayer implementation
//==============================================================================

CVorbisPlayer::CVorbisPlayer( tAudioStartAudioInfo* pInfo, tAudioID id ) : CAudioPlayer( pInfo, id  )
{
	tErrType			ret = kNoErr;
	vorbis_info*		pVorbisInfo;
//	ogg_int64_t			lengthInSeconds, length;
	
// Try to open WAV or AIFF File
    if (pInfo)
        OpenFile((char *)pInfo->path->c_str());

	pDebugMPI_->SetDebugLevel( kAudioDebugLevel );

} // ---- end CVorbisPlayer() ----

// ==============================================================================
// ~CVorbisPlayer
// ==============================================================================
CVorbisPlayer::~CVorbisPlayer()
{
//	tErrType result;
CloseFile();
	
	// If anyone is listening, let them know we're done.
if (pListener_ && bSendDoneMessage_)
    {
//printf("~CVorbisPlayer: id_=%ld BEFO SendDoneMsg()\n", (long)id_);
	SendDoneMsg();
    }
		
// Free MPIs
if (pDebugMPI_)
	delete (pDebugMPI_);
//	printf(" CVorbisPlayer::dtor -- vaporizing...\n");
} // ---- end ~CVorbisPlayer() ----

// ==============================================================================
// OpenFile: 
// ==============================================================================
    void
CVorbisPlayer::OpenFile(char *path)
{
	vorbis_info*		pVorbisInfo;
//{static long c=0; printf("CVorbisPlayer::OpenFile %ld: START '%s'\n", c++, path);}

// Close vorbis file
S32 ret = ov_clear( &vorbisFile_ );
if ( ret < 0)
	printf("CVorbisPlayer::OpenFile: Could not close OggVorbis file '%s'\n", path);

// Open file
fileH_ = fopen( path, "r" );
pDebugMPI_->Assert( fileH_ > 0, "CVorbisPlayer::OpenFile : Unable to open '%s'\n", path );

	// Open Ogg-compressed file
	int ov_ret = ov_open( fileH_, &vorbisFile_, NULL, 0 );
//	printf("VorbisPlayer::ctor: ov_open() returned: %d.\n", (int)ov_ret);
pDebugMPI_->AssertNoErr( ov_ret,  "CVorbisPlayer::OpenFile: Is Ogg file? '%s'\n",  path);
	  
	pVorbisInfo        = ov_info( &vorbisFile_, -1 );
	channels_          = pVorbisInfo->channels;
	samplingFrequency_ = pVorbisInfo->rate;
	
bComplete_ = false;

//#define PRINT_AUDIO_FILE_INFO
#ifdef PRINT_AUDIO_FILE_INFO
{ 
vorbis_info *d = pVorbisInfo;
long  totalSamples = (long)ov_pcm_total( &vorbisFile_, -1 );
long  totalBytes   = (long)ov_raw_total( &vorbisFile_, -1 );
float totalSeconds = 0.001f*(float)ov_time_total( &vorbisFile_, -1 );
long  uncompressedBytes = (long)(totalSeconds * (sizeof(S16) * samplingFrequency_));

printf("---- CVorbisPlayer::OpenFile:  file='%s' \n", path);
printf( "    fs=%ld channels=%ld samples=%ld (%g Seconds)\n", 
           samplingFrequency_, channels_, totalSamples, totalSeconds);
printf("     bitstream length=%ld bytes\n", totalBytes);
printf("     bitrate: lower=%ld upper=%ld nominal=%ld bps\n",
            d->bitrate_lower, d->bitrate_upper, d->bitrate_nominal);
printf("     --> faux \"calculated\" bitrate (file size/file time)=%ld bps\n", 
            (long)(((float) (8*totalBytes))/totalSeconds));
printf("     --> uncompressed bytes = %ld compression ratio=%g\n", uncompressedBytes, ((float) uncompressedBytes)/(float) totalBytes); 
}
#endif
} // ---- end OpenFile() ----

// ==============================================================================
// CloseFile: 
// ==============================================================================
    void 
CVorbisPlayer::CloseFile()
{
//printf("CVorbisPlayer::CloseFile.\n "); 
// Close vorbis file
S32 ret = ov_clear( &vorbisFile_ );
if ( ret < 0)
	printf("Could not close OggVorbis file.\n");

if (fileH_)
    {
    fclose(fileH_);
    fileH_ = NULL;
    }
} // ---- end CloseFile() ----

// ==============================================================================
// RewindFile: Seek to beginning of file
// ==============================================================================
    void 
CVorbisPlayer::RewindFile()
{
//printf("CVorbisPlayer::RewindFile.\n "); 
// GK FIXXX: some crashes have been seen in Vorbis looping.  Perhaps more reliable
// operation is possible with a File close and reopen.
ov_time_seek( &vorbisFile_, 0 );
} // ---- end RewindFile() ----

// ==============================================================================
// GetTime_mSec:   Return file position in time (milliSeconds)
// ==============================================================================
    U32 
CVorbisPlayer::GetTime_mSec( void )
{
ogg_int64_t vorbisTime = ov_time_tell( &vorbisFile_ );
U32 timeInMS           = (U32) vorbisTime;
//printf("Vorbis Player::GetTime: vorbisTime=%ld time(mSec)=%d\n", (long)vorbisTime, (int) timeInMS ); 

// GK FIXXX: should probably protect this with mutex
return (timeInMS);
} // ---- end GetTime_mSec() ----

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
	pKernelMPI_->GetHRTAsUsec( startTime );
#endif
	
	bytesToRead = numStereoFrames * sizeof(S16) * channels_;
	
// GK FIXX: pad short reads with zeros unless there is looping
// GK FIXX: looping is not seamless
// Read multiple times because vorbis doesn't always return requested # of bytes
// Notes that BytesToRead is decremented
bufPtr = (char *) pFileReadBuf_;
long fileEndReached = false;
long bytesReadThisRender   = 0;
long bytesToReadThisRender = bytesToRead;
while ( !fileEndReached && bytesToRead > 0 ) 
    {
//printf("Vorbis Player::Render: about to ov_read() %u bytes.\n ", bytesToRead);
		bytesRead = ov_read( &vorbisFile_, bufPtr, bytesToRead, 
	 		//VORBIS_LITTLE_ENDIAN, VORBIS_16BIT_WORD, VORBIS_SIGNED_DATA, 
	 		&dummy );
	 	// Tremor ov_read() = 16-bit signed native format 		
// printf("VorbisPlayer::Render: ov_read() %u bytes\n", bytesRead);
    // NOTE: this is different from other files in that we 
	    fileEndReached = (0 == bytesRead);
        bytesReadThisRender += bytesRead;
		if ( fileEndReached && shouldLoop_)
            {
            if (loopCounter_++ < loopCount_)
                {
//{static long c=0; printf("CVorbisPlayer::Render: Rewind %ld loop%ld/%ld\n", c++, loopCounter_, loopCount_);}
			    RewindFile();
                fileEndReached = false;
            	if (bSendLoopEndMessage_)
                    {
//{static long c=0; printf("CVorbisPlayer::Render%ld: bSendLoopEndMessage_=%d fileEndReached=%ld\n", c++, bSendLoopEndMessage_, fileEndReached);}
            		SendLoopEndMsg();
                    }
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

// Copy stereo samples to stereo output buffer
	if (2 == channels_) 
        {
        U32 samplesToProcess = framesToProcess*2;
		for (index = 0; index < samplesToProcess; index++) 			
			pOut[index] = pFileReadBuf_[index];
	    } 
// Copy mono sample to stereo output buffer
    else 
        {
		for (index = 0; index < framesToProcess; index++, pOut += 2) 
            {	
            S16 x = pFileReadBuf_[index];
			pOut[0] = x;
			pOut[1] = x;
		    }
    	}
	
//{static long c=0; printf("CVorbisPlayer::Render%ld: END framesRead=%ld\n", c++, framesRead);}
return (framesRead);  
} // ---- end Render() ----

// ==============================================================================
// SetAudioInfo :  
// ==============================================================================
    void 
CVorbisPlayer::SetAudioInfo(tAudioStartAudioInfo* pInfo, tAudioID id)
{
id_           = id;
priority_     = pInfo->priority;
pListener_    = pInfo->pListener;
payload_      = pInfo->payload;
optionsFlags_ = pInfo->flags;

bSendDoneMessage_    = (0 != (pInfo->flags & kAudioOptionsDoneMsgAfterComplete));
bSendLoopEndMessage_ = (0 != (pInfo->flags & kAudioOptionsLoopEndMsg));

// Set up looping
loopCount_   = payload_;
shouldLoop_  = (0 < loopCount_) && (optionsFlags_ & kAudioOptionsLooped);
loopCounter_ = 0;

//#define DEBUG_VORBISPLAYER_OPTIONS
#ifdef DEBUG_VORBISPLAYER_OPTIONS
{
char sFlags[50];
sFlags[0] = '\0';
if (optionsFlags_ & kAudioOptionsLoopEndMsg)
    strcat(sFlags, "SendLoopEnd=On");
else
    strcat(sFlags, "SendLoopEnd=Off");
if (optionsFlags_ & kAudioOptionsLooped)
    strcat(sFlags, "Loop=On ");
else
    strcat(sFlags, "Loop=Off ");
if (optionsFlags_ & kAudioOptionsDoneMsgAfterComplete)
    strcat(sFlags, "SendDone=On");
else
    strcat(sFlags, "SendDone=Off");

printf("CVorbisPlayer::SetAudioInfo: listener=%d bSendDoneMessage_=%d bSendLoopEndMessage_=%d flags=$%X '%s'\n", (kNull != pListener_), bSendDoneMessage_, bSendLoopEndMessage_, (unsigned int)optionsFlags_, sFlags);

printf("    payload=%d optionsFlags=$%X -> shouldLoop=%d\n", 
        (int)payload_, (unsigned int) optionsFlags_, shouldLoop_);
printf("    listener=%p DoneMessage=%d LoopEndMessage=%d flags=$%X '%s' loopCount=%ld ($%X)\n", 
        (void *)pListener_, bSendDoneMessage_, bSendLoopEndMessage_, (unsigned int)optionsFlags_, sFlags, 
            loopCount_, (unsigned int) loopCount_);
}
#endif // DEBUG_VORBISPLAYER_OPTIONS
} // ---- end SetAudioInfo() ----

LF_END_BRIO_NAMESPACE()

