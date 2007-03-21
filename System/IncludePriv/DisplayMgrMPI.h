#ifndef DISPLAYMGRMPI_H
#define DISPLAYMGRMPI_H

//==============================================================================
// $Source: $
//
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		DisplayMgrMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the System Kernel module. 
//
//==============================================================================

#include "CoreMPI.h"

class CDisplayMgrMPI : public ICoreMPI
{
public:
	// core functionality
	virtual tErrType	Init();			   
	virtual tErrType	DeInit(); 			
	virtual Boolean		IsInited();

	virtual tErrType	GetMPIVersion(tVersion *pVersion);		   
	virtual tErrType	GetMPIName(const CString **ppName);		

	virtual tErrType	GetModuleVersion(tVersion *pVersion);
	virtual tErrType	GetModuleName(const CString **ppName);	
	virtual tErrType	GetModuleOrigin(const CURI **ppURI);

	// class-specific functionality
	CDisplayMgrMPI();
	virtual ~CDisplayMgrMPI();

	tDisplayHandle      CreateHandle(U16 height, U16 width, tPixelFormat colorDepth, U8 *pBuffer = NULL );
	U8 *                GetBuffer (tDisplayHandle tDispHandle);
    U16                 GetPitch (tDisplayHandle tDispHandle);
    U16                 GetHeight(tDisplayHandle tDispHandle);
    U16                 GetWidth(tDisplayHandle tDispHandle);

	tErrType            Register(tDisplayHandle tDispHandle, S16 xPos = 0, S16 yPos = 0, tDisplayHandle insertAfter = 0, 
								 tDisplayScreen screen = kDisplayScreenAllScreens);
	tErrType            Register(tDisplayHandle tDispHandle, S16 xPos = 0, S16 yPos = 0, tDisplayZOrder initialZOrder = tDisplayOnTop, 
                                 tDisplayScreen screen = kDisplayScreenAllScreens);
	tErrType            UnRegister(tDisplayScreen screen, tDisplayHandle tDispHandle);

	tErrType            DestroyHandle(tDisplayHandle tDispHandle, Boolean destroyBuffer);

	tErrType            LockBuffer(tDisplayHandle tDispHandle);
	tErrType            UnlockBuffer(tDisplayHandle tDispHandle, tRect *dirtyRect = NULL);

	tErrType            Invalidate(tDisplayScreen screen, tRect *dirtyRect = NULL);
	U16                 GetNumberOfScreens();
	tDisplayScreenStats *GetScreenStats(tDisplayScreen screen);

private:
	class CDisplayMgrMPIImpl *mpImpl;
};


#endif // DISPLAYMGRMPI_H

// eof
