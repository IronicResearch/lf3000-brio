//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		RawPlayer.cpp
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
	
//printf("CRawPlayer::ctor -- start \n");
//	pDebugMPI_->DebugOut( kDbgLvlVerbose, "CRawPlayer::ctor -- start...\n");

	// Get Kernel MPI
	pKernelMPI_ =  new CKernelMPI();
	result = pKernelMPI_->IsValid();
	pDebugMPI_->Assert((true == result), "CRawPlayer::ctor -- Couldn't create KernelMPI.\n");

	// Setup Mutex object for protecting render calls
	result = pKernelMPI_->InitMutex( render_mutex_, attr );
	pDebugMPI_->Assert((kNoErr == result), "CRawPlayer::ctor -- Couldn't init mutex.\n");

	// Allocate player's sample buffer
	pPcmBuffer_ = new S16[ kAudioOutBufSizeInWords ];

	shouldLoop_  = (0 < payload_) && (0 != (optionsFlags_ & kAudioOptionsLooped));
    loopCount_   = payload_;
    loopCounter_ = 0;

//printf("CRawPlayer:ctor: payload_=%d optionsFlags=$%X -> shouldLoop=%d \n", 
//        (int)payload_, (unsigned int) optionsFlags_, shouldLoop_);
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
    strcat(s, "SendDone=On ");
else
    strcat(s, "SendDone=Off ");

printf("CRawPlayer::ctor: listener=%d flags=$%X '%s'\n", (kNull != pListener_), (unsigned int)optionsFlags_, s);
printf("CRawPlayer::ctor bDoneMessage_=%d shouldLoop_=%d loopCount=%ld\n", bDoneMessage_, shouldLoop_, loopCount_);
}
#endif // DEBUG_RAWPLAYER_OPTIONS

	// 
    // ---- Open audio file
    //
//    printf("Will open '%s'\n", pInfo->path->c_str());
    fileType_ = kRawPlayer_FileType_Unknown;
    fileH_ = NULL;
    strcpy(inFilePath, pInfo->path->c_str()); 

    // Try to open WAV or AIFF File
    inFile_ = OpenSoundFile(inFilePath, &inFileInfo_, SFM_READ);
	if (inFile_)
		{
    	long fileFormatType = (inFileInfo_.format & SF_FORMAT_TYPEMASK);
    	if 	    (SF_FORMAT_AIFF == fileFormatType)
            fileType_ = kRawPlayer_FileType_AIFF; 
    	else if (SF_FORMAT_WAV == fileFormatType)
            fileType_ = kRawPlayer_FileType_WAV;
    	else
    		printf("CRawPlayer::ctor: do not support input file type=%ld\n", fileFormatType);

        if (kRawPlayer_FileType_Unknown != fileType_)
            {
        	dataSampleRate_ = inFileInfo_.samplerate;
        	audioDataSize_  = inFileInfo_.frames*inFileInfo_.channels*sizeof(S16);		
            channels_       = inFileInfo_.channels;   
            totalFrames_    = inFileInfo_.frames;
            }
        else
            {
            }
        }
    // If WAV/AIFF opens failed, try to open as Brio "RAW" file
    else
        {
//		printf("Unable to open as WAV/AIFF file '%s'\n", inFilePath);
    	fileH_ = fopen( inFilePath, "r" );
    	pDebugMPI_->Assert( fileH_ > 0, "CRawPlayer::ctor : Unable to open '%s'\n", inFilePath );

    // Read Brio "Raw" audio header
        fileType_ = kRawPlayer_FileType_Brio;

    	tAudioHeader	brioHeader;		// header data for Brio "RAW" audio
        tAudioHeader *bH = &brioHeader;
    	bytesRead = fread( bH, 1, sizeof(tAudioHeader), fileH_);
pDebugMPI_->Assert((bytesRead == sizeof(tAudioHeader)), "CRawPlayer::ctor: Unable to read RAW audio header.\n");

    	dataSampleRate_ = bH->sampleRate;
    	audioDataSize_  = bH->dataSize;		
        channels_       = 1 + (0 != (bH->flags & kAudioHeader_StereoBit));
    	totalFrames_    = audioDataSize_ / (sizeof(S16)*channels_);

printf("Brio Raw Header: sizeof()=%d dataOffset=%d ch=%ld fs=%u Hz dataSize=%ld\n", 
    sizeof(tAudioHeader), (int)bH->offsetToData, channels_, (unsigned int)bH->sampleRate, bH->dataSize);

pDebugMPI_->Assert( (sizeof(tAudioHeader) == bH->offsetToData), "CRawPlayer::ctor: offsetToData=%ld, but should be %d.  Is this Brio Raw Audio file ?\n", bH->offsetToData , sizeof(tAudioHeader));
		}
	
//printf("AF info: ch=%ld fs=%ld Hz frames=%ld\n", channels_, dataSampleRate_, totalFrames_);

	// Most of the member vars set by superclass; these are RAW specific.
	framesRemaining_ = totalFrames_;
	
//	pDebugMPI_->DebugOut( kDbgLvlVerbose, "CRawPlayer::ctor #Frames=%d optionsFlags=%X\n", (int)totalFrames_, (int)optionsFlags_);

//printf("CRawPlayer::ctor -- end \n");
} // ---- end CRawPlayer() ----

// ==============================================================================
// ~CRawPlayer
// ==============================================================================
CRawPlayer::~CRawPlayer()
{
//printf("~CRawPlayer: start\n");

	tErrType result;
	
	result = pKernelMPI_->LockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CRawPlayer::dtor -- Couldn't lock mutex.\n");

	// If anyone is listening, let them know we're done.
	if (pListener_ && bDoneMessage_)
		SendDoneMsg();

	// Free sample buffer
	if (pPcmBuffer_)
		delete pPcmBuffer_;

	// Close the file
    if (fileH_)
        {
	    fclose( fileH_ );
        fileH_ = NULL;
        }
	if (inFile_)
        {
        CloseSoundFile(&inFile_);
        inFile_ = NULL;
        }

	result = pKernelMPI_->UnlockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CRawPlayer::dtor: Couldn't unlock mutex.\n");
	result = pKernelMPI_->DeInitMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CRawPlayer::dtor -- Couldn't deinit mutex.\n");

	// Free MPIs
	if (pDebugMPI_)
        {
		delete pDebugMPI_;  
        pDebugMPI_ = NULL;
        }
	if (pKernelMPI_)    
        {
		delete pKernelMPI_;  
        pKernelMPI_ = NULL;
        }
//	pDebugMPI_->DebugOut( kDbgLvlVerbose, " CRawPlayer::dtor -- vaporizing...\n");
//printf("~CRawPlayer: end\n");
} // ---- end ~CRawPlayer() ----

// ==============================================================================
// Rewind
// ==============================================================================
void CRawPlayer::Rewind()
{
// Point back to start of audio data and reset total.
    if (fileH_)
	    fseek( fileH_, sizeof(tAudioHeader), SEEK_SET);
    if (inFile_)
        RewindSoundFile( inFile_);

	framesRemaining_ = totalFrames_;
} // ---- end Rewind() ----

//==============================================================================
// GetAudioTime_mSec :   NOT DONE
//==============================================================================
U32 CRawPlayer::GetAudioTime_mSec( void ) 
{
printf("CRawPlayer::GetAudioTime_mSec: Unimplemented\n");
	return 0;
} // ---- end GetAudioTime_mSec() ----

//==============================================================================
// SendDoneMsg
//==============================================================================
void CRawPlayer::SendDoneMsg( void )
{
	const tEventPriority	kPriorityTBD = 0;
	tAudioMsgAudioCompleted	data;

	data.audioID = id_;	        
	data.payload = loopCount_;
	data.count   = 1;

//printf("CRawPlayer::SendDoneMsg audioID=%d\n", id_);

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pListener_);
} // ---- end SendDoneMsg() ----

//==============================================================================
// RenderBuffer: 
//==============================================================================
U32 CRawPlayer::RenderBuffer( S16* pOutBuff, U32 numStereoFrames )
{	
	tErrType result;
	U32		index;
	U32		framesRead = 0, framesToProcess = 0;
	U32		totalBytesRead = 0, bytesRead = 0, bytesToRead = 0;
	S16*	pCurSample;
	S16* 	bufferPtr;

//{static long c=0; printf("CRawPlayer::RenderBuffer: start %ld numStereoFrames=%ld\n", c++, numStereoFrames);}
//printf("CRawPlayer::RenderBuffer : shouldLoop=%d \n", shouldLoop_);

	// Don't want to try to render if stop() or dtor() have been entered.
	result = pKernelMPI_->TryLockMutex( render_mutex_ );
	
	// TODO/dg: this is a really ugly hack.  need to figure out what to return
	// in the case of render being called while stopping/dtor is running.
	if (EBUSY == result)
		return numStereoFrames;
	else
		pDebugMPI_->Assert((kNoErr == result), "CRawPlayer::RenderBuffer -- Couldn't lock mutex.\n");

	// Read data from file to output buffer
	bufferPtr = pPcmBuffer_;
    long fileEndReached = false;
	bytesToRead = numStereoFrames * sizeof(S16) * channels_;
	while ( !fileEndReached && bytesToRead > 0) 
        {
//printf("totalBytesRead	
        if (kRawPlayer_FileType_Brio == fileType_)
            {		
		    bytesRead = fread( (void *)bufferPtr, sizeof(char), bytesToRead, fileH_ );
	 		}
        else if (kRawPlayer_FileType_WAV  == fileType_ ||
                 kRawPlayer_FileType_AIFF == fileType_)
            {		
            U32 framesToRead = bytesToRead/( sizeof(S16) * channels_);
			U32 framesRead = sf_readf_short(inFile_, bufferPtr, framesToRead);
// FIXXX: silly temporary conversion
            bytesRead = framesRead * sizeof(S16) * channels_;
	 		}

// Loop audio seamlessly
        fileEndReached = ( bytesRead < bytesToRead );
		if ( fileEndReached && shouldLoop_)
            {
//{static long c=0; printf("CRawPlayer::RenderBuffer: Rewind %ld loop%ld/%ld\n", c++, loopCounter_+1, loopCount_);}
            if (++loopCounter_ < loopCount_)
                {
			    Rewind();
                fileEndReached = false;
                }
		    }
		bytesToRead    -= bytesRead;
    	bufferPtr      += bytesRead;
		totalBytesRead += bytesRead;
	    }
		
	// Copy requested frames to the output buffer.
	framesRead      = totalBytesRead / (sizeof(S16) * channels_);
	framesToProcess = framesRead;
	pCurSample      = pPcmBuffer_;

// Copy Stereo data to stereo output buffer
    U32 samplesToProcess = channels_*framesToProcess;
	if (2 == channels_) 
        {
		for (index = 0; index < samplesToProcess; index++) 			
			pOutBuff[index] = pCurSample[index];
// Fan out mono data to stereo output buffer
	    } 
    else {
		for (index = 0; index < samplesToProcess; index++, pOutBuff += 2) {	
            S16 x = pCurSample[index];
			pOutBuff[0] = x;
			pOutBuff[1] = x;
		}
	}
			
	result = pKernelMPI_->UnlockMutex( render_mutex_ );
	pDebugMPI_->Assert((kNoErr == result), "CRawPlayer::RenderBuffer -- Couldn't unlock mutex.\n");

//#define RAWPLAYER_SENDDONE_IN_RENDERBUFFER
#ifdef RAWPLAYER_SENDDONE_IN_RENDERBUFFER
	if (pListener_ && bDoneMessage_ && fileEndReached)
        {
//{static long c=0; printf("CRawPlayer::RenderBuffer%ld: bDoneMessage_=%d fileEndReached=%ld\n", c++, bDoneMessage_, fileEndReached);}
		SendDoneMsg();
        }
#endif // RAWPLAYER_SENDDONE_IN_RENDERBUFFER

//{static long c=0; printf("CRawPlayer::RenderBuffer: end %ld framesRead=%ld\n", c++, framesRead);}
	return (framesRead);
} // ---- end RenderBuffer() ----

LF_END_BRIO_NAMESPACE()

// EOF	
