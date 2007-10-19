#ifndef LF_BRIO_USBDEVICEMPI_H
#define LF_BRIO_USBDEVICEMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		USBDeviceMPI.h
//
// Description: Defines the Module Public Interface (MPI) for the USBDevice
// Manager module.
//
// Note: USB behavior is required to always operate.  The reason is that it is
// critical for system restore.  So, if an app is misbehaving because it has
// been corrupted, etc., USB must still work.  To ensure this, a watchdog timer
// is launched whenever the USB is plugged into a host.  As usual, this will
// generate an event.  The application must then prepare itself for connection
// to the host and either call Shutdown or EnableUSBDeviceDrivers.  If the
// application fails to call EnableUSBDeviceDrivers or Shutdown before the
// watchdog timer expires, the system will reset and the application will die.
// The watchdog timer is set to a sensible default when the application is
// launched.  Its value can be queried and set using GetUSBDeviceWatchdog and
// SetUSBDeviceWatchdog.
//
// On some systems, enabling some drivers requires that no application is
// running.  On such systems, EnableUSBDeviceDrivers is a one-way function when
// called with a driver that is supported but requires that no application is
// running.
//
//==============================================================================

#include <CoreMPI.h>
#include <USBDeviceTypes.h>
#include <EventMPI.h>
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
class CUSBDeviceMPI : public ICoreMPI {
public:	
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;		
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	CUSBDeviceMPI();
	virtual ~CUSBDeviceMPI();
	
	// register handler
	tErrType RegisterEventListener(const IEventListener *pListener);
	// unregister handler
	tErrType UnregisterEventListener(const IEventListener *pListener);
	
	// Get usb device state.  The USBDeviceData returned can be inspected to
	// determine which drivers are supported (i.e., Mass Storage, HID).	 Also,
	// it can tell you the current state of the USB connection (i.e., is it
	// enabled or not, is it enabled or not).  If the USB Device is enabled, the
	// USBDeviceData also tells you which driver(s) are currently active.  See
	// USBDeviceTypes for details on the USBDeviceData structure.
	tUSBDeviceData GetUSBDeviceState() const;

	// Enable USB Device Drivers.  The drivers argument is a bit mask that says
	// which drivers should be enabled. The underlying implementation may or may
	// not support multiple USB drivers simultaneously. Also, underlying
	// implementations may not all support the same drivers.  If the user
	// attempts to enable too many drivers, or to enable an unsupported driver,
	// this function returns a suitable error.  In this case, the actual USB
	// behavior is undefined, but the user can call GetUSBDeviceState to
	// determine what actually happend.
    // 
    // Some drivers on some systems require that no application is running when
    // USB is enabled.  For these drivers, EnableUSBDeviceDrivers never returns.
	tErrType EnableUSBDeviceDrivers(U32 drivers);

 	// DisableUSBDeviceDrivers is the opposite of EnableUSBDeviceDrivers
	tErrType DisableUSBDeviceDrivers(U32 drivers);

    // Get the current value in seconds of the watchdog timer.
    // kUSBDeviceInvalidWatchdog is returned if an error occurs.
    U32 GetUSBDeviceWatchdog(void);

    // Set the watchdog to timerSec.  If timerSec is 0, the watchdog is
    // disabled.
    void SetUSBDeviceWatchdog(U32 timerSec);

private:
	class CUSBDeviceModule*	pModule_;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_USBDEVICEMPI_H

// eof
