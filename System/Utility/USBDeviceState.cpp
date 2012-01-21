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
#include <linux/input.h>
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
extern int open_input_device(const char *input_name);
tUSBDeviceData GetCurrentUSBDeviceState(void)
{
		FILE *usb_device_fd;
		int usb_input_fd;
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
		else
		{
			usb_input_fd = open_input_device("USB");
			if(usb_input_fd >= 0)
			{
				ioctl(usb_input_fd, EVIOCGSW(sizeof(int)), &usbState);
				close(usb_input_fd);
			}
		}
		
		data.USBDeviceState = (usbState != 0) ? kUSBDeviceConnected : 0;
		data.USBDeviceState |= (gUSBState.USBDeviceState & ~kUSBDeviceConnected);
		return data;
}

//----------------------------------------------------------------------------
// Returns the USB product & vendor ID from the given sysfs path, e.g.,
// "/sys/class/usb_device/usbdev1.1"
//----------------------------------------------------------------------------
U32 FindDevice(LeapFrog::Brio::CPath path)
{
	unsigned int vid = 0, pid = 0;
	CPath file = path + "/device/idVendor";
	FILE* fp = fopen(file.c_str(), "r");
	if (fp) {
		fscanf(fp, "%x", &vid);
		fclose(fp);
	}
	file = path + "/device/idProduct";
	fp = fopen(file.c_str(), "r");
	if (fp) {
		fscanf(fp, "%x", &pid);
		fclose(fp);
	}
	return (vid << 16) | pid;
}

LF_END_BRIO_NAMESPACE()

// EOF
