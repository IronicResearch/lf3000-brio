//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		DisplayEmul.cpp
//
// Description:
//		Configure display manager for emulation target
//
//==============================================================================

#include <SystemTypes.h>
#include <SystemErrors.h>
#include <DisplayPriv.h>
#include <DisplayMPI.h>
#include <BrioOpenGLConfig.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

LF_BEGIN_BRIO_NAMESPACE()

namespace 
{	
	// X11 variables
	Window				x11Window;
	Display*			x11Display;
	long				x11Screen;
	XVisualInfo*		x11Visual;
	Colormap			x11Colormap;

	// Display context
	struct tDisplayContext*	dc_;
	GC						gc;
	Pixmap					pixmap;
	_XImage*				image;
	tRect					rect;
}

//============================================================================
// CDisplayModule: Emulation of hardware-specific functions
//============================================================================
//----------------------------------------------------------------------------
void CDisplayModule::InitModule()
{
	// Initialize display manager for emulation target
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
	dbg_.Assert(x11Display != NULL, "Unable to open X display\n");
	x11Screen = XDefaultScreen( x11Display );
	
	// Gets the window parameters
	sRootWindow = RootWindow(x11Display, x11Screen);
	i32Depth = DefaultDepth(x11Display, x11Screen);
	x11Visual = new XVisualInfo;
	XMatchVisualInfo( x11Display, x11Screen, i32Depth, TrueColor, x11Visual);
	dbg_.Assert(x11Visual != NULL, "Unable to acquire visual\n");
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
	
	// Create X graphics context for any drawing we need to do
	gc = XCreateGC(x11Display, x11Window, 0, NULL);
}

//----------------------------------------------------------------------------
void CDisplayModule::DeInitModule()
{
	// On the target, this is where file descriptors get closed, etc
	// in emulation, you probably don't need to do anything here
	if (x11Window) 
		XDestroyWindow(x11Display, x11Window);
    if (x11Colormap) 
    	XFreeColormap( x11Display, x11Colormap );
	if (x11Display) 
		XCloseDisplay(x11Display);
}


//----------------------------------------------------------------------------
void CDisplayModule::InitOpenGL(void* pCtx)
{
	// Pass back essential display context info for OpenGL bindings
	tOpenGLContext*		ctx = (tOpenGLContext*)pCtx;
	XWindowAttributes	attr;
	XGetWindowAttributes(x11Display, x11Window, &attr);
	ctx->eglDisplay = (NativeDisplayType)x11Display;
	ctx->eglWindow  = (NativeWindowType)x11Window;
	ctx->width		= attr.width;
	ctx->height		= attr.height;
}
      
//----------------------------------------------------------------------------
void CDisplayModule::DeinitOpenGL()
{
	// Nothing to do on emulation target
}

//----------------------------------------------------------------------------
void CDisplayModule::EnableOpenGL()
{
	// Nothing to do on emulation target
}
      
//----------------------------------------------------------------------------
void CDisplayModule::DisableOpenGL()
{
	// Nothing to do on emulation target
}

//----------------------------------------------------------------------------
tDisplayHandle CDisplayModule::CreateHandle(U16 height, U16 width,
                                        tPixelFormat colorDepth, U8 *pBuffer)
{
	tDisplayContext *dc =  new struct tDisplayContext;

	int depth;
	switch (colorDepth) 
	{
		case kPixelFormatRGB4444:	depth = 16; break;
		case kPixelFormatRGB565:	depth = 16; break;
		case kPixelFormatARGB8888: 	
		default:					depth = 24; //32; break;
	}		

	dc->width = width;
	dc->height = height;
	dc->colorDepth = colorDepth;

	// TODO/dm: Bind these to display context
	/* Pixmap */ pixmap = XCreatePixmap(x11Display, x11Window, width, height, depth);
	/* _XImage* */ image = XGetImage(x11Display, pixmap, 0, 0, width, height, AllPlanes, ZPixmap);
	
	dc->pitch = image->bytes_per_line;
	dc->pBuffer = (U8*)image->data;
	dc->x = 0;
	dc->y = 0;
	dc->isAllocated = true;
	
	memset(dc->pBuffer, 0, width * height);
	
	rect.left = rect.top = 0;
	rect.right = width;
	rect.bottom = height;
	
	return (tDisplayHandle)dc;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::Register(tDisplayHandle hndl, S16 xPos, S16 yPos,
                            tDisplayHandle insertAfter, tDisplayScreen screen)
{
	struct tDisplayContext* dc = (struct tDisplayContext*)hndl;
	rect.left = dc->x = xPos;
	rect.top  = dc->y = yPos;
	rect.right = rect.left + dc->width;
	rect.bottom = rect.top + dc->height;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::Register(tDisplayHandle hndl, S16 xPos, S16 yPos,
                             tDisplayZOrder initialZOrder,
                             tDisplayScreen screen)
{
	struct tDisplayContext* dc = (struct tDisplayContext*)hndl;
	rect.left = dc->x = xPos;
	rect.top  = dc->y = yPos;
	rect.right = rect.left + dc->width;
	rect.bottom = rect.top + dc->height;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::Invalidate(tDisplayScreen screen, tRect *pDirtyRect)
{
	XPutImage(x11Display, x11Window, gc, image, 0, 0, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
	XFlush(x11Display);
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::UnRegister(tDisplayHandle hndl, tDisplayScreen screen)
{
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::DestroyHandle(tDisplayHandle hndl, Boolean destroyBuffer)
{
	struct tDisplayContext* dc = (struct tDisplayContext*)hndl;
	if (image)
		XDestroyImage(image);
	image = NULL;
	if (pixmap)
		XFreePixmap(x11Display, pixmap);
	pixmap = 0;
 	delete dc;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::SetAlpha(tDisplayHandle hndl, U8 level, 
	Boolean enable)
{
    dbg_.DebugOut(kDbgLvlCritical, "SetAlpha not implemented\n");
    return kNoImplErr;
}

//----------------------------------------------------------------------------
U16 CDisplayModule::GetPitch(tDisplayHandle hndl) const
{
	struct tDisplayContext* dc = (struct tDisplayContext*)hndl;
	return dc->pitch;
}

//----------------------------------------------------------------------------
U8* CDisplayModule::GetBuffer(tDisplayHandle hndl) const
{
	struct tDisplayContext* dc = (struct tDisplayContext*)hndl;
	return dc->pBuffer;
}

//----------------------------------------------------------------------------
U16 CDisplayModule::GetHeight(tDisplayHandle hndl) const
{
	struct tDisplayContext* dc = (struct tDisplayContext*)hndl;
	return dc->height;
}

//----------------------------------------------------------------------------
U16 CDisplayModule::GetWidth(tDisplayHandle hndl) const
{
	struct tDisplayContext* dc = (struct tDisplayContext*)hndl;
	return dc->width;
}

//----------------------------------------------------------------------------
U32 CDisplayModule::GetScreenSize()
{
	return (U32)((320<<16)|(240));
}

//----------------------------------------------------------------------------
enum tPixelFormat CDisplayModule::GetPixelFormat(void)
{
	return kPixelFormatARGB8888;
}

LF_END_BRIO_NAMESPACE()
// EOF
