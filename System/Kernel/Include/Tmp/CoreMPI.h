
#ifndef COREMPI_H
#define COREMPI_H

//==============================================================================
// $Source: $
//
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		CoreMPI.h
//
// Description:
//		Defines the ICoreMPI interface class 
//		from which all System, Product & Cartridge MPIs are derived. 
//
//==============================================================================


#include <StringTypes.h>


class ICoreMPI {
public:
	ICoreMPI() {};
	virtual ~ICoreMPI(){};

	virtual tErrType	Init() = 0;		
	virtual tErrType	DeInit() = 0;	

	virtual Boolean		IsInited() = 0;	
	
	virtual tErrType	GetMPIVersion(tVersion *pVersion) = 0;		   
	virtual tErrType	GetMPIName(const CString **ppName) = 0;		

	virtual tErrType	GetModuleVersion(tVersion *pVersion) = 0;
	virtual tErrType	GetModuleName(const CString **ppName) = 0;	
	virtual tErrType	GetModuleOrigin(const CURI **ppURI) = 0;
};

#endif // COREMPI_H

// eof






