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

//lookup table providing hardware tPixelFormat equivalents
enum tLayerPixelFormat {
	kLayerPixelFormatError 		= 0,
	kLayerPixelFormatRGB4444 	= 0x2211, //A4R4G4B4
	kLayerPixelFormatRGB565		= 0x4432, //R5G6B5
	kLayerPixelFormatARGB8888	= 0x0653, //A8R8G8B8
};

#endif
