/*
 * jidcthw.c
 *
 * libjpeg IDCT performed by LF1000's decoder block.
 *
 * Copyright (C) 2010, LeapFrog Enterprises, Inc.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"		/* Private declarations for DCT subsystem */

#ifdef DCT_HW_SUPPORTED


/*
 * This module is specialized to the case DCTSIZE = 8.
 */

#if DCTSIZE != 8
  Sorry, this code only copes with 8x8 DCTs. /* deliberate syntax err */
#endif

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
typedef			unsigned short		U16;
typedef			unsigned int		U32;
typedef			signed short		S16;
typedef			signed int			S32;

//----------------------------------------------------------------------------
void init_idct_hw(j_decompress_ptr cinfo)
{
	// Open device driver for IDCT register block
	int dev = cinfo->idct->dev_idct = open("/dev/idct", O_RDWR|O_SYNC);

	// Map IDCT register block at page-aligned physical address
	cinfo->idct->reg_idct = mmap(0, REG_IDCT_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev, REG_IDCT_ALIGN);

	// Compensate register mapping for non-page-aligned physical address
	cinfo->idct->reg_idct_8 = (U8*)cinfo->idct->reg_idct;
	cinfo->idct->reg_idct_8 += REG_IDCT_OFFSET;
	cinfo->idct->reg_idct_32 = (U32*)cinfo->idct->reg_idct_8;

	// Enable clock
	cinfo->idct->reg_idct_32[0x7C0>>2] = 0x03;

	// Reset registers
	cinfo->idct->reg_idct_32[0x080>>2] = 0x02;
	cinfo->idct->reg_idct_32[0x080>>2] = 0x00;
}

//----------------------------------------------------------------------------
void deinit_idct_hw(j_decompress_ptr cinfo)
{
	// Release IDCT mapping
	munmap(cinfo->idct->reg_idct, REG_IDCT_SIZE);
	cinfo->idct->reg_idct = NULL;
	cinfo->idct->reg_idct_8 = NULL;
	cinfo->idct->reg_idct_32 = NULL;

	// Close IDCT driver
	close(cinfo->idct->dev_idct);
	cinfo->idct->dev_idct = -1;
}

//----------------------------------------------------------------------------
static void decode_idct(j_decompress_ptr cinfo, U32* pDataIn, U32* pDataOut)
{
	void*			pRegIdct	= cinfo->idct->reg_idct;
	volatile U8*	pRegIdct8	= cinfo->idct->reg_idct_8;
	volatile U32*	pRegIdct32	= cinfo->idct->reg_idct_32;

	// Load IDCT data registers (64 x 16-bit)
	memcpy((void*)pRegIdct32, pDataIn, 32 * sizeof(U32));

	// Run IDCT transform
//	pRegIdct32[0x080>>2] &= ~0x04;	// 12-bit range
	pRegIdct32[0x080>>2] |= 0x04;	// 9-bit range
	pRegIdct32[0x080>>2] |= 0x01;
	while ((pRegIdct32[0x088>>2] & 0x01) != 0x01)
		;

	// Return transformed data
	memcpy(pDataOut, (const void*)pRegIdct32, 32 * sizeof(U32));
}

/*
 * Perform dequantization and inverse DCT on one block of coefficients.
 */

GLOBAL(void)
jpeg_idct_hw (j_decompress_ptr cinfo, jpeg_component_info * compptr,
		 JCOEFPTR coef_block,
		 JSAMPARRAY output_buf, JDIMENSION output_col)
{
	JCOEFPTR inptr;
	ISLOW_MULT_TYPE * quantptr;
	JSAMPROW outptr;
	JSAMPLE *range_limit = IDCT_range_limit(cinfo);
	S32 workspace[DCTSIZE2], output[DCTSIZE2];
	S32 *wsptr = workspace, *ptr = output;

	int ctr, b = 0, div;

	inptr = coef_block;
	quantptr = (ISLOW_MULT_TYPE *) compptr->dct_table;

	for(ctr = 0; ctr < DCTSIZE2; ctr+=2, b++)
	{
		short one = inptr[ctr+0] * quantptr[ctr+0];
		short two = inptr[ctr+1] * quantptr[ctr+1];

		// TODO: shift these 16-bit values into 12-bit IDCT inputs?
		// TODO: unroll loop?

		workspace[b] =  ((two) << 16) | (one);
	}

	decode_idct(cinfo, workspace, output);

	for (ctr = 0; ctr < DCTSIZE; ctr++)
	{
		outptr = output_buf[ctr] + output_col;

		short zero	= ptr[0];
		short one	= (ptr[0] >> 16);
		short two	= ptr[1];
		short three	= (ptr[1] >> 16);
		short four	= ptr[2];
		short five	= (ptr[2] >> 16);
		short six	= ptr[3];
		short seven	= (ptr[3] >> 16);

		outptr[0] = range_limit[zero & RANGE_MASK];
		outptr[1] = range_limit[one & RANGE_MASK];
		outptr[2] = range_limit[two & RANGE_MASK];
		outptr[3] = range_limit[three & RANGE_MASK];
		outptr[4] = range_limit[four & RANGE_MASK];
		outptr[5] = range_limit[five & RANGE_MASK];
		outptr[6] = range_limit[six & RANGE_MASK];
		outptr[7] = range_limit[seven & RANGE_MASK];

		ptr += 4;
	}
}

#endif /* DCT_HW_SUPPORTED */
