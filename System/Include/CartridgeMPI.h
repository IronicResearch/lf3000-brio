#ifndef LF_BRIO_CARTRIDGEMPI_H
#define LF_BRIO_CARTRIDGEMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		CartridgeMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the Cartridge Manager module. 
//
//==============================================================================

#include <CoreMPI.h>
#include <CartridgeTypes.h>
#include <EventMPI.h>
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
class CCartridgeMPI : public ICoreMPI {
public:	
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;		
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	CCartridgeMPI();
	virtual ~CCartridgeMPI();

	/// Register handler
	tErrType RegisterEventListener(const IEventListener *pListener);
	
	/// Unregister handler
	tErrType UnregisterEventListener(const IEventListener *pListener);
	
	/// Get current power state which is the same as the last message passing
	/// through the event manager
	tCartridgeData GetCartridgeState() const;
	
private:
	class CCartridgeModule*		pModule_;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_CARTRIDGEMPI_H

// eof
