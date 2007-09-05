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

// Default heap allocation for OEM OpenGL library
const U32	kHeap1DMeg = 8;
const U32	kHeap2DMeg = 8;		

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
	Boolean				bFSAA;			// fullscreen anti-aliasing feature (LF1000 only)
};

//==============================================================================
// BrioOpenGLConfig
//==============================================================================
class BrioOpenGLConfig
{
public:
	BrioOpenGLConfig(U32 size1D = kHeap1DMeg, U32 size2D = kHeap2DMeg);
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

