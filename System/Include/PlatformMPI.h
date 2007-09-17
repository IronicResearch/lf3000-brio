#ifndef LF_BRIO_PLATFOMMPI_H
#define LF_BRIO_PLATFOMMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		PlatformMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the Platform Manager module. 
//
//==============================================================================

#include <CoreMPI.h>
#include <PlatformTypes.h>
#include <EventMPI.h>
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
class CPlatformMPI : public ICoreMPI {
public:	
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;		
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	CPlatformMPI();
	virtual ~CPlatformMPI();

	// register handler
	tErrType RegisterEventListener(const IEventListener *pListener);
	
	// unregister handler
	tErrType UnregisterEventListener(const IEventListener *pListener);
	
	// Get platform state
	tPlatformData	GetPlatformState() const;

private:
	class CPlatformModule*	pModule_;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_PLATFOMMPI_H

// eof
