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

#ifndef	__NX_DSP_H__
#define	__NX_DSP_H__

#include <stdint.h>
#include <pthread.h>

#include <nx_alloc_mem.h>
#include <nxp-v4l2-media.h>

#define	DISPLAY_MAX_BUF_SIZE	2

enum {
	DISPLAY_PORT_LCD	= 0x00,
	DISPLAY_PORT_HDMI	= 0x01,
};

enum {
	DISPLAY_MODULE_MLC0			= 0x00,
	DISPLAY_MODULE_MLC1			= 0x01,
};

typedef struct DISPLAY_INFO			DISPLAY_INFO;
typedef struct DISPLAY_HANDLE_INFO	DISPLAY_HANDLE_INFO, *DISPLAY_HANDLE;

struct DISPLAY_INFO {
	int32_t		port;		// port
	int32_t		module;		// module

	int32_t		width;		// width
	int32_t		height;		// height

	int32_t		left;		// left
	int32_t		top;		// top
	int32_t		right;		// right
	int32_t		bottom;		// bottom
};

struct DISPLAY_HANDLE_INFO {
	V4L2_PRIVATE_HANDLE	hPrivate;

	//  Setting values
	DISPLAY_INFO		displayInfo;
	int32_t				mlcId;
	int32_t				hdmiId;
	
	//  Buffer Control Informations
	NX_VID_MEMORY_INFO	*videoBuf[DISPLAY_MAX_BUF_SIZE];
	int32_t				numPlane;			// video memory plane
	int32_t				numberV4L2ReqBuf;	// vidoe buffer size
	int32_t				lastQueueIdx;

	bool				streamOnFlag;		// on/off flag
	pthread_mutex_t		hMutex;
};

#ifdef __cplusplus
extern "C"{
#endif

DISPLAY_HANDLE	NX_DspInit			( DISPLAY_INFO *pDspInfo );
void			NX_DspClose			( DISPLAY_HANDLE hDisplay );
int32_t			NX_DspStreamControl	( DISPLAY_HANDLE hDisplay, bool enable );	

int32_t			NX_DspQueueBuffer	( DISPLAY_HANDLE hDisplay, NX_VID_MEMORY_INFO *pVidBuf );
int32_t			NX_DspDequeueBuffer	( DISPLAY_HANDLE hDisplay );

#ifdef __cplusplus
}
#endif

#endif	//	__NX_DSP_H__
