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

	// Activate USB Device Drivers.  The drivers argument is a bit mask that
	// says which drivers should be activated. The underlying implementation may
	// or may not support multiple USB drivers simultaneously. Also, underlying
	// implementations may not all support the same drivers.  If the user
	// attempts to activate too many drivers, or to activate an unsupported
	// driver, this function returns a suitable error.  In this case, the actual
	// USB behavior is undefined, but the user can call GetUSBDeviceState to
	// determine what actually happend.
	tErrType ActivateUSBDeviceDrivers(U32 drivers);

 	// DeactivateUSBDeviceDrivers is the opposite of ActivateUSBDeviceDrivers
	tErrType DeactivateUSBDeviceDrivers(U32 drivers);

private:
	class CUSBDeviceModule*	pModule_;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_USBDEVICEMPI_H

// eof
