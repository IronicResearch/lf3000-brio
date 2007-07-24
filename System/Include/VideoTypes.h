#ifndef LF_BRIO_VIDEOTYPES_H
#define LF_BRIO_VIDEOTYPES_H
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		VideoTypes.h
//
// Description:
//		Type definitions for the Brio Video subsystem.
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <GroupEnumeration.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Typedefs
//==============================================================================

typedef tHndl 		tVideoHndl;

const tVideoHndl	kInvalidVideoHndl = static_cast<tVideoHndl>(0);

// Video resource types
const tRsrcType 	kVideoRsrcOggVorbis = MakeRsrcType(kUndefinedNumSpaceDomain, kGroupVideo, 1);
const tRsrcType 	kVideoRsrcOggTheora = MakeRsrcType(kUndefinedNumSpaceDomain, kGroupVideo, 2);

// Video info
struct tVideoInfo {
	U32		width;
	U32		height;
	U64		fps;
};

// Video surface
struct tVideoSurf {
	U32		width;
	U32		height;
	U32		pitch;
	U8*		buffer;
};

// Video time frame
struct tVideoTime {
	S64		frame;
	S64		time;
};

LF_END_BRIO_NAMESPACE()

#endif	// LF_BRIO_VIDEOTYPES_H

// EOF
