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

// pins used for LCD and board-specific tasks
#define PIN_LCD_ENABLE			10
#define PIN_BACKLIGHT_ENABLE	9
#define PIN_BLUE_LED			29

// size of the frame buffer (note: this is from drivers/mlc/mlc_config.h)
#define FB_SIZE					0x11A000

// lookup table providing hardware tPixelFormat equivalents
enum tLayerPixelFormat {
	kLayerPixelFormatError 		= 0,
	kLayerPixelFormatRGB4444 	= 0x2211, //A4R4G4B4
	kLayerPixelFormatRGB565		= 0x4432, //R5G6B5
	kLayerPixelFormatARGB8888	= 0x0653, //A8R8G8B8
};

#endif
