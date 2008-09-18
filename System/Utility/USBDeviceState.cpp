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

//----------------------------------------------------------------------------
// Returns the system's power state
//----------------------------------------------------------------------------
tUSBDeviceData GetCurrentUSBDeviceState(void)
{
		FILE *usb_device_fd;
		int ret;
		tUSBDeviceData data;
		
		usb_device_fd = fopen("/sys/devices/platform/lf1000-usbgadget/vbus", "r");
		if(usb_device_fd == NULL)
			return data;

		ret = fscanf(usb_device_fd, "%ld\n", &data.USBDeviceState);
		fclose(usb_device_fd);
		return data;
}

//----------------------------------------------------------------------------

LF_END_BRIO_NAMESPACE()

// EOF
