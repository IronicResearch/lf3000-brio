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

#include <SystemTypes.h>
#include <SystemErrors.h>
#include <BrioOpenGLConfig.h>
#include <DebugMPI.h>
#include <EmulationConfig.h>
#include <DisplayPriv.h>

#ifndef  EMULATION
#ifdef   LF1000
#include "GLES/libogl.h"
#endif
#ifdef   LF2000
#include <vr5_swap_buffer_callback.h>
#endif
#include <signal.h>
#endif

// LF2000
typedef struct {
	int	width;
	int height;
} WindowType;

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
	tOpenGLContext		ctx = {NULL, 0, 0, 0, 0, 0, 0};
	CDisplayMPI*		dispmgr = 0;
	NativeDisplayType	display = 0;	// FIXME typedef change
//	NativeWindowType	hwnd = 0;
	WindowType			hwnd;			// FIXME LF2000 type
	bool				isEnabled = false;
	bool				isHandled = false;
	
	//--------------------------------------------------------------------------
	void BOGLExitHandler(void)
	{
		if (isEnabled)
		{
			CDebugMPI	dbg(kGroupDisplay);
			dbg.DebugOut(kDbgLvlCritical, "BrioOpenGLConfig() context active on exit\n");
			eglTerminate(eglGetDisplay(ctx.eglDisplay));
		}
	}
	
	//--------------------------------------------------------------------------
	void BOGLSignalHandler(int signum)
	{
		if (isEnabled)
		{
			CDebugMPI	dbg(kGroupDisplay);
			dbg.DebugOut(kDbgLvlCritical, "BrioOpenGLConfig() context active on signal %d\n", signum);
			eglTerminate(eglGetDisplay(ctx.eglDisplay));
		}
		_exit(128 + signum);
	}
	
	//--------------------------------------------------------------------------
	void AbortIfEGLError(const char* pszLocation)
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
#ifndef LF2000
	// TODO/dm: Pass back essential callback info to MagicEyes GLES lib
	// TODO/dm: All these callbacks should be moved into libDisplay.so module
	//			but MagicEyes libogl.a links against them *and* EGL funcs.
	//--------------------------------------------------------------------------
	___OAL_MEMORY_INFORMATION__ 	meminfo;
	//--------------------------------------------------------------------------
#ifdef LF1000
	extern "C" GLESOALbool GLESOAL_Initalize( ___OAL_MEMORY_INFORMATION__* pMemoryInfo, int FSAAEnb)
#else // !LF1000
	extern "C" int  GLESOAL_Initalize(___OAL_MEMORY_INFORMATION__* pMemoryInfo ) 
#endif
	{
		CDebugMPI	dbg(kGroupDisplay);
#ifdef LF1000
		// OGL needs 2 layers for fullscreen anti-aliasing option
		ctx.bFSAA = FSAAEnb;
#endif
		dispmgr->InitOpenGL(&ctx);
		*pMemoryInfo = meminfo;
		dbg.DebugOut(kDbgLvlVerbose, "GLESOAL_Initalize: %08X, %08X, %08X,%08X, %08X, %08X, %08X\n", \
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
		CDebugMPI	dbg(kGroupDisplay);		
		// 3D layer must be disabled before this callback returns
		dbg.DebugOut(kDbgLvlVerbose, "GLESOAL_Finalize\n");
		dispmgr->DisableOpenGL();
		dispmgr->DeinitOpenGL();
		isEnabled = false;
	}
#endif // LF1000
#endif // !LF2000
	//--------------------------------------------------------------------------
	extern "C" void GLESOAL_SwapBufferCallback( void ) 
	{
#ifdef DEBUG		
		CDebugMPI	dbg(kGroupDisplay);
		dbg.DebugOut(kDbgLvlVerbose, "GLESOAL_SwapBufferCallback\n");
#endif

		// Enable 3D layer on 1st update
		if (!isEnabled) 
		{
			dispmgr->EnableOpenGL(&ctx);
			isEnabled = true;
		}
#ifndef LF1000
		// 3D layer needs to sync to OGL calls (LF2530 only)
		dispmgr->UpdateOpenGL();
#endif
	}
#ifndef LF2000
#ifdef LF1000
	//--------------------------------------------------------------------------
	extern "C" void GLESOAL_SetWindow( void* /*pNativeWindow*/ ) { }
	
	//--------------------------------------------------------------------------
	extern "C" void GLESOAL_GetWindowSize( int* pWidth, int* pHeight )
	{
	    *pWidth  = ctx.width;
	    *pHeight = ctx.height;
	}

#ifdef LF1000
	// (added for LF1000)
	extern "C" void GLESOAL_Sleep(unsigned long Milliseconds )
	{
		usleep( Milliseconds*1000);
	}

	// (added for LF1000)
	extern "C" void GLESOAL_SetDisplayAddress(
			const unsigned int DisplayBufferPhysicalAddress )
	{
		dispmgr->SetOpenGLDisplayAddress(DisplayBufferPhysicalAddress);
	}

	// (added for LF1000)
	extern "C" void GLESOAL_WaitForDisplayAddressPatched( void )
	{
		dispmgr->WaitForDisplayAddressPatched();
	}
#endif // LF1000
#endif // !LF2000
#endif // !EMULATION
}

//==============================================================================
//----------------------------------------------------------------------
BrioOpenGLConfig::BrioOpenGLConfig(U32 size1D, U32 size2D)
	: 
	eglDisplay(0), eglConfig(0), eglSurface(0), eglContext(0)
{
	CDebugMPI				dbg(kGroupDisplay);
	dbg.SetDebugLevel(kDisplayDebugLevel);
	dbg.DebugOut(kDbgLvlCritical, "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	if(!dispmgr)
		dispmgr = new CDisplayMPI();

#ifndef  EMULATION
	// Setup exit handlers to disable OGL context
	if (!isHandled) {
		atexit(BOGLExitHandler);
		signal(SIGTERM, BOGLSignalHandler);
		isHandled = true;
	}
#endif
	
	// Make sure only one OGL context is active at a time
	if (isEnabled) {
		dbg.DebugOut(kDbgLvlCritical, "BrioOpenGLConfig() detected previous OGL context active\n");
		eglTerminate(eglGetDisplay(ctx.eglDisplay));
		dbg.DebugOut(kDbgLvlVerbose, "eglTerminate()\n");
	}
	
#ifndef EMULATION
#ifdef 	LF1000
	// Init OpenGL hardware callback struct
	meminfo.Memory1D_SizeInMbyte = size1D;
	meminfo.Memory2D_SizeInMbyte = size2D;
	ctx.pOEM = &meminfo;
	// FIXME/dm: How is LF1000 OGL lib configured for FSAA option?
	ctx.bFSAA = false;
#endif
	// Too soon to call InitOpenGL on LF1000, though we still need EGL params
	ctx.eglDisplay = display; // FIXME typedef
	ctx.eglWindow = &hwnd; // something non-NULL 
#ifdef 	LF2000
	disp_.InitOpenGL(&ctx);
	hwnd.width = ctx.owidth;
	hwnd.height = ctx.oheight;
#endif
#else
	/*
		Step 0 - Create a NativeWindowType that we can use it for OpenGL ES output
	*/
	disp_.InitOpenGL(&ctx);
#endif
	
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
	dbg.DebugOut(kDbgLvlVerbose, "eglInitialize(): success = %d, version = %d.%d\n", success, iMajorVersion, iMinorVersion);
	dbg.DebugOut(kDbgLvlVerbose, "EGL vendor = %s\n", eglQueryString(eglDisplay, EGL_VENDOR));
	dbg.DebugOut(kDbgLvlVerbose, "EGL version = %s\n", eglQueryString(eglDisplay, EGL_VERSION));
	dbg.DebugOut(kDbgLvlVerbose, "EGL extensions = %s\n", eglQueryString(eglDisplay, EGL_EXTENSIONS));

#ifndef LF1000
	eglBindAPI(EGL_OPENGL_ES_API);
#endif

	// NOTE: 3D layer can only be enabled after MagicEyes lib 
	// sets up 3D accelerator, but this cannot happen
	// until GLESOAL_Initalize() callback gets mappings.

	// NOTE: LF1000 GLESOAL_Initalize() callback now occurs during 
	// eglCreateWindowSurface() instead of eglInitialize().
	
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
		#ifdef LF2000
	    EGL_STENCIL_SIZE,   8,
		#endif
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
	dbg.Assert(success && iConfigs == 1, "eglChooseConfig() failed %d\n", iConfigs);

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
	dbg.DebugOut(kDbgLvlVerbose, "OpenGL ES vendor = %s\n", glGetString(GL_VENDOR));
	dbg.DebugOut(kDbgLvlVerbose, "OpenGL ES version = %s\n", glGetString(GL_VERSION));
	dbg.DebugOut(kDbgLvlVerbose, "OpenGL ES extensions = %s\n", glGetString(GL_EXTENSIONS));
	dbg.DebugOut(kDbgLvlVerbose, "OpenGL ES renderer = %s\n", glGetString(GL_RENDERER));

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

#ifdef LF2000
	// Enable display context layer visibility after initial buffer swap
	disp_.EnableOpenGL(&ctx);
	isEnabled = true;
#ifndef EMULATION
	__vr5_set_swap_buffer_callback(GLESOAL_SwapBufferCallback);
#endif
#endif

	// Store handle for use in Display MPI functions
	hndlDisplay = ctx.hndlDisplay;
	dbg.DebugOut(kDbgLvlVerbose, "display handle = %p\n", hndlDisplay);
}

//----------------------------------------------------------------------
BrioOpenGLConfig::~BrioOpenGLConfig()
{
#ifdef LF2000
	// Disable 3D layer before disabling accelerator
#ifndef EMULATION
	__vr5_set_swap_buffer_callback(0);
#endif
	disp_.DisableOpenGL();
	isEnabled = false;
#endif
	
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

#ifdef LF2000
	// Exit OpenGL hardware
	disp_.DeinitOpenGL();
#endif
	
	if(dispmgr)
	{
		delete dispmgr;
		dispmgr = 0;
	}
}

#ifndef LF1000
BrioOpenGLConfig::BrioOpenGLConfig(enum tBrioOpenGLVersion brioOpenGLVersion)
	:
	eglDisplay(0), eglConfig(0), eglSurface(0), eglContext(0)
{
	CDebugMPI				dbg(kGroupDisplay);
	dbg.SetDebugLevel(kDisplayDebugLevel);
	if(!dispmgr)
		dispmgr = new CDisplayMPI();

#ifndef  EMULATION
	// Setup exit handlers to disable OGL context
	if (!isHandled) {
		atexit(BOGLExitHandler);
		signal(SIGTERM, BOGLSignalHandler);
		isHandled = true;
	}
#endif

	// Make sure only one OGL context is active at a time
	if (isEnabled) {
		dbg.DebugOut(kDbgLvlCritical, "BrioOpenGLConfig() detected previous OGL context active\n");
		eglTerminate(eglGetDisplay(ctx.eglDisplay));
		dbg.DebugOut(kDbgLvlVerbose, "eglTerminate()\n");
	}

#ifndef EMULATION
	// Too soon to call InitOpenGL on LF1000, though we still need EGL params
	ctx.eglDisplay = display; // FIXME typedef
	ctx.eglWindow = &hwnd; // something non-NULL

	disp_.InitOpenGL(&ctx);
	hwnd.width = ctx.owidth;
	hwnd.height = ctx.oheight;

#else
	/*
		Step 0 - Create a NativeWindowType that we can use it for OpenGL ES output
	*/
	disp_.InitOpenGL(&ctx);
#endif

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
	dbg.DebugOut(kDbgLvlVerbose, "eglInitialize(): success = %d, version = %d.%d\n", success, iMajorVersion, iMinorVersion);
	dbg.DebugOut(kDbgLvlVerbose, "EGL vendor = %s\n", eglQueryString(eglDisplay, EGL_VENDOR));
	dbg.DebugOut(kDbgLvlVerbose, "EGL version = %s\n", eglQueryString(eglDisplay, EGL_VERSION));
	dbg.DebugOut(kDbgLvlVerbose, "EGL extensions = %s\n", eglQueryString(eglDisplay, EGL_EXTENSIONS));

	eglBindAPI(EGL_OPENGL_ES_API);
	// NOTE: 3D layer can only be enabled after MagicEyes lib
	// sets up 3D accelerator, but this cannot happen
	// until GLESOAL_Initalize() callback gets mappings.

	// NOTE: LF1000 GLESOAL_Initalize() callback now occurs during
	// eglCreateWindowSurface() instead of eglInitialize().


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
	    EGL_ALPHA_SIZE,     8,
	    EGL_DEPTH_SIZE,     16,
	    EGL_STENCIL_SIZE,   8,
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
	EGLint ai32ContextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, brioOpenGLVersion, EGL_NONE };
	eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, ai32ContextAttribs);
	AbortIfEGLError("eglCreateContext");
	dbg.DebugOut(kDbgLvlVerbose, "OpenGL ES vendor = %s\n", glGetString(GL_VENDOR));
	dbg.DebugOut(kDbgLvlVerbose, "OpenGL ES version = %s\n", glGetString(GL_VERSION));
	dbg.DebugOut(kDbgLvlVerbose, "OpenGL ES extensions = %s\n", glGetString(GL_EXTENSIONS));
	dbg.DebugOut(kDbgLvlVerbose, "OpenGL ES renderer = %s\n", glGetString(GL_RENDERER));

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

	// Enable display context layer visibility after initial buffer swap
	disp_.EnableOpenGL(&ctx);
	isEnabled = true;
#ifndef EMULATION
	__vr5_set_swap_buffer_callback(GLESOAL_SwapBufferCallback);
#endif

	// Store handle for use in Display MPI functions
	hndlDisplay = ctx.hndlDisplay;
	dbg.DebugOut(kDbgLvlVerbose, "display handle = %p\n", hndlDisplay);
}
#endif

LF_END_BRIO_NAMESPACE()

// eof
