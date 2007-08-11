//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		BrioOpenGLConfig.cpp
//
// Description:
//		Configure emulation for the environment
//
//==============================================================================

#include <GLES/egl.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <BrioOpenGLConfig.h>
#include <DebugMPI.h>
#include <EmulationConfig.h>

#ifndef  EMULATION
#include "GLES/libogl.h"
#endif


#ifdef  EMULATION
//============================================================================
// Stub function to allow emulation builds to link
//============================================================================
inline GLfixed B2FIX(GLubyte x) { return (x * 0x10000 / 0xFF); }
void glColor4ub( GLubyte red,
                 GLubyte green,
                 GLubyte blue,
                 GLubyte alpha )
{
	glColor4x( B2FIX(red), B2FIX(green), B2FIX(blue), B2FIX(alpha));
}
#endif


LF_BEGIN_BRIO_NAMESPACE()

#if 0	// NOTBUG/dm: enable for development
#define	PRINTF	printf
#else
#define PRINTF(...)
#endif

//==============================================================================
namespace
{
	//--------------------------------------------------------------------------
	tOpenGLContext		ctx;
	CDisplayMPI*		dispmgr;
	
	//--------------------------------------------------------------------------
	void AbortIfEGLError(char* pszLocation)
	{
		/*
			eglGetError returns the last error that has happened using egl,
			not the status of the last called function. The user has to
			check after every single egl call or at least once every frame.
		*/
		EGLint iErr = eglGetError();
		if (iErr != EGL_SUCCESS)
		{
			CDebugMPI	dbg(kGroupDisplay);
			dbg.Assert(false, "%s failed (%d).\n", pszLocation, iErr);
		}
	}

#ifndef EMULATION
	// TODO/dm: Pass back essential callback info to MagicEyes GLES lib
	// TODO/dm: All these callbacks should be moved into libDisplay.so module
	//			but MagicEyes libogl.a links against them *and* EGL funcs.
	//--------------------------------------------------------------------------
	___OAL_MEMORY_INFORMATION__ 	meminfo;
	//--------------------------------------------------------------------------
	extern "C" int  GLESOAL_Initalize(___OAL_MEMORY_INFORMATION__* pMemoryInfo ) 
	{
		*pMemoryInfo = meminfo;
	    PRINTF("GLESOAL_Initalize: %08X, %08X, %08X,%08X, %08X, %08X, %08X\n", \
		    pMemoryInfo->VirtualAddressOf3DCore, \
		    pMemoryInfo->Memory1D_VirtualAddress, \
		    pMemoryInfo->Memory1D_PhysicalAddress, \
		    pMemoryInfo->Memory1D_SizeInMbyte, \
		    pMemoryInfo->Memory2D_VirtualAddress, \
		    pMemoryInfo->Memory2D_PhysicalAddress, \
		    pMemoryInfo->Memory2D_SizeInMbyte);
		
		// 3D layer must not be enabled until after this callback returns
		return 1; 
	}

	//--------------------------------------------------------------------------
	extern "C" void GLESOAL_Finalize( void ) 
	{
		// 3D layer must be disabled before this callback returns
		PRINTF("GLESOAL_Finalize\n");
	}

	//--------------------------------------------------------------------------
	extern "C" void GLESOAL_SwapBufferCallback( void ) 
	{ 
		PRINTF("GLESOAL_SwapBufferCallback\n");

		// 3D layer needs to sync to OGL calls
		dispmgr->UpdateOpenGL();
	}

	//--------------------------------------------------------------------------
	extern "C" void GLESOAL_SetWindow( void* /*pNativeWindow*/ ) { }
	
	//--------------------------------------------------------------------------
	extern "C" void GLESOAL_GetWindowSize( int* pWidth, int* pHeight )
	{
	    *pWidth  = ctx.width;
	    *pHeight = ctx.height;
	}
#endif // !EMULATION
}


//==============================================================================
//----------------------------------------------------------------------
BrioOpenGLConfig::BrioOpenGLConfig(U32 size1D, U32 size2D)
	: 
	eglDisplay(0), eglConfig(0), eglSurface(0), eglContext(0)
{
	CDebugMPI				dbg(kGroupDisplay);
	dispmgr = &disp_;

#ifndef EMULATION
	// Init OpenGL hardware callback struct
	meminfo.Memory1D_SizeInMbyte = size1D;
	meminfo.Memory2D_SizeInMbyte = size2D;
	ctx.pOEM = &meminfo;
#endif
	/*
		Step 0 - Create a NativeWindowType that we can use it for OpenGL ES output
	*/
	disp_.InitOpenGL(&ctx);

	/*
		Step 1 - Get the default display.
		EGL uses the concept of a "display" which in most environments
		corresponds to a single physical screen. Since we usually want
		to draw to the main screen or only have a single screen to begin
		with, we let EGL pick the default display.
		Querying other displays is platform specific.
	*/
	eglDisplay = eglGetDisplay(ctx.eglDisplay);

	/*
		Step 2 - Initialize EGL.
		EGL has to be initialized with the display obtained in the
		previous step. We cannot use other EGL functions except
		eglGetDisplay and eglGetError before eglInitialize has been
		called.
		If we're not interested in the EGL version number we can just
		pass NULL for the second and third parameters.
	*/
	EGLint iMajorVersion, iMinorVersion;
	dbg.DebugOut(kDbgLvlVerbose, "eglInitialize()\n");
	bool success = eglInitialize(eglDisplay, &iMajorVersion, &iMinorVersion);
	dbg.Assert(success, "eglInitialize() failed\n");

	// NOTE: 3D layer can only be enabled after MagicEyes lib 
	// sets up 3D accelerator, but this cannot happen
	// until GLESOAL_Initalize() callback gets mappings.
	PRINTF("eglInitialize post-init layer enable\n");
	disp_.EnableOpenGL();

	/*
		Step 3 - Specify the required configuration attributes.
		An EGL "configuration" describes the pixel format and type of
		surfaces that can be used for drawing.
		For now we just want to use a 16 bit RGB surface that is a 
		Window surface, i.e. it will be visible on screen. The list
		has to contain key/value pairs, terminated with EGL_NONE.
	 */
	const EGLint pi32ConfigAttribs[] =
	{
	    EGL_RED_SIZE,       8,
	    EGL_GREEN_SIZE,     8,
	    EGL_BLUE_SIZE,      8,
	    EGL_ALPHA_SIZE,     EGL_DONT_CARE,
	    EGL_DEPTH_SIZE,     16,
	    EGL_STENCIL_SIZE,   EGL_DONT_CARE,
	    EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
	    EGL_NONE,           EGL_NONE
	};

	/*
		Step 4 - Find a config that matches all requirements.
		eglChooseConfig provides a list of all available configurations
		that meet or exceed the requirements given as the second
		argument. In most cases we just want the first config that meets
		all criteria, so we can limit the number of configs returned to 1.
	*/
	int iConfigs;
	dbg.DebugOut(kDbgLvlVerbose, "eglChooseConfig()\n");
	success = eglChooseConfig(eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &iConfigs);
	dbg.Assert(success && iConfigs == 1, "eglChooseConfig() failed\n");

	/*
		Step 5 - Create a surface to draw to.
		Use the config picked in the previous step and the native window
		handle when available to create a window surface. A window surface
		is one that will be visible on screen inside the native display (or 
		fullscreen if there is no windowing system).
		Pixmaps and pbuffers are surfaces which only exist in off-screen
		memory.
	*/
	dbg.DebugOut(kDbgLvlVerbose, "eglCreateWindowSurface()\n");
	eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, ctx.eglWindow, NULL);
	AbortIfEGLError("eglCreateWindowSurface");

	/*
		Step 6 - Create a context.
		EGL has to create a context for OpenGL ES. Our OpenGL ES resources
		like textures will only be valid inside this context
		(or shared contexts)
	*/
	dbg.DebugOut(kDbgLvlVerbose, "eglCreateContext()\n");
	eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, NULL);
	AbortIfEGLError("eglCreateContext");

	/*
		Step 7 - Bind the context to the current thread and use our 
		window surface for drawing and reading.
		Contexts are bound to a thread. This means you don't have to
		worry about other threads and processes interfering with your
		OpenGL ES application.
		We need to specify a surface that will be the target of all
		subsequent drawing operations, and one that will be the source
		of read operations. They can be the same surface.
	*/
	dbg.DebugOut(kDbgLvlVerbose, "eglMakeCurrent()\n");
	eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
	AbortIfEGLError("eglMakeCurrent");

	// Clear garbage pixels from previous OpenGL context (embedded target)
	glClearColorx(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	dbg.DebugOut(kDbgLvlVerbose, "eglSwapBuffers()\n");
	eglSwapBuffers(eglDisplay, eglSurface);
	AbortIfEGLError("eglSwapBuffers (BOGL ctor)");
}

//----------------------------------------------------------------------
BrioOpenGLConfig::~BrioOpenGLConfig()
{
	// Disable 3D layer before disabling accelerator
	disp_.DisableOpenGL();
	
	/*
		Step 9 - Terminate OpenGL ES and destroy the window (if present).
		eglTerminate takes care of destroying any context or surface created
		with this display, so we don't need to call eglDestroySurface or
		eglDestroyContext here.
	*/
	eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) ;
	if(eglContext) eglDestroyContext(eglDisplay, eglContext);
	if(eglSurface) eglDestroySurface(eglDisplay, eglSurface);
	eglTerminate(eglDisplay);
	eglContext = NULL;
	eglSurface = NULL;
	
	/*
		Step 10 - Destroy the eglWindow.
		Again, this is platform specific and delegated to a separate function.
	*/

	// Exit OpenGL hardware
	disp_.DisableOpenGL();
	disp_.DeinitOpenGL(); 
}

LF_END_BRIO_NAMESPACE()

// eof
