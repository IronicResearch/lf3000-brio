//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		EmulationConfig.cpp
//
// Description:
//		Configure emulation for the environment
//
//==============================================================================

#include <SystemErrors.h>
#include <EmulationConfig.h>
#include <DisplayTypes.h>		// for BrioOpenGLConfig definition

#include <stdio.h>
#include <stdlib.h>
#include <cassert>



//==============================================================================
namespace
{
	CPath	gModuleSearchPath;
	U32		gWindow;

	/*!****************************************************************************
	 @Function		TestEGLError
	 @Input			pszLocation		location in the program where the error took
									place. ie: function name
	 @Return		bool			true if no EGL error was detected
	 @Description	Tests for an EGL error and prints it
	******************************************************************************/
	bool TestEGLError(char* pszLocation)
	{
		/*
			eglGetError returns the last error that has happened using egl,
			not the status of the last called function. The user has to
			check after every single egl call or at least once every frame.
		*/
		EGLint iErr = eglGetError();
		if (iErr != EGL_SUCCESS)
		{
			printf("%s failed (%d).\n", pszLocation, iErr);
			return false;
		}
	
		return true;
	}
}


//==============================================================================
namespace LeapFrog
{
	namespace Brio
	{
		//----------------------------------------------------------------------
		EmulationConfig::EmulationConfig( )
		{
			gWindow = 0;
		}

		//----------------------------------------------------------------------
		EmulationConfig& EmulationConfig::Instance( )
		{
			static EmulationConfig minst;
			return minst;
		}

		//----------------------------------------------------------------------
		bool EmulationConfig::Initialize( const char* pathIn )
		{
			CPath path = pathIn;
			path += "/Libs/LightningGCC_emulation/Module";
			SetModuleSearchPath(path.c_str());
			return true;
		}

		//----------------------------------------------------------------------
		void EmulationConfig::SetModuleSearchPath( const char* path )
		{
			gModuleSearchPath = path;
		}

		//----------------------------------------------------------------------
		const char* EmulationConfig::GetModuleSearchPath( ) const
		{
			return gModuleSearchPath.c_str();
		}
		
		//----------------------------------------------------------------------
		void EmulationConfig::SetLcdDisplayWindow( U32 hwnd )
		{
			gWindow = hwnd;
		}
		
		//----------------------------------------------------------------------
		U32 EmulationConfig::GetLcdDisplayWindow( ) const
		{
			return gWindow;
		}
		
		//----------------------------------------------------------------------
		BrioOpenGLConfig::BrioOpenGLConfig()
			: x11Window(0), x11Display(0), x11Screen(0), x11Visual(0), x11Colormap(0),
			eglDisplay(0), eglConfig(0), eglSurface(0), eglContext(0)
		{
			const int WINDOW_WIDTH = 240;
			const int WINDOW_HEIGHT= 320;

			/*
				Step 0 - Create a NativeWindowType that we can use it for OpenGL ES output
			*/
			Window					sRootWindow;
		    XSetWindowAttributes	sWA;
			unsigned int			ui32Mask;
			int						i32Depth;
		
			// Initializes the display and screen
			x11Display = XOpenDisplay( 0 );
			if (!x11Display)
			{
				assert(!"Error: Unable to open X display\n");
			}
			x11Screen = XDefaultScreen( x11Display );
			
			// Gets the window parameters
			sRootWindow = RootWindow(x11Display, x11Screen);
			i32Depth = DefaultDepth(x11Display, x11Screen);
			x11Visual = new XVisualInfo;
			XMatchVisualInfo( x11Display, x11Screen, i32Depth, TrueColor, x11Visual);
			if (!x11Visual)
			{
				assert(!"Error: Unable to acquire visual\n");
			}
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
			if (!eglInitialize(eglDisplay, &iMajorVersion, &iMinorVersion))
			{
				assert(!"Error: eglInitialize() failed.\n");
			}
		
			/*
				Step 3 - Specify the required configuration attributes.
				An EGL "configuration" describes the pixel format and type of
				surfaces that can be used for drawing.
				For now we just want to use a 16 bit RGB surface that is a 
				Window surface, i.e. it will be visible on screen. The list
				has to contain key/value pairs, terminated with EGL_NONE.
			 */
			EGLint pi32ConfigAttribs[3];
			pi32ConfigAttribs[0] = EGL_SURFACE_TYPE;
			pi32ConfigAttribs[1] = EGL_WINDOW_BIT;
			pi32ConfigAttribs[2] = EGL_NONE;
		
			/*
				Step 4 - Find a config that matches all requirements.
				eglChooseConfig provides a list of all available configurations
				that meet or exceed the requirements given as the second
				argument. In most cases we just want the first config that meets
				all criteria, so we can limit the number of configs returned to 1.
			*/
			int iConfigs;
			if (!eglChooseConfig(eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &iConfigs)
				|| (iConfigs != 1))
			{
				assert(!"Error: eglChooseConfig() failed.\n");
			}
		
			/*
				Step 5 - Create a surface to draw to.
				Use the config picked in the previous step and the native window
				handle when available to create a window surface. A window surface
				is one that will be visible on screen inside the native display (or 
				fullscreen if there is no windowing system).
				Pixmaps and pbuffers are surfaces which only exist in off-screen
				memory.
			*/
			eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, (NativeWindowType)x11Window, NULL);
			if (!TestEGLError("eglCreateWindowSurface"))
			{
				assert(!"Error: eglCreateWindowSurface() failed.\n");
			}
		
			/*
				Step 6 - Create a context.
				EGL has to create a context for OpenGL ES. Our OpenGL ES resources
				like textures will only be valid inside this context
				(or shared contexts)
			*/
			eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, NULL);
			if (!TestEGLError("eglCreateContext"))
			{
				assert(!"Error: eglCreateContext() failed.\n");
			}
		
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
			eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
			if (!TestEGLError("eglMakeCurrent"))
			{
				assert(!"Error: eglMakeCurrent() failed.\n");
			}
			LeapFrog::Brio::EmulationConfig::Instance().SetLcdDisplayWindow(x11Window);
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
			if (x11Window) XDestroyWindow(x11Display, x11Window);
		    if (x11Colormap) XFreeColormap( x11Display, x11Colormap );
			if (x11Display) XCloseDisplay(x11Display);
		}
		
	}	// end of Brio namespace
}		// end of LeapFrog namespace

// eof
