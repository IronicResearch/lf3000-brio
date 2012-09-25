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
#ifdef LF1000
#include <GLES/libogl.h>
#endif
#include <algorithm>
#include <list>

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/lf1000/lf1000fb.h>

LF_BEGIN_BRIO_NAMESPACE()

#define RGBFB					0	// index of RGB framebuffer
#define OGLFB					1	// index of OGL framebuffer
#define YUVFB					2	// index of YUV framebuffer
#define NUMFB					3	// number of framebuffer devices

#define	REG3D_PHYS				0xC001A000UL	// 3D engine register block
#define REG3D_SIZE				0x00002000

//tDisplayHandle		hogl = NULL;
std::vector<tDisplayHandle> hogls;
namespace 
{
	const char*					FBDEV[NUMFB] = {"/dev/fb0", "/dev/fb1", "/dev/fb2"};
	int 						fbdev[NUMFB] = {-1, -1, -1};
	struct fb_fix_screeninfo 	finfo[NUMFB];
	struct fb_var_screeninfo 	vinfo[NUMFB];
	U8*							fbmem[NUMFB] = {NULL, NULL, NULL};
	bool						fbviz[NUMFB] = {false, false, false};
	bool						fblnd[NUMFB] = {false, false, false};
	int							fboff[NUMFB] = {0, 0, 0};

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
	void*						pmem2d = MAP_FAILED;
	//tDisplayHandle		hogl = NULL;
	//tDisplayHandle				hdcogl[2] = {NULL, NULL};
	tDisplayContext				dcmem1;		// memory block context for 1D heap
	tDisplayContext				dcmem2;		// memory block context for 2D heap
	
	std::list<tBuffer>			gBufListUsed;			// list of allocated buffers
	std::list<tBuffer>			gBufListFree;			// list of freed buffers
	U32							gMarkBufStart = 0;		// framebuffer start marker
	U32							gMarkBufEnd = 0;		// framebuffer end marker
	tMutex 						gListMutex = PTHREAD_MUTEX_INITIALIZER;	// list mutex

	const char*					XRES = "/sys/devices/platform/lf1000-dpc/xres";
	const char*					YRES = "/sys/devices/platform/lf1000-dpc/yres";
	unsigned int				xres = 0;	// screen size X
	unsigned int				yres = 0;	// screen size Y
	S16							dxres = 0;	// screen delta X
	S16							dyres = 0;	// screen delta Y
	U16							vxres = 0;	// viewport size X
	U16							vyres = 0;	// viewport size Y
	U16							oxres = 0;
	U16							oyres = 0;
	unsigned int				xscale = 0;	// video scaler X
	unsigned int				yscale = 0; // video scaler Y
}

//============================================================================
// CDisplayFB: Implementation of hardware-specific functions
//============================================================================
CDisplayFB::CDisplayFB(CDisplayModule* pModule) :
	CDisplayDriver(pModule)
{
}

//----------------------------------------------------------------------------
CDisplayFB::~CDisplayFB()
{
}

//----------------------------------------------------------------------------
void CDisplayFB::InitModule()
{
	int r;
	struct stat stbuf;

	// Query screen size independently of display mode resolution
	FILE* f = fopen(XRES, "r");
	if (f) {
		fscanf(f, "%u", &xres);
		fclose(f);
	}
	f = fopen(YRES, "r");
	if (f) {
		fscanf(f, "%u", &yres);
		fclose(f);
	}
	f = fopen("/sys/devices/system/board/lcd_size", "r");
	if (f) {
		fscanf(f, "%ux%u", &xres, &yres);
		fclose(f);
	}
	dbg_.DebugOut(kDbgLvlImportant, "%s: Screen = %u x %u\n", __FUNCTION__, xres, yres);
	
	for (int n = 0; n < NUMFB; n++)
	{
		// Open framebuffer device
		fbdev[n] = open(FBDEV[n], O_RDWR | O_SYNC);
		//dbg_.Assert(fbdev[n] >= 0, "%s: Error opening %s\n", __FUNCTION__, FBDEV[n]);
		
		// Query framebuffer info
		r = ioctl(fbdev[n], FBIOGET_FSCREENINFO, &finfo[n]);
		//dbg_.Assert(r == 0, "%s: Error querying %s\n", __FUNCTION__, FBDEV[n]);
		
		r = ioctl(fbdev[n], FBIOGET_VSCREENINFO, &vinfo[n]);
		//dbg_.Assert(r == 0, "%s: Error querying %s\n", __FUNCTION__, FBDEV[n]);
		dbg_.DebugOut(kDbgLvlImportant, "%s: Screen = %d x %d, pitch = %d\n", __FUNCTION__, vinfo[n].xres, vinfo[n].yres, finfo[n].line_length);
	
		// Conditionally blank layers after initial launch and not playing transition video
		if (stat("/tmp/splash", &stbuf) == 0)
		{
			if (n == YUVFB && ((stat("/tmp/trans_anim", &stbuf) != 0) || (stat("/tmp/vdaemon_play", &stbuf) != 0)))
				r = ioctl(fbdev[n], FBIOBLANK, 1);
		}
		else if (stat("/flags/main_app", &stbuf) == 0)
		{
			r = ioctl(fbdev[n], FBIOBLANK, 1);
		}

		// Map framebuffer into userspace
		fbmem[n] = (U8*)mmap((void*)finfo[n].smem_start, finfo[n].smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbdev[n], 0);
		//dbg_.Assert(fbmem[n] != MAP_FAILED, "%s: Error mapping %s\n", __FUNCTION__, FBDEV[n]);
		dbg_.DebugOut(kDbgLvlImportant, "%s: Mapped %08lx to %p, size %08x\n", __FUNCTION__, finfo[n].smem_start, fbmem[n], finfo[n].smem_len);
	}

	// Calculate delta XY for screen size vs display resolution
	dxres = 0;
	dyres = 0;
	xres = vxres = (xres) ? xres : vinfo[RGBFB].xres;
	yres = vyres = (yres) ? yres : vinfo[RGBFB].yres;
	
	// Setup framebuffer allocator lists and markers
	gBufListUsed.clear();
	gBufListFree.clear();
	gMarkBufStart = xres * yres * 2 * 4;//2 screens worth and 4 bytes per pixel.  Reserving for EGL.
	gMarkBufEnd   = finfo[RGBFB].smem_len + finfo[OGLFB].smem_len + finfo[YUVFB].smem_len;

	// Pre-allocate OGL display contexts compatible with Nexell EGL framebuffer usage
	//hdcogl[0] = CreateHandle(yres, xres, kPixelFormatARGB8888, NULL);
	//hdcogl[1] = CreateHandle(yres, xres, kPixelFormatARGB8888, NULL);
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
	// Adjust effective screen size coordinates if viewport active
	pModule_->GetViewport(pModule_->GetCurrentDisplayHandle(), dxres, dyres, vxres, vyres, oxres, oyres);
	if ((oxres > 0 && oxres < xres) || (oyres > 0 && oyres < yres))
		return (oyres << 16) | (oxres);
	return (yres << 16) | (xres);
}

//----------------------------------------------------------------------------
tPixelFormat CDisplayFB::GetPixelFormat(void)
{
	return kPixelFormatARGB8888;
}

//----------------------------------------------------------------------------
int GetZOrder(tDisplayContext* ctx)
{
	int n = ctx->layer;
	int z = n;
	int r = 0;

	if (n == RGBFB && ctx->initialZOrder != kDisplayOnOverlay)
		z = 1;
	else if (n == OGLFB && ctx->initialZOrder == kDisplayOnOverlay)
		z = 0;

#if 0
	if (n != YUVFB && (z != n || z != NONSTD_TO_POS(vinfo[n].nonstd)))
	{
		vinfo[n].nonstd &= ~(3<<LF1000_NONSTD_PRIORITY);
		vinfo[n].nonstd |=  (z<<LF1000_NONSTD_PRIORITY);
		r = ioctl(fbdev[n], FBIOPUT_VSCREENINFO, &vinfo[n]);
	}
#endif
	
	return z;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::SetPixelFormat(int n, U16 width, U16 height, U16 depth, tPixelFormat colorDepth, bool isBlockAddr)
{
	int r = kNoErr;

	// Select pixel format masks for RGB context
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
			case kPixelFormatYUV420:
				vinfo[n].nonstd &= ~(LF1000_NONSTD_FORMAT_MASK << LF1000_NONSTD_FORMAT);
				vinfo[n].nonstd |= (LAYER_FORMAT_YUV420 << LF1000_NONSTD_FORMAT);
				break;
			case kPixelFormatYUYV422:
				vinfo[n].nonstd &= ~(LF1000_NONSTD_FORMAT_MASK << LF1000_NONSTD_FORMAT);
				vinfo[n].nonstd |= (LAYER_FORMAT_YUV422 << LF1000_NONSTD_FORMAT);
				break;
			case kPixelFormatXRGB8888:
				vinfo[n].blue.length = vinfo[n].green.length =
				vinfo[n].red.length = 8;
				vinfo[n].transp.length = 0;
				vinfo[n].transp.offset = 24;
				break;
		}
		vinfo[n].blue.offset  = 0;
		vinfo[n].green.offset = vinfo[n].blue.offset + vinfo[n].blue.length;
		vinfo[n].red.offset   = vinfo[n].green.offset + vinfo[n].green.length;
		// Swizzle RGB for OGL
		/*if (isBlockAddr && colorDepth == kPixelFormatARGB8888) {
			vinfo[n].blue.offset  = vinfo[n].red.offset;
			vinfo[n].red.offset   = 0;
		}*/
		// Block addressing mode needed for OGL framebuffer context?
		if (isBlockAddr)
			vinfo[n].nonstd |=  (1<<LF1000_NONSTD_PLANAR);
		else
			vinfo[n].nonstd &= ~(1<<LF1000_NONSTD_PLANAR);
		// Change effective resolution for any onscreen context
		vinfo[n].xres = width;
		vinfo[n].yres = height;
		r = ioctl(fbdev[n], FBIOPUT_VSCREENINFO, &vinfo[n]);
		r = ioctl(fbdev[n], FBIOGET_VSCREENINFO, &vinfo[n]);
		r = ioctl(fbdev[n], FBIOGET_FSCREENINFO, &finfo[n]);
	}

	return (r == 0) ? kNoErr : kNoImplErr;
}

//----------------------------------------------------------------------------
tDisplayHandle CDisplayFB::CreateHandle(U16 height, U16 width, tPixelFormat colorDepth, U8 *pBuffer)
{
	tDisplayContext* ctx = new tDisplayContext;

	int depth;
	int n, r;
	switch (colorDepth) 
	{
		// Default to lower RGB layer -- FIXME
		case kPixelFormatRGB4444:	depth = 16; n = OGLFB; break;
		case kPixelFormatRGB565:	depth = 16; n = OGLFB; break;
		case kPixelFormatRGB888:	depth = 24; n = OGLFB; break;
		case kPixelFormatXRGB8888:	depth = 32; n = OGLFB; break;
		default:
		case kPixelFormatARGB8888: 	depth = 32; n = OGLFB; break;
		case kPixelFormatYUV420:	depth = 8 ; n = YUVFB; break;
		case kPixelFormatYUYV422:	depth = 16; n = YUVFB; break;
	}		
	
	// OGL framebuffer context?
	if (colorDepth == kPixelFormatRGB565 && pBuffer == pmem2d ||
		colorDepth == kPixelFormatARGB8888 && pBuffer == fbmem[OGLFB])
		n = OGLFB;

	// Update dxres && dyres in case someone called SetViewPort without calling GetScreenStats
	pModule_->GetViewport(pModule_->GetCurrentDisplayHandle(), dxres, dyres, vxres, vyres, oxres, oyres);
	// FIXME: RGB lower layer needed when viewport active
	// FIXME: Switch RGB layer prior to isUnderlay setting
	if (n == RGBFB && (dxres > 0 || dyres > 0))
		n = OGLFB;
	
	// Block addressing mode needed for OGL framebuffer context?
	if (colorDepth == kPixelFormatRGB565 && pBuffer == pmem2d ||
		colorDepth == kPixelFormatARGB8888 && pBuffer == fbmem[OGLFB])
		r = SetPixelFormat(n, width, height, depth, colorDepth, true);
	else if (pBuffer == NULL)
		r = SetPixelFormat(n, width, height, depth, colorDepth, false);

	// Pitch depends on width for normal RGB modes, otherwise
	// finfo[n].line_length may get updated after SetPixelFormat().
	int line_length = width * depth/8;
	//if (n == YUVFB || (colorDepth == kPixelFormatRGB565 && pBuffer == pmem2d))
		line_length = finfo[n].line_length;
	int offset = 0;
	int aligned = (n == YUVFB) ? ALIGN(height * line_length, k1Meg) : 0;
	
	memset(ctx, 0, sizeof(tDisplayContext));
	ctx->width				= width;
	ctx->height				= height;
	ctx->colorDepthFormat 	= colorDepth;
	ctx->depth				= depth;
	ctx->bpp				= depth/8;
	ctx->pitch				= (pBuffer != NULL) ? width * depth/8 : line_length;
	ctx->isAllocated		= (pBuffer != NULL);
	ctx->pBuffer			= (pBuffer != NULL) ? pBuffer : fbmem[n] + offset;
	ctx->offset 			= offset;
	ctx->layer				= n;
	ctx->isVideo			= (n == YUVFB);
	ctx->isPlanar			= (n == YUVFB); // (finfo[n].type == FB_TYPE_PLANES);
	ctx->initialZOrder		= kDisplayOnBottom; // default
	ctx->rect.right			= width;
	ctx->rect.bottom		= height;
	ctx->xscale 			= width;
	ctx->yscale 			= height;
	ctx->isOpenGL			= false;

	// Allocate framebuffer memory if onscreen context
	if (pBuffer == NULL)
	{	
		if (!AllocBuffer(ctx, aligned))
		{
			dbg_.DebugOut(kDbgLvlCritical, "%s: No framebuffer allocation available\n", __FUNCTION__);
			delete ctx;
			return kInvalidDisplayHandle;
		}
		// Clear the onscreen display buffer
		memset(ctx->pBuffer, 0, ctx->pitch * ctx->height);
	}
	
	// Fixup offscreen context info if cloned from onscreen context (VideoMPI)
	if (pBuffer >= fbmem[n] && pBuffer <= fbmem[n] + finfo[n].smem_len)
	{
		ctx->offset = pBuffer - fbmem[n];
		ctx->pitch  = finfo[n].line_length;
		if (n == YUVFB)
		switch(NONSTD_TO_POS(vinfo[n].nonstd))
		{
		case 0:
			ctx->initialZOrder = kDisplayOnOverlay;
			break;
		case 1:
			ctx->initialZOrder = kDisplayOnTop;
			break;
		case 2:
			ctx->initialZOrder = kDisplayOnBottom;
			break;
		}
	}

	dbg_.DebugOut(kDbgLvlVerbose, "%s: %p: %dx%d (%d) @ %p\n", __FUNCTION__, ctx, width, height, ctx->pitch, ctx->pBuffer);
	
	return (tDisplayHandle)ctx;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::DestroyHandle(tDisplayHandle hndl, Boolean destroyBuffer)
{
	tDisplayContext* ctx = (tDisplayContext*)hndl;
	
	dbg_.DebugOut(kDbgLvlVerbose, "%s: %p: %dx%d (%d) @ %p\n", __FUNCTION__, ctx, ctx->width, ctx->height, ctx->pitch, ctx->pBuffer);

	pModule_->UnRegister(hndl, 0);

	// Reset screen size deltas when viewport context destroyed
	if (ctx->viewport) {
		dxres = dyres = 0;
		vxres = xres;
		vyres = yres;
	}

	if (!ctx->isAllocated)
		DeAllocBuffer(ctx);
	delete ctx;
	
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::RegisterLayer(tDisplayHandle hndl, S16 xPos, S16 yPos)
{
	tDisplayContext* ctx = (tDisplayContext*)hndl;

	dbg_.DebugOut(kDbgLvlVerbose, "%s: %p: %dx%d (%d) @ %p\n", __FUNCTION__, ctx, ctx->width, ctx->height, ctx->pitch, ctx->pBuffer);

	U16 width			= ctx->rect.right - ctx->rect.left;
	U16 height			= ctx->rect.bottom - ctx->rect.top;
	ctx->rect.left 		= ctx->x = xPos;
	ctx->rect.top  		= ctx->y = yPos;
	ctx->rect.right		= xPos + width;
	ctx->rect.bottom 	= yPos + height;
	
	// Offscreen contexts do not affect screen
	if (ctx->isAllocated && ctx->isOpenGL)
		return kNoErr;

	// Set XY onscreen position
	int n = ctx->layer;
	int z = n;
	int r = 0;

	// Change to upper RGB layer for explicit overlay registration
	if (n == OGLFB && ctx->initialZOrder== kDisplayOnOverlay)
		n = ctx->layer = RGBFB;

#if 0 // FIXME
	// Change to upper RGB layer if lower RGB layer is in use for OGL and no viewport active
	if (n == OGLFB && fbviz[OGLFB] && ctx->initialZOrder == kDisplayOnTop && hogl != NULL && hndl != hogl && !(vxres < xres || vyres < yres))
		n = ctx->layer = RGBFB;
#else
	z = GetZOrder(ctx);
#endif

	// Disable *any* layer's pixel format or resolution change
	if (fbviz[n] && (ctx->width != vinfo[n].xres || ctx->height != vinfo[n].yres || ctx->depth != vinfo[n].bits_per_pixel))
	{
		SetVisible(ctx, false);
	}
	
	// Adjust Z-order for YUV layer?
	if (n == YUVFB) 
	{
		int z;
		switch(ctx->initialZOrder)
		{
		case kDisplayOnTop:
			z = 1;
			break;
		case kDisplayOnBottom:
			z = 2;
			break;
		case kDisplayOnOverlay:
			z = 0;
			break;
		}
		vinfo[n].nonstd &= ~(3<<LF1000_NONSTD_PRIORITY);
		vinfo[n].nonstd |=  (z<<LF1000_NONSTD_PRIORITY);
		//r = ioctl(fbdev[n], FBIOPUT_VSCREENINFO, &vinfo[n]);
	}
	r = SetPixelFormat(n, ctx->width, ctx->height, ctx->depth, ctx->colorDepthFormat, ctx->isOpenGL);
	r = SetWindowPosition(ctx, xPos, yPos, width, height);
	if (n == YUVFB) 
	{
		struct lf1000fb_vidscale_cmd cmd;
		cmd.sizex = (xscale) ? xscale : ctx->xscale;
		cmd.sizey = (yscale) ? yscale : ctx->yscale;
		cmd.apply = 1;
		r = ioctl(fbdev[n], LF1000FB_IOCSVIDSCALE, &cmd);
	}
	// Set framebuffer address offset
	vinfo[n].yoffset = ctx->offset / finfo[n].line_length;
	vinfo[n].xoffset = (ctx->offset % finfo[n].line_length) * 8 / vinfo[n].bits_per_pixel;
	r = ioctl(fbdev[n], FBIOPAN_DISPLAY, &vinfo[n]);

	// Set alpha blend state
	SetAlpha(ctx, ctx->alphaLevel, ctx->isBlended);
	
	// Defer layer visibility until Update() or SwapBuffers()
	
	return (r == 0) ? kNoErr : kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::UnRegisterLayer(tDisplayHandle hndl)
{
	tDisplayContext* ctx = (tDisplayContext*)hndl;

	dbg_.DebugOut(kDbgLvlVerbose, "%s: %p: %dx%d (%d) @ %p\n", __FUNCTION__, ctx, ctx->width, ctx->height, ctx->pitch, ctx->pBuffer);

	// Nullify flipped contexts tracked in SwapBuffers()
	if (pdcVisible_) {
		if (pdcVisible_->flippedContext == ctx)
			pdcVisible_->flippedContext = NULL;
		if (pdcVisible_ == ctx)
			pdcVisible_ = NULL;
	}

	// Offscreen contexts do not affect screen
	if (ctx->isAllocated)
		return kNoErr;
	
	int r = SetVisible(ctx, false);
	

	return (r == 0) ? kNoErr : kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::Update(tDisplayContext* dc, int sx, int sy, int dx, int dy, int width, int height)
{
	if (pdcVisible_ == NULL)
		return kDisplayDisplayNotInListErr;

	tDisplayContext *dcdst = pdcVisible_;
	if (dcdst->flippedContext)
		dcdst = dcdst->flippedContext;
	
	// Copy offscreen context to primary display context
	if (dc->isAllocated && !dc->offset)
	{
		switch (dc->colorDepthFormat) 
		{
		case kPixelFormatRGB4444: 	
			RGB4444ARGB(dc, dcdst, sx, sy, dx, dy, width, height); 
			break;
		case kPixelFormatRGB565: 	
			RGB565ARGB(dc, dcdst, sx, sy, dx, dy, width, height); 
			break;
		case kPixelFormatRGB888: 	
			RGB2ARGB(dc, dcdst, sx, sy, dx, dy, width, height); 
			break;
		default:
		case kPixelFormatARGB8888: 	
			switch (dcdst->colorDepthFormat)
			{
			case kPixelFormatRGB4444: 	
				ARGB2RGB4444(dc, dcdst, sx, sy, dx, dy, width, height); 
				break;
			case kPixelFormatRGB565: 	
				ARGB2RGB565(dc, dcdst, sx, sy, dx, dy, width, height); 
				break;
			case kPixelFormatRGB888: 	
				ARGB2RGB(dc, dcdst, sx, sy, dx, dy, width, height); 
				break;
			default:
			case kPixelFormatARGB8888:	
				ARGB2ARGB(dc, dcdst, sx, sy, dx, dy, width, height); 
				break;
			case kPixelFormatYUV420: 	
				RGB2YUV(dc, dcdst, sx, sy, dx, dy, width, height); 
				break;
			}
			break;
		case kPixelFormatYUV420: 	
			YUV2ARGB(dc, dcdst, sx, sy, dx, dy, width, height); 
			break;
		case kPixelFormatYUYV422: 	
			YUYV2ARGB(dc, dcdst, sx, sy, dx, dy, width, height); 
			break;
		}
	}

	// Make sure primary display context is enabled
	if (!dcdst->isEnabled || !fbviz[dcdst->layer])
	{
		RegisterLayer(dcdst, dcdst->x, dcdst->y);
		SetVisible(dcdst, true);
	}
	// Ditto for any other onscreen display context
	if (dc != dcdst && !dc->isAllocated && (!dc->isEnabled || !fbviz[dc->layer]))
	{
		RegisterLayer(dc, dc->x, dc->y);
		SetVisible(dc, true);
	}

	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::SwapBuffers(tDisplayHandle hndl, Boolean waitVSync)
{
	tDisplayContext* ctx = (tDisplayContext*)hndl;
	int n = ctx->layer;
	
	// Update invalidated offscreen regions prior to page flip (VideoMPI)
	if (ctx->initialZOrder != kDisplayOnBottom)
		pdcVisible_->flippedContext = ctx;
	if (ctx->isVideo)
		pModule_->Invalidate(0, NULL);

	// Note pages are stacked vertically for RGB, horizontally for YUV
	vinfo[n].yoffset = ctx->offset / finfo[n].line_length;
	vinfo[n].xoffset = (ctx->offset % finfo[n].line_length) * 8 / vinfo[n].bits_per_pixel;
	int z = GetZOrder(ctx);
	int r = ioctl(fbdev[n], FBIOPAN_DISPLAY, &vinfo[n]);

	// Make sure swapped display context is enabled
	if (!ctx->isEnabled)
	{
		SetVisible(ctx, true);
	}
	
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
	int z = GetZOrder(ctx);
	int r = ioctl(fbdev[n], FBIO_WAITFORVSYNC, 0);
	
	return (r == 0);
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::SetWindowPosition(tDisplayHandle hndl, S16 x, S16 y, U16 width, U16 height)
{
	tDisplayContext* ctx = (tDisplayContext*)hndl;
	
	struct lf1000fb_position_cmd cmd;
	cmd.left   = ctx->rect.left   = ctx->x = x;
	cmd.top    = ctx->rect.top    = ctx->y = y;
	cmd.right  = ctx->rect.right  = ctx->rect.left + std::min(ctx->width, width);
	cmd.bottom = ctx->rect.bottom = ctx->rect.top + std::min(ctx->height, height);
	cmd.apply  = 1;

	dbg_.DebugOut(kDbgLvlVerbose, "%s: %p: %d,%d .. %d,%d\n", __FUNCTION__, ctx, ctx->x, ctx->y, ctx->rect.right, ctx->rect.bottom);

	// Auto-center UI elements on larger screens by delta XY
	if (ctx->width < xres)
	{
		cmd.left	+= dxres;
		cmd.right	+= dxres;
	}
	if(ctx->height < yres)
	{
		cmd.top		+= dyres;
		cmd.bottom	+= dyres;
	}

	int n = ctx->layer;
	int z = GetZOrder(ctx);
	int r = ioctl(fbdev[n], LF1000FB_IOCSPOSTION, &cmd);

	return (r == 0) ? kNoErr : kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::SetVisible(tDisplayHandle hndl, Boolean visible)
{
	tDisplayContext* ctx = (tDisplayContext*)hndl;

	int n = ctx->layer;
	int z = GetZOrder(ctx);
	int r = ioctl(fbdev[n], FBIOBLANK, !visible);

	if (r == 0)
		ctx->isEnabled = fbviz[n] = visible;

	return (r == 0) ? kNoErr : kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::SetWindowPosition(tDisplayHandle hndl, S16 x, S16 y, U16 width, U16 height, Boolean visible)
{
	tErrType r;

	r = SetWindowPosition(hndl, x, y, width, height);

	if (r == 0)
	{
		r = SetVisible(hndl, visible);
	}
	
	return (r == 0) ? kNoErr : kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::GetWindowPosition(tDisplayHandle hndl, S16& x, S16& y, U16& width, U16& height, Boolean& visible)
{
	tDisplayContext* ctx = (tDisplayContext*)hndl;
	
	struct lf1000fb_position_cmd cmd;
	int n = ctx->layer;
	int z = GetZOrder(ctx);
	int r = ioctl(fbdev[n], LF1000FB_IOCGPOSTION, &cmd);
	
	x = ctx->rect.left = cmd.left;
	y = ctx->rect.top  = cmd.top;
	ctx->rect.right    = cmd.right;
	ctx->rect.bottom   = cmd.bottom;
	width  = ctx->rect.right - ctx->rect.left;
	height = ctx->rect.bottom - ctx->rect.top;
	visible = ctx->isEnabled;
	
	return (r == 0) ? kNoErr : kNoImplErr;
}
//----------------------------------------------------------------------------
tErrType CDisplayFB::SetVideoScaler(tDisplayHandle hndl, U16 width, U16 height, Boolean centered)
{
	tDisplayContext* ctx = (tDisplayContext*)hndl;

	dbg_.DebugOut(kDbgLvlVerbose, "%s: %p: %dx%d (%d)\n", __FUNCTION__, ctx, width, height, ctx->layer);

	if (ctx->layer == YUVFB) {
		ctx->xscale = width;
		ctx->yscale = height;
		xscale = yscale = 0;
	}
	else {
		xscale = width;
		yscale = height;
	}
	
	struct lf1000fb_vidscale_cmd cmd;
	cmd.sizex = width;
	cmd.sizey = height;
	cmd.apply = 1;
	int n = YUVFB; //ctx->layer;
	int r = ioctl(fbdev[n], LF1000FB_IOCSVIDSCALE, &cmd);
	
	return (r == 0) ? kNoErr : kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::GetVideoScaler(tDisplayHandle hndl, U16& width, U16& height, Boolean& centered)
{
	tDisplayContext* ctx = (tDisplayContext*)hndl;
	
	struct lf1000fb_vidscale_cmd cmd;
	int n = YUVFB; //ctx->layer;
	int r = ioctl(fbdev[n], LF1000FB_IOCGVIDSCALE, &cmd);
	width = cmd.sizex;
	height = cmd.sizey;
	
	return (r == 0) ? kNoErr : kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::SetAlpha(tDisplayHandle hndl, U8 level, Boolean enable)
{
	tDisplayContext* ctx = (tDisplayContext*)hndl;

	ctx->alphaLevel = level;
	ctx->isBlended  = enable;
	
	if(ctx->isOpenGL && level != 100 && enable && ctx->colorDepthFormat == kPixelFormatARGB8888)
	{
		ctx->colorDepthFormat = kPixelFormatXRGB8888;
		SetPixelFormat(ctx->layer, ctx->width, ctx->height, ctx->depth, ctx->colorDepthFormat, ctx->isOpenGL);
		SetWindowPosition(hndl, ctx->x, ctx->y,  ctx->rect.right -  ctx->rect.left,   ctx->rect.bottom - ctx->rect.top);
	}
	
	struct lf1000fb_blend_cmd cmd;
	cmd.alpha = level * ALPHA_STEP / 100;
	cmd.enable = enable;
	int n = ctx->layer;
	int z = GetZOrder(ctx);
	int r = ioctl(fbdev[n], LF1000FB_IOCSALPHA, &cmd);
	fblnd[n] = (r == 0) ? enable : false;
	
	return (r == 0) ? kNoErr : kNoImplErr;
}

//----------------------------------------------------------------------------
U8 	CDisplayFB::GetAlpha(tDisplayHandle hndl) const
{
	tDisplayContext* ctx = (tDisplayContext*)hndl;

	struct lf1000fb_blend_cmd cmd;
	int n = ctx->layer;
	int z = GetZOrder(ctx);
	int r = ioctl(fbdev[n], LF1000FB_IOCGALPHA, &cmd);
	
	return (r == 0) ? cmd.alpha * 100 / ALPHA_STEP : 0;
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
static inline bool Check3DEngine(U32* preg32, U32& cntl32, U32& stat32, U32& clok32)
{
	// Check 3D engine running status to avoid hanging in OGL init callback
	cntl32 = preg32[0];
	stat32 = preg32[0x0014>>2]; // idle status=05BF8E41
	clok32 = preg32[0x1FC0>>2]; // disabled clock=00000000
	if (clok32 != 0 || (stat32 & 0x00BF0000) != 0x00BF0000) {
		FILE* fp = fopen("/tmp/3dlockup", "w");
		if (fp) {
			fprintf(fp, "status=%08X\n", (unsigned)stat32);
			fclose(fp);
		}
		return false;
	}
	return true;
}

//----------------------------------------------------------------------------
void CDisplayFB::InitOpenGL(void* pCtx)
{
	// Dereference OpenGL context for MagicEyes OEM memory size config
	tOpenGLContext* 				pOglCtx = (tOpenGLContext*)pCtx;
	int 							n = OGLFB;
#ifdef LF1000
	___OAL_MEMORY_INFORMATION__* 	pMemInfo = (___OAL_MEMORY_INFORMATION__*)pOglCtx->pOEM;
	
	// Open driver for 3D engine registers
	fdreg3d = open(DEV3D, O_RDWR | O_SYNC);
	dbg_.Assert(fdreg3d >= 0, "%s: Opening %s failed\n", __FUNCTION__, DEV3D);

	// Map 3D engine register space
	preg3d = mmap(0, REG3D_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fdreg3d, REG3D_PHYS);
	dbg_.DebugOut(kDbgLvlValuable, "%s: %016lX mapped to %p\n", __FUNCTION__, REG3D_PHYS, preg3d);

	// Check 3D engine running status to avoid hanging in OGL init callback
	U32 cntl32, stat32, clok32;
	Check3DEngine((U32*)preg3d, cntl32, stat32, clok32);
	dbg_.Assert(clok32 == 0, "%s: Reg3D control=%08X, status=%08X, clock=%08X\n",
			__FUNCTION__, (unsigned)cntl32, (unsigned)stat32, (unsigned)clok32);

	// Clamp requested 1D/2D heap sizes to MagicEyes OGL lib limits
	if (pMemInfo->Memory2D_SizeInMbyte > 14)
		pMemInfo->Memory2D_SizeInMbyte = 14;
	if (pMemInfo->Memory1D_SizeInMbyte + pMemInfo->Memory2D_SizeInMbyte > 16)
		pMemInfo->Memory1D_SizeInMbyte = 16 - pMemInfo->Memory2D_SizeInMbyte;

	// Get OpenGL context memory heap size requests
	mem1size = pMemInfo->Memory1D_SizeInMbyte << 20;
	mem2size = pMemInfo->Memory2D_SizeInMbyte << 20;
	U32 align = ALIGN(mem2size, k4Meg);
	U32 delta = align - mem2size;

	// Allocate 4Meg aligned buffer for 2D heap
	memset(&dcmem1, 0, sizeof(dcmem1));
	memset(&dcmem2, 0, sizeof(dcmem2));
	tDisplayContext* pdb = &dcmem2;
	for (mem2size = align; mem2size >= k4Meg; mem2size -= k4Meg) {
		if (AllocBuffer(pdb, mem2size)) {
			mem2phys = pdb->basephys + pdb->offset;
			pmem2d = pdb->pBuffer;
			break;
		}
	}
	// Check if 1Meg buffer can fit into 4Meg buffer alignment margin
	if (delta >= mem1size) {
		mem2size -= delta;
		mem1size = delta;
		mem1phys = mem2phys + mem2size;
		pmem1d = pdb->pBuffer + mem2size;
	}
	else {
		// Allocate 1Meg aligned buffer for 1D heap
		pdb = &dcmem1;
		for ( ; mem1size >= k1Meg; mem1size -= k1Meg) {
			if (AllocBuffer(pdb, mem1size)) {
				mem1phys = pdb->basephys + pdb->offset;
				pmem1d = pdb->pBuffer;
				break;
			}
		}
	}
	dbg_.DebugOut(kDbgLvlValuable, "%s: %08X mapped to %p, size = %08X\n", __FUNCTION__, mem1phys, pmem1d, mem1size);
	dbg_.DebugOut(kDbgLvlValuable, "%s: %08X mapped to %p, size = %08X\n", __FUNCTION__, mem2phys, pmem2d, mem2size);

	// Copy the required mappings into the MagicEyes callback init struct
	pMemInfo->VirtualAddressOf3DCore	= (unsigned int)preg3d;
	pMemInfo->Memory1D_VirtualAddress	= (unsigned int)pmem1d;
	pMemInfo->Memory1D_PhysicalAddress	= mem1phys;
	pMemInfo->Memory1D_SizeInMbyte		= mem1size >> 20;
	pMemInfo->Memory2D_VirtualAddress	= (unsigned int)pmem2d;
	pMemInfo->Memory2D_PhysicalAddress	= mem2phys;
	pMemInfo->Memory2D_SizeInMbyte		= mem2size >> 20;
#endif // LF1000

	// Query viewport size to use
	pModule_->GetViewport(pModule_->GetCurrentDisplayHandle(), dxres, dyres, vxres, vyres, oxres, oyres);

	// Create DisplayMPI context for OpenGL framebuffer
#ifdef LF1000
	pmem2d = (U8*)pmem2d + 0x20000000;
	tDisplayHandle hogl = CreateHandle(vyres, vxres, kPixelFormatRGB565, (U8*)pmem2d);
	tDisplayContext *dcogl = (tDisplayContext*)hogl;
	dcogl->isOpenGL = true;
	hogls.push_back(hogl);
#else
	//hogl = CreateHandle(vyres, vxres, kPixelFormatRGB565, fbmem[n]);
	//SetPixelFormat(n, vxres, vyres, 32, kPixelFormatRGB565, true); 	// swizzle RGB
	tDisplayHandle hogl = CreateHandle(vyres, vxres, kPixelFormatARGB8888, fbmem[n]);
	tDisplayContext *dcogl = (tDisplayContext*)hogl;
	dcogl->isOpenGL = true;
	hogls.push_back(hogl);
	SetPixelFormat(n, vxres, vyres, 32, kPixelFormatARGB8888, true); 	// swizzle RGB
#endif

	// Pass back essential display context info for OpenGL bindings
	pOglCtx->width 		= vinfo[n].xres;
	pOglCtx->height 	= vinfo[n].yres;
	pOglCtx->owidth		= oxres;
	pOglCtx->oheight	= oyres;
#ifdef LF1000
	pOglCtx->eglDisplay = &finfo[n]; // non-NULL ptr
	pOglCtx->eglWindow 	= &vinfo[n]; // non-NULL ptr
#else
	pOglCtx->eglDisplay = n;				// display index
	//Minh Saelock - eglWindow is already pointing to a valid struct on LF2000
	//pOglCtx->eglWindow  = &vinfo[n].xres;	// ptr to xres, yres
#endif
	pOglCtx->hndlDisplay = hogl;

	// Reset initial HW context state
	RegisterLayer(hogl, 0, 0);
	SetAlpha(hogl, 0, false);
	SetWindowPosition(hogl, 0, 0, pOglCtx->width, pOglCtx->height);
}

//----------------------------------------------------------------------------
void CDisplayFB::DeinitOpenGL(void* pCtx)
{
	tOpenGLContext* 				pOglCtx = (tOpenGLContext*)pCtx;
#ifdef LF1000
	// Release framebuffer allocations used by OpenGL context
	if (dcmem1.pBuffer)
		DeAllocBuffer(&dcmem1);
	DeAllocBuffer(&dcmem2);
#endif
	// Release DisplayMPI context
	if(!pOglCtx)
	{
		DestroyHandle(hogls.back(), false);
		hogls.pop_back();
	} else {
		std::vector<tDisplayHandle>::iterator deleter = find(hogls.begin(), hogls.end(), pOglCtx->hndlDisplay);
		if(deleter != hogls.end())
			hogls.erase(deleter);
		DestroyHandle(pOglCtx->hndlDisplay, false);
	}
	
	
#ifdef LF1000
	// Release framebuffer mappings and drivers used by OpenGL
	munmap(preg3d, REG3D_SIZE);
	
	close(fdreg3d);
#endif
}

//----------------------------------------------------------------------------
void CDisplayFB::EnableOpenGL(void* pCtx)
{
#if defined(LF1000) || defined(LF2000)
	tOpenGLContext* 				pOglCtx = (tOpenGLContext*)pCtx;
	if(!pOglCtx)
	{	
		tDisplayContext *dcogl = (tDisplayContext*)hogls.back();
		RegisterLayer(dcogl, dcogl->x, dcogl->y);
		SetVisible(dcogl, true);
	} else {
		tDisplayContext *dcogl = (tDisplayContext*)pOglCtx->hndlDisplay;
		RegisterLayer(pOglCtx->hndlDisplay, dcogl->x, dcogl->y);
		SetVisible(pOglCtx->hndlDisplay, true);
	}
#endif
}

//----------------------------------------------------------------------------
void CDisplayFB::DisableOpenGL(void* pCtx)
{
#if defined(LF1000) || defined(LF2000)
	tOpenGLContext* 				pOglCtx = (tOpenGLContext*)pCtx;
	if(!pOglCtx)
		SetVisible(hogls.back(), false);
	else
		SetVisible(pOglCtx->hndlDisplay, false);
#endif
}

//----------------------------------------------------------------------------
void CDisplayFB::UpdateOpenGL(void* pCtx)
{
#ifdef LF2000
	tOpenGLContext* 				pOglCtx = (tOpenGLContext*)pCtx;
	// Re-enable OpenGL context which is not registered
	if(!pOglCtx)
	{
		if (!hogls.empty() && !fbviz[OGLFB])
		{
			tDisplayContext *dcogl = (tDisplayContext*)hogls.back();
			if (dcogl->isEnabled) {
				RegisterLayer(dcogl, dcogl->x, dcogl->y);
				SetVisible(dcogl, true);
			}
		}
	} else {
		if (pOglCtx->hndlDisplay != NULL && !fbviz[OGLFB])
		{
			tDisplayContext *dcogl = (tDisplayContext*)pOglCtx->hndlDisplay;
			if (dcogl->isEnabled) {
				RegisterLayer(pOglCtx->hndlDisplay, dcogl->x, dcogl->y);
				SetVisible(pOglCtx->hndlDisplay, true);
			}
		}
	}
#endif
}

//----------------------------------------------------------------------------
void CDisplayFB::WaitForDisplayAddressPatched(void)
{
#ifdef LF1000
	int n = OGLFB;
	int r = 0; 
	do {
		r = ioctl(fbdev[n], FBIO_WAITFORVSYNC, 0);
		if (r == 1)
			kernel_.TaskSleep(1);
	} while (r != 0);
#endif
}

//----------------------------------------------------------------------------
void CDisplayFB::SetOpenGLDisplayAddress(const unsigned int DisplayBufferPhysicalAddress)
{
#ifdef LF1000
	tDisplayContext *dcogl = (tDisplayContext*)hogl;
	// Re-enable OpenGL context which is not registered
	if (hogl != NULL && !fbviz[OGLFB])
	{
		if (dcogl->isEnabled) {
			RegisterLayer(hogl, dcogl->x, dcogl->y);
			SetVisible(hogl, true);
		}
	}
	int n = OGLFB;
	unsigned int offset = DisplayBufferPhysicalAddress - finfo[n].smem_start;
	offset &= ~0x20000000;
	dcogl->offset = offset;
	vinfo[n].yoffset = offset / finfo[n].line_length;
	vinfo[n].xoffset = (offset % finfo[n].line_length)  * 8 / vinfo[n].bits_per_pixel;;
	int r = ioctl(fbdev[n], FBIOPAN_DISPLAY, &vinfo[n]);
#endif
}

//----------------------------------------------------------------------------
U32	CDisplayFB::GetDisplayMem(tDisplayMem memtype)
{
	U32 mem = 0;
	std::list<tBuffer>::iterator it;

	switch (memtype) {
	case kDisplayMemTotal:
		return finfo[RGBFB].smem_len;
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
bool CDisplayFB::AllocBuffer(tDisplayContext* pdc, U32 aligned)
{
	U32 bufsize = (aligned) ? aligned : pdc->pitch * pdc->height;
	tBuffer buf;
	std::list<tBuffer>::iterator it;
	
	kernel_.LockMutex(gListMutex);

#if 1 //def LF1000
	// All display contexts now reference common base address
	pdc->basephys 	= finfo[RGBFB].smem_start;
	pdc->baselinear = finfo[YUVFB].smem_start;
	pdc->pBuffer 	= (pdc->isPlanar) ? fbmem[YUVFB] : fbmem[RGBFB];
#else
	// Framebuffers reference separate base addresses
	pdc->basephys 	= finfo[pdc->layer].smem_start;
	pdc->baselinear = finfo[pdc->layer].smem_start;
	pdc->pBuffer 	= fbmem[pdc->layer];

	pdc->offset		= fboff[pdc->layer];
	pdc->pBuffer	+= pdc->offset;
	if (pdc->layer != RGBFB)
		fboff[pdc->layer] += bufsize;
	fboff[pdc->layer] %= finfo[pdc->layer].smem_len;
#endif

#if 1 //def LF1000
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
		if (gMarkBufEnd - bufsize < gMarkBufStart) {	
			kernel_.UnlockMutex(gListMutex);
			return false;
		}
		pdc->offset = gMarkBufEnd - bufsize - finfo[RGBFB].smem_len - finfo[OGLFB].smem_len;
		pdc->pBuffer += pdc->offset;
//		gMarkBufEnd -= bufsize;
	}
	else {
		// Allocate unaligned buffer at top of heap if there's room
		if (gMarkBufStart + bufsize > gMarkBufEnd) {	
			kernel_.UnlockMutex(gListMutex);
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
#endif

	kernel_.UnlockMutex(gListMutex);
	return true;
}

//----------------------------------------------------------------------------
bool CDisplayFB::DeAllocBuffer(tDisplayContext* pdc)
{
	tBuffer buf;
	std::list<tBuffer>::iterator it;
	
	kernel_.LockMutex(gListMutex);
	U32 markStart = gMarkBufStart;
	U32 markEnd   = gMarkBufEnd;

#if 1 //def LF1000
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
#endif

	kernel_.UnlockMutex(gListMutex);
	return true;
}

//----------------------------------------------------------------------------

LF_END_BRIO_NAMESPACE()
// EOF
