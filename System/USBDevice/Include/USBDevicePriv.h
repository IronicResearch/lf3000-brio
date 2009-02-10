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
#include <EventTypes.h>
#include <USBDeviceTypes.h>
LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Constants
//==============================================================================
const CString			kUSBDeviceModuleName	= "USBDevice";
const CURI				kModuleURI				= "/LF/System/USBDevice";
const tVersion			kUSBDeviceModuleVersion	= 3;
const tEventPriority	kUSBDeviceEventPriority	= 0;	//TBD/tp: make replacable?


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_USBDEVICEPRIV_H

// eof
