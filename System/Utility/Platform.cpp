//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		Platform.cpp
//
// Description:
//		Utility functions for platform identification.
//
//============================================================================
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <Utility.h>
#include <ButtonTypes.h>

LF_BEGIN_BRIO_NAMESPACE()

//----------------------------------------------------------------------------
// Returns the system platform ID
//----------------------------------------------------------------------------
U32 GetPlatformID()
{
	int 	id = 0;
	FILE* 	fd = fopen( "/sys/devices/platform/lf1000-gpio/board_id", "r" );
	if (!fd)
			fd = fopen( "/flags/board_id", "r" );
	if (fd)
	{
		fscanf(fd, "%x", &id);
		fclose(fd);
	}
	return (U32)id;
}

//----------------------------------------------------------------------------
// Returns the system platform family
//----------------------------------------------------------------------------
CString GetPlatformFamily()
{
	FILE* 	fd = fopen( "/sys/devices/system/board/platform_family", "r" );
	if (fd)
	{
		char buf[10];
		fscanf(fd, "%s", buf);
		fclose(fd);
		return CString(buf);
	}
	return "Unknown";
}

//----------------------------------------------------------------------------
// Returns the system platform name
//----------------------------------------------------------------------------
CString GetPlatformName()
{
	FILE* 	fd = fopen( "/sys/devices/system/board/platform", "r" );
	if (fd)
	{
		char buf[10];
		fscanf(fd, "%s", buf);
		fclose(fd);
		return CString(buf);
	}

	switch (GetPlatformID())
	{
		case 0: 					// LF1000 devboard
		case 3:						// Didj FF
			return "Didj";
		case 5: 					// Didj-TS, Acorn FF
		case 6:
		case 7: 					// Emerald FF
		case 1:						// Emerald POP TV out
		case 2:						// Emerald EP/PP no supercap
		case 0xA: 					// Emerald CIP Samsung SDRAM
			return "Emerald";
		case 0xB: 					// Madrid FF
		case 0xC: 					// Madrid POP TV out
		case 0xD:					// Madrid LFP100
			return "Madrid";
	}
	return "Unknown";
}

//----------------------------------------------------------------------------
// Returns the system state of the selected platform capability
//----------------------------------------------------------------------------
bool HasPlatformCapability(tPlatformCaps caps)
{
	struct stat stbuf;
	switch (caps)
	{
	case kCapsLF2000:
	case kCapsTouchscreen:
		return (0 == stat("/sys/devices/platform/lf2000-touchscreen", &stbuf));
	case kCapsCamera:
		return ((0 == stat("/sys/devices/platform/nx-ohci.0/usb1", &stbuf)) ||
				(0 == stat("/sys/devices/platform/vip.0", &stbuf)) );
	case kCapsAccelerometer:
		return (0 == stat("/sys/devices/platform/lf2000-aclmtr/enable", &stbuf));
	case kCapsMicrophone:
		return (0 == stat("/sys/class/sound/pcmC0D0c", &stbuf));
	case kCapsScreenLEX:
		return ("LEX" == GetPlatformFamily());
	case kCapsScreenLPAD:
		return ("LPAD" == GetPlatformFamily());
	case kCapsWifi:
		return false;
	case kCapsCameraFront:
		return (0 == stat("/sys/devices/platform/vip.1", &stbuf));
	case kCapsButtonMask(kButtonUp):
	case kCapsButtonMask(kButtonDown):
	case kCapsButtonMask(kButtonRight):
	case kCapsButtonMask(kButtonLeft):
		return true;
	case kCapsButtonMask(kButtonA):
	case kCapsButtonMask(kButtonB):
	case kCapsButtonMask(kButtonLeftShoulder):
	case kCapsButtonMask(kButtonRightShoulder):
	case kCapsButtonMask(kButtonMenu):
	case kCapsButtonMask(kButtonHint):
	case kCapsButtonMask(kButtonPause):
		return ("LEX" == GetPlatformFamily());
	case kCapsButtonMask(kButtonBrightness):
		return ("Emerald" == GetPlatformName());
	case kCapsButtonMask(kHeadphoneJackDetect):
	case kCapsButtonMask(kCartridgeDetect):
	case kCapsButtonMask(kButtonVolumeDown):
	case kCapsButtonMask(kButtonVolumeUp):
		return true;
	case kCapsButtonMask(kButtonEscape):
		return ("LPAD" == GetPlatformFamily());
	}
	return false;
}

LF_END_BRIO_NAMESPACE()

// EOF
