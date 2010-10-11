//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		DisplayFB.cpp
//
// Description:
//		Display driver for Linux framebuffer interface.
//
//==============================================================================

#include <SystemTypes.h>
#include <SystemErrors.h>
#include <DisplayPriv.h>
#include <BrioOpenGLConfig.h>
#include <GLES/libogl.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

#ifndef FBIO_WAITFORVSYNC
#define FBIO_WAITFORVSYNC	 _IOW('F', 0x20, __u32)
#endif

LF_BEGIN_BRIO_NAMESPACE()

#define RGBFB					0	// index of RGB framebuffer
#define YUVFB					2	// index of YUV framebuffer
#define NUMFB					3	// number of framebuffer devices

#define	REG3D_PHYS				0xC001A000UL	// 3D engine register block
#define REG3D_SIZE				0x00002000

namespace 
{
	const char*					FBDEV[NUMFB] = {"/dev/fb0", "/dev/fb1", "/dev/fb2"};
	int 						fbdev[NUMFB] = {-1, -1, -1};
	struct fb_fix_screeninfo 	finfo[NUMFB];
	struct fb_var_screeninfo 	vinfo[NUMFB];
	U8*							fbmem[NUMFB] = {NULL, NULL, NULL};
	int 						index = 0; // FIXME -- just for testing
	
	const char*					DEV3D = "/dev/ga3d";
	const char*					DEVMEM = "/dev/mem";
	int							fdreg3d = -1;
	int							fdmem = -1;
	unsigned int				mem1phys;
	unsigned int				mem2phys;
	unsigned int				mem1size;
	unsigned int				mem2size;
	void*						preg3d = NULL;
	void*						pmem1d = NULL;
	void*						pmem2d = NULL;
	tDisplayHandle				hogl = NULL;
}

//============================================================================
// CDisplayFB: Implementation of hardware-specific functions
//============================================================================
//----------------------------------------------------------------------------
void CDisplayFB::InitModule()
{
	int r;

	for (int n = 0; n < NUMFB; n++)
	{
		// Open framebuffer device
		fbdev[n] = open(FBDEV[n], O_RDWR | O_SYNC);
		dbg_.Assert(fbdev[n] >= 0, "%s: Error opening %s\n", __FUNCTION__, FBDEV[n]);
		
		// Query framebuffer info
		r = ioctl(fbdev[n], FBIOGET_FSCREENINFO, &finfo[n]);
		dbg_.Assert(r == 0, "%s: Error querying %s\n", __FUNCTION__, FBDEV[n]);
		
		r = ioctl(fbdev[n], FBIOGET_VSCREENINFO, &vinfo[n]);
		dbg_.Assert(r == 0, "%s: Error querying %s\n", __FUNCTION__, FBDEV[n]);
		dbg_.DebugOut(kDbgLvlImportant, "%s: Screen = %d x %d, pitch = %d\n", __FUNCTION__, vinfo[n].xres, vinfo[n].yres, finfo[n].line_length);
	
		// Reset page flip offsets
		vinfo[n].xoffset = vinfo[n].yoffset = 0;
		r = ioctl(fbdev[n], FBIOPUT_VSCREENINFO, &vinfo[n]);
		
		// Map framebuffer into userspace
		fbmem[n] = (U8*)mmap(0, finfo[n].smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbdev[n], 0);
		dbg_.Assert(fbmem[n] != MAP_FAILED, "%s: Error mapping %s\n", __FUNCTION__, FBDEV[n]);
		dbg_.DebugOut(kDbgLvlImportant, "%s: Mapped %08lx to %p, size %u\n", __FUNCTION__, finfo[n].smem_start, fbmem[n], finfo[n].smem_len);
	}
}

//----------------------------------------------------------------------------
void CDisplayFB::DeInitModule()
{
	for (int n = 0; n < NUMFB; n++)
	{
		// Release framebuffer mapping and device
		munmap(fbmem[n], finfo[n].smem_len);
		
		close(fbdev[n]);
	}
}

//----------------------------------------------------------------------------
U32	CDisplayFB::GetScreenSize()
{
	return (vinfo[RGBFB].yres << 16) | (vinfo[RGBFB].xres);
}

//----------------------------------------------------------------------------
tPixelFormat CDisplayFB::GetPixelFormat(void)
{
	return kPixelFormatARGB8888;
}

//----------------------------------------------------------------------------
tDisplayHandle CDisplayFB::CreateHandle(U16 height, U16 width, tPixelFormat colorDepth, U8 *pBuffer)
{
	tDisplayContext* ctx = new tDisplayContext;

	int depth;
	int n, r;
	switch (colorDepth) 
	{
		case kPixelFormatRGB4444:	depth = 16; n = RGBFB; break;
		case kPixelFormatRGB565:	depth = 16; n = RGBFB; break;
		case kPixelFormatRGB888:	depth = 24; n = RGBFB; break; 	
		default:
		case kPixelFormatARGB8888: 	depth = 32; n = RGBFB; break;
		case kPixelFormatYUV420:	depth = 8 ; n = YUVFB; break;
		case kPixelFormatYUYV422:	depth = 16; n = YUVFB; break;
	}		

	// Select pixel format masks for RGB context
	if (n == RGBFB)
	{
		vinfo[n].bits_per_pixel = depth;
		switch (colorDepth)
		{
			case kPixelFormatRGB4444:
				vinfo[n].blue.length = vinfo[n].green.length = 
				vinfo[n].red.length = vinfo[n].transp.length = 4;
				vinfo[n].transp.offset = 12;
				break;
			case kPixelFormatRGB565:
				vinfo[n].blue.length = vinfo[n].red.length = 5;
				vinfo[n].green.length = 6;
				vinfo[n].transp.offset = vinfo[n].transp.length = 0;
				break;
			case kPixelFormatRGB888:
				vinfo[n].blue.length = vinfo[n].green.length = 
				vinfo[n].red.length = 8;
				vinfo[n].transp.offset = vinfo[n].transp.length = 0;
				break;
			case kPixelFormatARGB8888:
				vinfo[n].blue.length = vinfo[n].green.length = 
				vinfo[n].red.length = vinfo[n].transp.length = 8;
				vinfo[n].transp.offset = 24;
				break;
		}
		vinfo[n].blue.offset  = 0;
		vinfo[n].green.offset = vinfo[n].blue.offset + vinfo[n].blue.length;
		vinfo[n].red.offset   = vinfo[n].green.offset + vinfo[n].green.length;
		// Block addressing mode needed for OGL framebuffer context?
		if (colorDepth == kPixelFormatRGB565 && pBuffer == pmem2d)
			vinfo[n].nonstd |= (1<<23);
		else
			vinfo[n].nonstd &= ~(1<<23);
		r = ioctl(fbdev[n], FBIOPUT_VSCREENINFO, &vinfo[n]);
		r = ioctl(fbdev[n], FBIOGET_VSCREENINFO, &vinfo[n]);
		r = ioctl(fbdev[n], FBIOGET_FSCREENINFO, &finfo[n]);
	}
	
	int offset = index * vinfo[n].yres * finfo[n].line_length;
	index++;
	
	memset(ctx, 0, sizeof(tDisplayContext));
	ctx->width				= width;
	ctx->height				= height;
	ctx->colorDepthFormat 	= colorDepth;
	ctx->depth				= depth;
	ctx->bpp				= depth/8;
	ctx->pitch				= finfo[n].line_length;
	ctx->isAllocated		= (pBuffer != NULL);
	ctx->pBuffer			= (pBuffer != NULL) ? pBuffer : fbmem[n] + offset;
	ctx->offset 			= offset;
	ctx->layer				= n;
	ctx->isOverlay			= (n == YUVFB);
	ctx->isPlanar			= (finfo[n].type == FB_TYPE_PLANES);

	return (tDisplayHandle)ctx;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::DestroyHandle(tDisplayHandle hndl, Boolean destroyBuffer)
{
	tDisplayContext* ctx = (tDisplayContext*)hndl;
	
	delete ctx;
	index--;
	
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::RegisterLayer(tDisplayHandle hndl, S16 xPos, S16 yPos)
{
	tDisplayContext* ctx = (tDisplayContext*)hndl;
	
	ctx->rect.left		= xPos;
	ctx->rect.top		= yPos;
	ctx->rect.right		= xPos + ctx->width;
	ctx->rect.bottom 	= yPos + ctx->height;
	
	int n = ctx->layer;
	int r = ioctl(fbdev[n], FBIOBLANK, 0);
	
	return (r == 0) ? kNoErr : kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::UnRegisterLayer(tDisplayHandle hndl)
{
	tDisplayContext* ctx = (tDisplayContext*)hndl;

	int n = ctx->layer;
	int r = ioctl(fbdev[n], FBIOBLANK, 1);
	
	return (r == 0) ? kNoErr : kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::Update(tDisplayContext* dc, int sx, int sy, int dx, int dy, int width, int height)
{
	return kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::SwapBuffers(tDisplayHandle hndl, Boolean waitVSync)
{
	tDisplayContext* ctx = (tDisplayContext*)hndl;
	int n = ctx->layer;
	
	// Note pages are stacked vertically for RGB, horizontally for YUV
	vinfo[n].yoffset = ctx->offset / finfo[n].line_length;
	vinfo[n].xoffset = ctx->offset % finfo[n].line_length;
	int r = ioctl(fbdev[n], FBIOPAN_DISPLAY, &vinfo[n]);

	pdcVisible_->flippedContext = ctx;

	// Wait for VSync option via layer dirty bit?
	if (waitVSync) 
	{
		do {
			r = ioctl(fbdev[n], FBIO_WAITFORVSYNC, 0);
			if (r == 1)
				kernel_.TaskSleep(1);
		} while (r != 0);
	}
	
	return (r == 0) ? kNoErr : kNoImplErr;
}

//----------------------------------------------------------------------------
Boolean	CDisplayFB::IsBufferSwapped(tDisplayHandle hndl)
{
	tDisplayContext* ctx = (tDisplayContext*)hndl;
	
	int n = ctx->layer;
	int r = ioctl(fbdev[n], FBIO_WAITFORVSYNC, 0);
	
	return (r == 0);
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::SetWindowPosition(tDisplayHandle hndl, S16 x, S16 y, U16 width, U16 height, Boolean visible)
{
	return kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::GetWindowPosition(tDisplayHandle hndl, S16& x, S16& y, U16& width, U16& height, Boolean& visible)
{
	return kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::SetAlpha(tDisplayHandle hndl, U8 level, Boolean enable)
{
	return kNoImplErr;
}

//----------------------------------------------------------------------------
U8 	CDisplayFB::GetAlpha(tDisplayHandle hndl) const
{
	return 0;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::SetBacklight(tDisplayScreen screen, S8 backlight)
{
	return kNoImplErr;
}

//----------------------------------------------------------------------------
S8	CDisplayFB::GetBacklight(tDisplayScreen screen)
{
	return 0;
}

//----------------------------------------------------------------------------
void CDisplayFB::InitOpenGL(void* pCtx)
{
	// Dereference OpenGL context for MagicEyes OEM memory size config
	tOpenGLContext* 				pOglCtx = (tOpenGLContext*)pCtx;
	___OAL_MEMORY_INFORMATION__* 	pMemInfo = (___OAL_MEMORY_INFORMATION__*)pOglCtx->pOEM;
	int 							n = RGBFB;
	
	// Open driver for 3D engine registers
	fdreg3d = open(DEV3D, O_RDWR | O_SYNC);
	dbg_.Assert(fdreg3d >= 0, "%s: Opening %s failed\n", __FUNCTION__, DEV3D);

	// Map 3D engine register space
	preg3d = mmap(0, REG3D_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fdreg3d, REG3D_PHYS);
	dbg_.DebugOut(kDbgLvlValuable, "%s: %016lX mapped to %p\n", __FUNCTION__, REG3D_PHYS, preg3d);

	// Open driver for mapping framebuffer memory
	fdmem = open(DEVMEM, O_RDWR | O_SYNC);
	dbg_.Assert(fdmem >= 0, "%s: Opening %s failed\n", __FUNCTION__, DEVMEM);
	
	// Map memory block for 1D heap = command buffer, vertex buffers (not framebuffer)
	finfo[n].smem_start &= ~0x20000000;
	mem1phys = finfo[n].smem_start;
	mem1size = k1Meg;
	pmem1d = mmap((void*)mem1phys, mem1size, PROT_READ | PROT_WRITE, MAP_SHARED, fdmem, mem1phys);
	dbg_.DebugOut(kDbgLvlValuable, "%s: %08X mapped to %p, size = %08X\n", __FUNCTION__, mem1phys, pmem1d, mem1size);

	// Map memory block for 2D heap = framebuffer, Zbuffer, textures
	mem2phys = ALIGN(finfo[n].smem_start + mem1size, k4Meg);
	mem2size = finfo[n].smem_len - ALIGN((mem2phys - mem1phys), k1Meg);
	pmem2d = mmap((void*)mem2phys, mem2size, PROT_READ | PROT_WRITE, MAP_SHARED, fdmem, mem2phys);
	dbg_.DebugOut(kDbgLvlValuable, "%s: %08X mapped to %p, size = %08X\n", __FUNCTION__, mem2phys, pmem2d, mem2size);

	// Copy the required mappings into the MagicEyes callback init struct
	pMemInfo->VirtualAddressOf3DCore	= (unsigned int)preg3d;
	pMemInfo->Memory1D_VirtualAddress	= (unsigned int)pmem1d;
	pMemInfo->Memory1D_PhysicalAddress	= mem1phys;
	pMemInfo->Memory1D_SizeInMbyte		= mem1size >> 20;
	pMemInfo->Memory2D_VirtualAddress	= (unsigned int)pmem2d;
	pMemInfo->Memory2D_PhysicalAddress	= mem2phys;
	pMemInfo->Memory2D_SizeInMbyte		= mem2size >> 20;

	// Create DisplayMPI context for OpenGL framebuffer
	hogl = CreateHandle(vinfo[n].yres, vinfo[n].xres, kPixelFormatRGB565, (U8*)pmem2d);

	// Pass back essential display context info for OpenGL bindings
	pOglCtx->width 		= vinfo[n].xres;
	pOglCtx->height 	= vinfo[n].yres;
	pOglCtx->eglDisplay = &finfo[n]; // non-NULL ptr
	pOglCtx->eglWindow 	= &vinfo[n]; // non-NULL ptr
	pOglCtx->hndlDisplay = hogl;
}

//----------------------------------------------------------------------------
void CDisplayFB::DeinitOpenGL()
{
	DestroyHandle(hogl, false);
	
	// Release framebuffer mappings and drivers used by OpenGL
	munmap(pmem2d, mem2size);
	munmap(pmem1d, mem1size);
	munmap(preg3d, REG3D_SIZE);
	
	close(fdmem);
	close(fdreg3d);
}

//----------------------------------------------------------------------------
void CDisplayFB::EnableOpenGL(void* pCtx)
{
	int n = RGBFB;
	int r = ioctl(fbdev[n], FBIOBLANK, 0);
}

//----------------------------------------------------------------------------
void CDisplayFB::DisableOpenGL()
{
	int n = RGBFB;
	int r = ioctl(fbdev[n], FBIOBLANK, 1);
}

//----------------------------------------------------------------------------
void CDisplayFB::UpdateOpenGL()
{
}

//----------------------------------------------------------------------------
void CDisplayFB::WaitForDisplayAddressPatched(void)
{
	int n = RGBFB;
	int r = 0; 
	do {
		r = ioctl(fbdev[n], FBIO_WAITFORVSYNC, 0);
		if (r == 1)
			kernel_.TaskSleep(1);
	} while (r != 0);
}

//----------------------------------------------------------------------------
void CDisplayFB::SetOpenGLDisplayAddress(const unsigned int DisplayBufferPhysicalAddress)
{
	unsigned int offset = DisplayBufferPhysicalAddress - mem1phys;
	int n = RGBFB;
	offset &= ~0x20000000;
	vinfo[n].yoffset = offset / finfo[n].line_length;
	vinfo[n].xoffset = offset % finfo[n].line_length;
	int r = ioctl(fbdev[n], FBIOPAN_DISPLAY, &vinfo[n]);
}

//----------------------------------------------------------------------------
U32	CDisplayFB::GetDisplayMem(tDisplayMem memtype)
{
}

//----------------------------------------------------------------------------

LF_END_BRIO_NAMESPACE()
// EOF
