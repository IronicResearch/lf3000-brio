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
	U8 			*gFrameBuffer;
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
	int 			baseAddr, fb_size;

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

	// ask for the Frame Buffer base address
	baseAddr = ioctl(gDevLayer, MLC_IOCQADDRESS, 0);
	dbg_.Assert(baseAddr >= 0,
			"DisplayModule::InitModule: MLC layer ioctl failed");

	// get the frame buffer's size
	fb_size = ioctl(gDevLayer, MLC_IOCQFBSIZE, 0);
	dbg_.Assert(fb_size > 0,
			"DisplayModule::InitModule: MLC layer ioctl failed");

	// get access to the Frame Buffer
	gFrameBuffer = (U8 *)mmap(0, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED,
		   						gDevLayer, baseAddr);
	dbg_.Assert(gFrameBuffer >= 0,
			"DisplayModule::InitModule: failed to mmap() frame buffer");

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
	enum tLayerPixelFormat hwFormat;
	static tDisplayContext *GraphicsContext = new struct tDisplayContext;
	U8 bpp;

	GraphicsContext->height = height;
	GraphicsContext->width = width;
	GraphicsContext->colorDepth = colorDepth;

	switch(colorDepth) {
		case kLayerPixelFormatRGB4444:
		bpp = 4;
		hwFormat = kLayerPixelFormatRGB4444;
		break;

		case kLayerPixelFormatARGB8888:
		bpp = 4;
		hwFormat = kLayerPixelFormatRGB4444;
		break;

		default:
		case kPixelFormatRGB565:
		bpp = 2;
		hwFormat = kLayerPixelFormatRGB565;
		break;
	}
	GraphicsContext->pitch = bpp*width;

	// apply to device
	ioctl(gDevLayer, MLC_IOCTFORMAT, hwFormat);
	ioctl(gDevLayer, MLC_IOCTHSTRIDE, bpp);
	ioctl(gDevLayer, MLC_IOCTVSTRIDE, GraphicsContext->pitch);
	SetDirtyBit();

	return (tDisplayHandle)GraphicsContext;
}

tErrType CDisplayModule::Invalidate(tDisplayScreen screen, tRect *pDirtyRect)
{
	SetDirtyBit();
    return kNoErr;
}

// This method does not have much meaning given the kernel-level frame buffer.
tErrType CDisplayModule::UnRegister(tDisplayHandle hndl, tDisplayScreen screen)
{
	SetDirtyBit();
	((struct tDisplayContext *)hndl)->isAllocated = false;
	return kNoErr;
}

// There is no buffer to destroy, so destroyBuffer is ignored.
tErrType CDisplayModule::DestroyHandle(tDisplayHandle hndl, 
									   Boolean destroyBuffer)
{
	delete (struct tDisplayContext *)hndl;
	return kNoErr;
}

tErrType CDisplayModule::RegisterLayer(tDisplayHandle hndl, S16 xPos, S16 yPos)
{
	union mlc_cmd c;
	struct tDisplayContext *context;
	
	context = (struct tDisplayContext *)hndl;
	context->pBuffer = gFrameBuffer;
	context->x = xPos;
	context->y = yPos;
	context->isAllocated = true;

	// apply to device
	c.position.top = yPos;
	c.position.left = xPos;
	c.position.right = xPos + context->width;
	c.position.bottom = yPos + context->height;
	ioctl(gDevLayer, MLC_IOCSPOSITION, &c);
	SetDirtyBit();

	return kNoErr;
}

void CDisplayModule::SetDirtyBit(void)
{
	ioctl(gDevLayer, MLC_IOCTDIRTY, 0);
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

U8 *CDisplayModule::GetBuffer(tDisplayHandle hndl) const
{
	return ((struct tDisplayContext *)hndl)->pBuffer;
}

U16 CDisplayModule::GetPitch(tDisplayHandle hndl) const
{
	return ((struct tDisplayContext *)hndl)->pitch;
}

tErrType CDisplayModule::SetAlpha(tDisplayHandle hndl, U8 level, 
		Boolean enable)
{
	int r;

	if(level > 100) 
		level = 100;
	level = (level*ALPHA_STEP)/100;
	
	r = ioctl(gDevLayer, MLC_IOCTALPHA, level);
	r = ioctl(gDevLayer, MLC_IOCTBLEND, enable);
	SetDirtyBit();
	return kNoErr;
}

U16 CDisplayModule::GetHeight(tDisplayHandle hndl) const
{
	return ((struct tDisplayContext *)hndl)->height;
}

U16 CDisplayModule::GetWidth(tDisplayHandle hndl) const
{
	return ((struct tDisplayContext *)hndl)->width;
}

LF_END_BRIO_NAMESPACE()
// EOF
