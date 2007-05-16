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
#include <sys/mman.h>
#include <unistd.h>
#include "gpio_ioctl.h"
#include "mlc_ioctl.h"
#include "GLES/libogl.h"

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// Local device driver handles
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	int			gDevGpio;
	int			gDevMlc;
	int			gDevLayer;
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
	
	// open MLC device
	gDevMlc = open("/dev/mlc", O_RDWR|O_SYNC);
	dbg_.Assert(gDevMlc >= 0, 
			"DisplayModule::InitModule: failed to open MLC device");
	
	// open MLC 2D RGB layer device
	gDevLayer = open(RGB_LAYER_DEV, O_RDWR|O_SYNC);
	dbg_.Assert(gDevLayer >= 0, 
			"DisplayModule::InitModule: failed to open MLC 2D Layer device");

	c.outvalue.port = 1;	// GPIO port B
	c.outvalue.value = 1;	// set pins high

	c.outvalue.pin	= PIN_LCD_ENABLE;
	r = ioctl(gDevGpio, GPIO_IOCSOUTVAL, &c);
	dbg_.Assert(r >= 0, 
			"DisplayModule::InitModule: GPIO ioctl failed");
	c.outvalue.pin	= PIN_BACKLIGHT_ENABLE;
	r = ioctl(gDevGpio, GPIO_IOCSOUTVAL, &c);
	dbg_.Assert(r >= 0, 
			"DisplayModule::InitModule: GPIO ioctl failed");
	c.outvalue.pin	= PIN_BLUE_LED;
	r = ioctl(gDevGpio, GPIO_IOCSOUTVAL, &c);
	dbg_.Assert(r >= 0, 
			"DisplayModule::InitModule: GPIO ioctl failed");
}

//----------------------------------------------------------------------------
void CDisplayModule::DeInitModule()
{
	close(gDevGpio);
	close(gDevMlc);
	close(gDevLayer);
}

//----------------------------------------------------------------------------
U32 CDisplayModule::GetScreenSize(void)
{
	union mlc_cmd c;
	int r;

	r = ioctl(gDevMlc, MLC_IOCGSCREENSIZE, &c);
	dbg_.Assert(r == 0, "DisplayModule::GetScreenSize: ioctl failed");

	// TODO: struct screensize_cmd is pretty much the same thing...
	return (U32)(((c.screensize.height)<<16)|(c.screensize.width));
}

//----------------------------------------------------------------------------
enum tPixelFormat CDisplayModule::GetPixelFormat(void)
{
	int format = ioctl(gDevLayer, MLC_IOCQFORMAT, 0);
	dbg_.Assert(format >= 0, "DisplayModule::GetPixelFormat: ioctl failed");

	switch(format) {
		case kLayerPixelFormatRGB4444:
		return kPixelFormatRGB4444;
		case kLayerPixelFormatRGB565:
		return kPixelFormatRGB565;
		case kLayerPixelFormatARGB8888:
		return kPixelFormatARGB8888;
	}
	return kPixelFormatError;
}

// The frame buffer was already allocated by the hardware, therefore pBuffer
// is ignored.
tDisplayHandle CDisplayModule::CreateHandle(U16 height, U16 width, 
										tPixelFormat colorDepth, U8 *pBuffer)
{
	GraphicsContext.height = height;
	GraphicsContext.width = width;
	GraphicsContext.colorDepth = colorDepth;

	return (tDisplayHandle)&GraphicsContext;
}

tErrType CDisplayModule::RegisterLayer(tDisplayHandle hndl, S16 xPos, S16 yPos)
{
	int addr;
	struct tDisplayContext *context;
	
	// ask for the frame buffer address
	addr = ioctl(gDevLayer, MLC_IOCQADDRESS, 0);
	if(addr < 0) {
		dbg_.DebugOut(kDbgLvlCritical, "ioctl failed");
		return kNoImplErr;
	}

	context = (struct tDisplayContext *)hndl;
	context->pBuffer = (U8 *)addr;
	context->x = xPos;
	context->y = yPos;
	context->isAllocated = true;

	return kNoErr;
}


// We are not implementing multiple 2D RGB layers or multiple screens, so
// insertAfter and screen are ignored.
tErrType CDisplayModule::Register(tDisplayHandle hndl, S16 xPos, S16 yPos,
							tDisplayHandle insertAfter, tDisplayScreen screen)
{
	return RegisterLayer(hndl, xPos, yPos);
}

// We are not implementing multiple 2D RGB layers or multiple screens, so
// initialZOrder and screen are ignored.
tErrType CDisplayModule::Register(tDisplayHandle hndl, S16 xPos, S16 yPos,
                             tDisplayZOrder initialZOrder,
                             tDisplayScreen screen)
{
	return RegisterLayer(hndl, xPos, yPos);
}

LF_END_BRIO_NAMESPACE()
// EOF
