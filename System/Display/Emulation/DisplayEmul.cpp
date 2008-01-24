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
#include <EmulationConfig.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

LF_BEGIN_BRIO_NAMESPACE()

namespace 
{	
	// X11 variables
	const int 			WINDOW_WIDTH = 320;
	const int 			WINDOW_HEIGHT= 240;
	Window				x11Window;
	Display*			x11Display;
	long				x11Screen;
	XVisualInfo*		x11Visual;
	Colormap			x11Colormap;

	// Display context
	GC						gc;
	S8						brightness_ = 0;
	S8						contrast_ = 0;
	S8						backlight_ = 100;
}

//============================================================================
// CDisplayModule: Emulation of hardware-specific functions
//============================================================================
//----------------------------------------------------------------------------
void CDisplayModule::InitModule()
{
	// Initialize display manager for emulation target

	// Create an X window for either 2D or 3D target
	Window					sRootWindow;
    XSetWindowAttributes	sWA;
	unsigned int			ui32Mask;
	int						i32Depth;

	// Initializes the display and screen
	x11Display = XOpenDisplay( 0 );
	dbg_.Assert(x11Display != NULL, "CDisplayModule::InitModule: Unable to open X display\n");
	x11Screen = XDefaultScreen( x11Display );
	
	// Gets the window parameters
	sRootWindow = RootWindow(x11Display, x11Screen);
	i32Depth = DefaultDepth(x11Display, x11Screen);
	x11Visual = new XVisualInfo;
	XMatchVisualInfo( x11Display, x11Screen, i32Depth, TrueColor, x11Visual);
	dbg_.Assert(x11Visual != NULL, "CDisplayModule::InitModule: Unable to acquire visual\n");
    x11Colormap = XCreateColormap( x11Display, sRootWindow, x11Visual->visual, AllocNone );
    sWA.colormap = x11Colormap;
	sWA.background_pixel = WhitePixel(x11Display, x11Screen);
	
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
	
	EmulationConfig::Instance().SetLcdDisplayWindow(static_cast<U32>(x11Window));
}

//----------------------------------------------------------------------------
void CDisplayModule::DeInitModule()
{
	// Release the X window and its resources 
	if (gc)
		XFreeGC(x11Display, gc);
	if (x11Window) 
		XDestroyWindow(x11Display, x11Window);
    if (x11Colormap) 
    	XFreeColormap(x11Display, x11Colormap );
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
void CDisplayModule::EnableOpenGL(void* pCtx)
{
	// Nothing to do on emulation target
}
      
//----------------------------------------------------------------------------
void CDisplayModule::UpdateOpenGL()
{
	// Nothing to do on emulation target
}
      
//----------------------------------------------------------------------------
void CDisplayModule::DisableOpenGL()
{
	// Nothing to do on emulation target
}

#ifdef LF1000	// Defined independently of EMULATION
//----------------------------------------------------------------------------
void CDisplayModule::WaitForDisplayAddressPatched()
{
	// Nothing to do on emulation target
}

//----------------------------------------------------------------------------
void CDisplayModule::SetOpenGLDisplayAddress(const unsigned int DisplayBufferPhysicalAddress)
{
	// Nothing to do on emulation target
}
#endif

//----------------------------------------------------------------------------
tDisplayHandle CDisplayModule::CreateHandle(U16 height, U16 width,
                                        tPixelFormat colorDepth, U8 *pBuffer)
{
	tDisplayContext *dc =  new struct tDisplayContext;

	// X window needs pixmap of same color depth as desktop ("24")
	int depth,bpp;
	switch (colorDepth) 
	{
		case kPixelFormatRGB4444:	bpp = 16; depth = 16; break;
		case kPixelFormatRGB565:	bpp = 16; depth = 16; break;
		case kPixelFormatRGB888:	bpp = 24; depth = 24; break; 	
		default:
		case kPixelFormatARGB8888: 	bpp = 32; depth = 24; break;
		case kPixelFormatYUV420:	bpp = 8;  depth = 8 ; break;
		case kPixelFormatYUYV422:	bpp = 16; depth = 16; break;
	}		

	dc->width = width;
	dc->height = height;
	dc->colorDepthFormat = colorDepth;
	dc->depth = bpp;
	dc->bpp = bpp/8;
	dc->pitch = width * dc->bpp;
	dc->pBuffer = pBuffer;
	dc->x = 0;
	dc->y = 0;
	dc->isAllocated = (pBuffer != NULL);
	dc->isOverlay = (colorDepth == kPixelFormatYUYV422);	
	dc->isPlanar = (colorDepth == kPixelFormatYUV420);	
	dc->rect.left = dc->rect.top = 0;
	dc->rect.right = width;
	dc->rect.bottom = height;
	dc->pixmap = 0;
	dc->image = NULL;
	
	// Offscreen context allocated by caller?
	if (dc->isAllocated)
		return reinterpret_cast<tDisplayHandle>(dc);

	// YUV planar format needs double-width surface for U and V buffers
	if (colorDepth == kPixelFormatYUV420)
		width *= 2;
	
	// Bind X pixmap and image data to display context
	Pixmap pixmap = XCreatePixmap(x11Display, x11Window, width, height, depth);
	_XImage* image = XGetImage(x11Display, pixmap, 0, 0, width, height, AllPlanes, ZPixmap);
	dbg_.Assert(image != NULL, "CDisplayModule::CreateHandle: Unable to create X pixmap\n");
	
	dc->pixmap = pixmap;
	dc->image = image;
	if (colorDepth == kPixelFormatARGB8888 || colorDepth == kPixelFormatYUV420) {
		dc->bpp = image->bitmap_unit;
		dc->depth = image->bits_per_pixel;
		dc->pitch = image->bytes_per_line;
	}
	dc->pBuffer = (U8*)image->data;

	memset(dc->pBuffer, 0, width * height);
	
	return reinterpret_cast<tDisplayHandle>(dc);
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::RegisterLayer(tDisplayHandle hndl, S16 xPos, S16 yPos)
{
	struct tDisplayContext* dc = (struct tDisplayContext*)hndl;
	dc->rect.left = dc->x = xPos;
	dc->rect.top  = dc->y = yPos;
	dc->rect.right = dc->rect.left + dc->width;
	dc->rect.bottom = dc->rect.top + dc->height;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::UnRegisterLayer(tDisplayHandle hndl)
{
	// Nothing to do on emulation target
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::Update(tDisplayContext* dc, int sx, int sy, int dx, int dy, int width, int height)
{
	tDisplayContext*	pdcDest = pdcPrimary_;
	tRect			rect = dc->rect;
	
	// Repack YUV format pixmap into ARGB format for X window
	if (dc->isPlanar)
		YUV2ARGB(dc, pdcDest, sx, sy, dx, dy, width, height);
	else if (dc->isOverlay)
		YUYV2ARGB(dc, pdcDest, sx, sy, dx, dy, width, height);
	else if (dc->colorDepthFormat == kPixelFormatRGB888)
		RGB2ARGB(dc, pdcDest, sx, sy, dx, dy, width, height);
	else if (dc->colorDepthFormat == kPixelFormatRGB565)
		RGB565ARGB(dc, pdcDest, sx, sy, dx, dy, width, height);
	else if (dc->colorDepthFormat == kPixelFormatRGB4444)
		RGB4444ARGB(dc, pdcDest, sx, sy, dx, dy, width, height);
	else if (dc->isAllocated)
		ARGB2ARGB(dc, pdcDest, sx, sy, dx, dy, width, height);
	else
		pdcDest = dc;

	// Transfer pixmap to registered destination coordinates, or as-is if primary layer pixmap 
	sx = (pdcDest == pdcPrimary_) ? dx : 0;
	sy = (pdcDest == pdcPrimary_) ? dy  : 0;
	XPutImage(x11Display, x11Window, gc, pdcDest->image, sx, sy, dx, dy, width, height);
	XFlush(x11Display);
	
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::DestroyHandle(tDisplayHandle hndl, Boolean destroyBuffer)
{
	struct tDisplayContext* dc = (struct tDisplayContext*)hndl;
	if (dc == NULL)
		return kNoErr;
	UnRegister(hndl, 0);
	if (dc->image)
		XDestroyImage(dc->image);
	if (dc->pixmap)
		XFreePixmap(x11Display, dc->pixmap);
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
U8 CDisplayModule::GetAlpha(tDisplayHandle hndl) const
{
	return 0;
}

//----------------------------------------------------------------------------
tPixelFormat CDisplayModule::GetPixelFormat(tDisplayHandle hndl) const
{
	struct tDisplayContext* dc = (struct tDisplayContext*)hndl;
	return dc->colorDepthFormat;
}

//----------------------------------------------------------------------------
U16 CDisplayModule::GetPitch(tDisplayHandle hndl) const
{
	struct tDisplayContext* dc = (struct tDisplayContext*)hndl;
	return dc->pitch;
}

//----------------------------------------------------------------------------
U16 CDisplayModule::GetDepth(tDisplayHandle hndl) const
{
	struct tDisplayContext* dc = (struct tDisplayContext*)hndl;
	return dc->depth;
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
	return (U32)((WINDOW_WIDTH<<16)|(WINDOW_HEIGHT));
}

//----------------------------------------------------------------------------
enum tPixelFormat CDisplayModule::GetPixelFormat(void)
{
	return kPixelFormatARGB8888;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::SetBrightness(tDisplayScreen screen, S8 brightness)
{
	// Nothing to do on emulation target
	brightness_ = brightness;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::SetContrast(tDisplayScreen screen, S8 contrast)
{
	// Nothing to do on emulation target
	contrast_ = contrast;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::SetBacklight(tDisplayScreen screen, S8 backlight)
{
	// Nothing to do on emulation target
	backlight_ = backlight;
	return kNoErr;
}

//----------------------------------------------------------------------------
S8	CDisplayModule::GetBrightness(tDisplayScreen screen)
{
	// Nothing to do on emulation target
	return brightness_;
}

//----------------------------------------------------------------------------
S8	CDisplayModule::GetContrast(tDisplayScreen screen)
{
	// Nothing to do on emulation target
	return contrast_;
}

//----------------------------------------------------------------------------
S8	CDisplayModule::GetBacklight(tDisplayScreen screen)
{
	// Nothing to do on emulation target
	return backlight_;
}

LF_END_BRIO_NAMESPACE()
// EOF
