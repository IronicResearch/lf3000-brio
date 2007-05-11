#ifndef LF_BRIO_BRIOOPENGLCONFIG_H
#define LF_BRIO_BRIOOPENGLCONFIG_H
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

#ifdef EMULATION	
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#include <GLES/egl.h>
#include <GLES/gl.h>

#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
// BrioOpenGLConfig
//==============================================================================
class BrioOpenGLConfig
{
public:
	BrioOpenGLConfig();
	~BrioOpenGLConfig();

	//TODO/tp: Figure out minimal subset of variables to expose
	
#ifdef EMULATION	
	// X11 variables
	Window				x11Window;
	Display*			x11Display;
	long				x11Screen;
	XVisualInfo*		x11Visual;
	Colormap			x11Colormap;
#endif

	// EGL variables
	EGLDisplay			eglDisplay;
	EGLConfig			eglConfig;
	EGLSurface			eglSurface;
	EGLContext			eglContext;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_BRIOOPENGLCONFIG_H

// EOF

