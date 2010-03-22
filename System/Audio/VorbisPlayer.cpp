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
#include <fcntl.h>
#include <sys/mman.h>
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
#include <Utility.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Defines
//==============================================================================
#define VORBIS_LITTLE_ENDIAN	0
#define VORBIS_BIG_ENDIAN		1
#define VORBIS_16BIT_WORD		2
#define VORBIS_SIGNED_DATA		1

#undef USE_MMAP_FILE_IO

typedef struct {
	void*	pmap;
	int		fd;
	size_t	length;
	size_t	cursor;
} tFileMapping;

//==============================================================================
// Global variables
//==============================================================================
static U32 numVorbisPlayers = 0;
static U32 maxNumVorbisPlayers = kAudioMaxVorbisStreams;

#ifdef USE_MMAP_FILE_IO
//==============================================================================
// File IO callback implementation
//==============================================================================
size_t mm_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	tFileMapping*	pfm = (tFileMapping*)datasource;
	size_t			length = size * nmemb;
	void*			psrc;
	
	if (length + pfm->cursor > pfm->length)
		length = pfm->length - pfm->cursor;
	psrc = (void*)((unsigned)pfm->pmap + pfm->cursor);
	memcpy(ptr, psrc, length);
	pfm->cursor += length;
	return length;
}

//==============================================================================
// File IO callback implementation
//==============================================================================
int mm_seek_func(void *datasource, ogg_int64_t offset, int whence)
{
	tFileMapping*	pfm = (tFileMapping*)datasource;

	switch (whence) {
		case SEEK_SET: pfm->cursor = offset; break;
		case SEEK_CUR: pfm->cursor += offset; break;
		case SEEK_END: pfm->cursor = pfm->length - offset; break;
	}
	if (pfm->cursor > pfm->length)
		pfm->cursor = pfm->length;
	else if (pfm->cursor < 0)
		pfm->cursor = 0;
	return 0;
}

//==============================================================================
// File IO callback implementation
//==============================================================================
long mm_tell_func(void *datasource)
{
	tFileMapping*	pfm = (tFileMapping*)datasource;
	return pfm->cursor;
}

//==============================================================================
// File IO callback implementation
//==============================================================================
int mm_close_func(void *datasource)
{
	tFileMapping*	pfm = (tFileMapping*)datasource;

	if (pfm->fd > -1) {
		munmap(pfm->pmap, pfm->length);
		close(pfm->fd);
		pfm->pmap = NULL;
		pfm->fd = -1;
	}
	return 0;
}
#endif // USE_MMAP_FILE_IO

//==============================================================================
// CVorbisPlayer implementation
//==============================================================================

CVorbisPlayer::CVorbisPlayer( tAudioStartAudioInfo* pInfo, tAudioID id	) :
	CAudioPlayer( pInfo, id	 )
{
	tErrType			ret = kNoErr;
	vorbis_info*		pVorbisInfo;
	
	pDebugMPI_->SetDebugLevel( kAudioDebugLevel );

	TimeStampOn(1);

#ifdef USE_MMAP_FILE_IO	
	// Open file
	fd_ = open( pInfo->path->c_str(), O_RDONLY );
	pDebugMPI_->Assert( fd_ > 0, 
						"VorbisPlayer::ctor : Unable to open file: '%s'\n",
						pInfo->path->c_str() );
	struct stat sbuf;
	fstat(fd_, &sbuf);
	
	// Map file
	tFileMapping* fmap = new tFileMapping;
	dataSource_ = fmap;
	fmap->fd = fd_;
	fmap->length = sbuf.st_size; 
	fmap->cursor = 0;
	fmap->pmap = mmap(0, fmap->length, PROT_READ, MAP_SHARED, fd_, 0);
	pDebugMPI_->Assert( fmap->pmap != NULL, 
						"VorbisPlayer::ctor : Unable to map file: '%s'\n",
						pInfo->path->c_str() );
	
	// Setup file callbacks
	oggCallbacks_.read_func = mm_read_func;
	oggCallbacks_.seek_func = mm_seek_func;
	oggCallbacks_.tell_func = mm_tell_func;
	oggCallbacks_.close_func = mm_close_func;
	
	// Open Ogg-compressed file
	int ov_ret = ov_open_callbacks( dataSource_, &vorbisFile_, NULL, 0, oggCallbacks_ );
	pDebugMPI_->AssertNoErr( ov_ret, 
							 "VorbisPlayer::ctor: Is Ogg file? '%s', ov_ret=%d\n",
							 pInfo->path->c_str(), ov_ret);
#else	
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
#endif
	
	TimeStampOff(1);

	pVorbisInfo		   = ov_info( &vorbisFile_, -1 );
	channels_		   = pVorbisInfo->channels;
	samplingFrequency_ = pVorbisInfo->rate;
	
	// Time lapse delta needs to be multiple of playback quantum (16 msec)
	if (optionsFlags_ & kAudioOptionsTimeEvent) {
		bIsTimeEvent_ = true;
		timeDelta_	= payload_;
		timeDelta_ 	/= kAudioFramesPerMS;
		timeDelta_	*= kAudioFramesPerMS;
	}

	numVorbisPlayers++;

}

//==============================================================================
// ~CVorbisPlayer
//==============================================================================
CVorbisPlayer::~CVorbisPlayer()
{
	tErrType result;

	if (pReadBuf_)
		delete[] pReadBuf_;
	
	// Close vorbis file
	S32 ret = ov_clear( &vorbisFile_ );
	pDebugMPI_->Assert(ret >= 0,
		"%s.%d: Could not close OggVorbis file.\n",
		__FUNCTION__, __LINE__);

#ifdef USE_MMAP_FILE_IO
	// Release file map struct after ov_clear() calls close() callback
	if (dataSource_)
		delete (tFileMapping*)dataSource_;
#endif
	
	// Free MPIs
	if (pDebugMPI_)
		delete (pDebugMPI_);

	numVorbisPlayers--;
}

U32 CVorbisPlayer::GetNumPlayers(void)
{
	return numVorbisPlayers;
}

U32 CVorbisPlayer::GetMaxPlayers(void)
{
	// Contingency for reverting max Vorbis channels for Didj titles
	if (FileSize("/flags/vorbis"))
		return kAudioMinVorbisStreams;
	return maxNumVorbisPlayers;
}

Boolean CVorbisPlayer::IsVorbisPlayer(CAudioPlayer *pPlayer)
{
	CVorbisPlayer *p;
	if (p = dynamic_cast<CVorbisPlayer *>(pPlayer))
		return true;
	else
		return false;
}

// ==============================================================================
// RewindFile: Seek to beginning of file
// ==============================================================================
void CVorbisPlayer::RewindFile()
{
	ov_time_seek( &vorbisFile_, 0 );
	bIsDone_ = false;
}

// ==============================================================================
// GetAudioTime_mSec:	Return file position in time (milliSeconds)
// ==============================================================================
U32 CVorbisPlayer::GetAudioTime_mSec( void )
{
#ifdef USE_VORBIS
	ogg_int64_t vorbisTime = (ogg_int64_t)(ov_time_tell( &vorbisFile_ ) * 1000);
	U32 timeInMS		   = (U32) vorbisTime;
#else
	ogg_int64_t vorbisTime = ov_time_tell( &vorbisFile_ );
	U32 timeInMS		   = (U32) vorbisTime;
#endif

	// GK FIXXX: should probably protect this with mutex
	return (timeInMS);
}

// ==============================================================================
// Seek
// ==============================================================================
Boolean CVorbisPlayer::SeekAudioTime(U32 timeMilliSeconds)
{
#ifdef USE_VORBIS
	int error = ov_time_seek(&vorbisFile_, timeMilliSeconds / 1000.0);
#else
	int error = ov_time_seek(&vorbisFile_, timeMilliSeconds);
#endif
	//bSeeked = true;
	return error == 0;
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
	
	bytesToRead = numStereoFrames * sizeof(S16) * channels_;

	if(bIsDone_)
		return 0;
	
	// Read multiple times because vorbis doesn't always return requested # of
	// bytes Notes that BytesToRead is decremented
	bufPtr = (char *) pReadBuf_;
	long bytesReadThisRender   = 0;
	long bytesToReadThisRender = bytesToRead;
	while ( bytesToRead > 0 ) 
	{
		TimeStampOn(2);

		bytesRead = ov_read( &vorbisFile_, bufPtr, bytesToRead,
					#ifdef USE_VORBIS
							VORBIS_LITTLE_ENDIAN,
							VORBIS_16BIT_WORD,
							VORBIS_SIGNED_DATA,
					#endif
							 &dummy );

		TimeStampOff(2);

		if ( bytesRead == 0)
			break;

		bytesReadThisRender += bytesRead;		
		bytesToRead	   -= bytesRead;
		totalBytesRead += bytesRead;
		bufPtr		   += bytesRead;
	}
	
	// Convert bytes back into sample frames
	framesToProcess = totalBytesRead / sizeof(S16) / channels_;
			
	// Save total number of frames decoded
	framesRead = framesToProcess;

	// Track elapsed playback intervals for time events
	if (bIsTimeEvent_) {
		timeLapsed_		+= framesRead / kAudioFramesPerMS;
		bIsTimeElapsed_	= (timeLapsed_ % timeDelta_ == 0) ? true : false;
	}
	
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
	
	bIsDone_ = (numStereoFrames > framesRead);

	return (framesRead);  
}

LF_END_BRIO_NAMESPACE()

