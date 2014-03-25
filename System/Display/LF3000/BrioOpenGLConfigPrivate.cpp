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
#include <map>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <BrioOpenGLConfigPrivate.h>
#include <DebugMPI.h>
#include <EmulationConfig.h>
#include <DisplayPriv.h>
#include <stdio.h>
#include <Utility.h>

#include <signal.h>
#include <GLES/gl.h>


// LF2000
typedef struct {
	int	width;
	int height;
	void* pctx;
} WindowType;

LF_BEGIN_BRIO_NAMESPACE()

#if 0	// NOTBUG/dm: enable for development
#define	PRINTF	printf
#else
#define PRINTF(...)
#endif
std::map<BrioOpenGLConfigPrivate *, tOpenGLContext>		brioopenglconfig_context_map;

//==============================================================================
namespace
{
	//--------------------------------------------------------------------------
	std::map<void *, tOpenGLContext*>					surface_context_map;
	CDisplayMPI*		dispmgr = 0;
	int					dispmgrcount = 0;
	NativeDisplayType	display = 0;	// FIXME typedef change
	bool				isEnabled = false;
	bool				isHandled = false;
	bool				swapBufferSet = false;
	
	//--------------------------------------------------------------------------
	void BOGLExitHandler(void)
	{
		if (isEnabled)
		{
			CDebugMPI	dbg(kGroupDisplay);
			dbg.DebugOut(kDbgLvlCritical, "BrioOpenGLConfig() context active on exit\n");
			eglTerminate(eglGetCurrentDisplay());
		}
	}
	
	//--------------------------------------------------------------------------
	void BOGLSignalHandler(int signum)
	{
		if (isEnabled)
		{
			CDebugMPI	dbg(kGroupDisplay);
			dbg.DebugOut(kDbgLvlCritical, "BrioOpenGLConfig() context active on signal %d\n", signum);
			eglTerminate(eglGetCurrentDisplay());
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
			dispmgr->EnableOpenGL(surface_context_map.begin()->second);
			isEnabled = true;
		}
		// 3D layer needs to sync to OGL calls (LF2530 only)
		dispmgr->UpdateOpenGL(surface_context_map.begin()->second);
	}
	extern "C" void GLESOAL_SwapBufferCallback2( EGLDisplay dpy, EGLSurface surface )
	{
#ifdef DEBUG		
		CDebugMPI	dbg(kGroupDisplay);
		dbg.DebugOut(kDbgLvlVerbose, "GLESOAL_SwapBufferCallback\n");
#endif

		// Enable 3D layer on 1st update
		if (!isEnabled) 
		{
			dispmgr->EnableOpenGL(surface_context_map[surface]);
			isEnabled = true;
		}
		// 3D layer needs to sync to OGL calls (LF2530 only)
		dispmgr->UpdateOpenGL(surface_context_map[surface]);
	}
}

//==============================================================================
void SetSwapBufferCallback()
{
	eglSetSwapbufferCallback(GLESOAL_SwapBufferCallback2);
	swapBufferSet = true;
}
//----------------------------------------------------------------------
BrioOpenGLConfigPrivate::BrioOpenGLConfigPrivate(U32 size1D, U32 size2D)
	: 
	eglDisplay(0), eglConfig(0), eglSurface(0), eglContext(0)
{
	(void)size1D;
	(void)size2D;
	Init(kBrioOpenGL11);
}

//----------------------------------------------------------------------
BrioOpenGLConfigPrivate::~BrioOpenGLConfigPrivate()
{
	// Disable 3D layer before disabling accelerator
	disp_.DisableOpenGL(&brioopenglconfig_context_map[this]);
	isEnabled = false;
	
	/*
		Step 9 - Terminate OpenGL ES and destroy the window (if present).
		eglTerminate takes care of destroying any context or surface created
		with this display, so we don't need to call eglDestroySurface or
		eglDestroyContext here.
	*/
	tOpenGLContext &ctx = brioopenglconfig_context_map[this];
	if(ctx.width != ctx.owidth || ctx.height != ctx.oheight)
		eglDisableDrawingOrgSize();
	eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) ;
	if(eglContext) eglDestroyContext(eglDisplay, eglContext);
	if(eglSurface) eglDestroySurface(eglDisplay, eglSurface);
	eglTerminate(eglDisplay);
	surface_context_map.erase(eglSurface);
	eglContext = NULL;
	eglSurface = NULL;
	
	/*
		Step 10 - Destroy the eglWindow.
		Again, this is platform specific and delegated to a separate function.
	*/
	delete (WindowType*)ctx.eglWindow;
	// Exit OpenGL hardware
	disp_.DeinitOpenGL(&ctx);
	brioopenglconfig_context_map.erase(this);
	
	if(--dispmgrcount == 0 && dispmgr)
	{
		delete dispmgr;
		dispmgr = 0;
	}
}

BrioOpenGLConfigPrivate::BrioOpenGLConfigPrivate(enum tBrioOpenGLVersion brioOpenGLVersion)
	:
	eglDisplay(0), eglConfig(0), eglSurface(0), eglContext(0)
{
	Init(brioOpenGLVersion);
}

void BrioOpenGLConfigPrivate::Init(U32 size1D, U32 size2D)
{
	(void)size1D;
	(void)size2D;
	Init(kBrioOpenGL11);
}

void BrioOpenGLConfigPrivate::Init(enum tBrioOpenGLVersion brioOpenGLVersion)
{
	CDebugMPI				dbg(kGroupDisplay);
	dbg.SetDebugLevel(kDisplayDebugLevel);
	if(!dispmgr)
		dispmgr = new CDisplayMPI();
	dispmgrcount++;

	tOpenGLContext &ctx = brioopenglconfig_context_map[this];
	// Setup exit handlers to disable OGL context
	if (!isHandled) {
		atexit(BOGLExitHandler);
		signal(SIGTERM, BOGLSignalHandler);
		isHandled = true;
	}
	// Too soon to call InitOpenGL on LF1000, though we still need EGL params
	ctx.eglDisplay = display; // FIXME typedef
	WindowType *hwnd = new WindowType;
	ctx.eglWindow = hwnd; // something non-NULL
	disp_.InitOpenGL(&ctx);
	hwnd->width = ctx.width;
	hwnd->height = ctx.height;
	hwnd->pctx = ctx.hndlDisplay;

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
	bool using_32_bit = true;
	const EGLint pi32ConfigAttribs[] =
	{
	    EGL_RED_SIZE,       using_32_bit ? 8 : 5,
	    EGL_GREEN_SIZE,     using_32_bit ? 8 : 6,
	    EGL_BLUE_SIZE,      using_32_bit ? 8 : 5,
	    EGL_ALPHA_SIZE,     using_32_bit ? 8 : 0,
	    EGL_DEPTH_SIZE,     16,
	    EGL_STENCIL_SIZE,   using_32_bit ? 8 : 0,
	    EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
	    EGL_RENDERABLE_TYPE, brioOpenGLVersion == kBrioOpenGL11 ? EGL_OPENGL_ES_BIT : EGL_OPENGL_ES2_BIT, 
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
	EGLint ai32ContextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, brioOpenGLVersion, EGL_NONE };
	eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, ai32ContextAttribs);
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
	dbg.DebugOut(kDbgLvlVerbose, "OpenGL ES vendor = %s\n", glGetString(GL_VENDOR));
	dbg.DebugOut(kDbgLvlVerbose, "OpenGL ES version = %s\n", glGetString(GL_VERSION));
	dbg.DebugOut(kDbgLvlVerbose, "OpenGL ES extensions = %s\n", glGetString(GL_EXTENSIONS));
	dbg.DebugOut(kDbgLvlVerbose, "OpenGL ES renderer = %s\n", glGetString(GL_RENDERER));
	if(ctx.width != ctx.owidth || ctx.height != ctx.oheight)
	{
		eglSetDrawingOrgSize(ctx.owidth, ctx.oheight);
		eglEnableDrawingOrgSize();
	}

	surface_context_map[eglSurface] = &brioopenglconfig_context_map[this];
	// Clear garbage pixels from previous OpenGL context (embedded target)
	if(brioOpenGLVersion == kBrioOpenGL11)
	{
		glEnableSpecialMode(GL_SPECIAL_MODE_LAST_TEXTURE_EN);
		glEnableSpecialMode(GL_SPECIAL_MODE_PIXEL_FOG_EN);
		glSetSpecialModeParam(GL_SPECIAL_MODE_PIXEL_FOG_EN, 3);
		glEnableSpecialMode(2);//glEnableSpecialMode(GL_SPECIAL_MODE_TEX_FILTER_OFF);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}

	glClearColorx(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	dbg.DebugOut(kDbgLvlVerbose, "eglSwapBuffers()\n");
	eglSwapBuffers(eglDisplay, eglSurface);
	AbortIfEGLError("eglSwapBuffers (BOGL ctor)");
	// Enable display context layer visibility after initial buffer swap
	disp_.EnableOpenGL(&ctx);
	isEnabled = true;
	if(!swapBufferSet)
		SetSwapBufferCallback();
	// Store handle for use in Display MPI functions
	hndlDisplay = ctx.hndlDisplay;
	dbg.DebugOut(kDbgLvlVerbose, "display handle = %p\n", hndlDisplay);
}

LF_END_BRIO_NAMESPACE()

// eof
