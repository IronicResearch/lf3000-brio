//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		OpenGLHW.cpp
//
// Description:
//		Configure OpenGL hardware target
//
//==============================================================================

#include <SystemTypes.h>
#include <SystemErrors.h>
#include <DisplayHW.h>
#include <DisplayPriv.h>
#include <DisplayMPI.h>
#include <BrioOpenGLConfig.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/lf1000/gpio_ioctl.h>
#include <linux/lf1000/mlc_ioctl.h>
#include "GLES/libogl.h"

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// Lightning hardware-specific defines
//============================================================================
#define PAGE_3D		2
#define	REG3D_PHYS	0xc001a000

#define	MEM1_PHYS	0x03000000
#define	MEM1_SIZE	0x00800000
#define	MEM1_VIRT	(void*)0xb1000000

#define	MEM2_PHYS	(MEM1_PHYS + MEM1_SIZE)
#define	MEM2_SIZE	MEM1_SIZE
#define	MEM2_VIRT	(void*)((unsigned int)MEM1_VIRT + MEM1_SIZE)

//============================================================================
// Local device driver handles
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	int			gDevLayer = -1;
	int			gDevLayerEven = -1;
	int			gDevLayerOdd = -1;
	int			gDevLayerVideo = -1;
	int			gDevGa3d = -1;
	int			gDevMem = -1;
	void*		gpMem1 = NULL;
	void*		gpMem2 = NULL;
	void*		gpReg3d = NULL;
	unsigned int gMem1Phys = MEM1_PHYS;
	unsigned int gMem2Phys = MEM2_PHYS;
	unsigned int gMem1Size = MEM1_SIZE;
	unsigned int gMem2Size = MEM2_SIZE;
	unsigned int gRegSize = PAGE_3D * 0x1000;
	bool					FSAAval = false; // fullscreen anti-aliasing
	tDisplayScreenStats		screen;
	tDisplayContext			dc;
}

//----------------------------------------------------------------------------
void CDisplayModule::InitOpenGL(void* pCtx)
{
	dbg_.DebugOut(kDbgLvlVerbose, "InitOpenGLHW: enter\n");

	// Dereference OpenGL context for MagicEyes memory size overides
	tOpenGLContext* 				pOglCtx = (tOpenGLContext*)pCtx;
	___OAL_MEMORY_INFORMATION__* 	pMemInfo = (___OAL_MEMORY_INFORMATION__*)pOglCtx->pOEM;
	unsigned int					mem2Virt;
#ifdef LF1000
	unsigned int					baseAddr;
#endif

	// 2nd heap size and addresses must be 4 Meg aligned
	gMem1Size = ((pMemInfo->Memory1D_SizeInMbyte+3) & ~3) << 20;
	gMem2Size = ((pMemInfo->Memory2D_SizeInMbyte+3) & ~3) << 20;
	gMem2Phys = gMem1Phys + gMem1Size;
	mem2Virt = (unsigned int)MEM1_VIRT + gMem1Size;

	// Need 2 layers for LF1000 fullscreen anti-aliasing option
	FSAAval = pOglCtx->bFSAA;

	// Open device driver for 3D layer
	if (FSAAval) {
		// open Even layer
		gDevLayerEven = open(OGL_LAYER_EVEN_DEV, O_WRONLY);
		dbg_.Assert(gDevLayerEven >= 0, "DisplayModule::InitOpenGL: " OGL_LAYER_EVEN_DEV " driver failed");
		dbg_.DebugOut(kDbgLvlVerbose, "DisplayModule::InitOpenGL: " OGL_LAYER_EVEN_DEV " driver opened\n");
		// open odd layer
		gDevLayerOdd = open(OGL_LAYER_ODD_DEV, O_WRONLY);
		dbg_.Assert(gDevLayerEven >= 0, "DisplayModule::InitOpenGL: " OGL_LAYER_ODD_DEV " driver failed");
		dbg_.DebugOut(kDbgLvlVerbose, "DisplayModule::InitOpenGL: " OGL_LAYER_ODD_DEV " driver opened\n");
		gDevLayer = gDevLayerOdd;
	}
	else {
		gDevLayer = open(OGL_LAYER_DEV, O_WRONLY);
		dbg_.Assert(gDevLayer >= 0, "DisplayModule::InitOpenGL: " OGL_LAYER_DEV " driver failed");
		dbg_.DebugOut(kDbgLvlVerbose, "DisplayModule::InitOpenGL: " OGL_LAYER_DEV " driver opened\n");
	}

#ifdef LF1000
	baseAddr = ioctl(gDevLayer, MLC_IOCQADDRESS, 0);
//	ioctl(gDevLayer, MLC_IOCTADDRESS, LIN2XY(baseAddr));

#if 0	// Horrible hack to get 3D layer mappings to work	
	tDisplayHandle hndl = CreateHandle(240, 320, kPixelFormatYUV420, NULL);
	RegisterLayer(hndl, 0, 0);
//	UnRegister(hndl, 0);
#else	// Work-around for Video layer enable bug (in MLC driver???)	
	gDevLayerVideo = open(YUV_LAYER_DEV, O_RDWR|O_SYNC);
	union mlc_cmd c;
	c.overlaysize.srcwidth = 320;
	c.overlaysize.srcheight = 240;
	c.overlaysize.dstwidth = 320;
	c.overlaysize.dstheight = 240;
	ioctl(gDevLayerVideo, MLC_IOCSOVERLAYSIZE, &c);
	c.position.top = 0;
	c.position.left = 0;
	c.position.right = 320;
	c.position.bottom = 240;
	ioctl(gDevLayerVideo, MLC_IOCSPOSITION, &c);
	ioctl(gDevLayerVideo, MLC_IOCTLAYEREN, 1);
	ioctl(gDevLayerVideo, MLC_IOCTDIRTY, 1);
#endif	
#endif	// LF1000
	
	// Open device driver for 3D accelerator registers
	gDevGa3d = open("/dev/ga3d", O_RDWR|O_SYNC);
	dbg_.Assert(gDevGa3d >= 0, "DisplayModule::InitModule: /dev/ga3d driver failed");

	// Open memory driver for mapping register space and framebuffer
	gDevMem = open("/dev/mem", O_RDWR|O_SYNC);
	dbg_.Assert(gDevMem >= 0, "DisplayModule::InitModule: /dev/mem driver failed");

	// Map 3D engine register space
	gRegSize = PAGE_3D * getpagesize();  
	gpReg3d = mmap(0, gRegSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED | MAP_POPULATE, gDevGa3d, REG3D_PHYS);
	dbg_.DebugOut(kDbgLvlVerbose, "InitOpenGLHW: %08X mapped to %p\n", REG3D_PHYS, gpReg3d);

	// Map memory block for 1D heap = command buffer, vertex buffers (not framebuffer)
//	gMem1Phys |= 0x20000000;
	gpMem1 = mmap(MEM1_VIRT, gMem1Size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED | MAP_POPULATE, gDevMem, gMem1Phys);
	dbg_.DebugOut(kDbgLvlImportant, "InitOpenGLHW: %08X mapped to %p, size = %08X\n", gMem1Phys, gpMem1, gMem1Size);

	// Map memory block for 2D heap = framebuffer, Zbuffer, textures
//	gMem2Phys |= 0x20000000;
	gpMem2 = mmap((void*)mem2Virt, gMem2Size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED | MAP_POPULATE, gDevMem, gMem2Phys);
	dbg_.DebugOut(kDbgLvlImportant, "InitOpenGLHW: %08X mapped to %p, size = %08X\n", gMem2Phys, gpMem2, gMem2Size);

	// Pass back essential display context info for OpenGL bindings
	pOglCtx->width = 320;
	pOglCtx->height = 240;
	pOglCtx->eglDisplay = &screen;
	pOglCtx->eglWindow = &dc;

	// Copy the required mappings into the MagicEyes callback init struct
	pMemInfo->VirtualAddressOf3DCore	= (unsigned int)gpReg3d;
	pMemInfo->Memory1D_VirtualAddress	= (unsigned int)gpMem1;
	pMemInfo->Memory1D_PhysicalAddress	= gMem1Phys;
	pMemInfo->Memory1D_SizeInMbyte		= gMem1Size >> 20;
	pMemInfo->Memory2D_VirtualAddress	= (unsigned int)gpMem2;
	pMemInfo->Memory2D_PhysicalAddress	= gMem2Phys;
	pMemInfo->Memory2D_SizeInMbyte		= gMem2Size >> 20;
 
	dbg_.DebugOut(kDbgLvlVerbose, "InitOpenGLHW: exit\n");
}

//----------------------------------------------------------------------------
void CDisplayModule::DeinitOpenGL()
{
	dbg_.DebugOut(kDbgLvlVerbose, "DeInitOpenGLHW: enter\n");

	munmap(gpReg3d, gRegSize);
	munmap(gpMem1, gMem1Size);
	munmap(gpMem2, gMem2Size);
	close(gDevMem);
	close(gDevGa3d);
	if(FSAAval) {
		close(gDevLayerEven);
		close(gDevLayerOdd);
	}
	else {
		close(gDevLayer);
	}
	
	dbg_.DebugOut(kDbgLvlVerbose, "DeInitOpenGLHW: exit\n");
}
//----------------------------------------------------------------------------
void CDisplayModule::EnableOpenGL(void* pCtx)
{
	// Dereference OpenGL context for MagicEyes FSAA enable
	tOpenGLContext* pOglCtx = (tOpenGLContext*)pCtx;
	FSAAval = pOglCtx->bFSAA;
	dbg_.DebugOut(kDbgLvlVerbose, "EnableOpenGLHW: FSAA = %s\n", FSAAval ? "enabled" : "disabled");

	// Enable 3D layer as render target after accelerator enabled
	int	layer = gDevLayer;
    
	// Position 3D layer
	union mlc_cmd c;
	c.position.left = c.position.top = 0;
	c.position.right = 320;
	c.position.bottom = 240;

	// Enable 3D layer
	ioctl(layer, MLC_IOCTLAYEREN, (void *)1);
	ioctl(layer, MLC_IOCSPOSITION, (void *)&c);
	ioctl(layer, MLC_IOCTFORMAT, 0x4432);
	ioctl(layer, MLC_IOCTHSTRIDE, 2);
	if(FSAAval) {
		ioctl(layer, MLC_IOCTVSTRIDE, 8192);
		ioctl(gDevLayerEven, MLC_IOCT3DENB, (void *)1);
		ioctl(gDevLayerOdd, MLC_IOCT3DENB, (void *)1);
	}
	else {
		ioctl(layer, MLC_IOCTVSTRIDE, 4096);
		ioctl(layer, MLC_IOCT3DENB, (void *)1);
	}
	ioctl(layer, MLC_IOCTDIRTY, (void *)1);

//	ioctl(gDevLayerVideo, MLC_IOCTLAYEREN, 0);
//	ioctl(gDevLayerVideo, MLC_IOCTDIRTY, 1);
}

//----------------------------------------------------------------------------
void CDisplayModule::UpdateOpenGL()
{
	// 3D layer needs to sync to OGL calls
	ioctl(gDevLayer, MLC_IOCTDIRTY, (void *)1);
}

//----------------------------------------------------------------------------
void CDisplayModule::DisableOpenGL()
{
	// Disable 3D layer render target before accelerator disabled
	int layer = gDevLayer;
	if(FSAAval) {
		ioctl(gDevLayerEven, MLC_IOCT3DENB, (void *)0);
		ioctl(gDevLayerOdd, MLC_IOCT3DENB, (void *)0);
		ioctl(gDevLayerEven, MLC_IOCTLAYEREN, (void *)0);
		ioctl(gDevLayerEven, MLC_IOCTDIRTY, (void *)1);
		ioctl(gDevLayerOdd, MLC_IOCTLAYEREN, (void *)0);
		ioctl(gDevLayerOdd, MLC_IOCTDIRTY, (void *)1);
	}
	else {
		ioctl(layer, MLC_IOCT3DENB, (void *)0);
		ioctl(layer, MLC_IOCTLAYEREN, (void *)0);
		ioctl(layer, MLC_IOCTDIRTY, (void *)1);
	}
}

#ifdef LF1000
//----------------------------------------------------------------------------
// (added for LF1000)
void CDisplayModule::WaitForDisplayAddressPatched(void)
{
	if(FSAAval) {
		while(ioctl(gDevLayerEven, MLC_IOCQDIRTY, (void *)0));
		while(ioctl(gDevLayerOdd , MLC_IOCQDIRTY, (void *)0));
	}
	else {
		while(ioctl(gDevLayer , MLC_IOCQDIRTY, (void *)0));
	}
	// Now safe to disable video layer
	ioctl(gDevLayerVideo, MLC_IOCTLAYEREN, 0);
	ioctl(gDevLayerVideo, MLC_IOCTDIRTY, 1);
}

//----------------------------------------------------------------------------
// (added for LF1000)
void CDisplayModule::SetOpenGLDisplayAddress(
		const unsigned int DisplayBufferPhysicalAddress)
{
	if(FSAAval) {
		ioctl(gDevLayerEven, MLC_IOCTFORMAT, 0x4432); /*R5G6B5*/
		ioctl(gDevLayerOdd , MLC_IOCTFORMAT, 0x4432);

		ioctl(gDevLayerEven, MLC_IOCTHSTRIDE, 2);
		ioctl(gDevLayerEven, MLC_IOCTVSTRIDE, 8192);
		ioctl(gDevLayerOdd , MLC_IOCTHSTRIDE, 2);
		ioctl(gDevLayerOdd , MLC_IOCTVSTRIDE, 8192);

		ioctl(gDevLayerEven, MLC_IOCTADDRESS, MEM1_PHYS);
		ioctl(gDevLayerOdd , MLC_IOCTADDRESS, MEM1_PHYS+4096);

		ioctl(gDevLayerOdd, MLC_IOCTBLEND, (void *)1); //enable Alpha
		ioctl(gDevLayerOdd, MLC_IOCTALPHA, 8); //set to 50%

		ioctl(gDevLayerEven, MLC_IOCTDIRTY, (void *)1);
		ioctl(gDevLayerOdd , MLC_IOCTDIRTY, (void *)1);
	}
	else {
		ioctl(gDevLayer, MLC_IOCTFORMAT, 0x4432); /*R5G6B5*/

		ioctl(gDevLayer, MLC_IOCTHSTRIDE, 2);
		ioctl(gDevLayer, MLC_IOCTVSTRIDE, 4096);

		ioctl(gDevLayer, MLC_IOCTADDRESS, MEM1_PHYS);
		ioctl(gDevLayer, MLC_IOCTDIRTY, (void *)0);
	}
}
#endif

LF_END_BRIO_NAMESPACE()
// EOF
