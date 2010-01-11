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

// Controls info
struct tControlInfo {

};

typedef std::vector<tControlInfo> tCameraControls;

// Frame info
struct tFrameInfo {
	tCaptureFormat	pixelformat;
	U16				width;
	U16				height;
	U32				index;
	void *			data;
	U32				size;
};

LF_END_BRIO_NAMESPACE()

#endif	// LF_BRIO_CAMERATYPES_H

// EOF
