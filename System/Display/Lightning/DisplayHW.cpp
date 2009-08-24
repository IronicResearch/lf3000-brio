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

#define _FILE_OFFSET_BITS       64      // for correct off_t type

#include <SystemTypes.h>
#include <SystemErrors.h>
#include <DisplayPriv.h>
#include <DisplayMPI.h>
#include <BrioOpenGLConfig.h>
#include <ButtonTypes.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <linux/lf1000/gpio_ioctl.h>
#include <linux/lf1000/dpc_ioctl.h>
#include <linux/lf1000/mlc_ioctl.h>
#include "GLES/libogl.h"
#include <list>

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// Local event listener
//============================================================================
namespace
{
	const tEventType DisplayButtonEvents[] = {kAllButtonEvents};
	
	#define	SCREEN_BRIGHT_LEVELS	(2 * 3)	// have 4 distinct levels
	/* 
	 * lcdBacklight array describes a complete cycle of screen adjustment
	 * levels, so array index always increments, mod number of array entries.
	 * First array entry is the default setting.
	 */
	S8 lcdBacklight[SCREEN_BRIGHT_LEVELS] = 
		{ BACKLIGHT_LEVEL_2, BACKLIGHT_LEVEL_3, BACKLIGHT_LEVEL_4,
		  BACKLIGHT_LEVEL_3, BACKLIGHT_LEVEL_2, BACKLIGHT_LEVEL_1 };
	
	
	class BrightnessListener : public IEventListener
	{
		CDisplayModule*	mpDisplayMPI;
	public:
		BrightnessListener(CDisplayModule* pModule):
			IEventListener(DisplayButtonEvents, ArrayCount(DisplayButtonEvents)),
			mpDisplayMPI(pModule)
			{}
		
		tEventStatus Notify(const IEventMessage& msg)
		{
			tEventType event_type = msg.GetEventType();
			if (event_type == kButtonStateChanged)
			{
				const CButtonMessage& buttonmsg = dynamic_cast<const CButtonMessage&>(msg);
				tButtonData data = buttonmsg.GetButtonState();
				U32 buttonPressed = data.buttonTransition & data.buttonState;
				
				if (kButtonBrightness & buttonPressed)
				{
					static int brightIndex = 1;
					mpDisplayMPI->SetBacklight(0, lcdBacklight[brightIndex]);
					brightIndex++;
					brightIndex = brightIndex % SCREEN_BRIGHT_LEVELS; // keep ptr in range
				}
			}
		}
	};
}

//============================================================================
// Local device driver handles
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	int			gDevDpc = -1;
	int			gDevMlc = -1;
	int			gDevLayer = -1;
	int			gDevOpenGL = -1;
	int			gDevOverlay = -1;
	U8 			*gFrameBuffer = NULL;
	U8 			*gOpenGLBuffer = NULL;
	U8			*gOverlayBuffer = NULL;
	U8			*gPlanarBuffer = NULL;
	int			gFrameSize = 0;
	int			gFrameSizeTotal = 0;
	int			gOpenGLSize = 0;
	int			gOverlaySize = 0;
	int			gPlanarSize = 0;
	U32			gFrameBase = 0;
	U32			gOpenGLBase = 0;
	U32			gOverlayBase = 0;
	U32			gPlanarBase = 0;
	bool		bPrimaryLayerEnabled = false;
	U16			gScreenWidth = 320;
	U16			gScreenHeight = 240;
	BrightnessListener* gpBrightnessListener = NULL;

	std::list<tBuffer>	gBufListUsed;			// list of allocated buffers
	std::list<tBuffer>	gBufListFree;			// list of freed buffers
	U32					gMarkBufStart = 0;		// framebuffer start marker
	U32					gMarkBufEnd = 0;		// framebuffer end marker
}

	//----------------------------------------------------------------------------
	bool CDisplayModule::AllocBuffer(tDisplayContext* pdc, U32 aligned)
	{
		U32 bufsize = (aligned) ? aligned : pdc->pitch * pdc->height;
#define UNIFIED
#ifndef UNIFIED		
		if (pdc->isOverlay) {
			pdc->pBuffer = gPlanarBuffer;
			pdc->offset = 0;
			pdc->basephys = gOverlayBase; 
			pdc->baselinear = gPlanarBase;
			return true;
		}
#endif
		{
			tBuffer buf;
			std::list<tBuffer>::iterator it;

			// All display contexts now reference common base address
			pdc->basephys = gFrameBase;
			pdc->baselinear = gPlanarBase;
			pdc->pBuffer = (pdc->isPlanar) ? gPlanarBuffer : gFrameBuffer;

			// Look on for available buffer on free list first
			for (it = gBufListFree.begin(); it != gBufListFree.end(); it++) {
				buf = *it;
				// Move from free list to allocated list if we find one that fits
				if (buf.length == bufsize) {
					gBufListFree.erase(it);
					pdc->offset = buf.offset;
					pdc->pBuffer += pdc->offset;
					gBufListUsed.push_back(buf);
					dbg_.DebugOut(kDbgLvlVerbose, "AllocBuffer: recycle offset %08X, length %08X\n", (unsigned)buf.offset, (unsigned)buf.length);
					return true;
				}
			}

			// Allocate another buffer from the heap
			if (aligned) {
				// Allocate aligned buffer from end of heap (OGL, YUV)
				if (gMarkBufEnd - bufsize < gMarkBufStart)
					return false;
				pdc->offset = gMarkBufEnd - bufsize;
				pdc->pBuffer += pdc->offset;
				gMarkBufEnd -= bufsize;
			}
			else {
				// Allocate unaligned buffer at top of heap if there's room
				if (gMarkBufStart + bufsize > gMarkBufEnd)
					return false;
				pdc->offset = gMarkBufStart;
				pdc->pBuffer += pdc->offset;
				pdc->isPrimary = (pdc->offset == 0) ? true : false;
				gMarkBufStart += bufsize;
			}

			// Add buffer to allocated list
			buf.length = bufsize;
			buf.offset = pdc->offset;
			buf.aligned = aligned;
			gBufListUsed.push_back(buf);
			dbg_.DebugOut(kDbgLvlVerbose, "AllocBuffer: new buf offset %08X, length %08X\n", (unsigned)buf.offset, (unsigned)buf.length);
		}
		return true;
	}
	
	//----------------------------------------------------------------------------
	bool CDisplayModule::DeAllocBuffer(tDisplayContext* pdc)
	{
#ifndef UNIFIED		
		if (!pdc->isOverlay)
#endif
		{
			tBuffer buf;
			std::list<tBuffer>::iterator it;

			// Find allocated buffer for this display context
			for (it = gBufListUsed.begin(); it != gBufListUsed.end(); it++) {
				buf = *it;
				if (pdc->offset == buf.offset) {
					dbg_.DebugOut(kDbgLvlVerbose, "DeAllocBuffer: remove offset %08X, length %08X\n", (unsigned)buf.offset, (unsigned)buf.length);
					gBufListUsed.erase(it);
					break;
				}
			}

			// Reduce heap if at top location, otherwise update free list
			if (buf.aligned && buf.offset == gMarkBufEnd)
				gMarkBufEnd += buf.length;
			else if (buf.offset + buf.length == gMarkBufStart)
				gMarkBufStart -= buf.length;
			else
				gBufListFree.push_back(buf);
			
		}
		return true;
	}

//============================================================================
// CDisplayModule: Implementation of hardware-specific functions
//============================================================================
//----------------------------------------------------------------------------
void CDisplayModule::InitModule()
{
	// Initialize display manager for hardware target
	int 			baseAddr, fb_size;

	// open DPC device
	gDevDpc = open("/dev/dpc", O_RDWR|O_SYNC);
	dbg_.Assert(gDevDpc >= 0, 
			"DisplayModule::InitModule: failed to open DPC device");
	
	// open MLC device
	gDevMlc = open("/dev/mlc", O_RDWR|O_SYNC);
	dbg_.Assert(gDevMlc >= 0, 
			"DisplayModule::InitModule: failed to open MLC device");

	// Get screen size for future reference
	GetScreenSize();
	
	// open MLC 2D RGB layer device
	gDevLayer = open(RGB_LAYER_DEV, O_RDWR|O_SYNC);
	dbg_.Assert(gDevLayer >= 0, 
			"DisplayModule::InitModule: failed to open MLC 2D Layer device");

#if 0	// SW wants to leave bootup warning screen on as long as possible
	// Disable RGB layer since it is used externally for various firmware bitmaps
	ioctl(gDevLayer, MLC_IOCTLAYEREN, 0);
	SetDirtyBit(gDevLayer);
	bPrimaryLayerEnabled = false;
#endif
	
	// ask for the Frame Buffer base address
	baseAddr = ioctl(gDevLayer, MLC_IOCQADDRESS, 0);
	dbg_.Assert(baseAddr != -EFAULT,
			"DisplayModule::InitModule: MLC layer ioctl failed");
	gFrameBase = baseAddr;

	// get the frame buffer's size
	fb_size = ioctl(gDevLayer, MLC_IOCQFBSIZE, 0);
	dbg_.Assert(fb_size > 0,
			"DisplayModule::InitModule: MLC layer ioctl failed");
	gFrameSize = fb_size;

	// Open MLC 3D OpenGL RGB layer device
	gDevOpenGL = open(OGL_LAYER_DEV, O_RDWR|O_SYNC);
	dbg_.Assert(gDevLayer >= 0, 
			"DisplayModule::InitModule: failed to open MLC 3D Layer device");

	// Get 3D OpenGL RGB layer address for future reference 
	baseAddr = ioctl(gDevOpenGL, MLC_IOCQADDRESS, 0);
	dbg_.Assert(baseAddr != -EFAULT,
			"DisplayModule::InitModule: MLC layer ioctl failed");
	gOpenGLBase = baseAddr;

	// get the frame buffer's size
	fb_size = ioctl(gDevOpenGL, MLC_IOCQFBSIZE, 0);
	dbg_.Assert(fb_size > 0,
			"DisplayModule::InitModule: MLC layer ioctl failed");
	gOpenGLSize = fb_size;

	// Open MLC 2D YUV layer device
	gDevOverlay = open(YUV_LAYER_DEV, O_RDWR|O_SYNC);
	dbg_.Assert(gDevOverlay >= 0, 
			"DisplayModule::InitModule: failed to open MLC 2D Layer device");

	// Get the overlay buffer base address
	baseAddr = ioctl(gDevOverlay, MLC_IOCQADDRESS, 0);
	dbg_.Assert(baseAddr != -EFAULT,
			"DisplayModule::InitModule: MLC layer ioctl failed");
	gOverlayBase = baseAddr &= ~0x20000000;
	gPlanarBase = baseAddr | 0x20000000;
	
	// Get the overlay buffer's size
	fb_size = ioctl(gDevOverlay, MLC_IOCQFBSIZE, 0);
	dbg_.Assert(fb_size > 0,
			"DisplayModule::InitModule: MLC layer ioctl failed");
	gOverlaySize = fb_size;
	gPlanarSize = fb_size;

	// Calculate total framebuffer memory available for sub-allocation
#ifdef UNIFIED
	gFrameSizeTotal = gFrameSize + gOverlaySize + gOpenGLSize;
	gPlanarSize = gFrameSize = gFrameSizeTotal;
	gPlanarBase = gFrameBase | 0x20000000;
#else
	gFrameSizeTotal = gFrameSize; // + gOverlaySize + gOpenGLSize;
#endif
	
	// Setup buffer lists and markers
	gBufListUsed.clear();
	gBufListFree.clear();
	gMarkBufStart = 0;
	gMarkBufEnd   = gFrameSizeTotal; 

	// Map all framebuffer regions and sub-allocate per display context
	gFrameBuffer = (U8 *)mmap((void*)gFrameBase, gFrameSize, PROT_READ | PROT_WRITE, MAP_SHARED,
			gDevLayer, gFrameBase);
	dbg_.Assert(gFrameBuffer != MAP_FAILED,
			"DisplayModule::InitModule: failed to mmap() %s framebuffer", RGB_LAYER_DEV);
	dbg_.DebugOut(kDbgLvlValuable, 
			"DisplayModule::InitModule: mapped base %08X, size %08X to %p\n", 
			(unsigned int)gFrameBase, gFrameSize, gFrameBuffer);

	gPlanarBuffer = (U8 *)mmap(0, gPlanarSize, PROT_READ | PROT_WRITE, MAP_SHARED,
			gDevOverlay, gPlanarBase);
	dbg_.Assert(gPlanarBuffer != MAP_FAILED,
			"DisplayModule::InitModule: failed to mmap() %s framebuffer", YUV_LAYER_DEV);
	dbg_.DebugOut(kDbgLvlValuable, 
			"DisplayModule::InitModule: mapped base %08X, size %08X to %p\n", 
			(unsigned int)gPlanarBase, gPlanarSize, gPlanarBuffer);
	
	// Register our button listener to handle brightness button changes
	gpBrightnessListener = new BrightnessListener(this);
	eventmgr_.RegisterEventListener(gpBrightnessListener);
}

//----------------------------------------------------------------------------
void CDisplayModule::DeInitModule()
{
	dbg_.DebugOut(kDbgLvlVerbose, "DisplayModule::DeInitModuleHW: enter\n");

	eventmgr_.UnregisterEventListener(gpBrightnessListener);
	delete gpBrightnessListener;
	gpBrightnessListener = NULL;

	munmap(gFrameBuffer, gFrameSize);
	munmap(gPlanarBuffer, gPlanarSize);

	ioctl(gDevLayer, MLC_IOCTADDRESS, gFrameBase);
	ioctl(gDevOpenGL, MLC_IOCTADDRESS, gOpenGLBase);
	ioctl(gDevOverlay, MLC_IOCTADDRESS, gOverlayBase);
	
	close(gDevDpc);
	close(gDevMlc);
	close(gDevLayer);
	close(gDevOpenGL);
	close(gDevOverlay);

	dbg_.DebugOut(kDbgLvlVerbose, "DisplayModule::DeInitModuleHW: exit\n");
}

//----------------------------------------------------------------------------
U32 CDisplayModule::GetScreenSize(void)
{
	union mlc_cmd c;
	int r;

	r = ioctl(gDevMlc, MLC_IOCGSCREENSIZE, &c);
	dbg_.Assert(r == 0, "DisplayModule::GetScreenSize: ioctl failed");

	gScreenWidth = c.screensize.width;
	gScreenHeight = c.screensize.height;

	return (U32)(((c.screensize.height)<<16)|(c.screensize.width));
}

//----------------------------------------------------------------------------
// Returns pixel format of primary RGB layer
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
		case kLayerPixelFormatRGB888:
			return kPixelFormatRGB888;
	}
	// All MLC layers output same ARGB format combined
	return kPixelFormatARGB8888;
}

//----------------------------------------------------------------------------
// pBuffer == NULL indicates onscreen context.
// pBuffer != NULL indicates offscreen context.
tDisplayHandle CDisplayModule::CreateHandle(U16 height, U16 width, 
										tPixelFormat colorDepth, U8 *pBuffer)
{
	U32 hwFormat;
	tDisplayContext *GraphicsContext = new struct tDisplayContext;
	U32 bpp;
	U32 blend = 0;
	U32 aligned = 0;

	GraphicsContext->height = height;
	GraphicsContext->width = width;
	GraphicsContext->colorDepthFormat = colorDepth;
	GraphicsContext->pBuffer = pBuffer;
	GraphicsContext->offset = 0;
	GraphicsContext->isPrimary = false;
	GraphicsContext->isAllocated = (pBuffer != NULL);
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
		aligned = ALIGN(4096 * height, k1Meg);
		hwFormat = kLayerPixelFormatYUV420;
		GraphicsContext->isOverlay = true;
		GraphicsContext->isPlanar = true;
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

	// Offscreen context does not affect hardware settings
	if (GraphicsContext->isAllocated)
		return reinterpret_cast<tDisplayHandle>(GraphicsContext);

	// Allocate framebuffer region for onscreen display context
	if (AllocBuffer(GraphicsContext, aligned)) {
		dbg_.DebugOut(kDbgLvlValuable, "DisplayModule::CreateHandle: %p\n", GraphicsContext->pBuffer);
		// Clear framebuffer memory if pixel format change pending
		if (!GraphicsContext->isOverlay && ioctl(gDevLayer, MLC_IOCQFORMAT, 0) != hwFormat)
			memset(GraphicsContext->pBuffer, 0xFF, GraphicsContext->pitch * GraphicsContext->height);
	}
	else {
		// No framebuffer allocations left
		dbg_.DebugOut(kDbgLvlCritical, "DisplayModule::CreateHandle: No framebuffer available\n");
		delete GraphicsContext;
		return kInvalidDisplayHandle;
	}

	// apply to device
	int layer = GraphicsContext->layer = (GraphicsContext->isOverlay) ? gDevOverlay : gDevLayer;
	ioctl(layer, MLC_IOCTBLEND, blend);
	ioctl(layer, MLC_IOCTFORMAT, hwFormat);
	ioctl(layer, MLC_IOCTHSTRIDE, bpp);
	ioctl(layer, MLC_IOCTVSTRIDE, GraphicsContext->pitch);
	if (GraphicsContext->isPlanar)
	{
		// Switch video overlay linear address to XY Block address
		U32 addr = GraphicsContext->basephys + GraphicsContext->offset;
		ioctl(gDevOverlay, MLC_IOCTADDRESS, LIN2XY(addr));
		ioctl(gDevOverlay, MLC_IOCTADDRESSCB, LIN2XY(addr + GraphicsContext->pitch/2));
		ioctl(gDevOverlay, MLC_IOCTADDRESSCR, LIN2XY(addr + GraphicsContext->pitch/2 + GraphicsContext->pitch*(height/2)));
	}
	SetDirtyBit(layer);

	return reinterpret_cast<tDisplayHandle>(GraphicsContext);
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::Update(tDisplayContext *dc, int sx, int sy, int dx, int dy, int width, int height)
{
	// Copy offscreen context to primary display context
	if (dc->isAllocated)
	{
		// FIXME: destination should be pdcVisible_, not necessarily pdcPrimary_
		// FIXME: onscreen display context is not necessarily ARGB8888 format
		if (pdcVisible_ == NULL)
			return kDisplayDisplayNotInListErr;
		// Make sure primary context is enabled
		if (!bPrimaryLayerEnabled)
			RegisterLayer(pdcVisible_, 0, 0);
		switch (dc->colorDepthFormat) 
		{
		case kPixelFormatRGB4444: 	RGB4444ARGB(dc, pdcVisible_, sx, sy, dx, dy, width, height); break;
		case kPixelFormatRGB565: 	RGB565ARGB(dc, pdcVisible_, sx, sy, dx, dy, width, height); break;
		case kPixelFormatRGB888: 	RGB2ARGB(dc, pdcVisible_, sx, sy, dx, dy, width, height); break;
		default:
		case kPixelFormatARGB8888: 	
			switch (pdcVisible_->colorDepthFormat)
			{
			case kPixelFormatRGB4444: 	ARGB2RGB4444(dc, pdcVisible_, sx, sy, dx, dy, width, height); break;
			case kPixelFormatRGB565: 	ARGB2RGB565(dc, pdcVisible_, sx, sy, dx, dy, width, height); break;
			case kPixelFormatRGB888: 	ARGB2RGB(dc, pdcVisible_, sx, sy, dx, dy, width, height); break;
			default:
			case kPixelFormatARGB8888:	ARGB2ARGB(dc, pdcVisible_, sx, sy, dx, dy, width, height); break;
			}
			break;
		case kPixelFormatYUV420: 	YUV2ARGB(dc, pdcVisible_, sx, sy, dx, dy, width, height); break;
		case kPixelFormatYUYV422: 	YUYV2ARGB(dc, pdcVisible_, sx, sy, dx, dy, width, height); break;
		}
	}
	// Enable video layer on update calls to minimize dead screen time
	if (dc->isOverlay) {
		ioctl(dc->layer, MLC_IOCTLAYEREN, (void *)1);
		SetDirtyBit(dc->layer);
	}
	// No hardware settings have actually changed
    return kNoErr;
}

//----------------------------------------------------------------------------
// Hide onscreen display context from being visible.
tErrType CDisplayModule::UnRegisterLayer(tDisplayHandle hndl)
{
	struct 	tDisplayContext *context = (struct tDisplayContext *)hndl;
	int 	layer = context->layer; // (context->isOverlay) ? gDevOverlay : gDevLayer;

	// Remove layer from visibility on screen
	ioctl(layer, MLC_IOCTLAYEREN, (void *)0);
	bPrimaryLayerEnabled = false;
	SetDirtyBit(layer);
	return kNoErr;
}

//----------------------------------------------------------------------------
// There is no buffer to destroy, so destroyBuffer is ignored.
tErrType CDisplayModule::DestroyHandle(tDisplayHandle hndl, 
									   Boolean destroyBuffer)
{
	(void )destroyBuffer;	/* Prevent unused variable warnings. */ 
	if (hndl == NULL)
		return kNoErr;
	UnRegister(hndl, 0);
	// Release mappings to framebuffer memory
	struct 	tDisplayContext *context = (struct tDisplayContext *)hndl;
	if (!context->isAllocated) {
		DeAllocBuffer(context);
		dbg_.DebugOut(kDbgLvlValuable, "DisplayModule::DestroyHandle: %p\n", context->pBuffer);
	}
	delete context;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::RegisterLayer(tDisplayHandle hndl, S16 xPos, S16 yPos)
{
	union mlc_cmd c;
	struct tDisplayContext *context = reinterpret_cast<tDisplayContext*>(hndl);
	
	// Nothing to do yet if offscreen context
	context->rect.left = context->x = xPos;
	context->rect.top  = context->y = yPos;
	context->rect.right = xPos + context->width;
	context->rect.bottom = yPos + context->height;
	if (context->isAllocated)
		return kNoErr;

	// apply to device
	c.position.top = yPos;
	c.position.left = xPos;
	c.position.right = xPos + context->width;
	c.position.bottom = yPos + context->height;

	// Change RGB layer assignments if supposed to be on bottom
	if (context->isUnderlay) {
		// Disable 3D layer if already active
		bool bOpenGLSwap = isOpenGLEnabled_;
		if (bOpenGLSwap) {
			DisableOpenGL();
		}
		// Swap 3D and 2D layer framebuffer addresses via reloading address registers
		isLayerSwapped_ = true;
		ioctl(gDevLayer, MLC_IOCTADDRESS, gOpenGLBase);
		context->layer = gDevOpenGL;
		ioctl(gDevOpenGL, MLC_IOCTADDRESS, gFrameBase);
		// Re-Enable 3D layer if previously active
		if (bOpenGLSwap) {
			tOpenGLContext ctx;
			ctx.bFSAA = false;
			EnableOpenGL(&ctx);
		}
	}
		
	// Reload format and stride registers for subsequent context references
	int layer = context->layer; // (context->isOverlay) ? gDevOverlay : gDevLayer;
	ioctl(layer, MLC_IOCTFORMAT, context->format);
	ioctl(layer, MLC_IOCTHSTRIDE, context->bpp);
	ioctl(layer, MLC_IOCTVSTRIDE, context->pitch);
	ioctl(layer, MLC_IOCSPOSITION, &c);
	
	// Defer enabling video layer until 1st Invalidate() call
	if (!context->isOverlay) {
		ioctl(layer, MLC_IOCTADDRESS, context->basephys + context->offset);
		ioctl(layer, MLC_IOCTLAYEREN, (void *)1);
		SetDirtyBit(layer);
		bPrimaryLayerEnabled = true;
		pdcVisible_ = context;
	}
	
	// Setup scaler registers too for video overlay
	if (context->isOverlay)
	{
		// Reset scaler output for display context destination
		// Video MPI will handle special scaler cases (TTP #2073)
		c.overlaysize.srcwidth = context->width;
		c.overlaysize.srcheight = context->height;
		c.overlaysize.dstwidth = context->width;
		c.overlaysize.dstheight = context->height;
		ioctl(layer, MLC_IOCSOVERLAYSIZE, &c);

		// Reload XY block address for planar video format
		if (context->isPlanar) 
		{
			U32 addr = context->basephys + context->offset;
			ioctl(layer, MLC_IOCTADDRESS, LIN2XY(addr));
			ioctl(layer, MLC_IOCTADDRESSCB, LIN2XY(addr + context->pitch/2));
			ioctl(layer, MLC_IOCTADDRESSCR, LIN2XY(addr + context->pitch/2 + context->pitch*(context->height/2)));
		}
		// Select video layer order
		if (context->isUnderlay)
			ioctl(gDevMlc, MLC_IOCTPRIORITY, 0);
		else
			ioctl(gDevMlc, MLC_IOCTPRIORITY, 3);
		
		// Clear video buffer to white pixels before visible
		for (U32 i = 0; i < context->height; i++)
		{
			memset(&context->pBuffer[i*4096], 0xFF, context->width); // white Y
			memset(&context->pBuffer[i*4096+context->pitch/2], 0x7F, context->width/2); // neutral U,V
		}
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
inline int GetDirtyBit(int layer)
{
	return ioctl(layer, MLC_IOCQDIRTY, 0);
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::SwapBuffers(tDisplayHandle hndl, Boolean waitVSync)
{
	tDisplayContext *context = reinterpret_cast<tDisplayContext*>(hndl);
	int		layer = context->layer;
	int		r;

	// Load display address register for selected display context
	U32 physaddr = context->basephys + context->offset;
	r = ioctl(layer, MLC_IOCTADDRESS, physaddr);
	dbg_.Assert(r >= 0, "DisplayModule::SwapBuffers: failed ioctl physaddr=%08X\n", (unsigned int)physaddr);
	SetDirtyBit(layer);
	pdcVisible_ = context;
	
	dbg_.DebugOut(kDbgLvlVerbose, "DisplayModule::SwapBuffers: virtaddr=%08X, physaddr=%08X\n", (unsigned int)context->pBuffer, (unsigned int)physaddr);

	// Optionally wait for display to update
	if (waitVSync) {
		int counter = 0;
		while (GetDirtyBit(layer) != 0) {
			usleep(10);
			if (++counter > 1667) // 16.7 msec max for 60 Hz
				return kDisplayInvalidScreenErr;
		}
	}
	return kNoErr;
}

//----------------------------------------------------------------------------
Boolean CDisplayModule::IsBufferSwapped(tDisplayHandle hndl)
{
	tDisplayContext *context = reinterpret_cast<tDisplayContext*>(hndl);

	// Query context layer dirty bit (cleared == updated)
	int r = GetDirtyBit(context->layer);
	return (r < 0) ? false : (r != 0) ? false : true;
}

//----------------------------------------------------------------------------
tDisplayHandle CDisplayModule::GetCurrentDisplayHandle()
{
	return reinterpret_cast<tDisplayHandle>(pdcVisible_);
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
	int 	layer = context->layer; // (context->isOverlay) ? gDevOverlay : gDevLayer;

	if (level > 100) 
		level = 100;
	level = (level*ALPHA_STEP)/100;
	
	r = ioctl(layer, MLC_IOCTALPHA, level);
	r = ioctl(layer, MLC_IOCTBLEND, enable);
	SetDirtyBit(layer);
	return kNoErr;
}

//----------------------------------------------------------------------------
U8 CDisplayModule::GetAlpha(tDisplayHandle hndl) const
{
	struct tDisplayContext *context = (struct tDisplayContext *)hndl;
	int layer = context->layer; // (context->isOverlay) ? gDevOverlay : gDevLayer;
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
	// translate logical brightness value range of [-128, 127]
	// to physical brightness range of (0,255]
	(void )screen;	/* Prevent unused variable warnings. */
	long	p = brightness + 128;
	int 			r;
	
	r = ioctl(gDevDpc, DPC_IOCTBRIGHTNESS, p);
	return (r < 0) ? kDisplayInvalidScreenErr : kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::SetContrast(tDisplayScreen screen, S8 contrast)
{
	(void )screen;	/* Prevent unused variable warnings. */
	unsigned long	p = (contrast + 128) >> 4;
	int 			r;
	
	r = ioctl(gDevDpc, DPC_IOCTCONTRAST, p);
	return (r < 0) ? kDisplayInvalidScreenErr : kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayModule::SetBacklight(tDisplayScreen screen, S8 backlight)
{	
	(void )screen;	/* Prevent unused variable warnings. */
	int 			r;

	r = ioctl(gDevDpc, DPC_IOCTBACKLIGHTVIRT, backlight);
	return (r < 0) ? kDisplayInvalidScreenErr : kNoErr;
}

//----------------------------------------------------------------------------
S8	CDisplayModule::GetBrightness(tDisplayScreen screen)
{
	(void )screen;	/* Prevent unused variable warnings. */
	unsigned long	p = 0;
	int 			r;
	
	r = ioctl(gDevDpc, DPC_IOCQBRIGHTNESS, p);
	return (r < 0) ? 0 : (r & 0xFF) - 128;
}

//----------------------------------------------------------------------------
S8	CDisplayModule::GetContrast(tDisplayScreen screen)
{
	(void )screen;	/* Prevent unused variable warnings. */
	unsigned long	p = 0;
	int 			r;
	
	r = ioctl(gDevDpc, DPC_IOCQCONTRAST, p);
	return (r < 0) ? 0 : ((r & 0xFF) << 4) - 128;
}

//----------------------------------------------------------------------------
S8	CDisplayModule::GetBacklight(tDisplayScreen screen)
{
	(void )screen;	/* Prevent unused variable warnings. */
	unsigned long	p = 0;
	int 			r;

	r = ioctl(gDevDpc, DPC_IOCQBACKLIGHTVIRT, &p);
	return (S8)(p & 0xFF);
}

LF_END_BRIO_NAMESPACE()
// EOF
