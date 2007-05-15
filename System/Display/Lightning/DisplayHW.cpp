//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		DisplayHW.cpp
//
// Description:
//		Configure display manager for hardware target
//
//==============================================================================

#include <SystemTypes.h>
#include <SystemErrors.h>
#include <DisplayPriv.h>
#include <DisplayMPI.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include "gpio_ioctl.h"

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// Local device driver handles
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	int			gDevGpio = -1;
	int			gDevDpc = -1;
	int			gDevMlc = -1;
	int			gDevGa3d = -1;
}

//============================================================================
// CDisplayModule: Implementation of hardware-specific functions
//============================================================================
//----------------------------------------------------------------------------
void CDisplayModule::InitModule()
{
	// Initialize display manager for hardware target
	union gpio_cmd 	c;
	int				r;		

	// open GPIO device
	gDevGpio = open("/dev/gpio", O_WRONLY|O_SYNC);
	dbg_.Assert(gDevGpio >= 0, 
			"DisplayModule::InitModule: failed to open GPIO device");
	
	// open DPC device XXX: no need for this yet
	gDevDpc = open("/dev/dpc", O_WRONLY);
	dbg_.Assert(gDevDpc >= 0, 
			"DisplayModule::InitModule: failed to open DPC device");
	
	// open MLC device
	gDevMlc = open("/dev/mlc", O_RDWR|O_SYNC);
	dbg_.Assert(gDevMlc >= 0, 
			"DisplayModule::InitModule: failed to open MLC device");
	
	// open 3D accelerator device
	gDevGa3d = open("/dev/ga3d", O_RDWR|O_SYNC);
	dbg_.Assert(gDevGa3d >= 0, 
			"DisplayModule::InitModule: failed to open 3D accelerator device");

	c.outvalue.port = 1;	// GPIO port B
	c.outvalue.value = 1;	// set pins high

	// enable LCD
	c.outvalue.pin	= 10;
	r = ioctl(gDevGpio, GPIO_IOCSOUTVAL, &c);
	// turn on LCD backlight
	c.outvalue.pin	= 9;
	r = ioctl(gDevGpio, GPIO_IOCSOUTVAL, &c);
	// turn on blue LED
	c.outvalue.pin	= 29;
	r = ioctl(gDevGpio, GPIO_IOCSOUTVAL, &c);
}

void CDisplayModule::CleanupModule()
{
	if(gDevGpio >= 0)
		close(gDevGpio);
	if(gDevDpc >= 0)
		close(gDevDpc);
	if(gDevMlc >= 0)
		close(gDevMlc);
	if(gDevGa3d >= 0)
		close(gDevGa3d);
}

LF_END_BRIO_NAMESPACE()
// EOF
