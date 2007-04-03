#ifndef KERNELPRIVATE_H
#define KERNELPRIVATE_H

//=============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		KernelPrivate.h
//
// Description:
//		Defines the interface for the private underlyingEventMgr module. 
//
//==============================================================================

#include <CoreModule.h>
#include "KernelMPI.h"

// Constants
const CString	kKernelModuleName		= "Kernel";
const tVersion	kKernelModuleVersion	= MakeVersion(0,1);

//==============================================================================
class CKernelModule : public ICoreModule {
public:	
	// core functionality
	virtual Boolean		IsValid() const;
	virtual tErrType	GetModuleVersion(tVersion &version) const;
	virtual tErrType	GetModuleName(ConstPtrCString &pName) const;	
	virtual tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;

	// class-specific functionality
	CKernelModule();
	virtual ~CKernelModule();
};

#endif // #ifndef KERNELPRIVATE_H

// EOF			

