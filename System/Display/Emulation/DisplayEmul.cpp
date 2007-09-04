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
	struct tDisplayContext*	dc_;
	GC						gc;
	Pixmap					pixmap;
	_XImage*				image;
	tRect					rect;
	S8						brightness_;
	S8						contrast_;
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
void CDisplayModule::EnableOpenGL()
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
	dc->colorDepthFormat = colorDepth;
	dc->depth = depth;
	dc->bpp = depth/8;

	if (colorDepth == kPixelFormatYUV420)
		height *= 2;
	
	// TODO/dm: Bind these to display context
	/* Pixmap */ pixmap = XCreatePixmap(x11Display, x11Window, width, height, depth);
	/* _XImage* */ image = XGetImage(x11Display, pixmap, 0, 0, width, height, AllPlanes, ZPixmap);
	
	dc->depth = image->bitmap_unit;
	dc->bpp = image->bits_per_pixel;
	dc->pitch = image->bytes_per_line;
	dc->pBuffer = (U8*)image->data;
	dc->x = 0;
	dc->y = 0;
	dc->isAllocated = true;
	dc->isOverlay = (colorDepth == kPixelFormatYUYV422);	
	dc->isPlanar = (colorDepth == kPixelFormatYUV420);	
	dc_ = dc;

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
// YUV to RGB color conversion: <http://en.wikipedia.org/wiki/YUV>
inline 	U8 clip(S16 X)			{ return (X < 0) ? 0 : (X > 255) ? 255 : static_cast<U8>(X); }
inline	S16 C(U8 Y)  			{ return (Y - 16); }
inline 	S16 D(U8 U)  			{ return (U - 128); }
inline 	S16 E(U8 V)  			{ return (V - 128); }
inline 	U8 R(U8 Y,U8 U,U8 V)	{ return clip(( 298 * C(Y)              + 409 * E(V) + 128) >> 8); }
inline 	U8 G(U8 Y,U8 U,U8 V)	{ return clip(( 298 * C(Y) - 100 * D(U) - 208 * E(V) + 128) >> 8); }
inline 	U8 B(U8 Y,U8 U,U8 V)	{ return clip(( 298 * C(Y) + 516 * D(U)              + 128) >> 8); }

//----------------------------------------------------------------------------
tErrType CDisplayModule::Invalidate(tDisplayScreen screen, tRect *pDirtyRect)
{
	if (dc_->isPlanar)
	{
		// Repack YUV planar format surface into ARGB format surface
		U8			buffer[WINDOW_WIDTH];
		U8* 		s = &buffer[0];
		U8*			d = reinterpret_cast<U8*>(image->data);
		U8*			su = d + image->bytes_per_line * image->height;
		U8*			sv = d + image->bytes_per_line * image->height*3/2;
		U8			y,z,u,v;
		int			i,j,m,n;
		for (i = 0; i < dc_->height; i++) 
		{
			memcpy(s, d, dc_->width);
			for (j = m = n = 0; m < dc_->width; j++, m+=2, n+=8) 
			{
				y = s[m+0];
				z = s[m+1];
				u = 0x7f; //su[j];
				v = 0x7f; //sv[j];
				d[n+0] = B(y,u,v); // y + u;
				d[n+1] = G(y,u,v); // y - u - v;
				d[n+2] = R(y,u,v); // y + v;
				d[n+3] = 0xFF;
				d[n+4] = B(z,u,v); // z + u;
				d[n+5] = G(z,u,v); // z - u - v;
				d[n+6] = R(z,u,v); // z + v;
				d[n+7] = 0xFF;
			}
			d += dc_->pitch;
			if (i % 2)
			{
				su += image->bytes_per_line;
				sv += image->bytes_per_line;
			}
		}
	}
	else if (dc_->isOverlay)
	{
		// Repack YUYV format surface into ARGB format surface
		U8			buffer[WINDOW_WIDTH*2];
		U8* 		s = &buffer[0];
		U8*			d = reinterpret_cast<U8*>(image->data);
		U8			y,z,u,v;
		int			i,j,m,n;
		for (i = 0; i < dc_->height; i++) 
		{
			memcpy(s, d, dc_->width * 2);
			for (j = m = n = 0; j < dc_->width; j+=2, m+=4, n+=8) 
			{
				y = s[m+0];
				u = s[m+1];
				z = s[m+2];
				v = s[m+3];
				d[n+0] = B(y,u,v); // y + u;
				d[n+1] = G(y,u,v); // y - u - v;
				d[n+2] = R(y,u,v); // y + v;
				d[n+3] = 0xFF;
				d[n+4] = B(z,u,v); // z + u;
				d[n+5] = G(z,u,v); // z - u - v;
				d[n+6] = R(z,u,v); // z + v;
				d[n+7] = 0xFF;
			}
			d += dc_->pitch;
		}
	}
	
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
	if (dc_ == dc)
 		dc_ = NULL;
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

LF_END_BRIO_NAMESPACE()
// EOF
