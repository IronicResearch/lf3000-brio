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

	std::list<tBuffer>	gBufListUsed;			// list of allocated buffers
	std::list<tBuffer>	gBufListFree;			// list of freed buffers
	U32					gMarkBufStart = 0;		// framebuffer start marker
	U32					gMarkBufEnd = 0;		// framebuffer end marker
	tMutex 		gListMutex = PTHREAD_MUTEX_INITIALIZER;	// list mutex
}

	//----------------------------------------------------------------------------
	U32	CDisplayLF1000::GetDisplayMem(tDisplayMem memtype)
	{
		U32 mem = 0;
		std::list<tBuffer>::iterator it;
	
		switch (memtype) {
		case kDisplayMemTotal:
			return gFrameSizeTotal;
		case kDisplayMemFree:
			return gMarkBufEnd - gMarkBufStart;
		case kDisplayMemUsed:
			for (it = gBufListUsed.begin(); it != gBufListUsed.end(); it++)
				mem += (*it).length;
			return mem;
		case kDisplayMemAligned:
			for (it = gBufListUsed.begin(); it != gBufListUsed.end(); it++)
				mem += (*it).aligned;
			return mem;
		case kDisplayMemFragmented:
			for (it = gBufListFree.begin(); it != gBufListFree.end(); it++)
				mem += (*it).length;
			return mem;
		}
		return 0;
	}
	
	//----------------------------------------------------------------------------
	bool CDisplayLF1000::AllocBuffer(tDisplayContext* pdc, U32 aligned)
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
			kernel_.LockMutex(gListMutex);
			
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
					kernel_.UnlockMutex(gListMutex);
					return true;
				}
			}

			// Allocate another buffer from the heap
			if (aligned) {
				// Allocate aligned buffer from end of heap (OGL, YUV)
				if (gMarkBufEnd - bufsize < gMarkBufStart)
				{	kernel_.UnlockMutex(gListMutex);
					return false;
				}
				pdc->offset = gMarkBufEnd - bufsize;
				pdc->pBuffer += pdc->offset;
				gMarkBufEnd -= bufsize;
			}
			else {
				// Allocate unaligned buffer at top of heap if there's room
				if (gMarkBufStart + bufsize > gMarkBufEnd)
				{	kernel_.UnlockMutex(gListMutex);
					return false;
				}
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
		kernel_.UnlockMutex(gListMutex);
		return true;
	}
	
	//----------------------------------------------------------------------------
	bool CDisplayLF1000::DeAllocBuffer(tDisplayContext* pdc)
	{
#ifndef UNIFIED		
		if (!pdc->isOverlay)
#endif
		{
			tBuffer buf;
			std::list<tBuffer>::iterator it;
			kernel_.LockMutex(gListMutex);
			U32 markStart = gMarkBufStart;
			U32 markEnd   = gMarkBufEnd;
			
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

			// Compact free list buffers back to heap when possible
			while (markStart != gMarkBufStart || markEnd != gMarkBufEnd) {
				markStart = gMarkBufStart;
				markEnd   = gMarkBufEnd;
				for (it = gBufListFree.begin(); it != gBufListFree.end(); it++) {
					buf = *it;
					if (buf.aligned && buf.offset == gMarkBufEnd) {
						gMarkBufEnd += buf.length;
						gBufListFree.erase(it);
						break;
					}
					if (!buf.aligned && buf.offset + buf.length == gMarkBufStart) {
						gMarkBufStart -= buf.length;
						gBufListFree.erase(it);
						break;
					}
				}
			}
		}
		kernel_.UnlockMutex(gListMutex);
		return true;
	}

//============================================================================
// CDisplayLF1000: Implementation of hardware-specific functions
//============================================================================
//----------------------------------------------------------------------------
void CDisplayLF1000::InitModule()
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

	// Implement /dev/mlc ioctl to query framebuffer base and size
	// independent of last loaded layer address. Workaround is to check for
	// 1 Meg alignment on /dev/layer0 and re-align as necessary.
	baseAddr = ioctl(gDevMlc, MLC_IOCQADDRESS, 0);
	if (baseAddr != -ENOTTY) {
		fb_size = ioctl(gDevMlc, MLC_IOCQFBSIZE, 0);
		dbg_.DebugOut(kDbgLvlValuable, "DisplayModule::InitModule: /dev/mlc base = %08X, size = %08X\n", baseAddr, fb_size);
		gFrameBase = baseAddr;
		gFrameSize = gFrameSizeTotal = fb_size;
	}
	
	// ask for the Frame Buffer base address
	baseAddr = ioctl(gDevLayer, MLC_IOCQADDRESS, 0);
	dbg_.Assert(baseAddr != -EFAULT,
			"DisplayModule::InitModule: MLC layer ioctl failed");
	// Check layer address for 1 Meg alignment
	if (baseAddr & (k1Meg-1)) {
		dbg_.DebugOut(kDbgLvlImportant, "DisplayModule::InitModule: addr %08X re-aligned for %s\n", baseAddr, RGB_LAYER_DEV);
		baseAddr &= ~(k1Meg-1);
	}
	if (gFrameBase == 0)
		gFrameBase = baseAddr;

	// get the frame buffer's size
	fb_size = ioctl(gDevLayer, MLC_IOCQFBSIZE, 0);
	dbg_.Assert(fb_size > 0,
			"DisplayModule::InitModule: MLC layer ioctl failed");
	if (gFrameSize == 0)
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
	ioctl(gDevOverlay, MLC_IOCTLAYEREN, 0);
	SetDirtyBit(gDevOverlay);

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
	if (gFrameSizeTotal == 0)
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
	
}

//----------------------------------------------------------------------------
void CDisplayLF1000::DeInitModule()
{
	dbg_.DebugOut(kDbgLvlVerbose, "DisplayModule::DeInitModuleHW: enter\n");

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
U32 CDisplayLF1000::GetScreenSize(void)
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
enum tPixelFormat CDisplayLF1000::GetPixelFormat(void)
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
tDisplayHandle CDisplayLF1000::CreateHandle(U16 height, U16 width, 
										tPixelFormat colorDepth, U8 *pBuffer)
{
	U32 hwFormat;
	tDisplayContext *GraphicsContext = new struct tDisplayContext;
	U32 bpp;
	U32 aligned = 0;

	memset(GraphicsContext, 0, sizeof(tDisplayContext));
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
	GraphicsContext->isBlended = false;
	GraphicsContext->alphaLevel = (100*ALPHA_STEP)/100;
	GraphicsContext->flippedContext = NULL;

	// Offscreen context does not affect hardware settings
	if (GraphicsContext->isAllocated)
		return reinterpret_cast<tDisplayHandle>(GraphicsContext);

	// Allocate framebuffer region for onscreen display context
	if (AllocBuffer(GraphicsContext, aligned)) {
		dbg_.DebugOut(kDbgLvlValuable, "DisplayModule::CreateHandle: %p\n", GraphicsContext->pBuffer);
		// Clear framebuffer memory if pixel format change pending
		memset(GraphicsContext->pBuffer, 0x00, GraphicsContext->pitch * GraphicsContext->height);
	}
	else {
		// No framebuffer allocations left
		dbg_.DebugOut(kDbgLvlCritical, "DisplayModule::CreateHandle: No framebuffer available\n");
		delete GraphicsContext;
		return kInvalidDisplayHandle;
	}

	// apply to device
	int layer = GraphicsContext->layer = (GraphicsContext->isOverlay) ? gDevOverlay : gDevLayer;
	ioctl(layer, MLC_IOCTBLEND, GraphicsContext->isBlended);
	ioctl(layer, MLC_IOCTALPHA, GraphicsContext->alphaLevel);
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
tErrType CDisplayLF1000::Update(tDisplayContext *dc, int sx, int sy, int dx, int dy, int width, int height)
{
	// Copy offscreen context to primary display context
	if (dc->isAllocated)
	{
		// FIXME: destination should be pdcVisible_, not necessarily pdcPrimary_
		// FIXME: onscreen display context is not necessarily ARGB8888 format
		if (pdcVisible_ == NULL)
			return kDisplayDisplayNotInListErr;
		tDisplayContext *context = pdcVisible_;
		if(context->flippedContext)
			context = context->flippedContext;
		switch (dc->colorDepthFormat) 
		{
		case kPixelFormatRGB4444: 	RGB4444ARGB(dc, context, sx, sy, dx, dy, width, height); break;
		case kPixelFormatRGB565: 	RGB565ARGB(dc, context, sx, sy, dx, dy, width, height); break;
		case kPixelFormatRGB888: 	RGB2ARGB(dc, context, sx, sy, dx, dy, width, height); break;
		default:
		case kPixelFormatARGB8888: 	
			switch (context->colorDepthFormat)
			{
			case kPixelFormatRGB4444: 	ARGB2RGB4444(dc, context, sx, sy, dx, dy, width, height); break;
			case kPixelFormatRGB565: 	ARGB2RGB565(dc, context, sx, sy, dx, dy, width, height); break;
			case kPixelFormatRGB888: 	ARGB2RGB(dc, context, sx, sy, dx, dy, width, height); break;
			default:
			case kPixelFormatARGB8888:	ARGB2ARGB(dc, context, sx, sy, dx, dy, width, height); break;
			case kPixelFormatYUV420: 	RGB2YUV(dc, context, sx, sy, dx, dy, width, height); break;
			}
			break;
		case kPixelFormatYUV420: 	YUV2ARGB(dc, context, sx, sy, dx, dy, width, height); break;
		case kPixelFormatYUYV422: 	YUYV2ARGB(dc, context, sx, sy, dx, dy, width, height); break;
		}
	}
	// Make sure primary display context is enabled
	if (!bPrimaryLayerEnabled) {
		tDisplayContext *context = pdcVisible_;
		if(context->flippedContext)
			context = context->flippedContext;
		U32 addr = context->basephys +context->offset;
		
		if (pdcVisible_->isPlanar) 
		{
			addr = LIN2XY(addr);
			if(context->basephys == 0)
				addr = gPlanarBase + (U32)context->pBuffer - (U32)gPlanarBuffer;
			ioctl(context->layer, MLC_IOCTADDRESSCB, addr + context->pitch/2);
			ioctl(context->layer, MLC_IOCTADDRESSCR, addr + context->pitch/2 + context->pitch*(context->height/2));
		}
		if(pdcVisible_->isOverlay) {
			int order = (pdcVisible_->isUnderlay) ? 2 : 0;
			int prior = ioctl(gDevMlc, MLC_IOCQPRIORITY, 0);
			if (order != prior) {
				ioctl(gDevMlc, MLC_IOCTPRIORITY, order);
				ioctl(gDevMlc, MLC_IOCTTOPDIRTY, 0);
			}
			isYUVLayerSwapped_ = (order != 0);
		}
		ioctl(pdcVisible_->layer, MLC_IOCTBLEND, pdcVisible_->isBlended);
		ioctl(pdcVisible_->layer, MLC_IOCTALPHA, pdcVisible_->alphaLevel);
		ioctl(pdcVisible_->layer, MLC_IOCTADDRESS, addr);
		ioctl(pdcVisible_->layer, MLC_IOCTLAYEREN, (void *)1);
		SetDirtyBit(pdcVisible_->layer);
		bPrimaryLayerEnabled = true;
	}
	// No hardware settings have actually changed
    return kNoErr;
}

//----------------------------------------------------------------------------
// Hide onscreen display context from being visible.
tErrType CDisplayLF1000::UnRegisterLayer(tDisplayHandle hndl)
{
	struct 	tDisplayContext *context = (struct tDisplayContext *)hndl;
	int 	layer = context->layer; // (context->isOverlay) ? gDevOverlay : gDevLayer;

	if (pdcVisible_ && context == pdcVisible_->flippedContext)
		pdcVisible_->flippedContext = NULL;
	if (context == pdcVisible_)
		pdcVisible_ = NULL;
	
	// Offscreen contexts do not affect screen
	if (context->isAllocated)
		return kNoErr;
	
	// Remove layer from visibility on screen
	ioctl(layer, MLC_IOCTLAYEREN, (void *)0);
	bPrimaryLayerEnabled = false;
	SetDirtyBit(layer);
	return kNoErr;
}

//----------------------------------------------------------------------------
// There is no buffer to destroy, so destroyBuffer is ignored.
tErrType CDisplayLF1000::DestroyHandle(tDisplayHandle hndl, 
									   Boolean destroyBuffer)
{
	(void )destroyBuffer;	/* Prevent unused variable warnings. */ 
	if (hndl == NULL)
		return kNoErr;
	pModule_->UnRegister(hndl, 0);
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
tErrType CDisplayLF1000::RegisterLayer(tDisplayHandle hndl, S16 xPos, S16 yPos)
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
	if (context->isUnderlay && !context->isOverlay) {
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
		int order = (context->isUnderlay) ? 2 : 0;
		int prior = ioctl(gDevMlc, MLC_IOCQPRIORITY, 0);
		if (order != prior) {
			ioctl(gDevMlc, MLC_IOCTPRIORITY, order);
			ioctl(gDevMlc, MLC_IOCTTOPDIRTY, 0);
		}
		isYUVLayerSwapped_ = (order != 0);
		
		// Defer enabling video layer until 1st Invalidate() call
		bPrimaryLayerEnabled = false;
	}

	SetDirtyBit(layer);

	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayLF1000::SetWindowPosition(tDisplayHandle hndl, S16 x, S16 y, U16 width, U16 height, Boolean visible)
{
	struct tDisplayContext* dc = (struct tDisplayContext*)hndl;
	int	layer = dc->layer;
	union mlc_cmd c;
	c.position.left = dc->rect.left = dc->x = x;
	c.position.top = dc->rect.top  = dc->y = y;
	c.position.right = dc->rect.right = dc->rect.left + std::min(dc->width, width);
	c.position.bottom = dc->rect.bottom = dc->rect.top + std::min(dc->height, height);
	ioctl(layer, MLC_IOCSPOSITION, &c);
	ioctl(layer, MLC_IOCTLAYEREN, visible);
	SetDirtyBit(layer);
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayLF1000::GetWindowPosition(tDisplayHandle hndl, S16& x, S16& y, U16& width, U16& height, Boolean& visible)
{
	struct tDisplayContext* dc = (struct tDisplayContext*)hndl;
	x = dc->rect.left;
	y = dc->rect.top;
	width = dc->rect.right - dc->rect.left;
	height = dc->rect.bottom - dc->rect.top;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayLF1000::SetVideoScaler(tDisplayHandle hndl, U16 width, U16 height, Boolean centered)
{
	struct tDisplayContext* dc = (struct tDisplayContext*)hndl;
	int	layer, r, dw, dh;
	union mlc_cmd c;

	// Open video layer device
	layer = dc->layer;
	if (layer != gDevOverlay)
		return kInvalidParamErr;
	
	// Get position info when video context was created
	r = ioctl(layer, MLC_IOCGPOSITION, &c);
	dw = (r == 0) ? c.position.right - c.position.left + 0 : 320;
	dh = (r == 0) ? c.position.bottom - c.position.top + 0 : 240;
	
	// Reposition with fullscreen centering instead of scaling
	if (centered) 
	{
		dw = width;
		dh = height;		
		c.position.left = (320 - width) / 2;
		c.position.right = c.position.left + width;
		c.position.top = (240 - height) / 2;
		c.position.bottom = c.position.top + height;
		ioctl(layer, MLC_IOCSPOSITION, &c);
	}
	// Special case handling for 320x176 scaling (non-letterbox)
	else if (176 == height && 320 == width) 
	{
		dw = 320;
		dh = 240;
		c.position.left = c.position.top = 0;
		c.position.right = c.position.left + dw;
		c.position.bottom = c.position.top + dh;
		ioctl(layer, MLC_IOCSPOSITION, &c);
	}

	// Set video scaler for video source and screen destination
	c.overlaysize.srcwidth = width;
	c.overlaysize.srcheight = height;
	c.overlaysize.dstwidth = dw; 
	c.overlaysize.dstheight = dh; 
	ioctl(layer, MLC_IOCSOVERLAYSIZE, &c);
	ioctl(layer, MLC_IOCTDIRTY, 0);

	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayLF1000::GetVideoScaler(tDisplayHandle hndl, U16& width, U16& height, Boolean& centered)
{
	return kNoImplErr;
}

//----------------------------------------------------------------------------
void CDisplayLF1000::SetDirtyBit(int layer)
{
	ioctl(layer, MLC_IOCTDIRTY, 0);
}

//----------------------------------------------------------------------------
inline int GetDirtyBit(int layer)
{
	return ioctl(layer, MLC_IOCQDIRTY, 0);
}

//----------------------------------------------------------------------------
tErrType CDisplayLF1000::SwapBuffers(tDisplayHandle hndl, Boolean waitVSync)
{
	tDisplayContext *context = reinterpret_cast<tDisplayContext*>(hndl);
	int		layer = context->layer;
	int		r;
	U32 	physaddr = context->basephys + context->offset;

	// Support for replicated YUV video context in planar memory block
	if (context->basephys == 0) {
		if (context->isPlanar 
				&& (U32)context->pBuffer >= (U32)gPlanarBuffer
				&& (U32)context->pBuffer <= (U32)gPlanarBuffer + gPlanarSize) {
			U32 offset = (U32)context->pBuffer - (U32)gPlanarBuffer;
			physaddr = gPlanarBase + offset;
			layer = gDevOverlay;
			// Update invalidated regions before page flip
			if (!isYUVLayerSwapped_)
				pdcVisible_->flippedContext = context;
			pModule_->Invalidate(0, NULL);
		}
		else 
			return kDisplayDisplayNotInListErr;
	}
	
	// Load display address register for selected display context
	r = ioctl(layer, MLC_IOCTADDRESS, physaddr);
	if (context->isOverlay) {
		ioctl(layer, MLC_IOCTADDRESSCB, physaddr + context->pitch/2);
		ioctl(layer, MLC_IOCTADDRESSCR, physaddr + context->pitch/2 + context->pitch*(context->height/2));
	}
	ioctl(layer, MLC_IOCTLAYEREN, (void *)1);
	SetDirtyBit(layer);
	if ((context->colorDepthFormat == kPixelFormatYUV420 && !isYUVLayerSwapped_)
		|| (context->colorDepthFormat != kPixelFormatYUV420 /* && isYUVLayerSwapped_ */))
		pdcVisible_->flippedContext = context;
	bPrimaryLayerEnabled = true;
	
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
Boolean CDisplayLF1000::IsBufferSwapped(tDisplayHandle hndl)
{
	tDisplayContext *context = reinterpret_cast<tDisplayContext*>(hndl);

	// Query context layer dirty bit (cleared == updated)
	int r = GetDirtyBit(context->layer);
	return (r < 0) ? false : (r != 0) ? false : true;
}

//----------------------------------------------------------------------------
tErrType CDisplayLF1000::SetAlpha(tDisplayHandle hndl, U8 level, 
		Boolean enable)
{
	int r;
	struct 	tDisplayContext *context = (struct tDisplayContext *)hndl;
	int 	layer = context->layer; // (context->isOverlay) ? gDevOverlay : gDevLayer;

	if (level > 100) 
		level = 100;
	level = (level*ALPHA_STEP)/100;
	
	context->isBlended = enable;
	context->alphaLevel = level;
	
	r = ioctl(layer, MLC_IOCTALPHA, level);
	r = ioctl(layer, MLC_IOCTBLEND, enable);
	SetDirtyBit(layer);
	return kNoErr;
}

//----------------------------------------------------------------------------
U8 CDisplayLF1000::GetAlpha(tDisplayHandle hndl) const
{
	struct tDisplayContext *context = (struct tDisplayContext *)hndl;
	int layer = context->layer; // (context->isOverlay) ? gDevOverlay : gDevLayer;
	int alpha = ioctl(layer, MLC_IOCQALPHA, 0);
	return (alpha < 0) ? 0 : (alpha*100)/ALPHA_STEP;
}

//----------------------------------------------------------------------------
tErrType CDisplayLF1000::SetBacklight(tDisplayScreen screen, S8 backlight)
{	
	(void )screen;	/* Prevent unused variable warnings. */
	int 			r;

	r = ioctl(gDevDpc, DPC_IOCTBACKLIGHTVIRT, backlight);
	return (r < 0) ? kDisplayInvalidScreenErr : kNoErr;
}

//----------------------------------------------------------------------------
S8	CDisplayLF1000::GetBacklight(tDisplayScreen screen)
{
	(void )screen;	/* Prevent unused variable warnings. */
	unsigned long	p = 0;
	int 			r;

	r = ioctl(gDevDpc, DPC_IOCQBACKLIGHTVIRT, &p);
	return (S8)(p & 0xFF);
}

LF_END_BRIO_NAMESPACE()
// EOF
