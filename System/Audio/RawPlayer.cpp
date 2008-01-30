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
//{static long c=0; printf("CRawPlayer::ctor: START%ld id =%d\n", c++, (int)id);}

// printf("CRawPlayer::ctor  Will open '%s'\n", pInfo->path->c_str());
    fileType_ = kRawPlayer_FileType_Unknown;
    fileH_    = NULL;

// Try to open WAV or AIFF File
    if (pInfo)
        OpenFile((char *)pInfo->path->c_str());
	
totalBytesRead_ = 0;

//printf("CRawPlayer::ctor -- end \n");
} // ---- end CRawPlayer() ----

// ==============================================================================
// ~CRawPlayer
// ==============================================================================
CRawPlayer::~CRawPlayer()
{
//printf("~CRawPlayer: start %p\n", (void *) this);

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

// If anyone is listening, let them know we're done.
if (pListener_ && bSendDoneMessage_)
    {
printf("~CRawPlayer: id_=%ld BEFO SendDoneMsg()\n", (long)id_);
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
// OpenFile :  Open specified file.  Closed existing file.
// ==============================================================================
    void 
CRawPlayer::OpenFile(char *path)
{
//{static long c=0; printf("CRawPlayer::OpenFile %ld: START '%s'\n", c++, path);}

//	tErrType			result;
if (inFile_)
    CloseFile();

// Try to open WAV or AIFF File
    SF_INFO	 inFileInfo;  
//    inFile_ = OpenSoundFile( (char *) pInfo->path->c_str(), &inFileInfo, SFM_READ);
    inFile_ = OpenSoundFile( path, &inFileInfo, SFM_READ);
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

//printf("WAV Header: frames=%ld ch=%ld fs=%d Hz \n",  channels_, inFileInfo.frames, (int)samplingFrequency_);
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
		printf("Unable to open as WAV/AIFF file '%s'\n",  path);
    	fileH_ = fopen(  path, "r" );
        if (!fileH_)
            printf("CRawPlayer::ctor : Unable to open '%s'\n",  path );

    // Read Brio "Raw" audio header
        fileType_ = kRawPlayer_FileType_Brio;
    	tAudioHeader	brioHeader;		
        tAudioHeader *bH = &brioHeader;
    	int bytesRead = fread( bH, 1, sizeof(tAudioHeader), fileH_);
pDebugMPI_->Assert((bytesRead == sizeof(tAudioHeader)), "CRawPlayer::ctor: Unable to read RAW audio header from '%s'\n", path);

    	samplingFrequency_ = bH->sampleRate;
    	audioDataBytes_    = bH->dataSize;		
        channels_          = 1 + (0 != (bH->flags & kAudioHeader_StereoBit));
//long    	totalFrames    = audioDataBytes_ / (sizeof(S16)*channels_);

//printf("Brio Raw Header: sizeof()=%d dataOffset=%d ch=%ld fs=%d Hz dataSize=%ld\n", 
//    sizeof(tAudioHeader), (int)bH->offsetToData, channels_, (int)bH->sampleRate, bH->dataSize);

pDebugMPI_->Assert( (sizeof(tAudioHeader) == bH->offsetToData), "CRawPlayer::ctor: offsetToData=%ld, but should be %d.  Is this Brio Raw Audio file ? '%s'\n", bH->offsetToData , sizeof(tAudioHeader), path);
		}

bComplete_ = false;
} // ---- end OpenFile() ----

// ==============================================================================
// CloseFile :  
// ==============================================================================
    void 
CRawPlayer::CloseFile()
{
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
} // ---- end CloseFile() ----

// ==============================================================================
// SetAudioInfo :  
// ==============================================================================
    void 
CRawPlayer::SetAudioInfo(tAudioStartAudioInfo* pInfo, tAudioID id)
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

//#define DEBUG_RAWPLAYER_OPTIONS
#ifdef DEBUG_RAWPLAYER_OPTIONS
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

printf("CRawPlayer::SetAudioInfo: listener=%d bSendDoneMessage_=%d bSendLoopEndMessage_=%d flags=$%X '%s'\n", (kNull != pListener_), bSendDoneMessage_, bSendLoopEndMessage_, (unsigned int)optionsFlags_, sFlags);

printf("    payload=%d optionsFlags=$%X -> shouldLoop=%d\n", 
        (int)payload_, (unsigned int) optionsFlags_, shouldLoop_);
printf("    listener=%p DoneMessage=%d LoopEndMessage=%d flags=$%X '%s' loopCount=%ld ($%X)\n", 
        (void *)pListener_, bSendDoneMessage_, bSendLoopEndMessage_, (unsigned int)optionsFlags_, sFlags, 
            loopCount_, (unsigned int) loopCount_);
}
#endif // DEBUG_RAWPLAYER_OPTIONS
} // ---- end SetAudioInfo() ----

// ==============================================================================
// ReadBytesFromFile :  Bytes actually read
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
S16     *bufPtr = pFileReadBuf_;
long    fileEndReached = false;

//totalBytesRead_ = 0;
//{static long c=0; printf("CRawPlayer::Render: start %ld pOut=%p pFileReadBuf_=%p framesToRender=%ld\n", c++, (void *) pOut, (void *) pFileReadBuf_, framesToRender);}
//printf("CRawPlayer::Render: shouldLoop=%d \n", shouldLoop_);
if (bComplete_)
    return (0);

//
// Read data from file to output buffer
//
// GK FIXX: pad short reads with zeros unless there is looping
// GK FIXX: looping is not seamless as compression shifts the audio and alters loop points.
	while ( !fileEndReached && bytesToRead > 0) 
        {
        bytesRead = ReadBytesFromFile(bufPtr, bytesToRead);
//{static long c=0; printf("CRawPlayer::Render%ld: bytesRead=%ld/%ld\n", c++, bytesRead, bytesToRead);}

// Loop audio:  send Done or LoopEnd message, but never both at end of loop
        fileEndReached = ( bytesRead < bytesToRead );
		if ( fileEndReached)
            {
        // Short read:  rewind file and get remaining bytes from another file read
            if (shouldLoop_)
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
            // Send loop end message
        	        if (bSendLoopEndMessage_)
                        {
//                {static long c=0; printf("CRawPlayer::Render%ld: bSendLoopEndMessage_=%d fileEndReached=%ld\n", c++, bSendLoopEndMessage_, fileEndReached);}
                		SendLoopEndMsg();
                        }
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
			pOut[index] = pFileReadBuf_[index];
    // Fill remainder with zeros -> Not needed if parent Render() covers this
//		for (; index < samplesToProcess; index++) 			
//			pOut[index] = 0;
	    } 
// Fan out mono data to stereo output buffer
    else 
        {
		for (index = 0; index < samplesToProcess; index++, pOut += 2) 
            {	
            S16 x = pFileReadBuf_[index];
			pOut[0] = x;
			pOut[1] = x;
		    }
	    }
			
//#define RAWPLAYER_SENDDONE_IN_RENDER
#ifdef RAWPLAYER_SENDDONE_IN_RENDER
	if (pListener_ && bSendDoneMessage_ && fileEndReached)
        {
//{static long c=0; printf("CRawPlayer::Render%ld: bDoneMessage_=%d fileEndReached=%ld\n", c++, bDoneMessage_, fileEndReached);}
		SendDoneMsg();
        }
#endif // RAWPLAYER_SENDDONE_IN_RENDER

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
// GetTime_mSec :   Return current position in audio file in milliseconds
// ==============================================================================
    U32 
CRawPlayer::GetTime_mSec( void ) 
{
U32 totalFramesRead = totalBytesRead_ / (sizeof(S16)*channels_);
U32 milliSeconds = (1000 * totalFramesRead)/samplingFrequency_;

//printf("CRawPlayer::GetTime_mSec: time=%ld ch=%ld totalFramesRead=%ld -> (fs=%ld)\n", 
//        milliSeconds, channels_, totalFramesRead, samplingFrequency_);

return (milliSeconds);
}   // ---- end GetTime_mSec() ----


LF_END_BRIO_NAMESPACE()

// EOF	
