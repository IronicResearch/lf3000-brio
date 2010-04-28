#ifndef LF_BRIO_AVIPLAYER_H
#define LF_BRIO_AVIPLAYER_H

//==============================================================================
// Copyright (c) 2002-2010 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// AVIPlayer.h
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

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Class:
//		CAVIPlayer
//
// AudioPlayer-derived class for playing AVI audio. 
//==============================================================================
class CAVIPlayer : public CAudioPlayer {
 public:
	CAVIPlayer( tAudioStartAudioInfo* pInfo, tAudioID id  );
	~CAVIPlayer();
		
	static U32 		GetNumPlayers(void);
	static U32 		GetMaxPlayers(void);
	static Boolean 	IsAVIPlayer(CAudioPlayer *pPlayer);

	void			RewindFile();
	U32 			GetAudioTime_mSec( void );
	Boolean			SeekAudioTime(U32 timeMilliSeconds);
	U32 			ReadBytesFromFile( void *d, U32 bytesToRead);

	// Returns # frames actually rendered without zero padding
	U32 			Render( S16 *pOut, U32 numStereoFrames );

private:
    U32				totalBytesRead_;
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_AVIPLAYER_H

// EOF	
