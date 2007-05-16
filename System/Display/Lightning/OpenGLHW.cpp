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

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "gpio_ioctl.h"
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
	int			gDevGa3d = -1;
	int			gDevMem = -1;
	void*		gpMem1 = NULL;
	void*		gpMem2 = NULL;
	void*		gpReg3d = NULL;
	int			gmem1size = MEM1_SIZE;
	int			gmem2size = MEM2_SIZE;
	int			gregsize = PAGE_3D * 0x1000;
}

//----------------------------------------------------------------------------
void CDisplayModule::InitOpenGL(void* pCtx)
{
	// Open device driver for 3D accelerator registers
	gDevGa3d = open("/dev/ga3d", O_RDWR|O_SYNC);
	dbg_.Assert(gDevGa3d >= 0, "DisplayModule::InitModule: /dev/ga3d driver failed");

	// Open memory driver for mapping register space and framebuffer
    gDevMem = open("/dev/mem", O_RDWR|O_SYNC);
	dbg_.Assert(gDevMem >= 0, "DisplayModule::InitModule: /dev/mem driver failed");

	// Map 3D engine register space
	gregsize = PAGE_3D * getpagesize();  
	gpReg3d = mmap(0, gregsize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED | MAP_POPULATE, gDevGa3d, REG3D_PHYS);

	// Map memory block for framebuffer
    gpMem1 = mmap(MEM1_VIRT, gmem1size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED | MAP_POPULATE, gDevMem, MEM1_PHYS);

	// Map memory block for (?)
    gpMem2 = mmap(MEM2_VIRT, gmem2size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED | MAP_POPULATE, gDevMem, MEM2_PHYS);

	// FIXME/dm:
	// Copy the required mappings into the MagicEyes callback init struct
	___OAL_MEMORY_INFORMATION__* pMemInfo = (___OAL_MEMORY_INFORMATION__*)pCtx;
    pMemInfo->VirtualAddressOf3DCore	= (unsigned int)gpReg3d;

    pMemInfo->Memory1D_VirtualAddress	= (unsigned int)gpMem1;
    pMemInfo->Memory1D_PhysicalAddress	= MEM1_PHYS;
    pMemInfo->Memory1D_SizeInMbyte	= (MEM1_SIZE-0x100000) >> 20;

    pMemInfo->Memory2D_VirtualAddress	= (unsigned int)gpMem2;
    pMemInfo->Memory2D_PhysicalAddress	= MEM2_PHYS;
    pMemInfo->Memory2D_SizeInMbyte	= MEM2_SIZE >> 20;
}

//----------------------------------------------------------------------------
void CDisplayModule::DeinitOpenGL()
{
    munmap(gpReg3d, gregsize);
    munmap(gpMem1, gmem1size);
    munmap(gpMem2, gmem2size);
    close(gDevMem);
	close(gDevGa3d);
}

LF_END_BRIO_NAMESPACE()
// EOF
