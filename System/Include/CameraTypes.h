#ifndef LF_BRIO_CAMERATYPES_H
#define LF_BRIO_CAMERATYPES_H
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		CameraTypes.h
//
// Description:
//		Type definitions for the Brio Camera subsystem.
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <GroupEnumeration.h>
#include <EventMessage.h>
#include <SystemEvents.h>
#include <DisplayTypes.h>

#include <vector>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Typedefs
//==============================================================================

// Video capture handle
typedef tHndl 		tVidCapHndl;

const tVidCapHndl	kInvalidVidCapHndl = static_cast<tVidCapHndl>(0);

// Image capture format.  Uncompressed formats are possible - these would be equivalent to
// DisplayTypes:tPixelFormat.  Since JPEG is compressed, it's not a pixel format in the
// proper sense
enum tCaptureFormat {
	kCaptureFormatError = 0,
	kCaptureFormatMJPEG
};

// Three components: image format, image resolution, and video frame rate, determine
// the camera's capture mode
struct tCaptureMode {
	tCaptureFormat	pixelformat;
	U16				width;
	U16				height;
	U32				fps_numerator;
	U32				fps_denominator;
};

typedef std::vector<tCaptureMode *> tCaptureModes;

enum tControlType {
	kControlTypeError = 0,
	kControlTypeBrightness,
	kControlTypeContrast,
	kControlTypeSaturation,
	kControlTypeHue,
	kControlTypeGamma,
	kControlPowerLineFreq,
	kControlTypeSharpness,
	kControlTypeBacklightComp,
};

// Controls info
struct tControlInfo {
	tControlType	type;
	S32				min;
	S32				max;
	S32				preset;
	S32				current;
};

typedef std::vector<tControlInfo*> tCameraControls;

// Frame info
struct tFrameInfo {
	tCaptureFormat	pixelformat;
	U16				width;
	U16				height;
	U32				index;
	void *			data;
	U32				size;
};

enum tBitmapFormat {
	kBitmapFormatError = 0,
	kBitmapFormatGrayscale8,
	kBitmapFormatRGB888,
	kBitmapFormatYCbCr888,
};

// Bitmap image (processed frame) info
struct tBitmapInfo {
	tBitmapFormat	format;
	U16				width;
	U16				height;
	U16				depth;
	U8 *			data;
	U32				size;
};

LF_END_BRIO_NAMESPACE()

#endif	// LF_BRIO_CAMERATYPES_H

// EOF
