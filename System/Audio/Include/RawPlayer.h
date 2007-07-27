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
//		Defines the class to manage the playing of raw audio.
//
//==============================================================================

// System includes
#include <CoreTypes.h>
#include <SystemTypes.h>
//#include <RsrcTypes.h>
#include <AudioTypes.h>
#include <AudioPlayer.h>
#include <AudioTypesPriv.h>
LF_BEGIN_BRIO_NAMESPACE()

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
	
	// Returns milliseconds since start of audio.
	U32 GetAudioTime( void );

	// Attempt to fill buffer at pOutBuff with numFrames of data.  
	// Returns number of frames actually rendered; zero when done.
	U32		RenderBuffer( S16 *pOutBuff, U32 numStereoFrames );

	void SendDoneMsg( void );

private:
	CKernelMPI* 	pKernelMPI_;		// For mutex calls
	tMutex			render_mutex_;		// Need to protect the renderbuffer call
										// because it's in a different thread.

	U32		numFrames_;			// frames of sample data
	S16*	pCurFrame_;			// ptr to current frame
	U32		framesLeft_;		// num frames left
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_RAWPLAYER_H

// EOF	
