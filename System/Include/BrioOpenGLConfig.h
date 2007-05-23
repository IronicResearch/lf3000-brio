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
#include <DisplayMPI.h>
LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// OpenGL Hardware/Emulation context
//==============================================================================
struct tOpenGLContext
{
	void*				pOEM;			// ptr to OEM-specific HW struct
	NativeDisplayType	eglDisplay;		// X display or HW context
	NativeWindowType	eglWindow;		// X window or HW context
	U16					width;
	U16					height;
};

//==============================================================================
// BrioOpenGLConfig
//==============================================================================
class BrioOpenGLConfig
{
public:
	BrioOpenGLConfig();
	~BrioOpenGLConfig();

	// EGL variables
	EGLDisplay			eglDisplay;
	EGLConfig			eglConfig;
	EGLSurface			eglSurface;
	EGLContext			eglContext;
	
private:
	CDisplayMPI			disp_;	
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_BRIOOPENGLCONFIG_H

// EOF

