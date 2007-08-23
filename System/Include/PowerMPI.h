#ifndef LF_BRIO_POWERMPI_H
#define LF_BRIO_POWERMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		PowerMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the Power Manager module. 
//
//==============================================================================

#include <CoreMPI.h>
#include <PowerTypes.h>
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
class CPowerMPI : public ICoreMPI {
public:	
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;		
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	CPowerMPI();
	virtual ~CPowerMPI();

	// Get power state
	tPowerData		GetPowerState() const;

private:
	class CPowerModule*		pModule_;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_POWERMPI_H

// eof
