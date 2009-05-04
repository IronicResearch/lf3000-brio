#ifndef LF_BRIO_MEMPLAYER_H
#define LF_BRIO_MEMPLAYER_H

//==============================================================================
// Copyright (c) 2002-2009 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// MemPlayer.h
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
//		CMemPlayer
//
// Class to manage the playing of raw audio from memory buffer. 
//==============================================================================
class CMemPlayer : public CAudioPlayer {
 public:
	CMemPlayer( tAudioStartAudioInfo* pInfo, tAudioID id  );
	~CMemPlayer();
		
	static U32 GetNumPlayers(void);
	static U32 GetMaxPlayers(void);
	static Boolean IsMemPlayer(CAudioPlayer *pPlayer);

	void	RewindFile();
	U32 GetAudioTime_mSec( void );
	U32 ReadBytesFromFile( void *d, U32 bytesToRead);

	// Returns # frames actually rendered without zero padding
	U32 Render( S16 *pOut, U32 numStereoFrames );

 private:
	U32				totalBytesRead_;
	void*			pReadData_;
	tGetStereoAudioStreamFcn	pRenderCallback_;
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_MEMPLAYER_H

// EOF	
