#ifndef DISPLAYMGRTYPES_H
#define DISPLAYMGRTYPES_H

//==============================================================================
// $Source: $
//
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		DisplayMgrTypes.h
//
// Description:
//		Defines enums and types for DisplayMgr. 
//
//==============================================================================

struct tRect {
	S16 left;
	S16 right;
	S16 top;
	S16 bottom;
};

enum {
	tPixelFormatError = -1,
	tPixelFormatRGB4444 = 0,
	tPixelFormatRGB565,
	tPixelFormatARGB8888,
};
#define tPixelFormat S16

enum {
	tDisplayOnTop,
	tDisplayOnBottom,
};
#define tDisplayZOrder U16

struct tDisplayScreenStats {
	U16 height;
	U16 width;
	U16 tPixelFormat;
	U16 pitch;
	CString *description; // i.e. "Tv", "LCD" based on platform
};

// tDisplayHandle is what is returned from a DisplayMgr->CreateContext
#define tDisplayHandle void* 

// tDisplayScreen is a bitmask of the available screens.
#define tDisplayScreen U32 
#define kDisplayScreenAllScreens ((tDisplayScreen)-1)
#define kDisplayScreenNoScreens ((tDisplayScreen)0)

#define kFirstSystemErr 0x00001000
enum 
{
	kDisplayNullPtrErr = kFirstSystemErr,
	kDisplayInvalidScreenErr,
	kDisplayDisplayNotInListErr,
	kDisplayNoScreensSelectedErr,
	kDisplayUnsupportedZOrder,
	kDisplayScreenInitErr,
};


#endif

// EOF

