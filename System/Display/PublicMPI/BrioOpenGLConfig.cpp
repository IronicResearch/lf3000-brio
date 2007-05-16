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
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "mlc_ioctl.h"
#include "GLES/libogl.h"
#endif

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
namespace
{
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
//		*pMemoryInfo = meminfo;
		memcpy(pMemoryInfo, &meminfo, sizeof(meminfo)); 
	    printf("GLESOAL_Initalize: %08X, %08X, %08X,%08X, %08X, %08X, %08X\n", 
		    pMemoryInfo->VirtualAddressOf3DCore,
		    pMemoryInfo->Memory1D_VirtualAddress,
		    pMemoryInfo->Memory1D_PhysicalAddress,
		    pMemoryInfo->Memory1D_SizeInMbyte,
		    pMemoryInfo->Memory2D_VirtualAddress,
		    pMemoryInfo->Memory2D_PhysicalAddress,
		    pMemoryInfo->Memory2D_SizeInMbyte);
		
		// 3D layer must not be enabled until after this callback returns

		return 1; 
	}

	//--------------------------------------------------------------------------
	extern "C" void GLESOAL_Finalize( void ) 
	{
		// 3D layer must be disabled before this callback returns
		printf("GLESOAL_Finalize\n");
		int	layer = open( "/dev/layer0", O_WRONLY);
		ioctl(layer, MLC_IOCT3DENB, (void *)0);
		ioctl(layer, MLC_IOCTDIRTY, (void *)1);
	    close(layer);
	}

	//--------------------------------------------------------------------------
	extern "C" void GLESOAL_SwapBufferCallback( void ) 
	{ 
//		printf("GLESOAL_SwapBufferCallback\n");
	}

	//--------------------------------------------------------------------------
	extern "C" void GLESOAL_SetWindow( void* pNativeWindow  ) { }
	
	//--------------------------------------------------------------------------
	extern "C" void GLESOAL_GetWindowSize( int* pWidth, int* pHeight )
	{
	    *pWidth  = 320;
	    *pHeight = 240;
	}
#endif // !EMULATION
}


//==============================================================================
//----------------------------------------------------------------------
BrioOpenGLConfig::BrioOpenGLConfig()
	: 
#ifdef EMULATION
	x11Window(0), x11Display(0), x11Screen(0), x11Visual(0), x11Colormap(0),
#endif
	eglDisplay(0), eglConfig(0), eglSurface(0), eglContext(0)
{
	CDebugMPI				dbg(kGroupDisplay);
	NativeWindowType		genWindow = 0;

#ifdef EMULATION
	const int WINDOW_WIDTH = 320;
	const int WINDOW_HEIGHT= 240;

	/*
		Step 0 - Create a NativeWindowType that we can use it for OpenGL ES output
	*/
	Window					sRootWindow;
    XSetWindowAttributes	sWA;
	unsigned int			ui32Mask;
	int						i32Depth;

	// Initializes the display and screen
	x11Display = XOpenDisplay( 0 );
	dbg.Assert(x11Display != NULL, "Unable to open X display\n");
	x11Screen = XDefaultScreen( x11Display );
	
	// Gets the window parameters
	sRootWindow = RootWindow(x11Display, x11Screen);
	i32Depth = DefaultDepth(x11Display, x11Screen);
	x11Visual = new XVisualInfo;
	XMatchVisualInfo( x11Display, x11Screen, i32Depth, TrueColor, x11Visual);
	dbg.Assert(x11Visual != NULL, "Unable to acquire visual\n");
    x11Colormap = XCreateColormap( x11Display, sRootWindow, x11Visual->visual, AllocNone );
    sWA.colormap = x11Colormap;
	
    // Add to these for handling other events
    sWA.event_mask = StructureNotifyMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask;
    ui32Mask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap;
	
	// Creates the X11 window
    x11Window = XCreateWindow( x11Display, RootWindow(x11Display, x11Screen), 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
								 0, CopyFromParent, InputOutput, CopyFromParent, ui32Mask, &sWA);
	XMapWindow(x11Display, x11Window);
	XFlush(x11Display);

	/*
		Step 1 - Get the default display.
		EGL uses the concept of a "display" which in most environments
		corresponds to a single physical screen. Since we usually want
		to draw to the main screen or only have a single screen to begin
		with, we let EGL pick the default display.
		Querying other displays is platform specific.
	*/
	eglDisplay = eglGetDisplay((NativeDisplayType)x11Display);
	genWindow = (NativeWindowType)x11Window;
#else	// !EMULATION
	// Init OpenGL hardware
	disp_.InitOpenGL(&meminfo);
	
	// EGL NativeWindow needs to be some non-NULL struct
	genWindow = malloc(sizeof(tDisplayScreenStats));

	// Lightning hardware just has one display
	eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
#endif	// EMULATION

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
	dbg.DebugOut(kDbgLvlCritical, "eglInitialize()\n");
	bool success = eglInitialize(eglDisplay, &iMajorVersion, &iMinorVersion);
	dbg.Assert(success, "eglInitialize() failed\n");

#ifndef EMULATION
		// NOTE: 3D layer can only be enabled after MagicEyes lib 
		// sets up 3D accelerator, but this cannot happen
		// until GLESOAL_Initalize() callback gets mappings.
//		usleep(5000);
		printf("eglInitialize post-init layer enable\n");
		int	layer = open( "/dev/layer0", O_WRONLY);
	    
		// Position 3D layer
		union mlc_cmd c;
		c.position.left = c.position.top = 0;
		c.position.right = 320;
		c.position.bottom = 240;
	
	    // Enable 3D layer
	    ioctl(layer, MLC_IOCTLAYEREN, (void *)1);
		ioctl(layer, MLC_IOCSPOSITION, (void *)&c);
		ioctl(layer, MLC_IOCTFORMAT, 0x4432);
		ioctl(layer, MLC_IOCTHSTRIDE, 2);
		ioctl(layer, MLC_IOCTVSTRIDE, 4096);
		ioctl(layer, MLC_IOCT3DENB, (void *)1);
		ioctl(layer, MLC_IOCTDIRTY, (void *)1);

		close(layer);
#endif

	/*
		Step 3 - Specify the required configuration attributes.
		An EGL "configuration" describes the pixel format and type of
		surfaces that can be used for drawing.
		For now we just want to use a 16 bit RGB surface that is a 
		Window surface, i.e. it will be visible on screen. The list
		has to contain key/value pairs, terminated with EGL_NONE.
	 */
#ifdef EMULATION
	EGLint pi32ConfigAttribs[3];
	pi32ConfigAttribs[0] = EGL_SURFACE_TYPE;
	pi32ConfigAttribs[1] = EGL_WINDOW_BIT;
	pi32ConfigAttribs[2] = EGL_NONE;
#else
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
#endif

	/*
		Step 4 - Find a config that matches all requirements.
		eglChooseConfig provides a list of all available configurations
		that meet or exceed the requirements given as the second
		argument. In most cases we just want the first config that meets
		all criteria, so we can limit the number of configs returned to 1.
	*/
	int iConfigs;
	dbg.DebugOut(kDbgLvlCritical, "eglChooseConfig()\n");
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
	dbg.DebugOut(kDbgLvlCritical, "eglCreateWindowSurface()\n");
	eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, genWindow, NULL);
	AbortIfEGLError("eglCreateWindowSurface");

	/*
		Step 6 - Create a context.
		EGL has to create a context for OpenGL ES. Our OpenGL ES resources
		like textures will only be valid inside this context
		(or shared contexts)
	*/
	dbg.DebugOut(kDbgLvlCritical, "eglCreateContext()\n");
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
	dbg.DebugOut(kDbgLvlCritical, "eglMakeCurrent()\n");
	eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
	AbortIfEGLError("eglMakeCurrent");
#ifdef EMULATION
	EmulationConfig::Instance().SetLcdDisplayWindow(x11Window);
#endif	// EMULATION

	glClearColorx(0x10000, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	dbg.DebugOut(kDbgLvlCritical, "eglSwapBuffers()\n");
	eglSwapBuffers(eglDisplay, eglSurface);
	AbortIfEGLError("eglSwapBuffers (BOGL ctor)");
}

//----------------------------------------------------------------------
BrioOpenGLConfig::~BrioOpenGLConfig()
{
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

	/*
		Step 10 - Destroy the eglWindow.
		Again, this is platform specific and delegated to a separate function.
	*/
#ifdef EMULATION
	if (x11Window) XDestroyWindow(x11Display, x11Window);
    if (x11Colormap) XFreeColormap( x11Display, x11Colormap );
	if (x11Display) XCloseDisplay(x11Display);
#endif	// EMULATION

	// Exit OpenGL hardware
	disp_.DeinitOpenGL(); 
}

LF_END_BRIO_NAMESPACE()

// eof
