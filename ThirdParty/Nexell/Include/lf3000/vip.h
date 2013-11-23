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

#ifndef __VIP_H__
#define	__VIP_H__

#include <pthread.h>
#include <nx_alloc_mem.h>
#include <nxp-v4l2-media.h>

#define	VIP_MAX_BUF_SIZE		32			//	currently set minimum size for encoding & display

//	VIP MODE
enum {
	VIP_MODE_CLIPPER   = 0x1,
	VIP_MODE_DECIMATOR = 0x2,
	VIP_MODE_CLIP_DEC  = 0x3,
};

typedef struct VIDEO_INPUT_INFO VIP_INFO_TYPE;

struct VIDEO_INPUT_INFO{
	int port;		//	video input processor's port number
	int mode;		//	1 : clipper only, 2 : decimator only, 3 : clipper --> decimator

	//	Camera Input Size
	int width;
	int height;

	//	fps : fpsNum / fspDen
	int fpsNum;		//	Frame per seconds's Numerate value
	int fpsDen;		//	Frame per seconds's Denominate value

	//	Cliper
	int cropX;
	int cropY;
	int cropWidth;
	int cropHeight;

	//	Decimator
	int outWidth;
	int outHeight;
};


typedef struct VIP_HANDLE_INFO VIP_HANDLE_INFO, *VIP_HANDLE_TYPE;

struct VIP_HANDLE_INFO{
	V4L2_PRIVATE_HANDLE	hPrivate;		//  private handle

	int	streamOnFlag;					//	on/off flag
	VIP_INFO_TYPE vipInfo;				//	Input Information
	NX_VID_MEMORY_INFO *pPrevMem;

	//	Setting Values
	int mode;							//	same as Video Input Type' mode
	int cliperId;						//	
	int sensorId;						//	
	int decimatorId;					//	

	int	curQueuedSize;
	//	Buffer Control Informatons
	NX_VID_MEMORY_INFO *pMgmtMem[VIP_MAX_BUF_SIZE];

	pthread_mutex_t hMutex;
};


#ifdef __cplusplus
extern "C"{
#endif

VIP_HANDLE_TYPE InitVip( VIP_INFO_TYPE *pVipInfo );
int VipQueueBuffer( VIP_HANDLE_TYPE hVip, NX_VID_MEMORY_INFO *pInfo );
int VipDequeueBuffer( VIP_HANDLE_TYPE hVip, NX_VID_MEMORY_INFO **ppInfo );
//	return current queue count & max queue size
int VipGetCurrentQueuedCount( VIP_HANDLE_TYPE hVip, int *maxSize );
void CloseVip( VIP_HANDLE_TYPE hVip );

#ifdef __cplusplus
}
#endif

#endif	//	__VIP_H__