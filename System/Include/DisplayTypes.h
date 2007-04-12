#ifndef LF_BRIO_DISPLAYTYPES_H
#define LF_BRIO_DISPLAYTYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		DisplayMgrTypes.h
//
// Description:
//		Defines enums and types for DisplayMgr. 
//
//==============================================================================
#include <SystemTypes.h>
#include <StringTypes.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================	   
// Display events
//==============================================================================
#define DISPLAY_EVENTS					\
	(kDisplayEvent1)					\
	(kDisplayEvent2)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupDisplay), DISPLAY_EVENTS)

const tEventType kAllDisplayEvents = AllEvents(kGroupDisplay);


//==============================================================================	   
// Display errors
//==============================================================================
#define DISPLAY_ERRORS				\
	(kDisplayNullPtrErr)			\
	(kDisplayInvalidScreenErr)		\
	(kDisplayDisplayNotInListErr)	\
	(kDisplayNoScreensSelectedErr)	\
	(kDisplayUnsupportedZOrder)		\
	(kDisplayScreenInitErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupDisplay), DISPLAY_ERRORS)


//==============================================================================	   
// Display types
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
typedef	S16	tPixelFormat;

enum {
	tDisplayOnTop,
	tDisplayOnBottom,
};
typedef	U16	tDisplayZOrder;

struct tDisplayScreenStats {
	U16 height;
	U16 width;
	U16 tPixelFormat;
	U16 pitch;
	CString *description; // i.e. "Tv", "LCD" based on platform
};

// tDisplayHandle is what is returned from a DisplayMgr->CreateContext
typedef	void*	tDisplayHandle;

// tDisplayScreen is a bitmask of the available screens.
typedef	U32	tDisplayScreen; 
const tDisplayScreen kDisplayScreenAllScreens	= static_cast<tDisplayScreen>(-1);
const tDisplayScreen kDisplayScreenNoScreens	= static_cast<tDisplayScreen>(0);


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_DISPLAYTYPES_H

// EOF

