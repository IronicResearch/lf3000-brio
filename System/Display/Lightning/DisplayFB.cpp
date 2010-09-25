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

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

LF_BEGIN_BRIO_NAMESPACE()

namespace 
{
	const char*					FBDEV = "/dev/fb0";
	int 						fbdev = -1;
	struct fb_fix_screeninfo 	finfo;
	struct fb_var_screeninfo 	vinfo;
	U8*							fbmem = 0;
}

//============================================================================
// CDisplayFB: Implementation of hardware-specific functions
//============================================================================
//----------------------------------------------------------------------------
void CDisplayFB::InitModule()
{
	int r;
	
	// Open framebuffer device
	fbdev = open(FBDEV, O_RDWR | O_SYNC);
	dbg_.Assert(fbdev >= 0, "%s: Error opening %s\n", __FUNCTION__, FBDEV);
	
	// Query framebuffer info
	r = ioctl(fbdev, FBIOGET_FSCREENINFO, &finfo);
	dbg_.Assert(r == 0, "%s: Error querying %s\n", __FUNCTION__, FBDEV);
	
	r = ioctl(fbdev, FBIOGET_VSCREENINFO, &vinfo);
	dbg_.Assert(r == 0, "%s: Error querying %s\n", __FUNCTION__, FBDEV);
	dbg_.DebugOut(kDbgLvlImportant, "%s: Screen = %d x %d\n", __FUNCTION__, vinfo.xres, vinfo.yres);

	// Map framebuffer into userspace
	fbmem = (U8*)mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbdev, 0);
	dbg_.Assert(fbmem != MAP_FAILED, "%s: Error mapping %s\n", __FUNCTION__, FBDEV);
	dbg_.DebugOut(kDbgLvlImportant, "%s: Mapped %08lx to %p, size %u\n", __FUNCTION__, finfo.smem_start, fbmem, finfo.smem_len);
}

//----------------------------------------------------------------------------
void CDisplayFB::DeInitModule()
{
	// Release framebuffer mapping and device
	munmap(fbmem, finfo.smem_len);
	
	close(fbdev);
}

//----------------------------------------------------------------------------
U32	CDisplayFB::GetScreenSize()
{
	return (vinfo.yres << 16) | (vinfo.xres);
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
	switch (colorDepth) 
	{
		case kPixelFormatRGB4444:	depth = 16; break;
		case kPixelFormatRGB565:	depth = 16; break;
		case kPixelFormatRGB888:	depth = 24; break; 	
		default:
		case kPixelFormatARGB8888: 	depth = 32; break;
		case kPixelFormatYUV420:	depth = 8 ; break;
		case kPixelFormatYUYV422:	depth = 16; break;
	}		
	
	memset(ctx, 0, sizeof(tDisplayContext));
	ctx->width				= width;
	ctx->height				= height;
	ctx->colorDepthFormat 	= colorDepth;
	ctx->depth				= depth;
	ctx->bpp				= depth/8;
	ctx->pitch				= width * ctx->bpp; // FIXME
	ctx->isAllocated		= (pBuffer != NULL);
	ctx->pBuffer			= (pBuffer != NULL) ? pBuffer : fbmem;
	
	return (tDisplayHandle)ctx;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::DestroyHandle(tDisplayHandle hndl, Boolean destroyBuffer)
{
	tDisplayContext* ctx = (tDisplayContext*)hndl;
	
	delete ctx;
	
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::RegisterLayer(tDisplayHandle hndl, S16 xPos, S16 yPos)
{
	return kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::UnRegisterLayer(tDisplayHandle hndl)
{
	return kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::Update(tDisplayContext* dc, int sx, int sy, int dx, int dy, int width, int height)
{
	return kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CDisplayFB::SwapBuffers(tDisplayHandle hndl, Boolean waitVSync)
{
	return kNoImplErr;
}

//----------------------------------------------------------------------------
Boolean	CDisplayFB::IsBufferSwapped(tDisplayHandle hndl)
{
	return false;
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
}

//----------------------------------------------------------------------------
void CDisplayFB::DeinitOpenGL()
{
}

//----------------------------------------------------------------------------
void CDisplayFB::EnableOpenGL(void* pCtx)
{
}

//----------------------------------------------------------------------------
void CDisplayFB::DisableOpenGL()
{
}

//----------------------------------------------------------------------------
void CDisplayFB::UpdateOpenGL()
{
}

//----------------------------------------------------------------------------
void CDisplayFB::WaitForDisplayAddressPatched(void)
{
}

//----------------------------------------------------------------------------
void CDisplayFB::SetOpenGLDisplayAddress(const unsigned int DisplayBufferPhysicalAddress)
{
}

//----------------------------------------------------------------------------
U32	CDisplayFB::GetDisplayMem(tDisplayMem memtype)
{
}

//----------------------------------------------------------------------------

LF_END_BRIO_NAMESPACE()
// EOF
