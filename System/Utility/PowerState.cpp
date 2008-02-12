//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		PowerState.cpp
//
// Description:
//		Implements Utility functions for Brio.
//
//============================================================================
#include <stdio.h>
#include <sys/sysinfo.h>
#include <PowerTypes.h>

#include "Utility.h"

LF_BEGIN_BRIO_NAMESPACE()

//----------------------------------------------------------------------------
// Returns the system's power state
//----------------------------------------------------------------------------
enum tPowerState GetCurrentPowerState(void)
{
		FILE *power_fd;
		int ret;
		unsigned int status;

		power_fd = fopen("/sys/devices/platform/lf1000-power/status", "r");
		if(power_fd == NULL)
			return kPowerNull;

		ret = fscanf(power_fd, "%d\n", &status);
		fclose(power_fd);
		if(ret < 1)
			return kPowerNull;

		switch(status) {
			case 1:
			return kPowerExternal;
			case 2:
			return kPowerBattery;
			case 3:
			return kPowerLowBattery;
			case 4:
			return kPowerCritical;
		}
		return kPowerNull;
}

//----------------------------------------------------------------------------

LF_END_BRIO_NAMESPACE()

// EOF
