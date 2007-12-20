#ifndef LF_BRIO_VORBISPLAYER_H
#define LF_BRIO_VORBISPLAYER_H

//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// VorbisPlayer.h
//
// Defines the class to manage the playing of Vorbis audio.
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
		
	void	RewindFile();
	U32 GetAudioTime_mSec( void );
	
	// Attempt to fill buffer at pOutBuff with numFrames of data.  
	// Returns number of frames actually rendered; zero when done.
	U32		Render( S16 *pOut, U32 numStereoFrames );

	void SendDoneMsg( void );

private:
	OggVorbis_File	vorbisFile_;		// codec context data
	ov_callbacks 	oggCallbacks_;		// set of callbacks to wrap resource mgr functions	
	S16 			*pReadBuf_;		// Vorbis decode buffer

//#define USE_VORBIS_PLAYER_MUTEX
#ifdef USE_VORBIS_PLAYER_MUTEX
	CKernelMPI* 	pKernelMPI_;		
	tMutex     		renderMutex_;		// For multi-threaded protection of renderbuffer call 
#endif // USE_VORBIS_PLAYER_MUTEX
										
	U32				filePosition_;			// position in the vorbis bitstream

    S32             loopCount_;
    S32             loopCounter_;

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
