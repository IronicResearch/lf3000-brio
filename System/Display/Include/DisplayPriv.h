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
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
// Constants
//==============================================================================
const CString			kDisplayModuleName		= "Display";
const tVersion			kDisplayModuleVersion	= 2;
const tEventPriority	kDisplayEventPriority	= 0;


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
	VTABLE_EXPORT const tDisplayScreenStats*	GetScreenStats(tDisplayScreen screen) const;
	VTABLE_EXPORT tErrType						Invalidate(tDisplayScreen screen, 
															tRect *pDirtyRect);

	// Graphics contexts (through tDisplayHandle)
	VTABLE_EXPORT tDisplayHandle	CreateHandle(U16 height, U16 width, 
												tPixelFormat colorDepth, U8 *pBuffer);
	VTABLE_EXPORT U8*				GetBuffer(tDisplayHandle hndl) const;
//	VTABLE_EXPORT U16				GetPitch(tDisplayHandle hndl) const;
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

private:
	void				InitModule( );
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
