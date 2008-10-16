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
#include <EventMPI.h>
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

	/// Register handler
	tErrType RegisterEventListener(const IEventListener *pListener);
	
	/// Unregister handler
	tErrType UnregisterEventListener(const IEventListener *pListener);
	
	/// Get current power state which is the same as the last message passing
	/// through the event manager
	enum tPowerState GetPowerState() const;
	
	/// Complete system shutdown.  This is the application's response to the kPowerShutdown
	/// message.  The application calls Shutdown() to indicate data has been saved and
	/// system shutdown can proceed immediately, overriding the shutdown watchdog timer.
	/// Negative number indicates error.
	int		Shutdown() const;
	
	/// Get shutdown time in milliseconds.  This sets the watchdog timer.  Negative number
	/// indicates error.
	int		GetShutdownTimeMS() const;
	
	/// Set shutdown time in milliseconds.  Returns actual watchdog time in milliseconds.
	/// Negative number indicates error.
	int		SetShutdownTimeMS(int) const;
	
	/// Reset system
	int		Reset() const;
	
private:
	class CPowerModule*		pModule_;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_POWERMPI_H

// eof
