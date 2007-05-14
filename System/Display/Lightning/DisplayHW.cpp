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
	int			gDevGpio;
	int			gDevDpc;
	int			gDevMlc;
	int			gDevGa3d;
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

	// Load GPIO driver for General Purpose I/O registers
	gDevGpio = open("/dev/gpio", O_WRONLY);
	dbg_.Assert(gDevGpio >= 0, "DisplayModule::InitModule: GPIO driver failed");
	
	// Load DPC driver for Display Controller registers
	gDevDpc = open("/dev/dpc", O_WRONLY);
	dbg_.Assert(gDevDpc >= 0, "DisplayModule::InitModule: DPC driver failed");
	
	// Load MLC driver for Multi-layer Controller registers
	gDevMlc = open("/dev/mlc", O_WRONLY);
	dbg_.Assert(gDevMlc >= 0, "DisplayModule::InitModule: MLC driver failed");
	
	// Load GA3D driver for 3D Graphics Accelerator registers
	gDevGa3d = open("/dev/ga3d", O_RDWR|O_SYNC);
	dbg_.Assert(gDevGa3d >= 0, "DisplayModule::InitModule: GA3D driver failed");

	// Enable LCD display output via GPIO pins
	c.outvalue.port = 1;	// GPIO port B
	c.outvalue.pin	= 10;	// LCD enable
	c.outvalue.value = 1;
	r = ioctl(gDevGpio, GPIO_IOCSOUTVAL, &c);
	c.outvalue.pin	= 9;	// LCD backlight
	r = ioctl(gDevGpio, GPIO_IOCSOUTVAL, &c);
	c.outvalue.pin	= 29;	// blue LED
	r = ioctl(gDevGpio, GPIO_IOCSOUTVAL, &c);
}

LF_END_BRIO_NAMESPACE()
// EOF
