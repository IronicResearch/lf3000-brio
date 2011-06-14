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
#include "Utility.h"

LF_BEGIN_BRIO_NAMESPACE()

//----------------------------------------------------------------------------
// Returns the system platform ID
//----------------------------------------------------------------------------
U32 GetPlatformID()
{
	int 	id = 0;
	FILE* 	fd = fopen( "/sys/devices/platform/lf1000-gpio/board_id", "r" );
	if (fd)
	{
		fscanf(fd, "%x", &id);
		fclose(fd);
	}
	return (U32)id;
}

//----------------------------------------------------------------------------
// Returns the system platform name
//----------------------------------------------------------------------------
CString GetPlatformName()
{
	switch (GetPlatformID())
	{
		case 0: 					// LF1000 devboard
		case 3:	return "Didj"; 		// Didj FF
		case 5: 					// Didj-TS, Acorn FF
		case 6:
		case 7: 					// Emerald FF
		case 1:						// Emerald POP TV out
		case 2:						// Emerald EP/PP no supercap
		case 0xA: return "Emerald";	// Emerald CIP Samsung SDRAM
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
	case kCapsTouchscreen:
		return (0 == stat("/sys/devices/platform/lf1000-touchscreen", &stbuf));
	case kCapsCamera:
		return (0 == stat("/sys/devices/platform/lf1000-ohci/usb1", &stbuf));
	case kCapsAccelerometer:
		return (0 == stat("/sys/devices/platform/lf1000-aclmtr/enable", &stbuf));
	}
	return false;
}

LF_END_BRIO_NAMESPACE()

// EOF
