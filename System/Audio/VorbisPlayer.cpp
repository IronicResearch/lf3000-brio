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
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <EventMPI.h>
#include <AudioTypes.h>
#include <AudioPriv.h>
#include <AudioTypesPriv.h>
#include <VorbisPlayer.h>

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

CVorbisPlayer::CVorbisPlayer( tAudioStartAudioInfo* pData, tAudioID id  ) : CAudioPlayer( pData, id  )
{
	tErrType 		ret = kNoErr;
	vorbis_info*	pVorbisInfo;
	ogg_int64_t		lengthInSeconds;
	ogg_int64_t		length;
	
//	printf( "Brio Vorbis Test: playback vorbis.\n" );
	
	// Allocate the player's sample buffer
	pPcmBuffer_ = new S16[ kAudioOutBufSizeInWords ];

	// Use rsrc manager to open the ogg file.
	ret = pRsrcMPI_->OpenRsrc( hRsrc_ );  
    if (ret != kNoErr)
        pDebugMPI_->DebugOut( kDbgLvlCritical, (const char *)"VorbisPlayer -- ctor: Could not open oggvorbis file." );
 
    // Keep track of where we are int he bitstream now that it's open.
    filePos_ = 0;

    // Setup the vorbisfile library's custom callback structure
	oggCallbacks_.read_func = &CVorbisPlayer::WrapperForVorbisRead;
	oggCallbacks_.seek_func = &CVorbisPlayer::WrapperForVorbisSeek;
	oggCallbacks_.tell_func = &CVorbisPlayer::WrapperForVorbisTell;
	oggCallbacks_.close_func = &CVorbisPlayer::WrapperForVorbisClose;
      
	// open the file
	ret = ov_open_callbacks( this, &vorbisFile_, NULL, 0, oggCallbacks_ );
	if ( ret < 0)
		printf("Could not open input as an OggVorbis file.\n\n");

	// Figure out how big the vorbis bitstream actually is.
	pVorbisInfo = ov_info( &vorbisFile_, -1 );
	if (pVorbisInfo->channels == 2)
		hasStereoData_ = true;
	else
		hasStereoData_ = false;
	
	dataSampleRate_ = pVorbisInfo->rate;
	
	printf("OggVorbis file's num channels is %d.\n", pVorbisInfo->channels);
	printf("OggVorbis file's sample rate is %ld.\n", pVorbisInfo->rate);

	lengthInSeconds = ov_time_total( &vorbisFile_, -1 );
	printf("OggVorbis file's length in seconds is %f.\n", (float)lengthInSeconds);

	length = ov_raw_total( &vorbisFile_, -1 );
	printf("OggVorbis file's bitstream length is %ld.\n", (long)length);
	length = ov_pcm_total( &vorbisFile_, -1 );
	printf("OggVorbis file's PCM length is %ld.\n", (long)length );
	
//	printf("CVorbisPlayer::ctor Header flags:%d\n", optionsFlags_);
}

//==============================================================================
//==============================================================================
CVorbisPlayer::~CVorbisPlayer()
{
	tErrType ret;
	
	// If there's anyone listening, let them know we're done.
	if ((pListener_ != kNull) && bDoneMessage_)
		SendDoneMsg();

	// Free the sample buffer
	if (pPcmBuffer_)
		delete pPcmBuffer_;
	
	// Close the vorbis file
	ret = ov_clear( &vorbisFile_ );
	if ( ret < 0)
		printf("Could not close OggVorbis file.\n");

//	printf(" CVorbisPlayer::dtor -- I'm HERE!!!\n\n\n\n");
}

U32 CVorbisPlayer::VorbisRead(
		void* data_ptr,
	    size_t byteSize, 
	    size_t sizeToRead )
{
	tErrType 	ret = kNoErr;
	U32			bytesRead;
	
//	printf("Vorbis Player::VorbisRead: requestToRead = %d\n", sizeToRead); fflush(stdout);

	ret	= pRsrcMPI_->ReadRsrc( hRsrc_, data_ptr, sizeToRead, &bytesRead ); 
    if (ret != kNoErr)
        pDebugMPI_->DebugOut( kDbgLvlCritical, (const char *)"VorbisRead: ReadRsrc() returned error %d.", ret );
	
//	printf("Vorbis Player::VorbisRead: ReadRsrc returned %d bytes\n", bytesRead); fflush(stdout);

	filePos_ += bytesRead;
	
	return bytesRead;
}

size_t CVorbisPlayer::WrapperForVorbisRead (
	void* data_ptr,			// A pointer to the data that the vorbis files need
    size_t byteSize,     	// Byte size on this particular system
    size_t sizeToRead,  	// Maximum number of items to be read
    void* pToObject)      	// A pointer to the o we passed into ov_open_callbacks
{
//	printf("Vorbis Player::WrapperForVorbisRead: Entering method.\n "); fflush(stdout);

	// Cast void ptr to a this ptr:
	CVorbisPlayer* mySelf = (CVorbisPlayer*)pToObject;
	
	// Call member function.
	return (size_t)mySelf->VorbisRead( data_ptr, byteSize, sizeToRead );
}


// FIXME/dg: THIS CODE IS STRAW... right now it always returns -1 to indicated that the stream is not seekable
int CVorbisPlayer::VorbisSeek(		
	ogg_int64_t offset,
    int origin ) 
{
	tErrType 		ret = kNoErr;
	tOptionFlags	seekOptions;
	
	switch (origin) {
		// Seek from start of file
		case SEEK_SET: 
			seekOptions = kSeekRsrcOptionSet;
			break;
	    
		// Seek from where we are
		case SEEK_CUR: 
			seekOptions = kSeekRsrcOptionCur;
			break;
	 		
		// Seek from the end of the file
		case SEEK_END:
			seekOptions = kSeekRsrcOptionEnd;
			break;

		default:
			printf("The 'origin' argument must be one of the following constants, defined in STDIO.H!\n");
			break;
	}
	 
	// Seek to the requested place in the bitstream.
	ret = pRsrcMPI_->SeekRsrc( hRsrc_, (U32)offset, seekOptions );
	
	filePos_ = offset;
	
	return -1;
}

int CVorbisPlayer::WrapperForVorbisSeek(
	void* pToObject, 		
	ogg_int64_t offset,				// Number of bytes from origin
    int origin )					// Initial position
{
//	printf("Vorbis Player::WrapperForVorbisSeek: Entering method.\n ");

	// Cast void ptr to a this ptr:
	CVorbisPlayer* mySelf = (CVorbisPlayer*)pToObject;
	
	// Call member function.
	return mySelf->VorbisSeek( offset, origin );
}


long CVorbisPlayer::VorbisTell( void )
{
	return filePos_;
}

long CVorbisPlayer::WrapperForVorbisTell( void* pToObject )
{
//	printf("Vorbis Player::WrapperForVorbisTell: Entering method.\n ");

	// Cast void ptr to a this ptr:
	CVorbisPlayer* mySelf = (CVorbisPlayer*)pToObject;
	
	// Call member function.
	return mySelf->VorbisTell();
}

int CVorbisPlayer::VorbisClose( void )
{
	tErrType ret = kNoErr;
	
	ret = pRsrcMPI_->CloseRsrc( hRsrc_ );
	if ( ret != kNoErr )
		printf("Could not CloseRsrc for OggVorbis file.\n");
	
	return ret;
}

int CVorbisPlayer::WrapperForVorbisClose( void* pToObject )
{
//	printf("Vorbis Player::WrapperForVorbisClose: Entering method.\n ");

	// Cast void ptr to a this ptr:
	CVorbisPlayer* mySelf = (CVorbisPlayer*)pToObject;
	
	// Call member function.
	return mySelf->VorbisClose( );
}


//==============================================================================
//==============================================================================
void CVorbisPlayer::Rewind()
{
	// Point curSample_ back to start and reset total.
}


void CVorbisPlayer::SendDoneMsg( void ) {
	const tEventPriority	kPriorityTBD = 0;
	tAudioMsgDataCompleted	data;
	data.audioID = id_;	// dummy
	data.payload = 101;	// dummy
	data.count = 1;

	CEventMPI	event;
	CAudioEventMessage	msg(data);
	event.PostEvent(msg, kPriorityTBD, pListener_);
}

//==============================================================================
//==============================================================================
U32 CVorbisPlayer::RenderBuffer( S16* pOutBuff, U32 numStereoFrames )
{	
	U32		index;
	int 	dummy;
	U32		framesToProcess = 0;
	U32		totalBytesRead = 0;
	U32		bytesRead = 0;
	U32 	bytesToRead= 0;
	char* 	bufferPtr = kNull;
	S16*	pCurSample;
	U32		framesRead = 0;
	
//	printf("Vorbis Player::RenderBuffer: Entering method.\n ");

	if (hasStereoData_)
		bytesToRead = numStereoFrames * 2 * 2;	// 16bit stereo
	else
		bytesToRead = numStereoFrames * 2;		// 16bit mono
	
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
		if ( bytesRead == 0 )
			break;
		
		// Keep track of where we are...
		bytesToRead -= bytesRead;
		totalBytesRead += bytesRead;
	 	bufferPtr += bytesRead;
	}

//	printf("Vorbis Player::RenderBuffer: read loop got %u total bytes.\n\n ", bytesRead);

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
	
	// Return the number of frames rendered.
	return framesRead;
}

// EOF	
