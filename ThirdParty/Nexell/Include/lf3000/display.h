//------------------------------------------------------------------------------
//
//  Copyright (C) 2013 Nexell Co. All Rights Reserved
//  Nexell Co. Proprietary & Confidential
//
//  NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//  Module      :
//  File        :
//  Description :
//  Author      : 
//  Export      :
//  History     :
//
//------------------------------------------------------------------------------

#ifndef	__DISPLAY__
#define	__DISPLAY__

#include <nx_alloc_mem.h>
#include <nxp-v4l2-media.h>

#define	MLC_DISPLAY_BUF_SIZE	2

typedef struct MLC_DISPLAY_INFO MLC_DISPLAY_INFO, *DISPLAY_HANDLE;

struct MLC_DISPLAY_INFO{
	V4L2_PRIVATE_HANDLE hDisplay;
	int	width;
	int	height;
	int	numPlane;
	int	numberV4L2ReqBuf;
	int	isOn;
	int	lastQueueIdx;
	NX_VID_MEMORY_INFO *videoBuf[MLC_DISPLAY_BUF_SIZE];
};

#ifdef __cplusplus
extern "C"{
#endif

DISPLAY_HANDLE InitDisplay(int width, int height, int left, int top, int right, int bottom, int option );
int DisplayQueueBuffer(DISPLAY_HANDLE handle, NX_VID_MEMORY_INFO *pVidBuf);
int DisplayDequeueBuffer(DISPLAY_HANDLE handle);
void CloseDisplay(DISPLAY_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif	//	__DISPLAY__
