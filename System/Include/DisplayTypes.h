#ifndef LF_BRIO_DISPLAYTYPES_H
#define LF_BRIO_DISPLAYTYPES_H

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

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <GLES/egl.h>
#include <GLES/gl.h>

#include <SystemTypes.h>
#include <StringTypes.h>


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


namespace LeapFrog
{
	namespace Brio
	{
		//==============================================================================
		// BrioOpenGLConfig
		//==============================================================================
		class BrioOpenGLConfig
		{
		public:
			BrioOpenGLConfig();
			~BrioOpenGLConfig();

			//TODO/tp: Figure out minimal subset of variables to expose
			
			// X11 variables
			Window				x11Window;
			Display*			x11Display;
			long				x11Screen;
			XVisualInfo*		x11Visual;
			Colormap			x11Colormap;
		
			// EGL variables
			EGLDisplay			eglDisplay;
			EGLConfig			eglConfig;
			EGLSurface			eglSurface;
			EGLContext			eglContext;
		};
	}	// end of Brio namespace
}		// end of LeapFrog namespace

#endif // LF_BRIO_DISPLAYTYPES_H

// EOF

