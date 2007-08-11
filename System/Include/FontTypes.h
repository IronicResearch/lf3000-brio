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
#include <DisplayTypes.h>

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
	S16		ascent;
	S16		descent;
	S16		height;
	S16		advance;
};

// Font properties
struct tFontProp {
	U16		version;
	U8		size;
	// version 2 properties
	U32		encoding;
	Boolean useEncoding;
};

// Font rendering attributes
struct tFontAttr {
	U16		version;
	U32		color;
	Boolean	direction;
	Boolean	antialias;
	// version 2 attributes
	U32		horizJust;
	U32		vertJust;
	U32		spaceExtra;
	U32		leading;
	Boolean	useKerning;
	Boolean useUnderlining;
};

// Font surface
struct tFontSurf {
	S32		width;
	S32		height;
	S32		pitch;
	U8*		buffer;
	tPixelFormat format;
};

LF_END_BRIO_NAMESPACE()

#endif	// LF_BRIO_FONTTYPES_H

// EOF
