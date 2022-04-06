#include <SystemTypes.h>
#include <SystemErrors.h>
#include <CameraPriv.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

//----------------------------------------------------------------------------
#define			PAGE_SIZE			4096
#define			REG_IDCT_PHYS		0xC000F800UL
#define			REG_IDCT_SIZE		PAGE_SIZE
#define			REG_IDCT_ALIGN		(REG_IDCT_PHYS & ~(PAGE_SIZE-1))
#define			REG_IDCT_OFFSET		(REG_IDCT_PHYS &  (PAGE_SIZE-1))

LF_BEGIN_BRIO_NAMESPACE()

//----------------------------------------------------------------------------
tErrType CCameraModule::InitIDCTInt()
{
	tErrType	kErr = kUnspecifiedErr;
#ifndef EMULATION
	U8 			*reg8;

	do
	{
		idctCtx_.fd = open(idctCtx_.file ,O_RDWR | O_SYNC);
		if(idctCtx_.fd < 0)
			continue;

		idctCtx_.reg = mmap(0, REG_IDCT_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, idctCtx_.fd, REG_IDCT_ALIGN);
		if(idctCtx_.reg == MAP_FAILED)
		{
			close(idctCtx_.fd);
			continue;
		}

		reg8 = reinterpret_cast<U8*>(idctCtx_.reg);
		reg8 += REG_IDCT_OFFSET;
		idctCtx_.reg32 = reinterpret_cast<U32*>(reg8);

		// Enable clock
		idctCtx_.reg32[0x7C0>>2] = 0x03;

		// Reset registers
		idctCtx_.reg32[0x080>>2] = 0x02;
		idctCtx_.reg32[0x080>>2] = 0x00;

		idctCtx_.reg32[0x080>>2] |= 0x04;	// 9-bit range
		//idctCtx_.reg32[0x080>>2] &= ~0x04;	// 11-bit range

		kErr = kNoErr;
	} while(0);
#else
	kErr = kNoErr;
#endif
	return kErr;
}

//----------------------------------------------------------------------------
tErrType CCameraModule::DeinitIDCTInt()
{
#ifndef EMULATION
	if(idctCtx_.reg != MAP_FAILED)
	{
		munmap(idctCtx_.reg, REG_IDCT_SIZE);
		idctCtx_.reg = MAP_FAILED;
	}

	if(idctCtx_.fd != -1)
	{
		close(idctCtx_.fd);
		idctCtx_.fd = -1;
	}
#endif
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CCameraModule::StartIDCT(S16 *ptr)
{
	// Load IDCT data registers (64 x 16-bit)
	memcpy((void*)idctCtx_.reg32, ptr, 64 * sizeof(S16));

	// Run IDCT transform
	idctCtx_.reg32[0x080>>2] |= 0x01;
}

//----------------------------------------------------------------------------
tErrType CCameraModule::RetrieveIDCT(S16 *ptr)
{
	while ((idctCtx_.reg32[0x88>>2] & 0x01) != 0x01)
		;	/* putting a sleep() here is bad for frame rate */

	memcpy(ptr, (const void*)idctCtx_.reg32, 64 * sizeof(S16));
}

LF_END_BRIO_NAMESPACE()
