#define _FILE_OFFSET_BITS	64	// for correct off_t type

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

typedef			unsigned char		U8;
typedef			signed short		S16;
typedef			unsigned short		U16;
typedef			unsigned int		U32;

//----------------------------------------------------------------------------
int				gDevIdct = -1;
void*			pRegIdct = NULL;
volatile U8*	pRegIdct8 = NULL;
volatile U32*	pRegIdct32 = NULL;

//----------------------------------------------------------------------------
void InitIDCT(void)
{
	// Open device driver for IDCT register block
	gDevIdct = open("/dev/idct", O_RDWR|O_SYNC);

	// Map IDCT register block at page-aligned physical address
	pRegIdct = mmap(0, REG_IDCT_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, gDevIdct, REG_IDCT_ALIGN);

	// Compensate register mapping for non-page-aligned physical address
	pRegIdct8 = (U8*)pRegIdct;
	pRegIdct8 += REG_IDCT_OFFSET;
	pRegIdct32 = (U32*)pRegIdct8;
	
	// Enable clock
	pRegIdct32[0x7C0>>2] = 0x03;
	
	// Reset registers
	pRegIdct32[0x080>>2] = 0x02;
	pRegIdct32[0x080>>2] = 0x00;
}

//----------------------------------------------------------------------------
void DecodeIDCT(S16* pDataIn, S16* pDataOut)
{
	// Load IDCT data registers (64 x 16-bit)
	memcpy((void*)pRegIdct32, pDataIn, 64 * sizeof(S16));
	
	// Run IDCT transform
	pRegIdct32[0x080>>2] &= ~0x04;	// 11-bit range
	pRegIdct32[0x080>>2] |= 0x01;
	while ((pRegIdct32[0x088>>2] & 0x01) != 0x01)
		;
	
	// Return transformed data
	memcpy(pDataOut, (const void*)pRegIdct32, 64 * sizeof(S16));
}

//----------------------------------------------------------------------------
void DeInitIDCT(void)
{
	// Release IDCT mapping
	munmap(pRegIdct, REG_IDCT_SIZE);
	pRegIdct = NULL;
	pRegIdct8 = NULL;
	pRegIdct32 = NULL;
	
	// Close IDCT driver
	close(gDevIdct);
}

