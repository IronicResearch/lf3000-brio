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
			fd = fopen( "/sys/devices/system/board/system_rev", "r" );
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
#ifdef EMULATION
	char* platformstr = getenv("ARIA_EMU_PLATFORM");
	if (platformstr)
		return CString(platformstr);
	return "Unknown";
#endif

	FILE* 	fd = fopen( "/sys/devices/system/board/platform_family", "r" );
	if (fd)
	{
		char buf[10];
		fscanf(fd, "%s", buf);
		fclose(fd);
		return CString(buf);
	}

	CString platform = GetPlatformName();
	if (platform == "Emerald")
		return "LEX";
	if (platform == "Madrid")
		return "LPAD";
	return "Unknown";
}

//----------------------------------------------------------------------------
// Returns the system platform name
//----------------------------------------------------------------------------
CString GetPlatformName()
{
#ifdef EMULATION
	char* platform = getenv("ARIA_EMU_PLATFORM");
	if (platform)
		return CString(platform);
	return "Unknown";
#endif

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
		case 0x32B:
			return "RIO";
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
	case kCapsLF1000:
		return (0 == stat("/sys/devices/platform/lf1000-alvgpio", &stbuf));
	case kCapsLF2000:
		return (0 == stat("/sys/devices/platform/lf2000-alive", &stbuf) &&
				"GLASGOW" != GetPlatformFamily());
	case kCapsLF3000:
		return (0 == stat("/sys/devices/platform/mali-utgard.0", &stbuf)); // FIXME
	case kCapsTouchscreen:
		return (0 == stat("/sys/devices/platform/lf1000-touchscreen", &stbuf)) ||
		       (0 == stat("/sys/devices/platform/lf2000-touchscreen", &stbuf));
	case kCapsCamera:
		if("GLASGOW" == GetPlatformFamily())
			return false;
		if ("Madrid" == GetPlatformName())
			return (0 == stat("/sys/devices/platform/lf1000-ohci/usb1", &stbuf));
		return (0 == stat("/sys/class/video4linux/video0", &stbuf));
	case kCapsAccelerometer:
		return (0 == stat("/sys/devices/platform/lf1000-aclmtr/enable", &stbuf)) ||
		       (0 == stat("/sys/devices/platform/lf2000-aclmtr/enable", &stbuf));
	case kCapsMicrophone:
		if ("Madrid" == GetPlatformName())
			return (0 == stat("/sys/devices/platform/lf1000-ohci/usb1", &stbuf));
		if ("Emerald" == GetPlatformName())
			return (0 == stat("/sys/class/sound/pcmC1D0c", &stbuf));
		return (0 == stat("/sys/class/sound/pcmC0D0c", &stbuf));		
	case kCapsScreenLEX:
		return ("LEX" == GetPlatformFamily());
	case kCapsScreenLPAD:
		return ("LPAD" == GetPlatformFamily());
	case kCapsWifi:
		return (0 == stat("/sys/class/net/wlan0", &stbuf) &&
				("GLASGOW" != GetPlatformFamily()) );
	case kCapsCameraFront:
		return (0 == stat("/sys/class/video4linux/video1", &stbuf));
	case kCapsMultiTouch:
		return ("RIO" == GetPlatformFamily() ||
				"CABO" == GetPlatformName()); // FIXME: query input device?
	case kCapsAnalogStick:
		return false;
	case kCapsGamePadController:
		return ("GLASGOW" == GetPlatformFamily());
	case kCapsVision:
		return ("GLASGOW" == GetPlatformFamily());
	case kCapsButtonMask(kButtonUp):
	case kCapsButtonMask(kButtonDown):
	case kCapsButtonMask(kButtonRight):
	case kCapsButtonMask(kButtonLeft):
		return ("GLASGOW" != GetPlatformFamily());
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
		return ("GLASGOW" != GetPlatformFamily());
	case kCapsButtonMask(kCartridgeDetect):
		return true;
	case kCapsButtonMask(kButtonVolumeDown):
	case kCapsButtonMask(kButtonVolumeUp):
		return ("GLASGOW" != GetPlatformFamily());
	case kCapsButtonMask(kButtonEscape):
		return ("LPAD" == GetPlatformFamily() ||
				"RIO" == GetPlatformFamily());
	case kCapsButtonMask(kButtonSync):
		return ("GLASGOW" == GetPlatformFamily()); // FIXME
	}
	return false;
}

LF_END_BRIO_NAMESPACE()

// EOF
