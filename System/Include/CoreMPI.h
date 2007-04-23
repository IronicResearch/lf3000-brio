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

#include <SystemTypes.h>
#include <StringTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


class ICoreMPI {
public:
	ICoreMPI() 	{};
	virtual ~ICoreMPI()	{};

	virtual Boolean			IsValid() const = 0;	
	virtual const CString*	GetMPIName() const = 0;		
	virtual tVersion		GetModuleVersion() const = 0;
	virtual const CString*	GetModuleName() const = 0;	
	virtual const CURI*		GetModuleOrigin() const = 0;

private:
	// Disable copy semantics
	ICoreMPI(const ICoreMPI&);
	ICoreMPI& operator=(const ICoreMPI&);
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_COREMPI_H

// eof
