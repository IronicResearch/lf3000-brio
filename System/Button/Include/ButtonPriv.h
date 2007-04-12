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

#include <SystemTypes.h>
#include <CoreModule.h>
#include "ButtonMPI.h"	// for tButtonData
LF_BEGIN_BRIO_NAMESPACE()


// Constants
const CString	kButtonModuleName		= "Button";
const tVersion	kButtonModuleVersion	= MakeVersion(0,1);

//==============================================================================
class CButtonModule : public ICoreModule {
public:	
	// ICoreModule functionality
	virtual Boolean		IsValid() const;
	virtual tErrType	GetModuleVersion(tVersion &version) const;
	virtual tErrType	GetModuleName(ConstPtrCString &pName) const;	
	virtual tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;

	// class-specific functionality
	VTABLE_EXPORT tErrType	GetButtonState(tButtonData& data) const;

private:
	// Limit object creation to the Module Manager interface functions
	CButtonModule();
	virtual ~CButtonModule();
	friend ICoreModule*	::CreateInstance(tVersion version);
	friend void			::DestroyInstance(ICoreModule*);
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_BUTTONPRIV_H

// eof
