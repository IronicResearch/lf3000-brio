#ifndef LF_BRIO_VORBISPLAYER_H
#define LF_BRIO_VORBISPLAYER_H

//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		VorbisPlayer.h
//
// Description:
//		Defines the class to manage the playing of Vorbis audio.
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
#include <KernelMPI.h>
#include <AudioTypes.h>
#include <AudioPlayer.h>
#include <AudioTypesPriv.h>
#include "tremor/ivorbiscodec.h"
#include "tremor/ivorbisfile.h"
LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Class:
//		CVorbisPlayer
//
// Description:
//		Class to manage the playing of Vorbis audioo. 
//==============================================================================
class CVorbisPlayer : public CAudioPlayer {
public:
	CVorbisPlayer( tAudioStartAudioInfo* pInfo, tAudioID id  );
	~CVorbisPlayer();
		
	// Reset player to start from beginning of sample
	void	Rewind();
	
	// These are the callback functions that Vorbis uses to get data from
	// the file via the Resource Manager.  We need the stupid wrappers so
	// Vorbis' C API can call us back in a way that allows us to have
	// access to our member variables.  We pass a 'this' ptr into the vorbis
	// lib, and it calls us back with the ptr.  Clever but complicated.
	U32 VorbisRead(
			void* data_ptr,
		    size_t byteSize, 
		    size_t sizeToRead );
		    
	static size_t WrapperForVorbisRead(
		void* data_ptr,					// A pointer to the data that the vorbis files need
	    size_t byteSize,     			// Byte size on this particular system
	    size_t sizeToRead,  			// Maximum number of items to be read
	    void* pToObject);      			// A pointer to the o we passed into 
	                         			// ov_open_callbacks
	int VorbisSeek(		
		ogg_int64_t offset,
	    int origin );

	static int WrapperForVorbisSeek(
		void* pToObject, 		
		ogg_int64_t offset,				// Number of bytes from origin
	    int origin );					// Initial position
		
	long VorbisTell( void );
	
	static long WrapperForVorbisTell( void* pToObject );

	int VorbisClose( void );
	
	static int WrapperForVorbisClose( void* pToObject );

	// Returns milliseconds since start of audio.
	U32 GetAudioTime( void );
	
	// Attempt to fill buffer at pOutBuff with numFrames of data.  
	// Returns number of frames actually rendered; zero when done.
	U32		RenderBuffer( S16 *pOutBuff, U32 numStereoFrames );

	void SendDoneMsg( void );

private:
	OggVorbis_File	vorbisFile_;		// codec context data
	ov_callbacks 	oggCallbacks_;		// set of callbacks to wrap resource mgr functions	
	S16 			*pPcmBuffer_;		// Pointer to the vorbis decode buffer
	CKernelMPI* 	pKernelMPI_;		// For mutex
	tMutex     		render_mutex_;		// Need to protect renderbuffer call 
										// because it's in a different thread.
	U32				filePos_;			// position in the vorbis byte stream
	Boolean			loop_;				// should the file loop when it reaches the end?
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_VORBISPLAYER_H

// EOF	
