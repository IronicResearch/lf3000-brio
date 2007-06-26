#ifndef LF_BRIO_FONTTYPES_H
#define LF_BRIO_FONTTYPES_H
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		FontTypes.h
//
// Description:
//		Type definitions for the Brio font subsystem.
//
//==============================================================================

// System includes
#include <GroupEnumeration.h>
#include <SystemTypes.h>
//#include <SystemErrors.h>

LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================	   
// Font resource types
//==============================================================================
const tRsrcType kFontRsrcTTF = MakeRsrcType(kUndefinedNumSpaceDomain, kGroupFont, 1);



//==============================================================================
// Typedefs
//==============================================================================

typedef tHndl 	tFontHndl;

const tFontHndl	kInvalidFontHndl = static_cast<tFontHndl>(0);

// Font metrics
struct tFontMetrics {
	U16		ascent;
	S16		descent;
	U16		height;
	U16		advance;
};

// Font properties
struct tFontProp {
	U16		version;
	U8		size;
};

// Font rendering attributes
struct tFontAttr {
	U16		version;
	U32		color;
	Boolean	direction;
	Boolean	antialias;
};

// Font surface
struct tFontSurf {
	U32		width;
	U32		height;
	U32		pitch;
	U8*		buffer;
};

LF_END_BRIO_NAMESPACE()

#endif	// LF_BRIO_FONTTYPES_H

// EOF
