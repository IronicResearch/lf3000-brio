#ifndef LF_BRIO_RAWPLAYER_H
#define LF_BRIO_RAWPLAYER_H

//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		RawPlayer.h
//
// Description:
//		Defines the class to manage the playing of audio files
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>

#include <AudioTypes.h>
#include <AudioPlayer.h>
#include <AudioTypesPriv.h>

#include "sndfileutil.h"

LF_BEGIN_BRIO_NAMESPACE()

#define kRawPlayer_FileType_Brio    1
#define kRawPlayer_FileType_AIFF    2
#define kRawPlayer_FileType_WAV     3
#define kRawPlayer_FileType_Unknown     (0)

//==============================================================================
// Class:
//		CRawPlayer
//
// Description:
//		Class to manage the playing of raw audioo. 
//==============================================================================
class CRawPlayer : public CAudioPlayer {
public:
	CRawPlayer( tAudioStartAudioInfo* pAudioInfo, tAudioID id  );
	~CRawPlayer();
		
	// Reset player to start from beginning of sample
	void	Rewind();
	
	U32 GetAudioTime_mSec( void );  // FIXX: NOT DONE: Time since start of audio

	// Attempt to fill buffer at pOutBuff with numFrames of data.  
	// Returns number of frames actually rendered; zero when done.
	U32		RenderBuffer( S16 *pOutBuff, U32 numStereoFrames );

	void SendDoneMsg( void );

private:
	CKernelMPI* 	pKernelMPI_;		// For mutex calls
	tMutex			render_mutex_;		// Need to protect the renderbuffer call
										// because it's in a different thread.

	S16 			*pPcmBuffer_;		// Audio sample buffer
	U32				totalFrames_;			
	U32				framesRemaining_;
    S32             loopCount_;
    S32             loopCounter_;

// Audio file info (WAV, AIFF, Brio "RAW"
    U32             fileType_;
    SNDFILE	        *inFile_;
    SF_INFO	        inFileInfo_;  // FIXX: make local variable when Rewind() fixed in sndfileutil.c
    char           inFilePath[1000]; 
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_RAWPLAYER_H

// EOF	
