#ifndef LF_BRIO_BUTTONMPI_H
#define LF_BRIO_BUTTONMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		ButtonMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the Button Manager module. 
//
//==============================================================================

#include <CoreMPI.h>
#include <ButtonTypes.h>
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
class CButtonMPI : public ICoreMPI {
public:	
	// core functionality
	virtual	Boolean		IsValid() const;
	virtual tErrType	GetMPIVersion(tVersion &version) const;		   
	virtual tErrType	GetMPIName(ConstPtrCString &pName) const;		
	virtual tErrType	GetModuleVersion(tVersion &version) const;
	virtual tErrType	GetModuleName(ConstPtrCString &pName) const;	
	virtual tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;

	// class-specific functionality
	CButtonMPI();
	virtual ~CButtonMPI();

	// Get button state
	tErrType	GetButtonState(tButtonData& data) const;

private:
	class CButtonModule*	pModule_;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_BUTTONMPI_H

// eof
