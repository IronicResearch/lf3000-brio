#ifndef LF_BRIO_BRIOOPENGLCONFIG_H
#define LF_BRIO_BRIOOPENGLCONFIG_H

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

#endif // LF_BRIO_BRIOOPENGLCONFIG_H

// EOF

