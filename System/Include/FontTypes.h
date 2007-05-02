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
#include <SystemTypes.h>
//#include <SystemErrors.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Typedefs
//==============================================================================

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
};

// Font surface
struct tFontSurf {
	U32		width;
	U32		height;
	U32		pitch;
	U32		*buffer;
};

LF_END_BRIO_NAMESPACE()

#endif	// LF_BRIO_FONTTYPES_H

// EOF
