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
#include "mlc_ioctl.h"

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
	int			gDevLayer[4];
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
	
	// open MLC layer devices
	// TODO: check for failure, how many layers do we want to open, where do
	//       we manage number of layers and their properties/usage?
	gDevLayer[0] = open("/dev/layer0", O_RDWR|O_SYNC);
	gDevLayer[1] = open("/dev/layer1", O_RDWR|O_SYNC);
	gDevLayer[2] = open("/dev/layer2", O_RDWR|O_SYNC);
	gDevLayer[3] = open("/dev/layer3", O_RDWR|O_SYNC);

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

void CDisplayModule::DeInitModule()
{
	close(gDevGpio);
	close(gDevDpc);
	close(gDevMlc);
	close(gDevGa3d);
	for(int i = 0; i < 4; i++)
		close(gDevLayer[i]);
}

U32 CDisplayModule::GetScreenSize(void)
{
	union mlc_cmd c;
	int r;

	r = ioctl(gDevMlc, MLC_IOCGSCREENSIZE, &c);
	dbg_.Assert(r == 0, "DisplayModule::GetScreenSize: ioctl failed");

	// TODO: struct screensize_cmd is pretty much the same thing...
	return (U32)(((c.screensize.height)<<16)|(c.screensize.width));
}

LF_END_BRIO_NAMESPACE()
// EOF
