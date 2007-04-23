#ifndef LF_BRIO_DISPLAYPRIV_H
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

	// Get button state
	tErrType	GetDisplayDimensions(U16& width, U16& height) const;

private:
	class CDisplayModule*	pModule_;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_DISPLAYMPI_H

// eof
