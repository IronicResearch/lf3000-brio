#ifndef LF_BRIO_USBDEVICETYPES_H
#define LF_BRIO_USBDEVICETYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		USBDeviceTypes.h
//
// Description:
//		Defines types for the USBDevice Manager module. 
//
//==============================================================================

#include <EventMessage.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================	   
// USBDevice Manager events
//==============================================================================
#define USBDEVICE_EVENTS				\
	(kUSBDeviceStateChange)				\
	(kUSBDevicePriorityStateChange)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupUSBDevice), USBDEVICE_EVENTS)

const tEventType kAllUSBDeviceEvents = AllEvents(kGroupUSBDevice);

//==============================================================================	   
// USBDevice Manager types
//==============================================================================	   

struct tUSBDeviceData {
	U32	USBDeviceState;		//Bit mask describing USB device state
	U32	USBDeviceDriver;	//Bit mask describing which device driver(s) are
                            //currently active.
	U32	USBDeviceSupports;	//Bit mask describing which device driver(s) are
                            //supported.
};

// Bit definitions for USBDeviceState
const U32 kUSBDeviceConnected	= (1 << 0);
const U32 kUSBDeviceEnabled		= (1 << 1);
const U32 kUSBDeviceTransfer	= (1 << 2);
const U32 kUSBDeviceError		= (1 << 3);
const U32 kUSBDeviceReset		= (1 << 4);
const U32 kUSBDeviceRTC			= (1 << 5);
const U32 kUSBDeviceHotPlug		= (1 << 6);


// Bit definitions for USBDeviceDriver and USBDeviceSupports
const U32 kUSBDeviceIsHID			= (1 << 0);
const U32 kUSBDeviceIsMassStorage	= (1 << 1);
const U32 kUSBDeviceIsEthernet		= (1 << 2);

class CUSBDeviceMessage : public IEventMessage {
 public:
	CUSBDeviceMessage( const tUSBDeviceData& data );
	CUSBDeviceMessage( const tUSBDeviceData& data, tEventType type );
	virtual U16	GetSizeInBytes() const;
	tUSBDeviceData GetUSBDeviceState() const;
 private:
	tUSBDeviceData mData;
};

//==============================================================================	   
// USBDevice errors
//==============================================================================
#define USBDEVICE_ERRORS			\
	(kUSBDeviceUnsupportedDriver)	\
	(kUSBDeviceTooManyDrivers)		\
	(kUSBDeviceFailure)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupUSBDevice), USBDEVICE_ERRORS)

const U32 kUSBDeviceInvalidWatchdog = 0xffffffff;

//==============================================================================
// USB widget Vendor/Product IDs
// These match the IDs returned by FindDevice()
//==============================================================================
#define USB_HUB_ID					0x1D6B0001		// Linux USB host hub
#define USB_CAM_ID					0x093A2801		// PixArt Imaging
#define USB_WIFI_ID					0x1286203C		// Marvell Semiconductor

LF_END_BRIO_NAMESPACE() 
#endif // LF_BRIO_USBDEVICETYPES_H

// eof
