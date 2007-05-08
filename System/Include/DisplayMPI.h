#ifndef LF_BRIO_DISPLAYMPI_H
#define LF_BRIO_DISPLAYMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		DisplayMPI.h
//
// Description:
//		Defines the interface for the private underlying Display manager module. 
//
//==============================================================================
#include <CoreMPI.h>
#include <DisplayTypes.h>
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
class CDisplayMPI : public ICoreMPI {
public:	
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;		
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	CDisplayMPI();
	virtual ~CDisplayMPI();

	// Screen functionality
	U16							GetNumberOfScreens() const;
	const tDisplayScreenStats*	GetScreenStats(tDisplayScreen screen) const;
	tErrType					Invalidate(tDisplayScreen screen, tRect *pDirtyRect = NULL);

	// Graphics contexts (through tDisplayHandle)
	tDisplayHandle      CreateHandle(U16 height, U16 width, tPixelFormat colorDepth, 
									U8 *pBuffer = NULL);
	U8*                 GetBuffer(tDisplayHandle hndl) const;
//	U16                 GetPitch(tDisplayHandle hndl) const;
	U16                 GetHeight(tDisplayHandle hndl) const;
	U16                 GetWidth(tDisplayHandle hndl) const;
	
	tErrType            Register(tDisplayHandle hndl, S16 xPos = 0, S16 yPos = 0, 
								 tDisplayHandle insertAfter = 0, 
								 tDisplayScreen screen = kDisplayScreenAllScreens);
	tErrType            Register(tDisplayHandle hndl, S16 xPos = 0, S16 yPos = 0, 
								 tDisplayZOrder initialZOrder = kDisplayOnTop, 
	                             tDisplayScreen screen = kDisplayScreenAllScreens);
	tErrType            UnRegister(tDisplayHandle hndl, tDisplayScreen screen);
	
	tErrType            DestroyHandle(tDisplayHandle hndl, Boolean destroyBuffer);
	
	tErrType            LockBuffer(tDisplayHandle hndl);
	tErrType            UnlockBuffer(tDisplayHandle hndl, tRect *pDirtyRect = NULL);
	
private:
	class CDisplayModule*	pModule_;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_DISPLAYMPI_H

// eof
