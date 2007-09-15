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


// Enable this to generate timing/profiling output for Vorbis decode
#define PROFILE_DECODE_LOOP		0

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
	
#if	PROFILE_DECODE_LOOP
	S32				totalUsecs_;
	S32				totalBytes_;
	U32				minUsecs_;
	U32				maxUsecs_;
#endif
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_VORBISPLAYER_H

// EOF	
