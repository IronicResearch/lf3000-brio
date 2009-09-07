#ifndef LF_BRIO_DISPLAYPRIV_H
#define LF_BRIO_DISPLAYPRIV_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		DisplayPriv.h
//
// Description:
//		Defines the interface for the private underlying Display manager module. 
//
//==============================================================================
#include <SystemTypes.h>
#include <CoreModule.h>
#include <EventTypes.h>
#include <DisplayTypes.h>
#include <DebugMPI.h>
#include <EventMPI.h>
#include <KernelMPI.h>
#include <string.h>

#ifdef EMULATION
#include <X11/Xlib.h>
#else
#include "DisplayHW.h"
#endif

LF_BEGIN_BRIO_NAMESPACE()



//==============================================================================
// Constants
//==============================================================================
const CString			kDisplayModuleName		= "Display";
const tVersion			kDisplayModuleVersion	= 2;
const tEventPriority	kDisplayEventPriority	= 0;
const tDebugLevel		kDisplayDebugLevel		= kDbgLvlCritical;

//==============================================================================
// Typedefs
//==============================================================================
struct tDisplayContext {
	U16 	width;		// from CreateHandle
	U16 	height;
	tPixelFormat colorDepthFormat; // pixel format enum
	U32 	format;		// cached HW format code
	U16		depth;		// bits per pixel
	U16		bpp;		// bytes per pixel
	U16 	pitch;		// bytes per line
	U8 		*pBuffer;
	S16 	x;			// from Register()
	S16 	y;
	U32		offset;		// offset bytes
	U32		basephys;	// base physical address
	U32		baselinear;	// base linear address
	bool	isPrimary;	// primary visible surface?
	bool 	isAllocated;// allocated by caller?
	bool 	isOverlay;	// video overlay layer?
	bool 	isPlanar;	// video overlay planar?
	bool	isUnderlay;	// layer is on bottom instead of top?
	int		layer;		// layer device handle
#ifdef EMULATION
	Pixmap	pixmap;		// X offscreen pixmap
	_XImage	*image;		// X pixmap internals
#endif
	tRect	rect;		// active rect from Register()
	void*	pdc;		// next dc in list
};

struct tBuffer {
	U32		length;		// size of buffer aligned
	U32		offset;		// offset of buffer from start
	U32		aligned;	// aligned to block boundary
};

//----------------------------------------------------------------------------
inline void RGB4444ARGB(tDisplayContext* sdc, tDisplayContext* ddc, int sx, int sy, int dx, int dy, int width, int height)
{
	// Repack 16bpp RGB4444 format surface into 32bpp ARGB format surface
	U8*			s = sdc->pBuffer + sy * sdc->pitch + sx * 2;
	U8*			d = ddc->pBuffer + dy * ddc->pitch + dx * 4;
	int			i,j,m,n;
	for (i = 0; i < height; i++) 
	{
		for (j = m = n = 0; j < width; j++, m+=2, n+=4) 
		{
			U16 color = (s[m+1] << 8) | (s[m]);
			d[n+0] = ((color & 0x000F) >> 0) << 4;
			d[n+1] = ((color & 0x00F0) >> 4) << 4;
			d[n+2] = ((color & 0x0F00) >> 8) << 4;
			d[n+3] = ((color & 0xF000) >> 12) << 4;
		}
		s += sdc->pitch;
		d += ddc->pitch;
	}
}

//----------------------------------------------------------------------------
inline void ARGB2RGB4444(tDisplayContext* sdc, tDisplayContext* ddc, int sx, int sy, int dx, int dy, int width, int height)
{
	// Repack 32bpp ARGB format surface into 16bpp RGB4444 format surface
	U8*			s = sdc->pBuffer + sy * sdc->pitch + sx * 4;
	U8*			d = ddc->pBuffer + dy * ddc->pitch + dx * 2;
	int			i,j,m,n;
	for (i = 0; i < height; i++) 
	{
		for (j = m = n = 0; j < width; j++, m+=4, n+=2) 
		{
			U8 b = s[m+0] >> 4;
			U8 g = s[m+1] >> 4;
			U8 r = s[m+2] >> 4;
			U8 a = s[m+3] >> 4;
			// U16 color = (a << 12) | (r << 8) | (g << 4) | (b << 0);
			d[n+0] = (g << 4) | b;
			d[n+1] = (a << 4) | r;
		}
		s += sdc->pitch;
		d += ddc->pitch;
	}
}

//----------------------------------------------------------------------------
inline void RGB565ARGB(tDisplayContext* sdc, tDisplayContext* ddc, int sx, int sy, int dx, int dy, int width, int height)
{
	// Repack 16bpp RGB565 format surface into 32bpp ARGB format surface
	U8*			s = sdc->pBuffer + sy * sdc->pitch + sx * 2;
	U8*			d = ddc->pBuffer + dy * ddc->pitch + dx * 4;
	int			i,j,m,n;
	for (i = 0; i < height; i++) 
	{
		for (j = m = n = 0; j < width; j++, m+=2, n+=4) 
		{
			U16 color = (s[m+1] << 8) | (s[m]);
			d[n+0] = ((color & 0x001F) >> 0) << 3;
			d[n+1] = ((color & 0x07E0) >> 5) << 2;
			d[n+2] = ((color & 0xF800) >> 11) << 3;
			d[n+3] = 0xFF;
		}
		s += sdc->pitch;
		d += ddc->pitch;
	}
}

//----------------------------------------------------------------------------
inline void ARGB2RGB565(tDisplayContext* sdc, tDisplayContext* ddc, int sx, int sy, int dx, int dy, int width, int height)
{
	// Repack 32bpp ARGB format surface into 16bpp RGB565 format surface
	U8*			s = sdc->pBuffer + sy * sdc->pitch + sx * 4;
	U8*			d = ddc->pBuffer + dy * ddc->pitch + dx * 2;
	int			i,j,m,n;
	for (i = 0; i < height; i++) 
	{
		for (j = m = n = 0; j < width; j++, m+=4, n+=2) 
		{
			U8 b = s[m+0] >> 3;
			U8 g = s[m+1] >> 2;
			U8 r = s[m+2] >> 3;
			U8 a = s[m+3];
			U16 color = (r << 11) | (g << 5) | (b << 0);
			d[n+0] = color & 0xFF;
			d[n+1] = color >> 8;
		}
		s += sdc->pitch;
		d += ddc->pitch;
	}
}

//----------------------------------------------------------------------------
inline void RGB2ARGB(tDisplayContext* sdc, tDisplayContext* ddc, int sx, int sy, int dx, int dy, int width, int height)
{
	// Repack 24bpp RGB888 format surface into 32bpp ARGB format surface
	U8*			s = sdc->pBuffer + sy * sdc->pitch + sx * 3;
	U8*			d = ddc->pBuffer + dy * ddc->pitch + dx * 4;
	int			i,j,m,n;
	for (i = 0; i < height; i++) 
	{
		for (j = m = n = 0; j < width; j++, m+=3, n+=4) 
		{
			d[n+0] = s[m+0];
			d[n+1] = s[m+1];
			d[n+2] = s[m+2];
			d[n+3] = 0xFF;
		}
		s += sdc->pitch;
		d += ddc->pitch;
	}
}

//----------------------------------------------------------------------------
inline void ARGB2RGB(tDisplayContext* sdc, tDisplayContext* ddc, int sx, int sy, int dx, int dy, int width, int height)
{
	// Repack 32bpp ARGB format surface into 24bpp RGB format surface
	U8*			s = sdc->pBuffer + sy * sdc->pitch + sx * 4;
	U8*			d = ddc->pBuffer + dy * ddc->pitch + dx * 3;
	int			i,j,m,n;
	for (i = 0; i < height; i++) 
	{
		for (j = m = n = 0; j < width; j++, m+=4, n+=3) 
		{
			d[n+0] = s[m+0];
			d[n+1] = s[m+1];
			d[n+2] = s[m+2];
		}
		s += sdc->pitch;
		d += ddc->pitch;
	}
}

//----------------------------------------------------------------------------
inline void ARGB2ARGB(tDisplayContext* sdc, tDisplayContext* ddc, int sx, int sy, int dx, int dy, int width, int height)
{
	// Copy 32bpp ARGB8888 format surface into 32bpp ARGB format surface
	U8*			s = sdc->pBuffer + sy * sdc->pitch + sx * 4;
	U8*			d = ddc->pBuffer + dy * ddc->pitch + dx * 4;
	for (int i = 0; i < height; i++) 
	{
		memcpy(d, s, 4 * width);
		s += sdc->pitch;
		d += ddc->pitch;
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

// RGB to YUV color conversion:  <http://en.wikipedia.org/wiki/YUV>
inline	U8 scale(S16 x)			{ return static_cast<U8>((x + 128) >> 8); }
inline	U8 Y(U8 R,U8 G,U8 B)	{ return scale( 66 * R + 129 * G +  25 * B) +  16; }
inline	U8 U(U8 R,U8 G,U8 B)	{ return scale(-38 * R -  74 * G + 112 * B) + 128; }
inline	U8 V(U8 R,U8 G,U8 B)	{ return scale(112 * R -  94 * G -  18 * B) + 128; }

//----------------------------------------------------------------------------
// Repack RGB format surface into YUV planar format surface
inline void RGB2YUV(tDisplayContext* sdc, tDisplayContext* ddc, int sx, int sy, int dx, int dy, int width, int height)
{
	// Repack RGB format surface into YUV planar format surface
	U8*			s = sdc->pBuffer + sy * sdc->pitch + sx * 4;
	U8*			d = ddc->pBuffer + dy * ddc->pitch + dx * 1;
	U8*			du = d + ddc->pitch/2; // U,V in double-width buffer
	U8*			dv = d + ddc->pitch/2 + ddc->pitch * ddc->height/2;
	U8			r,g,b,a;
	int			i,j,m,n;
	for (i = 0; i < height; i++) 
	{
		for (j = m = n = 0; j < width; j++, m+=4) 
		{
			b = s[m+0];
			g = s[m+1];
			r = s[m+2];
			a = s[m+3];
			d[j] = Y(r,g,b);
			du[n] = U(r,g,b);
			dv[n] = V(r,g,b);
			if (j % 2)
				n++;
		}
		s += sdc->pitch;
		d += ddc->pitch;
		if (i % 2)
		{
			du += ddc->pitch;
			dv += ddc->pitch;
		}
	}
}

//----------------------------------------------------------------------------
// Repack YUV planar format surface into ARGB format surface
inline void YUV2ARGB(tDisplayContext* sdc, tDisplayContext* ddc, int sx, int sy, int dx, int dy, int width, int height)
{
	// Repack YUV planar format surface into ARGB format surface
	U8*			s = sdc->pBuffer + sy * sdc->pitch + sx * 1;
	U8*			d = ddc->pBuffer + dy * ddc->pitch + dx * 4;
	U8*			su = s + sdc->pitch/2; // U,V in double-width buffer
	U8*			sv = s + sdc->pitch/2 + sdc->pitch * sdc->height/2;
	U8			y,z,u,v;
	int			i,j,m,n;
	for (i = 0; i < height; i++) 
	{
		for (j = m = n = 0; m < width; j++, m+=2, n+=8) 
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
		s += sdc->pitch;
		d += ddc->pitch;
		if (i % 2)
		{
			su += sdc->pitch;
			sv += sdc->pitch;
		}
	}
}

//----------------------------------------------------------------------------
// Repack YUYV format surface into ARGB format surface
inline void YUYV2ARGB(tDisplayContext* sdc, tDisplayContext* ddc, int sx, int sy, int dx, int dy, int width, int height)
{
	// Repack YUYV format surface into ARGB format surface
	U8*			s = sdc->pBuffer + sy * sdc->pitch + sx * 2;
	U8*			d = ddc->pBuffer + dy * ddc->pitch + dx * 4;
	U8			y,z,u,v;
	int			i,j,m,n;
	for (i = 0; i < height; i++) 
	{
		for (j = m = n = 0; j < width; j+=2, m+=4, n+=8) 
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
		s += sdc->pitch;
		d += ddc->pitch;
	}
}

//----------------------------------------------------------------------------
// Repack YUYV packed format surface into YUV planar format surface
inline void YUYV2YUV(tDisplayContext* sdc, tDisplayContext* ddc, int sx, int sy, int dx, int dy, int width, int height)
{
	// Repack YUYV packed format surface into YUV planar format surface
	U8*			s = sdc->pBuffer + sy * sdc->pitch + sx * 2;
	U8*			d = ddc->pBuffer + dy * ddc->pitch + dx * 1;
	U8*			du = d + ddc->pitch/2; // U,V in double-width buffer
	U8*			dv = d + ddc->pitch/2 + ddc->pitch * ddc->height/2;
	U8			y,z,u,v;
	int			i,j,m,n;
	for (i = 0; i < height; i++) 
	{
		for (j = m = n = 0; j < width; j+=2, m+=4, n++) 
		{
			y = s[m+0];
			u = s[m+1];
			z = s[m+2];
			v = s[m+3];
			d[j+0] = y;
			d[j+1] = z;
			du[n] = u;
			dv[n] = v;
		}
		s += sdc->pitch;
		d += ddc->pitch;
		if (i % 2)
		{
			du += ddc->pitch;
			dv += ddc->pitch;
		}
	}
}

//==============================================================================
class CDisplayModule : public ICoreModule {
public:	
	// ICoreModule functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// Screen functionality
	VTABLE_EXPORT U16							GetNumberOfScreens() const;
	VTABLE_EXPORT const tDisplayScreenStats*	GetScreenStats(tDisplayScreen screen);
	VTABLE_EXPORT tErrType						Invalidate(tDisplayScreen screen, 
															tRect *pDirtyRect);

	// Graphics contexts (through tDisplayHandle)
	VTABLE_EXPORT tDisplayHandle	CreateHandle(U16 height, U16 width, 
												tPixelFormat colorDepth, U8 *pBuffer);
	VTABLE_EXPORT U8*				GetBuffer(tDisplayHandle hndl) const;
	VTABLE_EXPORT tPixelFormat		GetPixelFormat(tDisplayHandle hndl) const;
	VTABLE_EXPORT U16				GetPitch(tDisplayHandle hndl) const;
	VTABLE_EXPORT U16				GetDepth(tDisplayHandle hndl) const;
	VTABLE_EXPORT U16				GetHeight(tDisplayHandle hndl) const;
	VTABLE_EXPORT U16 				GetWidth(tDisplayHandle hndl) const;
	
	VTABLE_EXPORT tErrType			Register(tDisplayHandle hndl, S16 xPos, S16 yPos, 
											tDisplayHandle insertAfter, 
											tDisplayScreen screen);
	VTABLE_EXPORT tErrType			Register(tDisplayHandle hndl, S16 xPos, S16 yPos, 
											tDisplayZOrder initialZOrder, 
											tDisplayScreen screen);
	VTABLE_EXPORT tErrType			UnRegister(tDisplayHandle hndl, tDisplayScreen screen);
	
	VTABLE_EXPORT tErrType			DestroyHandle(tDisplayHandle hndl, Boolean destroyBuffer);
	
	VTABLE_EXPORT tErrType			LockBuffer(tDisplayHandle hndl);
	VTABLE_EXPORT tErrType			UnlockBuffer(tDisplayHandle hndl, tRect *pDirtyRect);
	VTABLE_EXPORT tErrType 			SetAlpha(tDisplayHandle hndl, U8 level,
														Boolean enable=true);
	VTABLE_EXPORT U8 				GetAlpha(tDisplayHandle hndl) const;
	VTABLE_EXPORT void    			InitOpenGL(void* pCtx);
	VTABLE_EXPORT void    			DeinitOpenGL();
	VTABLE_EXPORT void    			EnableOpenGL(void* pCtx);
	VTABLE_EXPORT void    			DisableOpenGL();
	VTABLE_EXPORT void    			UpdateOpenGL();
	
	VTABLE_EXPORT tErrType			SetBrightness(tDisplayScreen screen, S8 brightness);
	VTABLE_EXPORT tErrType			SetContrast(tDisplayScreen screen, S8 contrast);
	VTABLE_EXPORT S8			GetBrightness(tDisplayScreen screen);
	VTABLE_EXPORT S8			GetContrast(tDisplayScreen screen);
#ifdef LF1000
	VTABLE_EXPORT void			WaitForDisplayAddressPatched(void);
	VTABLE_EXPORT void			SetOpenGLDisplayAddress(const unsigned int DisplayBufferPhysicalAddress);
#endif
	VTABLE_EXPORT tErrType		SetBacklight(tDisplayScreen screen, S8 backlight);
	VTABLE_EXPORT S8			GetBacklight(tDisplayScreen screen);

	VTABLE_EXPORT tErrType			SwapBuffers(tDisplayHandle hndl, Boolean waitVSync);
	VTABLE_EXPORT Boolean			IsBufferSwapped(tDisplayHandle hndl);
	VTABLE_EXPORT tDisplayHandle	GetCurrentDisplayHandle();
	VTABLE_EXPORT U32				GetDisplayMem(tDisplayMem memtype);

private:
	void				InitModule( );
	void				DeInitModule( );
	U32					GetScreenSize( );
	enum tPixelFormat	GetPixelFormat(void);
	tErrType 			RegisterLayer(tDisplayHandle hndl, S16 xPos, S16 yPos);
	tErrType 			UnRegisterLayer(tDisplayHandle hndl);
	void				SetDirtyBit(int layer);
	tErrType			Update(tDisplayContext* dc, int sx, int sy, int dx, int dy, int width, int height);
	CDebugMPI			dbg_;
	CKernelMPI			kernel_;
	CEventMPI			eventmgr_;
	tDisplayContext*	pdcPrimary_;
	tDisplayContext*	pdcVisible_;
	bool				isOpenGLEnabled_;
	bool				isLayerSwapped_;

	// Limit object creation to the Module Manager interface functions
	CDisplayModule();
	virtual ~CDisplayModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));

#ifndef EMULATION	
	bool				AllocBuffer(tDisplayContext* pdc, U32 aligned);
	bool				DeAllocBuffer(tDisplayContext* pdc);
#endif
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_DISPLAYPRIV_H

// eof
