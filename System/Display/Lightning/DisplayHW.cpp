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
#include <linux/lf1000/gpio_ioctl.h>
#include <linux/lf1000/dpc_ioctl.h>
#include <linux/lf1000/mlc_ioctl.h>
#include "GLES/libogl.h"

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
	int			gDevLayer;
	int			gDevOverlay;
	U8 			*gFrameBuffer;
	U8			*gOverlayBuffer;
	U8			*gPlanarBuffer;
	int			gFrameSize;
	int			gOverlaySize;
	int			gPlanarSize;
	U32			gFrameBase;
	U32			gOverlayBase;
	U32			gPlanarBase;
}

//============================================================================
// CDisplayModule: Implementation of hardware-specific functions
//============================================================================
//----------------------------------------------------------------------------
void CDisplayModule::InitModule()
{
	// Initialize display manager for hardware target
	int 			baseAddr, fb_size;

	dbg_.SetDebugLevel(kDbgLvlVerbose);
	
	// open GPIO device
	gDevGpio = open("/dev/gpio", O_WRONLY|O_SYNC);
	dbg_.Assert(gDevGpio >= 0, 
			"DisplayModule::InitModule: failed to open GPIO device");
	
	// open DPC device
	gDevDpc = open("/dev/dpc", O_RDWR|O_SYNC);
	dbg_.Assert(gDevDpc >= 0, 
			"DisplayModule::InitModule: failed to open DPC device");
	
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
	gFrameBase = baseAddr;

	// get the frame buffer's size
	fb_size = ioctl(gDevLayer, MLC_IOCQFBSIZE, 0);
	dbg_.Assert(fb_size > 0,
			"DisplayModule::InitModule: MLC layer ioctl failed");
	gFrameSize = fb_size;

	// get access to the Frame Buffer
	gFrameBuffer = (U8 *)mmap(0, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED,
		   						gDevLayer, baseAddr);
	dbg_.Assert(gFrameBuffer >= 0,
			"DisplayModule::InitModule: failed to mmap() frame buffer");
	dbg_.DebugOut(kDbgLvlVerbose, 
			"DisplayModule::InitModule: mapped base %08X, size %08X to %p\n", 
			baseAddr, fb_size, gFrameBuffer);

	// Open MLC 2D YUV layer device
	gDevOverlay = open(YUV_LAYER_DEV, O_RDWR|O_SYNC);
	dbg_.Assert(gDevOverlay >= 0, 
			"DisplayModule::InitModule: failed to open MLC 2D Layer device");

	// Get the overlay buffer base address
	baseAddr = ioctl(gDevOverlay, MLC_IOCQADDRESS, 0);
	dbg_.Assert(baseAddr >= 0,
			"DisplayModule::InitModule: MLC layer ioctl failed");
	gOverlayBase = baseAddr;
	
	// Get the overlay buffer's size
	fb_size = ioctl(gDevOverlay, MLC_IOCQFBSIZE, 0);
	dbg_.Assert(fb_size > 0,
			"DisplayModule::InitModule: MLC layer ioctl failed");
	gOverlaySize = fb_size;

	// Get access to the overlay buffer in user space
	gOverlayBuffer = (U8 *)mmap(0, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED,
		   						gDevOverlay, baseAddr);
	dbg_.Assert(gOverlayBuffer >= 0,
			"DisplayModule::InitModule: failed to mmap() frame buffer");
	dbg_.DebugOut(kDbgLvlVerbose, 
			"DisplayModule::InitModule: mapped base %08X, size %08X to %p\n", 
			baseAddr, fb_size, gOverlayBuffer);

	// Get access to the overlay planar array buffer in user space
	fb_size = 4096 * 4096;
	baseAddr += 0x20000000;
	gPlanarBuffer = (U8 *)mmap(0, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED,
		   						gDevOverlay, baseAddr);
	dbg_.Assert(gPlanarBuffer >= 0,
			"DisplayModule::InitModule: failed to mmap() frame buffer");
	dbg_.DebugOut(kDbgLvlVerbose, 
			"DisplayModule::InitModule: mapped base %08X, size %08X to %p\n", 
			baseAddr, fb_size, gPlanarBuffer);
	gPlanarBase = baseAddr;
	gPlanarSize = fb_size;
}

//----------------------------------------------------------------------------
void CDisplayModule::DeInitModule()
{
    munmap(gPlanarBuffer, gPlanarSize);
    munmap(gOverlayBuffer, gOverlaySize);
    munmap(gFrameBuffer, gFrameSize);
	close(gDevGpio);
	close(gDevDpc);
	close(gDevMlc);
	close(gDevLayer);
	close(gDevOverlay);
}

//----------------------------------------------------------------------------
U32 CDisplayModule::GetScreenSize(void)
{
	union mlc_cmd c;
	int r;

	r = ioctl(gDevMlc, MLC_IOCGSCREENSIZE, &c);
	dbg_.Assert(r == 0, "DisplayModule::GetScreenSize: ioctl failed");

	return (U32)(((c.screensize.height)<<16)|(c.screensize.width));
}

//----------------------------------------------------------------------------
// Returns pixel format of primary RGB layer
enum tPixelFormat CDisplayModule::GetPixelFormat(void)
{
	U32 format = ioctl(gDevLayer, MLC_IOCQFORMAT, 0);
	dbg_.Assert(format >= 0, "DisplayModule::GetPixelFormat: ioctl failed");

	switch(format) {
		case kLayerPixelFormatRGB4444:
			return kPixelFormatRGB4444;
		case kLayerPixelFormatRGB565:
			return kPixelFormatRGB565;
		case kLayerPixelFormatARGB8888:
			return kPixelFormatARGB8888;
		case kLayerPixelFormatRGB888:
			return kPixelFormatRGB888;
	}
	// All MLC layers output same ARGB format combined
	return kPixelFormatARGB8888;
}

//----------------------------------------------------------------------------
// The frame buffer was already allocated by the hardware, therefore pBuffer
// is ignored.
tDisplayHandle CDisplayModule::CreateHandle(U16 height, U16 width, 
										tPixelFormat colorDepth, U8 *pBuffer)
{
	U32 hwFormat;
	tDisplayContext *GraphicsContext = new struct tDisplayContext;
	U8 bpp;

	GraphicsContext->height = height;
	GraphicsContext->width = width;
	GraphicsContext->colorDepthFormat = colorDepth;
	GraphicsContext->isOverlay = false;
	GraphicsContext->isPlanar = false;

	switch(colorDepth) {
		case kPixelFormatRGB4444:
		bpp = 2;
		hwFormat = kLayerPixelFormatRGB4444;
		break;

		default:
		case kPixelFormatARGB8888:
		bpp = 4;
		hwFormat = kLayerPixelFormatARGB8888;
		break;

		case kPixelFormatRGB565:
		bpp = 2;
		hwFormat = kLayerPixelFormatRGB565;
		break;

		case kPixelFormatRGB888:
		bpp = 3;
		hwFormat = kLayerPixelFormatRGB888;
		break;

		case kPixelFormatYUV420:
		bpp = 1;
		width = 4096; // for YUV planar format pitch
		hwFormat = kLayerPixelFormatYUV420;
		GraphicsContext->isOverlay = true;
		GraphicsContext->isPlanar = true;
		// Switch video overlay linear address to XY Block address
		ioctl(gDevOverlay, MLC_IOCTADDRESS, LIN2XY(gOverlayBase));
		ioctl(gDevOverlay, MLC_IOCTADDRESSCB, LIN2XY(gOverlayBase + width*height));
		ioctl(gDevOverlay, MLC_IOCTADDRESSCR, LIN2XY(gOverlayBase + width*height*3/2));
		break;

		case kPixelFormatYUYV422:
		bpp = 2;
		hwFormat = kLayerPixelFormatYUYV422;
		GraphicsContext->isOverlay = true;
		break;
	}
	GraphicsContext->pitch = bpp*width;
	GraphicsContext->bpp = bpp;
	GraphicsContext->depth = 8*bpp;
	GraphicsContext->format = hwFormat;

	// apply to device
	int layer = (GraphicsContext->isOverlay) ? gDevOverlay : gDevLayer;
	ioctl(layer, MLC_IOCTBLEND, 0);
	ioctl(layer, MLC_IOCTFORMAT, hwFormat);
	ioctl(layer, MLC_IOCTHSTRIDE, bpp);
	ioctl(layer, MLC_IOCTVSTRIDE, GraphicsContext->pitch);
	SetDirtyBit(layer);

	return (tDisplayHandle)GraphicsContext;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::Invalidate(tDisplayScreen screen, tRect *pDirtyRect)
{
	// No hardware settings have actually changed
//	SetDirtyBit(gDevLayer);
    return kNoErr;
}

//----------------------------------------------------------------------------
// This method does not have much meaning given the kernel-level frame buffer.
tErrType CDisplayModule::UnRegister(tDisplayHandle hndl, tDisplayScreen screen)
{
	struct 	tDisplayContext *context = (struct tDisplayContext *)hndl;
	int 	layer = (context->isOverlay) ? gDevOverlay : gDevLayer;

	// Remove layer from visibility on screen
	ioctl(layer, MLC_IOCTLAYEREN, (void *)0);
	// Restore video overlay linear address for subsequent instances
	if (context->isOverlay && context->isPlanar)
		ioctl(layer, MLC_IOCTADDRESS, gOverlayBase);
	SetDirtyBit(layer);
	context->isAllocated = false;
	return kNoErr;
}

//----------------------------------------------------------------------------
// There is no buffer to destroy, so destroyBuffer is ignored.
tErrType CDisplayModule::DestroyHandle(tDisplayHandle hndl, 
									   Boolean destroyBuffer)
{
	delete (struct tDisplayContext *)hndl;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::RegisterLayer(tDisplayHandle hndl, S16 xPos, S16 yPos)
{
	union mlc_cmd c;
	struct tDisplayContext *context;
	
	context = (struct tDisplayContext *)hndl;
	context->pBuffer = (context->isPlanar) ? gPlanarBuffer : (context->isOverlay) ? gOverlayBuffer : gFrameBuffer;
	context->x = xPos;
	context->y = yPos;
	context->isAllocated = true;

	// apply to device
	c.position.top = yPos;
	c.position.left = xPos;
	c.position.right = xPos + context->width;
	c.position.bottom = yPos + context->height;

	// Reload format and stride registers for subsequent context references
	int layer = (context->isOverlay) ? gDevOverlay : gDevLayer;
	ioctl(layer, MLC_IOCTFORMAT, context->format);
	ioctl(layer, MLC_IOCTHSTRIDE, context->bpp);
	ioctl(layer, MLC_IOCTVSTRIDE, context->pitch);
	ioctl(layer, MLC_IOCSPOSITION, &c);
	ioctl(layer, MLC_IOCTLAYEREN, (void *)1);
	
	// Setup scaler registers too for video overlay
	if (context->isOverlay)
	{
		c.overlaysize.srcwidth = context->width;
		c.overlaysize.srcheight = context->height;
		c.overlaysize.dstwidth = context->width;
		c.overlaysize.dstheight = context->height;
		ioctl(layer, MLC_IOCSOVERLAYSIZE, &c);
	}

	SetDirtyBit(layer);

	return kNoErr;
}

//----------------------------------------------------------------------------
void CDisplayModule::SetDirtyBit(int layer)
{
	ioctl(layer, MLC_IOCTDIRTY, 0);
}

//----------------------------------------------------------------------------
// FIXME/dm: Actually this seems like the perfect method for video overlays!

// We are not implementing multiple 2D RGB layers or multiple screens, so
// insertAfter and screen are ignored.
tErrType CDisplayModule::Register(tDisplayHandle hndl, S16 xPos, S16 yPos,
							tDisplayHandle insertAfter, tDisplayScreen screen)
{
	return RegisterLayer(hndl, xPos, yPos);
}

//----------------------------------------------------------------------------
// We are not implementing multiple 2D RGB layers or multiple screens, so
// initialZOrder and screen are ignored.
tErrType CDisplayModule::Register(tDisplayHandle hndl, S16 xPos, S16 yPos,
                             tDisplayZOrder initialZOrder,
                             tDisplayScreen screen)
{
	return RegisterLayer(hndl, xPos, yPos);
}

//----------------------------------------------------------------------------
U8* CDisplayModule::GetBuffer(tDisplayHandle hndl) const
{
	return ((struct tDisplayContext *)hndl)->pBuffer;
}

//----------------------------------------------------------------------------
tPixelFormat CDisplayModule::GetPixelFormat(tDisplayHandle hndl) const
{
	return ((struct tDisplayContext *)hndl)->colorDepthFormat;
}

//----------------------------------------------------------------------------
U16 CDisplayModule::GetPitch(tDisplayHandle hndl) const
{
	return ((struct tDisplayContext *)hndl)->pitch;
}

//----------------------------------------------------------------------------
U16 CDisplayModule::GetDepth(tDisplayHandle hndl) const
{
	return ((struct tDisplayContext *)hndl)->depth;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::SetAlpha(tDisplayHandle hndl, U8 level, 
		Boolean enable)
{
	int r;
	struct 	tDisplayContext *context = (struct tDisplayContext *)hndl;
	int 	layer = (context->isOverlay) ? gDevOverlay : gDevLayer;

	if (level > 100) 
		level = 100;
	level = (level*ALPHA_STEP)/100;
	
	// Distinguish global vs per-pixel alpha for ARGB8888 format (bpp unchanged)
	if (context->colorDepthFormat == kPixelFormatARGB8888)
		ioctl(layer, MLC_IOCTFORMAT, context->format = (enable) ? kLayerPixelFormatXRGB8888 : kLayerPixelFormatARGB8888);
	r = ioctl(layer, MLC_IOCTALPHA, level);
	r = ioctl(layer, MLC_IOCTBLEND, enable);
	SetDirtyBit(layer);
	return kNoErr;
}

//----------------------------------------------------------------------------
U8 CDisplayModule::GetAlpha(tDisplayHandle hndl) const
{
	struct tDisplayContext *context = (struct tDisplayContext *)hndl;
	int layer = (context->isOverlay) ? gDevOverlay : gDevLayer;
	int alpha = ioctl(layer, MLC_IOCQALPHA, 0);
	return (alpha < 0) ? 0 : (alpha*100)/ALPHA_STEP;
}

//----------------------------------------------------------------------------
U16 CDisplayModule::GetHeight(tDisplayHandle hndl) const
{
	return ((struct tDisplayContext *)hndl)->height;
}

//----------------------------------------------------------------------------
U16 CDisplayModule::GetWidth(tDisplayHandle hndl) const
{
	return ((struct tDisplayContext *)hndl)->width;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::SetBrightness(tDisplayScreen screen, S8 brightness)
{
	unsigned long	p = brightness + 128;
	int 			r;
	
	r = ioctl(gDevDpc, DPC_IOCTBRIGHTNESS, p);
	return (r < 0) ? kDisplayInvalidScreenErr : kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::SetContrast(tDisplayScreen screen, S8 contrast)
{
	unsigned long	p = (contrast + 128) >> 4;
	int 			r;
	
	r = ioctl(gDevDpc, DPC_IOCTCONTRAST, p);
	return (r < 0) ? kDisplayInvalidScreenErr : kNoErr;
}

//----------------------------------------------------------------------------
S8	CDisplayModule::GetBrightness(tDisplayScreen screen)
{
	unsigned long	p = 0;
	int 			r;
	
	r = ioctl(gDevDpc, DPC_IOCQBRIGHTNESS, p);
	return (r < 0) ? 0 : (r & 0xFF) - 128;
}

//----------------------------------------------------------------------------
S8	CDisplayModule::GetContrast(tDisplayScreen screen)
{
	unsigned long	p = 0;
	int 			r;
	
	r = ioctl(gDevDpc, DPC_IOCQCONTRAST, p);
	return (r < 0) ? 0 : ((r & 0xFF) << 4) - 128;
}

LF_END_BRIO_NAMESPACE()
// EOF
