#ifndef FLASHLITE2_0NATIVE_H
#define FLASHLITE2_0NATIVE_H

//==============================================================================
// $Source: $
//
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		FlashLite20Native.h
//
// Description:
//		Defines the structures specific to the FlashLite20 native code extensions. 
//
//==============================================================================

#define tFlashNative unsigned long

// ActionScript extension function
typedef void (*tLfASExtensionFcn)(tFlashNative);

struct tFlashPlayerASExtensionEntry{
	const char* 		extensionName;
	tLfASExtensionFcn 	ExtensionFcn;	  				
};

struct tFlashPlayerRsrcASExtensionTable{
	unsigned short          fmtVersion;					// format version of this struct (kFlashPlayerRsrcASExtTableFmtVersion)
	unsigned short			reserved[4];				// reserved (must be 0)
	unsigned short			numExtensionFcns;
	const tFlashPlayerASExtensionEntry *extensionFcns;	  
};

extern tFlashPlayerRsrcASExtensionTable* GetExtensionTable(void);
extern tFlashPlayerRsrcASExtensionTable* GetSystemExtensionTable(void);
typedef tFlashPlayerRsrcASExtensionTable* (*PFNC_GET_EXTENSION_TABLE)(void);


#endif // FLASHLITE2_0NATIVE_H

// eof
