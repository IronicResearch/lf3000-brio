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
// Typedefs
//==============================================================================

typedef tHndl 	tFontHndl;

const tFontHndl	kInvalidFontHndl = static_cast<tFontHndl>(0);

// Font metrics
struct tFontMetrics {
	S16		ascent;			// max vertical above baseline 
	S16		descent;		// max vertical below baseline (negative)
	S16		height;			// max per-line vertical advance
	S16		advance;		// max per-glyph horizontal advance
};

// Font properties
struct tFontProp {
	U16		version;
	U8		size;
	// version 2 properties
	U32		encoding;		// kSystemCharEncoding* enum
	Boolean useEncoding;	// redundant
	// version 3 properties
	U32		loadFlags;		// FT_LOAD* flags
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
	S32		spaceExtra;		// extra per-glyph horizontal spacing
	S32		leading;		// extra per-line vertical spacing
	Boolean	useKerning;		// glyph-paired horizontal adjustments
	Boolean useUnderlining;
};

// Font surface
struct tFontSurf {
	S32		width;
	S32		height;
	S32		pitch;
	U8*		buffer;
	tPixelFormat format;	// kPixelFormat* enum
};

LF_END_BRIO_NAMESPACE()

#endif	// LF_BRIO_FONTTYPES_H

// EOF
