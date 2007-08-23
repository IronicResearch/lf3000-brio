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
#define OGL_LAYER_ID	1
#define OGL_LAYER_DEV	"/dev/layer1"

// layer to be used by DisplayManager for Video YUV
#define YUV_LAYER_ID	3
#define YUV_LAYER_DEV	"/dev/layer3"

// pins used for LCD and board-specific tasks
#ifdef  ME2530
#define PIN_LCD_ENABLE			10
#define PIN_BACKLIGHT_ENABLE	9
#define PIN_BLUE_LED			29
#else
#define PIN_LCD_ENABLE			31
#define PIN_BACKLIGHT_ENABLE	28
#endif

#define ALPHA_STEP				15

// Hardware layer format codes corresponding to tPixelFormat equivalents
const U32	kLayerPixelFormatRGB4444 	= 0x2211; //A4R4G4B4
const U32	kLayerPixelFormatRGB565		= 0x4432; //R5G6B5
const U32	kLayerPixelFormatARGB8888	= 0x0653; //A8R8G8B8
const U32	kLayerPixelFormatXRGB8888	= 0x4653; //X8R8G8B8 (HStride=4)
const U32	kLayerPixelFormatRGB888		= 0x4653; //R8G8B8 (HStride=3)
const U32	kLayerPixelFormatYUV420		= 0xFFF0; //YUV planar	
const U32	kLayerPixelFormatYUYV422	= 0xFFF2; //YUYV packed

#endif // LF_DISPLAYHW_H

// EOF
