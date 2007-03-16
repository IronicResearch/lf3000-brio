#ifndef LF_BRIO_COREMPI_H
#define LF_BRIO_COREMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		CoreMPI.h
//
// Description:
//		Defines the ICoreMPI interface class for lightweight module interface
//		objects.  All System, Product & Cartridge interface object classes
//		are derived from ICoreMPI.
//
//==============================================================================

#include <StringTypes.h>

class ICoreMPI {
public:
	ICoreMPI() 	{};
	virtual ~ICoreMPI()	{};

	virtual Boolean		IsValid() const = 0;	
	
	virtual tErrType	GetMPIVersion(tVersion &version) const = 0;		   
	virtual tErrType	GetMPIName(ConstPtrCString &pName) const = 0;		

	virtual tErrType	GetModuleVersion(tVersion &version) const = 0;
	virtual tErrType	GetModuleName(ConstPtrCString &pName) const = 0;	
	virtual tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const = 0;
};

#endif // LF_BRIO_COREMPI_H

// eof
