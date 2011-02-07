#ifndef LF_BRIO_DISPLAYTYPES_H
#define LF_BRIO_DISPLAYTYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		DisplayTypes.h
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
// Backlight levels
//==============================================================================

#define BACKLIGHT_LEVEL_1	(  -46 )
#define BACKLIGHT_LEVEL_2	(  -11 )
#define BACKLIGHT_LEVEL_3	(   31 )
#define BACKLIGHT_LEVEL_4	(   72 )

#define BACKLIGHT_LEVEL_MIN	BACKLIGHT_LEVEL_1
#define BACKLIGHT_LEVEL_MAX	BACKLIGHT_LEVEL_4

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

enum tPixelFormat {
	kPixelFormatError = 0,
	kPixelFormatRGB4444,
	kPixelFormatRGB565,
	kPixelFormatARGB8888,
	kPixelFormatRGB888,
	kPixelFormatYUV420,
	kPixelFormatYUYV422
};

enum tDisplayZOrder {
	kInvalidZOrder = 0,
	kDisplayOnTop,
	kDisplayOnBottom
};

struct tDisplayScreenStats {
	U16 height;
	U16 width;
	U16 tPixelFormat;
	U16 pitch;
	const char *description; // i.e. "Tv", "LCD" based on platform
};

// tDisplayHandle is what is returned from a DisplayMgr->CreateContext
typedef	void*	tDisplayHandle;
const tDisplayHandle kInvalidDisplayHandle = static_cast<tDisplayHandle>(0);

// tDisplayScreen is a bitmask of the available screens.
typedef	U32	tDisplayScreen; 
const tDisplayScreen kDisplayScreenAllScreens	= static_cast<tDisplayScreen>(-1);
const tDisplayScreen kDisplayScreenNoScreens	= static_cast<tDisplayScreen>(0);

// tDisplayMem type argument for GetDisplayMem()
enum tDisplayMem {
	kDisplayMemTotal,
	kDisplayMemFree,
	kDisplayMemUsed,
	kDisplayMemAligned,
	kDisplayMemFragmented,
};

/// tDisplayViewport enum types for SetViewport()/GetViewport()
enum tDisplayViewport {
	kViewportLegacy,						///< Legacy viewport (Emerald 320x240)
	kViewportFullscreen,					///< Fullscreen viewport (Madrid 480x272)
	kViewportScaledAspect,					///< Uniformly Scaled Aspect Ratio (360x272)
};

/// tDisplayOrientation enum types for SetOrientation()/GetOrientation()
enum tDisplayOrientation {
	kOrientationLandscape,					///< Landscape orientation (Emerald)
	kOrientationPortrait,					///< Portrait orientation (Madrid rotated 90)
	kOrientationLandscapeUpsideDown,		///< Landscape upside down (rotated 180)
	kOrientationPortraitUpsideDown,			///< Portrait upside down (rotated 270 | -90)
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_DISPLAYTYPES_H

// EOF

