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

struct CKeepAliveDelegate;

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
	
	/// GetPowerParam() gets power parameter, possibly read-only.
	/// \param	param Enumerated power parameter type
	/// \return	Returns power parameter value
	S32		GetPowerParam(tPowerParam param);

	/// SetPowerParam() sets power parameter (unless read-only).
	/// \param	param Enumerated power parameter type
	/// \param	value Power parameter value
	/// \return	Returns true if successful, false otherwise.
	Boolean SetPowerParam(tPowerParam param, S32 value);
	
	/// KeepAlive() keeps the system alive when there are activities.This function is 
	/// called internally by the SDK to keep the system alive when buttons are pushed.
	/// This function can also be called directly by game/application to keep the system 
	/// alive when buttons are not pushed (e.g.: wand games, movies). 
	static void KeepAlive();
		
	/// GetInactivityTimeouts() gets first and second inactivity timeout values.
	/// Currently supported only in Glasgow.
	/// \param	firstInterval First inactivity timeout interval in miliseconds
	/// \param	secondInterval Second inactivity timeout interval in miliseconds	
	static void GetInactivityTimeouts(int& firstInterval, int& secondInterval);
			
private:
	class CPowerModule*		pModule_;
	static struct CKeepAliveDelegate* pKeepAliveDelegate;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_POWERMPI_H

// eof
