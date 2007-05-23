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
#include <DisplayPriv.h>
#include <DisplayMPI.h>
#include <BrioOpenGLConfig.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "gpio_ioctl.h"
#include "mlc_ioctl.h"
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
	int			gDevGpio = -1;
	int			gDevDpc = -1;
	int			gDevMlc = -1;
	int			gDevLayer = -1;
	int			gDevGa3d = -1;
	int			gDevMem = -1;
	void*		gpMem1 = NULL;
	void*		gpMem2 = NULL;
	void*		gpReg3d = NULL;
	int			gmem1size = MEM1_SIZE;
	int			gmem2size = MEM2_SIZE;
	int			gregsize = PAGE_3D * 0x1000;
	tDisplayScreenStats		screen;
	tDisplayContext			dc;
}

//----------------------------------------------------------------------------
void CDisplayModule::InitOpenGL(void* pCtx)
{
	dbg_.DebugOut(kDbgLvlVerbose, "InitOpenGLHW: enter\n");

	// Open device driver for 3D accelerator registers
	gDevGa3d = open("/dev/ga3d", O_RDWR|O_SYNC);
	dbg_.Assert(gDevGa3d >= 0, "DisplayModule::InitModule: /dev/ga3d driver failed");

	// Open memory driver for mapping register space and framebuffer
    gDevMem = open("/dev/mem", O_RDWR|O_SYNC);
	dbg_.Assert(gDevMem >= 0, "DisplayModule::InitModule: /dev/mem driver failed");

	// Map 3D engine register space
	gregsize = PAGE_3D * getpagesize();  
	gpReg3d = mmap(0, gregsize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED | MAP_POPULATE, gDevGa3d, REG3D_PHYS);
	dbg_.DebugOut(kDbgLvlVerbose, "InitOpenGLHW: %08X mapped to %08X\n", REG3D_PHYS, gpReg3d);

	// Map memory block for framebuffer
    gpMem1 = mmap(MEM1_VIRT, gmem1size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED | MAP_POPULATE, gDevMem, MEM1_PHYS);
	dbg_.DebugOut(kDbgLvlVerbose, "InitOpenGLHW: %08X mapped to %08X\n", MEM1_PHYS, gpMem1);

	// Map memory block for (?)
    gpMem2 = mmap(MEM2_VIRT, gmem2size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED | MAP_POPULATE, gDevMem, MEM2_PHYS);
	dbg_.DebugOut(kDbgLvlVerbose, "InitOpenGLHW: %08X mapped to %08X\n", MEM2_PHYS, gpMem2);

	// Pass back essential display context info for OpenGL bindings
	tOpenGLContext* pOglCtx = (tOpenGLContext*)pCtx;
	pOglCtx->width = 320;
	pOglCtx->height = 240;
	pOglCtx->eglDisplay = &screen;
	pOglCtx->eglWindow = &dc;

	// Copy the required mappings into the MagicEyes callback init struct
	___OAL_MEMORY_INFORMATION__* pMemInfo = (___OAL_MEMORY_INFORMATION__*)pOglCtx->pOEM;
    pMemInfo->VirtualAddressOf3DCore	= (unsigned int)gpReg3d;

    pMemInfo->Memory1D_VirtualAddress	= (unsigned int)gpMem1;
    pMemInfo->Memory1D_PhysicalAddress	= MEM1_PHYS;
    pMemInfo->Memory1D_SizeInMbyte	= (MEM1_SIZE-0x100000) >> 20;

    pMemInfo->Memory2D_VirtualAddress	= (unsigned int)gpMem2;
    pMemInfo->Memory2D_PhysicalAddress	= MEM2_PHYS;
    pMemInfo->Memory2D_SizeInMbyte	= MEM2_SIZE >> 20;
 
 	dbg_.DebugOut(kDbgLvlVerbose, "InitOpenGLHW: exit\n");
}

//----------------------------------------------------------------------------
void CDisplayModule::DeinitOpenGL()
{
	dbg_.DebugOut(kDbgLvlVerbose, "DeInitOpenGLHW: enter\n");

    munmap(gpReg3d, gregsize);
    munmap(gpMem1, gmem1size);
    munmap(gpMem2, gmem2size);
    close(gDevMem);
	close(gDevGa3d);
//	close(gDevLayer);

	dbg_.DebugOut(kDbgLvlVerbose, "DeInitOpenGLHW: exit\n");
}
//----------------------------------------------------------------------------
void CDisplayModule::EnableOpenGL()
{
	// Enable 3D layer as render target after accelerator enabled
	int	layer = open( "/dev/layer0", O_WRONLY);
    
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
	ioctl(layer, MLC_IOCTVSTRIDE, 4096);
	ioctl(layer, MLC_IOCT3DENB, (void *)1);
	ioctl(layer, MLC_IOCTDIRTY, (void *)1);

	close(layer);
}

//----------------------------------------------------------------------------
void CDisplayModule::DisableOpenGL()
{
	// Disable 3D layer render target before accelerator disabled
	int	layer = open( "/dev/layer0", O_WRONLY);
	ioctl(layer, MLC_IOCT3DENB, (void *)0);
	ioctl(layer, MLC_IOCTDIRTY, (void *)1);
    close(layer);
}

LF_END_BRIO_NAMESPACE()
// EOF
