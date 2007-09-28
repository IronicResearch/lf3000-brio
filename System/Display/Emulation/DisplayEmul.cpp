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
		case kPixelFormatRGB4444:	bpp = 16; depth = 24; break;
		case kPixelFormatRGB565:	bpp = 16; depth = 24; break;
		case kPixelFormatRGB888:	bpp = 24; depth = 24; break; 	
		default:
		case kPixelFormatARGB8888: 	bpp = 32; depth = 24; break;
		case kPixelFormatYUV420:	bpp = 8;  depth = 24; break;
		case kPixelFormatYUYV422:	bpp = 16; depth = 24; break;
	}		

	dc->width = width;
	dc->height = height;
	dc->colorDepthFormat = colorDepth;
	
	// YUV planar format needs double-height surface for U and V buffers
	if (colorDepth == kPixelFormatYUV420)
		height *= 2;
	
	// Bind X pixmap and image data to display context
	Pixmap pixmap = XCreatePixmap(x11Display, x11Window, width, height, depth);
	_XImage* image = XGetImage(x11Display, pixmap, 0, 0, width, height, AllPlanes, ZPixmap);
	
	dc->pixmap = pixmap;
	dc->image = image;
	dc->depth = bpp; // image->bitmap_unit;
	dc->bpp = bpp/8; // image->bits_per_pixel;
	dc->pitch = image->bytes_per_line;
	dc->pBuffer = (U8*)image->data;
	dc->x = 0;
	dc->y = 0;
	dc->isAllocated = (pBuffer != NULL);
	dc->isOverlay = (colorDepth == kPixelFormatYUYV422);	
	dc->isPlanar = (colorDepth == kPixelFormatYUV420);	

	memset(dc->pBuffer, 0, width * height);
	
	dc->rect.left = dc->rect.top = 0;
	dc->rect.right = width;
	dc->rect.bottom = height;
	
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
}

//----------------------------------------------------------------------------
void RGB4444ARGB(tDisplayContext* dc)
{
	// Repack 16bpp RGB4444 format surface into 32bpp ARGB format surface
	U8			buffer[WINDOW_WIDTH*2];
	U8* 		s = &buffer[0];
	U8*			d = reinterpret_cast<U8*>(dc->image->data);
	int			i,j,m,n;
	for (i = 0; i < dc->height; i++) 
	{
		memcpy(s, d, dc->width*2);
		for (j = m = n = 0; j < dc->width; j++, m+=2, n+=4) 
		{
			U16 color = (s[m+1] << 8) | (s[m]);
			d[n+0] = ((color & 0x000F) >> 0) << 4;
			d[n+1] = ((color & 0x00F0) >> 4) << 4;
			d[n+2] = ((color & 0x0F00) >> 8) << 4;
			d[n+3] = ((color & 0xF000) >> 12) << 4;
		}
		d += dc->pitch;
	}
}

//----------------------------------------------------------------------------
void RGB565ARGB(tDisplayContext* dc)
{
	// Repack 16bpp RGB565 format surface into 32bpp ARGB format surface
	U8			buffer[WINDOW_WIDTH*2];
	U8* 		s = &buffer[0];
	U8*			d = reinterpret_cast<U8*>(dc->image->data);
	int			i,j,m,n;
	for (i = 0; i < dc->height; i++) 
	{
		memcpy(s, d, dc->width*2);
		for (j = m = n = 0; j < dc->width; j++, m+=2, n+=4) 
		{
			U16 color = (s[m+1] << 8) | (s[m]);
			d[n+0] = ((color & 0x001F) >> 0) << 3;
			d[n+1] = ((color & 0x07E0) >> 5) << 2;
			d[n+2] = ((color & 0xF800) >> 11) << 3;
			d[n+3] = 0xFF;
		}
		d += dc->pitch;
	}
}

//----------------------------------------------------------------------------
void RGB2ARGB(tDisplayContext* dc)
{
	// Repack 24bpp RGB888 format surface into 32bpp ARGB format surface
	U8			buffer[WINDOW_WIDTH*3];
	U8* 		s = &buffer[0];
	U8*			d = reinterpret_cast<U8*>(dc->image->data);
	int			i,j,m,n;
	for (i = 0; i < dc->height; i++) 
	{
		memcpy(s, d, dc->width*3);
		for (j = m = n = 0; j < dc->width; j++, m+=3, n+=4) 
		{
			d[n+0] = s[m+0];
			d[n+1] = s[m+1];
			d[n+2] = s[m+2];
			d[n+3] = 0xFF;
		}
		d += dc->pitch;
	}
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
// Repack YUV planar format surface into ARGB format surface
inline void YUV2ARGB(tDisplayContext* dc)
{
		// Repack YUV planar format surface into ARGB format surface
		U8			buffer[WINDOW_WIDTH];
		U8* 		s = &buffer[0];
		U8*			d = reinterpret_cast<U8*>(dc->image->data);
		U8*			su = d + dc->image->bytes_per_line * dc->height;
		U8*			sv = d + dc->image->bytes_per_line * dc->height*3/2;
		U8			y,z,u,v;
		int			i,j,m,n;
		for (i = 0; i < dc->height; i++) 
		{
			memcpy(s, d, dc->width);
			for (j = m = n = 0; m < dc->width; j++, m+=2, n+=8) 
			{
				y = s[m+0];
				z = s[m+1];
				u = su[j];
				v = sv[j];
				d[n+0] = B(y,u,v); // y + u;
				d[n+1] = G(y,u,v); // y - u - v;
				d[n+2] = R(y,u,v); // y + v;
				d[n+3] = 0xFF;
				d[n+4] = B(z,u,v); // z + u;
				d[n+5] = G(z,u,v); // z - u - v;
				d[n+6] = R(z,u,v); // z + v;
				d[n+7] = 0xFF;
			}
			d += dc->pitch;
			if (i % 2)
			{
				su += dc->image->bytes_per_line;
				sv += dc->image->bytes_per_line;
			}
		}
}

//----------------------------------------------------------------------------
// Repack YUYV format surface into ARGB format surface
inline void YUYV2ARGB(tDisplayContext* dc)
{
		// Repack YUYV format surface into ARGB format surface
		U8			buffer[WINDOW_WIDTH*2];
		U8* 		s = &buffer[0];
		U8*			d = reinterpret_cast<U8*>(dc->image->data);
		U8			y,z,u,v;
		int			i,j,m,n;
		for (i = 0; i < dc->height; i++) 
		{
			memcpy(s, d, dc->width * 2);
			for (j = m = n = 0; j < dc->width; j+=2, m+=4, n+=8) 
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
			d += dc->pitch;
		}
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::Update(tDisplayContext* dc)
{
	// Repack YUV format pixmap into ARGB format for X window
	if (dc->isPlanar)
		YUV2ARGB(dc);
	else if (dc->isOverlay)
		YUYV2ARGB(dc);
	else if (dc->colorDepthFormat == kPixelFormatRGB888)
		RGB2ARGB(dc);
	else if (dc->colorDepthFormat == kPixelFormatRGB565)
		RGB565ARGB(dc);
	else if (dc->colorDepthFormat == kPixelFormatRGB4444)
		RGB4444ARGB(dc);
		
	XPutImage(x11Display, x11Window, gc, dc->image, 0, 0, dc->rect.left, dc->rect.top, dc->rect.right - dc->rect.left, dc->rect.bottom - dc->rect.top);
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
