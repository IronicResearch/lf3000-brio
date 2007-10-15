#ifndef LF_BRIO_USBDEVICEPRIV_H
#define LF_BRIO_USBDEVICEPRIV_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		USBDevicePriv.h
//
// Description: Defines the interface for the private underlying USBDevice
// manager module.
//
//==============================================================================
#include <SystemTypes.h>
#include <CoreModule.h>
#include <EventTypes.h>
#include <USBDeviceTypes.h>
#include <DebugMPI.h>
LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Constants
//==============================================================================
const CString			kUSBDeviceModuleName	= "USBDevice";
const tVersion			kUSBDeviceModuleVersion	= 2;
const tEventPriority	kUSBDeviceEventPriority	= 0;	//TBD/tp: make replacable?


//==============================================================================
class CUSBDeviceModule : public ICoreModule {
public:	
	// ICoreModule functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	VTABLE_EXPORT tUSBDeviceData	GetUSBDeviceState() const;
	VTABLE_EXPORT tErrType			EnableUSBDeviceDrivers(U32 drivers);
	VTABLE_EXPORT tErrType			DisableUSBDeviceDrivers(U32 drivers);
    VTABLE_EXPORT U32				GetUSBDeviceWatchdog(void);
    VTABLE_EXPORT void				SetUSBDeviceWatchdog(U32 timerSec);

private:
	void				InitModule( );
	void				DeinitModule();
	CDebugMPI*			pDbg_;

	// Limit object creation to the Module Manager interface functions
	CUSBDeviceModule();
	virtual ~CUSBDeviceModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_USBDEVICEPRIV_H

// eof
