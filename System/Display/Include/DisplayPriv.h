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

//==============================================================================
// Typedefs
//==============================================================================
struct tDisplayContext {
	U16 width;			// from CreateHandle
	U16 height;
	tPixelFormat colorDepthFormat; // pixel format enum
	U32 format;			// cached HW format code
	U16	depth;			// bits per pixel
	U16	bpp;			// bytes per pixel
	U16 pitch;			// bytes per line
	U8 *pBuffer;
	S16 x;				// from Register()
	S16 y;
	bool isAllocated;	// toggled by CreateHandle()/DestroyHandle()
	bool isOverlay;		// video overlay layer?
	bool isPlanar;		// video overlay planar?
#ifdef EMULATION
	Pixmap	pixmap;		// X offscreen pixmap
	_XImage	*image;		// X pixmap internals
	tRect	rect;		// active rect from Register()
#endif
	void*	pdc;		// next dc in list
};

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

private:
	void				InitModule( );
	void				DeInitModule( );
	U32					GetScreenSize( );
	enum tPixelFormat	GetPixelFormat(void);
	tErrType 			RegisterLayer(tDisplayHandle hndl, S16 xPos, S16 yPos);
	void				SetDirtyBit(int layer);
	tErrType			Update(tDisplayContext* dc);
	CDebugMPI			dbg_;

	// Limit object creation to the Module Manager interface functions
	CDisplayModule();
	virtual ~CDisplayModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_DISPLAYPRIV_H

// eof
