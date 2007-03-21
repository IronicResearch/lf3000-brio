#ifndef FLASHLITE2_0TYPES_H
#define FLASHLITE2_0TYPES_H

//==============================================================================
// $Source: $
//
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		FlashLite20Types.h
//
// Description:
//		Defines the structures specific to the FlashLite20 Module. 
//
//==============================================================================


// Flash Creation Flags
enum {
    FC_4WayNavigation               = 0x00000001,       /* If bit 1 is set then use 4way navigation, otherwise 2way */
    FC_PlayerRendersText            = 0x00000002,       /* If bit 2 is set then the player will request character bitmaps to render text */
    FC_4WayWrap                     = 0x00000004,       /* If bits 1 and 3 are set then use a 4-way wrapping navigation mode */
    FC_FlashLiteSVG                 = 0x00000008,       /* If bit 4 is set, it indicates support for SVG content */
    // Bits 5 through 7 in use
	FC_JPEGDecode					= 0x00000080,		/* Attempt to decode JPEGS internally */
    FC_DynamicMemAvailable          = 0x00000100,       /* If bit 9 is set, it indicates that dynamic memory is available */
    FC_SharedObjects                = 0x00000200,       /* If bit 10 is set, it indicates that shared objects are enabled */
    FC_UseUTF16EncodingForm         = 0x00000400,       /* If bit 11 is set, then FI_Text stores UTF-16 chars */
    FC_UsePlayerEmbeddedVectorFont  = 0x00800000        /* If bit 24 is set, support for player embedded vector fonts is enabled */
};

// System Creation Flags
enum {
	flashCreateMemoryPool = 0x00000001, // if bit set, creates memory for you.
	flashCreateDisplayBuffer = 0x00000002, // if bit set, creates memory for you.
	flashLooping = 0x00000004, // if bit set, loops the initial movie.
};

struct tFlashCreationFlags {
	tRect size;
	tDisplayZOrder zOrder;
	U8  *pDisplayBuffer;
	U8  *pMemoryPool;
	U32 nMemoryPoolSize;
	U32 systemCreationFlags; 	// System Creation Flags
	U32 flashCreationFlags; 	// Flash Creation Flags
};


enum {
	tFlashStateUnloaded,
	tFlashStateStopped,
	tFlashStatePlaying,
	tFlashStatePaused,
};
typedef U32 tFlashMovieState;

#define kFirstSystemErr 0x00001000
#define kFirstFlashErr 0x00002000
enum {
	kFlashNullPtrErr = kFirstSystemErr,
	kFlashInvalidScreenErr,
	kFlashDisplayNotInListErr,
	kFlashNoScreensSelectedErr,
	kFlashUnsupportedZOrder,	
	
	kFlashMovieInvalid = kFirstFlashErr,
	kFlashCreatePlayerErr,
	kFlashRectangleInvalid,
	kFlashFrameBufferInvalid,
	kFlashMovieLoadErr,
	kFlashTaskCreateErr,
};


#endif // FLASHLITE2_0TYPES_H

// eof
