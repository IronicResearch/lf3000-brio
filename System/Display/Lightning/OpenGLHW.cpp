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
#include <errno.h>
#include <linux/lf1000/gpio_ioctl.h>
#include <linux/lf1000/mlc_ioctl.h>
#include "GLES/libogl.h"

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// Lightning hardware-specific defines
//============================================================================
#define PAGE_3D		2
#define	REG3D_PHYS	0xc001a000

#define	MEM1_SIZE	0x00800000	// 8Meg
#define	MEM1_VIRT	0xb1000000

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
	unsigned int gMem1Phys = 0; //MEM1_PHYS;
	unsigned int gMem2Phys = 0; //MEM2_PHYS;
	unsigned int gMem1Size = 0; //MEM1_SIZE;
	unsigned int gMem2Size = 0; //MEM2_SIZE;
	unsigned int gRegSize = PAGE_3D * 0x1000;
	bool					FSAAval = false; // fullscreen anti-aliasing
	tDisplayScreenStats		screen;
	tDisplayContext			dc;
	tDisplayHandle			hdc = &dc;
}

//----------------------------------------------------------------------------
void CDisplayModule::InitOpenGL(void* pCtx)
{
	dbg_.DebugOut(kDbgLvlVerbose, "InitOpenGLHW: enter\n");

	// Dereference OpenGL context for MagicEyes memory size overides
	tOpenGLContext* 				pOglCtx = (tOpenGLContext*)pCtx;
	___OAL_MEMORY_INFORMATION__* 	pMemInfo = (___OAL_MEMORY_INFORMATION__*)pOglCtx->pOEM;
	unsigned int					mem1Virt = MEM1_VIRT;
	unsigned int					mem2Virt;

	// Need 2 layers for LF1000 fullscreen anti-aliasing option
	FSAAval = pOglCtx->bFSAA;

	// open Even layer
	gDevLayerEven = open(OGL_LAYER_EVEN_DEV, O_WRONLY);
	dbg_.Assert(gDevLayerEven >= 0, "DisplayModule::InitOpenGL: " OGL_LAYER_EVEN_DEV " driver failed");
	dbg_.DebugOut(kDbgLvlVerbose, "DisplayModule::InitOpenGL: " OGL_LAYER_EVEN_DEV " driver opened\n");
	// open odd layer
	gDevLayerOdd = open(OGL_LAYER_ODD_DEV, O_WRONLY);
	dbg_.Assert(gDevLayerEven >= 0, "DisplayModule::InitOpenGL: " OGL_LAYER_ODD_DEV " driver failed");
	dbg_.DebugOut(kDbgLvlVerbose, "DisplayModule::InitOpenGL: " OGL_LAYER_ODD_DEV " driver opened\n");

	// Select 3D layer based on existing 2D layer usage
	gDevLayer = (isLayerSwapped_) ? gDevLayerEven : gDevLayerOdd;
	
	// Get framebuffer address from driver
	errno=0;
	gMem1Phys = ioctl(gDevLayer, MLC_IOCQADDRESS, 0);
// FIXED by BK
	dbg_.Assert(errno == 0, "DisplayModule::InitOpenGL: " OGL_LAYER_DEV " ioctl failed");
//	dbg_.Assert(gMem1Phys >= 0, "DisplayModule::InitOpenGL: " OGL_LAYER_DEV " ioctl failed");
	dbg_.DebugOut(kDbgLvlVerbose, "DisplayModule::InitOpenGL: Mem1Phys = %08X\n", gMem1Phys);

	// 1st heap size and addresses must be 1 Meg aligned
	// 2nd heap size and addresses must be 4 Meg aligned
	gMem1Size = ((pMemInfo->Memory1D_SizeInMbyte+1) & ~1) << 20;
	gMem2Size = ((pMemInfo->Memory2D_SizeInMbyte+3) & ~3) << 20;
	mem2Virt = mem1Virt + gMem1Size;

	// Now round down size to accomodate reserved 1Meg for 2D and Video
	gMem1Size -= k1Meg;
	gMem1Phys += (k1Meg - 1);
	gMem1Phys &= ~(k1Meg - 1);
	gMem2Phys = gMem1Phys + gMem1Size;
	gMem2Phys += (4 * k1Meg - 1);
	gMem2Phys &= ~(4 * k1Meg - 1);
	dbg_.DebugOut(kDbgLvlVerbose, "DisplayModule::InitOpenGL: Mem1Phys = %08X, Mem1Size = %08X\n", gMem1Phys, gMem1Size);
	dbg_.DebugOut(kDbgLvlVerbose, "DisplayModule::InitOpenGL: Mem2Phys = %08X, Mem2Size = %08X\n", gMem2Phys, gMem2Size);
	
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
	gpMem1 = mmap((void*)mem1Virt, gMem1Size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED | MAP_POPULATE, gDevMem, gMem1Phys);
	dbg_.DebugOut(kDbgLvlImportant, "InitOpenGLHW: %08X mapped to %p, size = %08X\n", gMem1Phys, gpMem1, gMem1Size);

	// Map memory block for 2D heap = framebuffer, Zbuffer, textures
	gpMem2 = mmap((void*)mem2Virt, gMem2Size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED | MAP_POPULATE, gDevMem, gMem2Phys);
	dbg_.DebugOut(kDbgLvlImportant, "InitOpenGLHW: %08X mapped to %p, size = %08X\n", gMem2Phys, gpMem2, gMem2Size);

	// Query HW to setup display context descriptors
	const tDisplayScreenStats* pScreen = GetScreenStats(0);
	screen = *pScreen;

	tDisplayHandle hndl = CreateHandle(screen.height, screen.width, kPixelFormatRGB565, reinterpret_cast<U8*>(gpMem2));
	tDisplayContext* pdc = reinterpret_cast<tDisplayContext*>(hndl);
	dc = *pdc;
	dc.pitch = 4096;
	dc.layer = gDevLayer;
	
	// Pass back essential display context info for OpenGL bindings
	pOglCtx->width = dc.width; //320;
	pOglCtx->height = dc.height; //240;
	pOglCtx->eglDisplay = &screen;
	pOglCtx->eglWindow = &dc;
	pOglCtx->hndlDisplay = hdc = &dc; //hndl;

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
	close(gDevLayerEven);
	close(gDevLayerOdd);
	
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
	int	layer = dc.layer = gDevLayer = (isLayerSwapped_) ? gDevLayerEven : gDevLayerOdd;
    
	// Position 3D layer
	union mlc_cmd c;
	c.position.left = c.position.top = 0;
	c.position.right = dc.width; //320;
	c.position.bottom = dc.height; //240;

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

	isOpenGLEnabled_ = true;
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

	isOpenGLEnabled_ = false;
}

#ifdef LF1000
//----------------------------------------------------------------------------
// (added for LF1000)
void CDisplayModule::WaitForDisplayAddressPatched(void)
{
	if(FSAAval) {
		while(ioctl(gDevLayerEven, MLC_IOCQDIRTY, (void *)0)) 
			usleep(100);
		while(ioctl(gDevLayerOdd , MLC_IOCQDIRTY, (void *)0)) 
			usleep(100);
	}
	else {
		while(ioctl(gDevLayer , MLC_IOCQDIRTY, (void *)0)) 
			usleep(100);
	}
}

//----------------------------------------------------------------------------
// (added for LF1000)
void CDisplayModule::SetOpenGLDisplayAddress(
		const unsigned int DisplayBufferPhysicalAddress)
{
	(void )DisplayBufferPhysicalAddress; /* Prevent unused variable warnings. */ 
	if(FSAAval) {
		ioctl(gDevLayerEven, MLC_IOCTFORMAT, 0x4432); /*R5G6B5*/
		ioctl(gDevLayerOdd , MLC_IOCTFORMAT, 0x4432);

		ioctl(gDevLayerEven, MLC_IOCTHSTRIDE, 2);
		ioctl(gDevLayerEven, MLC_IOCTVSTRIDE, 8192);
		ioctl(gDevLayerOdd , MLC_IOCTHSTRIDE, 2);
		ioctl(gDevLayerOdd , MLC_IOCTVSTRIDE, 8192);

		ioctl(gDevLayerEven, MLC_IOCTADDRESS, gMem1Phys);
		ioctl(gDevLayerOdd , MLC_IOCTADDRESS, gMem1Phys+4096);

		ioctl(gDevLayerOdd, MLC_IOCTBLEND, (void *)1); //enable Alpha
		ioctl(gDevLayerOdd, MLC_IOCTALPHA, 8); //set to 50%

		ioctl(gDevLayerEven, MLC_IOCTDIRTY, (void *)1);
		ioctl(gDevLayerOdd , MLC_IOCTDIRTY, (void *)1);
	}
	else {
		ioctl(gDevLayer, MLC_IOCTFORMAT, 0x4432); /*R5G6B5*/

		ioctl(gDevLayer, MLC_IOCTHSTRIDE, 2);
		ioctl(gDevLayer, MLC_IOCTVSTRIDE, 4096);

		ioctl(gDevLayer, MLC_IOCTADDRESS, gMem1Phys);
		ioctl(gDevLayer, MLC_IOCTDIRTY, (void *)1);
	}
}
#endif

LF_END_BRIO_NAMESPACE()
// EOF
