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

#include "DisplayHW.h"

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
	tPixelFormat colorDepth;
	U16 pitch;			// based on colorDepth
	U8 *pBuffer;
	S16 x;				// from Register()
	S16 y;
	bool isAllocated;	// toggled by CreateHandle()/DestroyHandle()
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
	VTABLE_EXPORT U16				GetPitch(tDisplayHandle hndl) const;
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
	VTABLE_EXPORT void    			InitOpenGL(void* pCtx);
	VTABLE_EXPORT void    			DeinitOpenGL();
	VTABLE_EXPORT void    			EnableOpenGL();
	VTABLE_EXPORT void    			DisableOpenGL();


private:
	void				InitModule( );
	void				DeInitModule( );
	U32					GetScreenSize( );
	enum tPixelFormat	GetPixelFormat(void);
	tErrType 			RegisterLayer(tDisplayHandle hndl, S16 xPos, S16 yPos);
	void				SetDirtyBit(void);
	CDebugMPI			dbg_;

	// Limit object creation to the Module Manager interface functions
	CDisplayModule();
	virtual ~CDisplayModule();
	friend ICoreModule*	::CreateInstance(tVersion version);
	friend void			::DestroyInstance(ICoreModule*);
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_DISPLAYPRIV_H

// eof
