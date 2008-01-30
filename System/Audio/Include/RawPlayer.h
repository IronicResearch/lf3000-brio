#ifndef LF_BRIO_RAWPLAYER_H
#define LF_BRIO_RAWPLAYER_H

//==============================================================================
// Copyright (c) 2002-2008 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// RawPlayer.h
//
// Defines the class to manage the playing of audio files
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
#define kRawPlayer_FileType_Unknown (0)

//==============================================================================
// Class:
//		CRawPlayer
//
// Class to manage the playing of raw audio. 
//==============================================================================
class CRawPlayer : public CAudioPlayer {
public:
	CRawPlayer( tAudioStartAudioInfo* pInfo, tAudioID id  );
	~CRawPlayer();
		
	void	RewindFile();
	U32     GetAudioTime_mSec( void );
	U32     ReadBytesFromFile( void *d, U32 bytesToRead);

// Returns # frames actually rendered without zero padding
	U32		Render( S16 *pOut, U32 numStereoFrames );

private:
    U32		        totalBytesRead_;
//	U32				totalFrames_;			
//	U32				framesRemaining_;

    U32             fileType_;  // WAV, AIFF, Brio "RAW"
    SNDFILE	        *inFile_;
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_RAWPLAYER_H

// EOF	
