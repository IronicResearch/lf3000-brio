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
		
	void	OpenFile(char *path);
	void	CloseFile();
	void	RewindFile();

	U32     GetTime_mSec( void );
	U32		Render( S16 *pOut, U32 numStereoFrames );
    inline void SetFileReadBuf(void *p)     { pFileReadBuf_ = (S16 *) p;}

	void SetAudioInfo( tAudioStartAudioInfo *pInfo, tAudioID id  );

private:
	OggVorbis_File	vorbisFile_;		// codec context data
	ov_callbacks 	oggCallbacks_;		// set of callbacks to wrap resource mgr functions	

	U32				filePosition_;			// Position in vorbis bitstream
};

LF_END_BRIO_NAMESPACE()
#endif		// LF_BRIO_VORBISPLAYER_H

// EOF	
