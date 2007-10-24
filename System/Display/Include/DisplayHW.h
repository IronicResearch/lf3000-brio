#ifndef LF_DISPLAYHW_H
#define LF_DISPLAYHW_H
//=============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//=============================================================================
//
// File:
//      DisplayHW.h
//
// Description:
//      Hardware-specific definitions for the DisplayManager
//
//==============================================================================

#include <SystemTypes.h>
LF_USING_BRIO_NAMESPACE()

// layer to be used by DisplayManager for 2D RGB
#define RGB_LAYER_ID	0
#define RGB_LAYER_DEV	"/dev/layer0"

// layer to be used by DisplayManager for 3D RGB  OpenGL rendering context
#define OGL_LAYER_ID		1
#define OGL_LAYER_DEV		"/dev/layer1"
#define OGL_LAYER_EVEN_ID	0
#define OGL_LAYER_EVEN_DEV	"/dev/layer0"
#define OGL_LAYER_ODD_ID	OGL_LAYER_ID
#define OGL_LAYER_ODD_DEV	OGL_LAYER_DEV

// layer to be used by DisplayManager for Video YUV
#ifdef LF1000
#define YUV_LAYER_ID	2
#define YUV_LAYER_DEV	"/dev/layer2"
#else
#define YUV_LAYER_ID	3
#define YUV_LAYER_DEV	"/dev/layer3"
#endif

// pins used for LCD and board-specific tasks
#ifdef  ME2530
#define PIN_LCD_ENABLE			10
#define PIN_BACKLIGHT_ENABLE	9
#define PIN_BLUE_LED			29
#elif   defined(LF2530)
#define PIN_LCD_ENABLE			31
#define PIN_BACKLIGHT_ENABLE	28
#elif   defined(LF1000)
#define PORT_LCD				0
#define PIN_BACKLIGHT_ENABLE	30
#endif

#define ALPHA_STEP				15

// Hardware layer format codes corresponding to tPixelFormat equivalents
const U32	kLayerPixelFormatRGB4444 	= 0x2211; //A4R4G4B4
const U32	kLayerPixelFormatRGB565		= 0x4432; //R5G6B5
const U32	kLayerPixelFormatARGB8888	= 0x0653; //A8R8G8B8
const U32	kLayerPixelFormatXRGB8888	= 0x4653; //X8R8G8B8 (HStride=4)
const U32	kLayerPixelFormatRGB888		= 0x4653; //R8G8B8 (HStride=3)
const U32	kLayerPixelFormatYUV420		= 0x0000; //YUV planar	
const U32	kLayerPixelFormatYUYV422	= 0x0002; //YUYV packed

const U32	k1Meg  = 1024 * 1024;
const U32 	k16Meg = 4096 * 4096;

//----------------------------------------------------------------------------
// Convert linear address to XY block address
inline U32 LIN2XY(U32 addr)
{
	U32 segment = addr / k16Meg;
	U32 offset  = addr % k16Meg;
	U32 y = offset / 4096;
	U32 x = offset % 4096;
	return 0x20000000 | (segment << 24) | (y << 12) | (x << 0);
}

#endif // LF_DISPLAYHW_H

// EOF
