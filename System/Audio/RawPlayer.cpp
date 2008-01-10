//==============================================================================
// Copyright (c) 2002-2007 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// RawPlayer.cpp
//
// Description:
//		The class to manage the playing of audio files (currently uncompressed)
//              Supports AIFF, WAV and the "RAW" Brio file, the latter of which
//              is technically not a RAW file.
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

CRawPlayer::CRawPlayer( tAudioStartAudioInfo* pInfo, tAudioID id  ) : CAudioPlayer( pInfo, id  )
{
	tErrType			result;
	
//printf("CRawPlayer::ctor -- start id =%d\n", id);

#ifdef USE_RAW_PLAYER_MUTEX
// Setup Mutex object for protecting render calls
	pKernelMPI_ =  new CKernelMPI();
	result = pKernelMPI_->IsValid();
	pDebugMPI_->Assert((true == result), "CRawPlayer::ctor: Unable to create KernelMPI.\n");

	const tMutexAttr 	attr = {0};
	result = pKernelMPI_->InitMutex( render_mutex_, attr );
	pDebugMPI_->Assert((kNoErr == result), "CRawPlayer::ctor: Unable to init mutex.\n");
#endif

// Allocate player's sample buffer
	pReadBuf_ = new S16[ 2*kAudioOutBufSizeInWords ];  // GK FIXX:  2x needed?

// Set up looping
	shouldLoopFile_ = (0 < payload_) && (0 != (optionsFlags_ & kAudioOptionsLooped));
    loopCount_      = payload_;
    loopCounter_    = 0;

//#define DEBUG_RAWPLAYER_OPTIONS
#ifdef DEBUG_RAWPLAYER_OPTIONS
{
char s[80];
s[0] = '\0';
if (optionsFlags_ & kAudioOptionsLooped)
    strcat(s, "Loop=On ");
else
    strcat(s, "Loop=Off ");
if (optionsFlags_ & kAudioOptionsDoneMsgAfterComplete)
    strcat(s, "SendDone=On");
else
    strcat(s, "SendDone=Off");

printf("CRawPlayer:ctor: payload_=%d optionsFlags=$%X -> shouldLoopFile=%d \n", 
        (int)payload_, (unsigned int) optionsFlags_, shouldLoopFile_);
printf("CRawPlayer:: listener=%p DoneMessage=%d flags=$%X '%s' loopCount=%ld\n", 
        (void *)pListener_, bDoneMessage_, (unsigned int)optionsFlags_, s, loopCount_);
}
#endif // DEBUG_RAWPLAYER_OPTIONS

// 
// ---- Open audio file
//
// printf("CRawPlayer::ctor  Will open '%s'\n", pInfo->path->c_str());
    fileType_ = kRawPlayer_FileType_Unknown;
    fileH_    = NULL;
    strcpy(inFilePath, pInfo->path->c_str()); 

// Try to open WAV or AIFF File
    SF_INFO	 inFileInfo;  
    inFile_ = OpenSoundFile(inFilePath, &inFileInfo, SFM_READ);
	if (inFile_)
		{
    	long fileFormatType = (inFileInfo.format & SF_FORMAT_TYPEMASK);
    	if 	    (SF_FORMAT_AIFF == fileFormatType)
            fileType_ = kRawPlayer_FileType_AIFF; 
    	else if (SF_FORMAT_WAV == fileFormatType)
            fileType_ = kRawPlayer_FileType_WAV;
    	else
    		printf("CRawPlayer::ctor: unsupported file type=%ld\n", fileFormatType);

        if (kRawPlayer_FileType_Unknown != fileType_)
            {
        	samplingFrequency_ = inFileInfo.samplerate;
        	audioDataBytes_    = inFileInfo.frames*inFileInfo.channels*sizeof(S16);		
            channels_          = inFileInfo.channels;   
//            totalFrames_    = inFileInfo.frames;
            }
        // For unsupported file type, just zero everything
        else
            {
        	samplingFrequency_ = 0;
        	audioDataBytes_    = 0;		
            channels_          = 0;   
//            totalFrames_    = 0;
            }
        }
    // If WAV/AIFF opens failed, try to open as Brio "RAW" file
    else
        {
//		printf("Unable to open as WAV/AIFF file '%s'\n", inFilePath);
    	fileH_ = fopen( inFilePath, "r" );
        if (!fileH_)
            printf("CRawPlayer::ctor : Unable to open '%s'\n", inFilePath );

    // Read Brio "Raw" audio header
        fileType_ = kRawPlayer_FileType_Brio;
    	tAudioHeader	brioHeader;		
        tAudioHeader *bH = &brioHeader;
    	int bytesRead = fread( bH, 1, sizeof(tAudioHeader), fileH_);
pDebugMPI_->Assert((bytesRead == sizeof(tAudioHeader)), "CRawPlayer::ctor: Unable to read RAW audio header.\n");

    	samplingFrequency_ = bH->sampleRate;
    	audioDataBytes_    = bH->dataSize;		
        channels_          = 1 + (0 != (bH->flags & kAudioHeader_StereoBit));
//long    	totalFrames    = audioDataBytes_ / (sizeof(S16)*channels_);

//printf("Brio Raw Header: sizeof()=%d dataOffset=%d ch=%ld fs=%d Hz dataSize=%ld\n", 
//    sizeof(tAudioHeader), (int)bH->offsetToData, channels_, (int)bH->sampleRate, bH->dataSize);

pDebugMPI_->Assert( (sizeof(tAudioHeader) == bH->offsetToData), "CRawPlayer::ctor: offsetToData=%ld, but should be %d.  Is this Brio Raw Audio file ?\n", bH->offsetToData , sizeof(tAudioHeader));
		}
	
//long    	totalFrames    = audioDataBytes_ / (sizeof(S16)*channels_);
//printf("CRawPlayer AF info: ch=%ld fs=%ld Hz frames=%ld\n", channels_, samplingFrequency_, totalFrames);
// Most of member vars set by superclass; these are RAW specific.
//framesRemaining_ = totalFrames_;
totalBytesRead_ = 0;

//printf("CRawPlayer::ctor -- end \n");
} // ---- end CRawPlayer() ----

// ==============================================================================
// ~CRawPlayer
// ==============================================================================
CRawPlayer::~CRawPlayer()
{
//printf("~CRawPlayer: start %p\n", (void *) this);

#ifdef USE_RAW_PLAYER_MUTEX
tErrType result = pKernelMPI_->LockMutex( render_mutex_ );
if (kNoErr != result)
    printf("~CRawPlayer: Couldn't lock render_mutex\n");
#endif

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
//        inFile_ = NULL;
        }

#ifdef USE_RAW_PLAYER_MUTEX
	result = pKernelMPI_->UnlockMutex( render_mutex_ );
if (kNoErr != result)
    printf("~CRawPlayer: Couldn't unlock render_mutex\n");

	result = pKernelMPI_->DeInitMutex( render_mutex_ );
if (kNoErr != result)
    printf("~CRawPlayer: Couldn't deinit render_mutex\n");
if (pKernelMPI_)    
    {
	delete pKernelMPI_;  
    pKernelMPI_ = NULL;
    }
#endif

// If anyone is listening, let them know we're done.
if (pListener_ && bDoneMessage_)
    {
//printf("~CRawPlayer: SSSSSSSSS befo SendDoneMsg()\n");
	SendDoneMsg();
    }

// Free MPIs
	if (pDebugMPI_)
        {
		delete pDebugMPI_;  
        pDebugMPI_ = NULL;
        }

//printf("~CRawPlayer: end pListener=%p bDoneMessage=%d\n", (void*)pListener_, bDoneMessage_);
} // ---- end ~CRawPlayer() ----

// ==============================================================================
// SendDoneMsg
// ==============================================================================
    void 
CRawPlayer::SendDoneMsg( void )
{
	const tEventPriority	kPriorityTBD = 0;
	tAudioMsgAudioCompleted	data;
printf("CRawPlayer::SendDoneMsg audioID=%ld\n", id_);

	data.audioID = id_;	        
	data.payload = loopCount_;
	data.count   = 1;

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pListener_);
}   // ---- end SendDoneMsg() ----

// ==============================================================================
// ReadBytesFromFile :   
//                      Return specified bytes read.
// ==============================================================================
    U32 
CRawPlayer::ReadBytesFromFile( void *d, U32 bytesToRead)
{
U32 bytesRead = 0;

if      (kRawPlayer_FileType_Brio == fileType_)
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
}   // ---- end ReadBytesFromFile() ----

// ==============================================================================
// Render:        Return framesRead
// ==============================================================================
    U32 
CRawPlayer::Render( S16 *pOut, U32 framesToRender )
{	
tErrType result;
U32		index;
U32		framesRead = 0, framesToProcess = 0;
U32		bytesRead = 0;
U32     bytesToRead = framesToRender * sizeof(S16) * channels_;
S16     *bufPtr = pReadBuf_;
long    fileEndReached = false;

//totalBytesRead_ = 0;

//{static long c=0; printf("CRawPlayer::Render: start %ld framesToRender=%ld\n", c++, framesToRender);}
//printf("CRawPlayer::Render: shouldLoopFile=%d \n", shouldLoopFile_);
if (bComplete_)
    return (0);

	// Don't want to try to render if stop() or dtor() have been entered.
	// TODO/dg: this is a really ugly hack.  Need to figure out what to return
	// in the case of render being called while stopping/dtor is running.
#ifdef USE_RAW_PLAYER_MUTEX
	result = pKernelMPI_->TryLockMutex( renderMutex_ );
	if (EBUSY == result)
        {
        printf("CRawPlayer::Render: EBUSY == renderMutex.\n");
 		return (0);
        }
    if (kNoErr != result)
        printf("CRawPlayer::Render: Unable to lock renderMutex.\n");
#endif

//
// Read data from file to output buffer
//
// GK FIXX: pad short reads with zeros unless there is looping
// GK FIXX: looping is not seamless
	while ( !fileEndReached && bytesToRead > 0) 
        {
        bytesRead = ReadBytesFromFile(bufPtr, bytesToRead);

// Loop audio seamlessly
        fileEndReached = ( bytesRead < bytesToRead );
		if ( fileEndReached)
            {
        // Short read:  rewind file and get remaining bytes from another file read
            if (shouldLoopFile_)
                {
                if (loopCounter_++ < loopCount_)
                    {
//{static long c=0; printf("CRawPlayer::Render: Rewind %ld loop%ld/%ld\n", c++, loopCounter_, loopCount_);}
		            totalBytesRead_ = 0;
    			    RewindFile();
//printf("bytesRead=%ld/%ld\n", bytesRead, bytesToRead);
                    bytesRead += ReadBytesFromFile(&bufPtr[bytesRead], bytesToRead-bytesRead);
//printf("POST bytesRead=%ld\n", bytesRead);
                    fileEndReached = false;
                    }
                }
        // Pad with zeros after last legitimate sample
            else
                {
                ClearBytes(&bufPtr[bytesRead], bytesToRead-bytesRead);
                }
		    }
        
		bytesToRead     -= bytesRead;
    	bufPtr          += bytesRead;
		totalBytesRead_ += bytesRead;
	    }
		
	// Copy to output buffer
    bComplete_ = fileEndReached;
        
	framesRead      = bytesRead / (sizeof(S16) * channels_);
	framesToProcess = framesRead;
    U32 samplesToProcess = channels_*framesToProcess;

// Copy Stereo data to stereo output buffer
	if (2 == channels_) 
        {
		for (index = 0; index < samplesToProcess; index++) 			
			pOut[index] = pReadBuf_[index];
    // Fill remainder with zeros -> Not needed if parent Render() covers this
//		for (; index < samplesToProcess; index++) 			
//			pOut[index] = 0;
	    } 
// Fan out mono data to stereo output buffer
    else 
        {
		for (index = 0; index < samplesToProcess; index++, pOut += 2) 
            {	
            S16 x = pReadBuf_[index];
			pOut[0] = x;
			pOut[1] = x;
		    }
	    }
			
#ifdef USE_RAW_PLAYER_MUTEX
	result = pKernelMPI_->UnlockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CRawPlayer::Render: Unable to unlock render_mutex.\n");
    if (kNoErr != result)   
        printf("CRawPlayer::Render: Unable to unlock renderMutex.\n");
#endif

//#define RAWPLAYER_SENDDONE_IN_RENDER
#ifdef RAWPLAYER_SENDDONE_IN_RENDER
	if (pListener_ && bDoneMessage_ && fileEndReached)
        {
//{static long c=0; printf("CRawPlayer::Render%ld: bDoneMessage_=%d fileEndReached=%ld\n", c++, bDoneMessage_, fileEndReached);}
		SendDoneMsg();
        }
#endif // RAWPLAYER_SENDDONE_IN_RENDER

//if (1 == waitForRender_)
//    {
//printf("RawPlayer::Render: set waitForRender to 1\n");
//    pKernelMPI_->TaskSleep( 100 );
//    waitForRender_ = 2;
//    }

//#define TEST_RAWPLAYER_GETAUDIOTIME_EQUATION
#ifdef TEST_RAWPLAYER_GETAUDIOTIME_EQUATION
{
U32 totalFramesRead = totalBytesRead_ / (sizeof(S16)*channels_);
U32 milliSeconds = (1000 * totalFramesRead)/samplingFrequency_;
printf("CRawPlayer::Render() totalFramesRead=%ld -> %ld milliSeconds\n", totalFramesRead, milliSeconds);
}
#endif // TEST_RAWPLAYER_GETAUDIOTIME_EQUATION

//{static long c=0; printf("CRawPlayer::Render%ld: END framesRead=%ld\n", c++, framesRead);}
return (framesRead);
} // ---- end Render() ----

// ==============================================================================
// RewindFile:    Set file ptr to start of file
// ==============================================================================
    void 
CRawPlayer::RewindFile()
{
if (fileH_)
    fseek( fileH_, sizeof(tAudioHeader), SEEK_SET);
if (inFile_)
    RewindSoundFile( inFile_);

//framesRemaining_ = totalFrames_;
}   // ---- end RewindFile() ----

// ==============================================================================
// GetAudioTime_mSec :   Return current position in audio file in milliSeconds
// ==============================================================================
    U32 
CRawPlayer::GetAudioTime_mSec( void ) 
{
U32 totalFramesRead = totalBytesRead_ / (sizeof(S16)*channels_);
U32 milliSeconds = (1000 * totalFramesRead)/samplingFrequency_;

//printf("CRawPlayer::GetAudioTime_mSec: time=%ld ch=%ld totalFramesRead=%ld -> (fs=%ld)\n", 
//        milliSeconds, channels_, totalFramesRead, samplingFrequency_);

return (milliSeconds);
}   // ---- end GetAudioTime_mSec() ----


LF_END_BRIO_NAMESPACE()

// EOF	
