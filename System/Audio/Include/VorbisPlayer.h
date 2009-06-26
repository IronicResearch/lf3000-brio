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
#ifdef USE_VORBIS
#include "vorbis/vorbisfile.h"
#else
#include "tremor/ivorbiscodec.h"
#include "tremor/ivorbisfile.h"
#endif
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
	CVorbisPlayer( tAudioStartAudioInfo* pInfo, tAudioID id	 );
	~CVorbisPlayer();

	static U32 GetNumPlayers(void);
	static U32 GetMaxPlayers(void);
	static Boolean IsVorbisPlayer(CAudioPlayer *pPlayer);

	void	RewindFile();
	U32		GetAudioTime_mSec( void );
	
	// Attempt to fill buffer at pOutBuff with numFrames of data.  Returns
	// number of frames actually rendered; zero when done.
	U32		Render( S16 *pOut, U32 numStereoFrames );

 private:
	OggVorbis_File	vorbisFile_; // codec context data
	ov_callbacks	oggCallbacks_; // set of callbacks to wrap resource mgr
								   // functions
	U32				filePosition_; // Position in vorbis bitstream
	
	int				fd_;			// file descriptor
	void*			dataSource_;	// data source for Vorbis callbacks
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_VORBISPLAYER_H

// EOF	
