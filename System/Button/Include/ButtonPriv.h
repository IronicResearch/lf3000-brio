#ifndef LF_BRIO_BUTTONPRIV_H
#define LF_BRIO_BUTTONPRIV_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		ButtonPriv.h
//
// Description:
//		Defines the interface for the private underlying Button manager module. 
//
//==============================================================================

#include <CoreModule.h>
#include "ButtonMPI.h"	// for tButtonData


// Constants
const CString	kButtonModuleName		= "Button";
const tVersion	kButtonModuleVersion	= MakeVersion(0,1);

//==============================================================================
class CButtonModule : public ICoreModule {
public:	
	// core functionality
	virtual Boolean		IsValid() const;
	virtual tErrType	GetModuleVersion(tVersion &version) const;
	virtual tErrType	GetModuleName(ConstPtrCString &pName) const;	
	virtual tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;

	// class-specific functionality
	CButtonModule();
	virtual ~CButtonModule();

	// Get button state
	virtual	tErrType	GetButtonState(tButtonData& data);
};


#endif // LF_BRIO_BUTTONPRIV_H

// eof
