#ifndef LF_DUSPLAYHW_H
#define LF_DUSPLAYHW_H
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

// layer to be used by DisplayManager for 2D RGB
#define RGB_LAYER_ID	0
#define RGB_LAYER_DEV	"/dev/layer0"

// layer to be used by DisplayManager for Video YUV
#define YUV_LAYER_ID	3
#define YUV_LAYER_DEV	"/dev/layer3"

// pins used for LCD and board-specific tasks
#define PIN_LCD_ENABLE			10
#define PIN_BACKLIGHT_ENABLE	9
#define PIN_BLUE_LED			29

#define ALPHA_STEP				15

// lookup table providing hardware tPixelFormat equivalents
enum tLayerPixelFormat {
	kLayerPixelFormatError 		= 0,
	kLayerPixelFormatRGB4444 	= 0x2211, //A4R4G4B4
	kLayerPixelFormatRGB565		= 0x4432, //R5G6B5
	kLayerPixelFormatARGB8888	= 0x0653, //A8R8G8B8
	kLayerPixelFormatRGB888		= 0x4653, //R8G8B8
	kLayerPixelFormatYUV420		= 0xFFF0, //YUV planar	
	kLayerPixelFormatYUYV422	= 0xFFF2, //YUYV packed
};

// TODO/dm: Implement actual lookup table for format codes!
// Enum values for YUV format codes currently need padding with 0xFFF0.
// Format codes may possibly change in future hardware register specs.

#endif
