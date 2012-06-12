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

#ifdef KHRONOS
#include <EGL/egl.h>
#include <GLES/gl.h>
#else
#include <GLES/egl.h>
#include <GLES/gl.h>
#endif

#include <SystemTypes.h>
#include <DisplayMPI.h>
LF_BEGIN_BRIO_NAMESPACE()

// Default heap allocation for OEM OpenGL library
// Default OpenGL framebuffer region increased to 16Megs on 64Meg Didj/Emerald units
const U32	kHeap1DMeg = 2;	// 1D heap size in Megs for vertex buffers
const U32	kHeap2DMeg = 14;// 2D heap size in Megs for color/depth buffers, textures 		

//==============================================================================
// OpenGL Hardware/Emulation context
//==============================================================================
struct tOpenGLContext
{
	void*				pOEM;			// ptr to OEM-specific HW struct
	NativeDisplayType	eglDisplay;		// X display or HW context -- FIXME: NativeDisplayType != EGLDisplay type
	NativeWindowType	eglWindow;		// X window or HW context
	U16					width;
	U16					height;
	Boolean				bFSAA;			// fullscreen anti-aliasing feature (LF1000 only)
	tDisplayHandle		hndlDisplay;	// display handle for OpenGL
};

enum tBrioOpenGLVersion
{
	kBrioOpenGL11 = 1,
	kBrioOpenGL20 = 2
};

//==============================================================================
// BrioOpenGLConfig
//==============================================================================
class BrioOpenGLConfig
{
	/// \class BrioOpenGLConfig
	///
	/// Class for binding an OpenGL ES context with Brio DisplayMPI context.
	/// Class members export EGL context variables for calling EGL functions
	/// like eglSwapBuffers(), plus DisplayMPI display context handle for
	/// calling DisplayMPI functions like SetWindowPosition() and SetAlpha().
	///
	/// LF1000: The OpenGL ES rendering context is RGB565 format only.
	/// Separate 1D and 2D heaps are allocated from framebuffer memory
	/// for supporting vertex buffers and textures.
	///
	/// LF2000: The OpenGL ES rendering context is ARGB8888 format.
	/// Memory for vertex buffers and textures are allocated on demand.
public:
	/// LF1000: Constructor for specifying 1D and 2D heap sizes.
	BrioOpenGLConfig(U32 size1D = kHeap1DMeg, U32 size2D = kHeap2DMeg);
	/// LF2000: Constructor for selecting OpenGL ES 1.1 or 2.0 context.
	BrioOpenGLConfig(enum tBrioOpenGLVersion brioOpenGLVersion);
	~BrioOpenGLConfig();

	// EGL variables
	EGLDisplay			eglDisplay;		///< EGL display returned by eglGetDisplay()
	EGLConfig			eglConfig;		///< EGL config returned by eglChooseConfig()
	EGLSurface			eglSurface;		///< EGL surface returned by eglCreateWindowSurface()
	EGLContext			eglContext;		///< EGL context returned by eglCreateContext()
	
	// Display MPI handle
	tDisplayHandle		hndlDisplay;	///< DisplayMPI context bound to EGL context

private:
	CDisplayMPI			disp_;	
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_BRIOOPENGLCONFIG_H

// EOF

