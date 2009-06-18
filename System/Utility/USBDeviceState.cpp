//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		USBDeviceState.cpp
//
// Description:
//		Implements Utility functions for Brio.
//
//============================================================================
#include <stdio.h>
#include <sys/sysinfo.h>
#include <USBDeviceTypes.h>

#include "Utility.h"

LF_BEGIN_BRIO_NAMESPACE()

static tUSBDeviceData gUSBState = {0, 0, 0};

//----------------------------------------------------------------------------
void SetCachedUSBDeviceState(tUSBDeviceData state)
{
	gUSBState = state;
}

//----------------------------------------------------------------------------
// Returns the system's USB state
//----------------------------------------------------------------------------
tUSBDeviceData GetCurrentUSBDeviceState(void)
{
		FILE *usb_device_fd;
		int ret;
		U32 usbState;
		tUSBDeviceData data;

		// Emerald is Ethernet device type
		data.USBDeviceSupports = kUSBDeviceIsEthernet;
		data.USBDeviceDriver   = kUSBDeviceIsEthernet;
		usbState = 0;

		usb_device_fd =
			fopen("/sys/devices/platform/lf1000-usbgadget/vbus", "r");
		if(usb_device_fd != NULL) {
			ret = fscanf(usb_device_fd, "%ld\n", &usbState);
			fclose(usb_device_fd);
		}
		data.USBDeviceState = (usbState != 0) ? kUSBDeviceConnected : 0;
		data.USBDeviceState |= gUSBState.USBDeviceState;
		return data;
}

//----------------------------------------------------------------------------

LF_END_BRIO_NAMESPACE()

// EOF
