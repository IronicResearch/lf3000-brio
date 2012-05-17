#ifndef LF_BRIO_BRIOOPENGL2CONFIG_H
#define LF_BRIO_BRIOOPENGL2CONFIG_H
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

#ifdef KHRONOS
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#endif

#include <SystemTypes.h>
#include <DisplayMPI.h>
LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// BrioOpenGLConfig
//==============================================================================
class BrioOpenGL2Config
{
public:
	BrioOpenGL2Config();
	~BrioOpenGL2Config();

	// EGL variables
	EGLDisplay			eglDisplay;
	EGLConfig			eglConfig;
	EGLSurface			eglSurface;
	EGLContext			eglContext;
	
	// Display MPI handle
	tDisplayHandle		hndlDisplay;

private:
	CDisplayMPI			disp_;	
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_BRIOOPENGLCONFIG_H

// EOF

