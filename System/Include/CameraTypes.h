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
typedef tHndl 		tAudCapHndl;

const tVidCapHndl	kInvalidVidCapHndl = static_cast<tVidCapHndl>(0);
//const tVidCapHndl	kInvalidAudCapHndl = static_cast<tAudCapHndl>(0);


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
	kControlTypeHorizontalFlip,
	kControlTypeVerticalFlip,
	kControlTypeRotate,
	kControlTypeAutoWhiteBalance,
	kControlTypeExposure,
	kControlTypeAutoExposure,
	kControlTypeGain,
	kControlTypeTemperature
};

enum tColorOrder {
	kOpenGlRgb,
	kDisplayRgb
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

// Image capture format.  Uncompressed formats are possible - these would be equivalent to
// DisplayTypes:tPixelFormat.  Since JPEG is compressed, it's not a pixel format in the
// proper sense
enum tCaptureFormat {
	kCaptureFormatError = 0,
	kCaptureFormatMJPEG,
	kCaptureFormatRAWYUYV,
	kCaptureFormatYUV420,
};

/// Three components: image format, image resolution, and video frame rate, determine
/// the camera's capture mode. Note video frame rate is used by Video4Linux in units
/// of time-per-frame, so the 'fps' fields are misnamed and actually referring to
/// inverse fps when using numerator:denominator ratio.
struct tCaptureMode {
	tCaptureFormat	pixelformat;
	U16				width;
	U16				height;
	U32				fps_numerator;		///< numerator for time-per-frame (inverse fps)
	U32				fps_denominator;	///< denominator for time-per-frame (inverse fps)
};

typedef std::vector<tCaptureMode *> tCaptureModes;

/// Camera device type passed to \ref CCameraMPI::SetCurrentCamera() or returned from \ref CCameraMPI::GetCurrentCamera().
/// kCameraNone is returned from \ref CCameraMPI::GetCurrentCamera() only if no camera device is present.
enum tCameraDevice {
	kCameraNone,		///< no camera device (GetCurrentCamera() only)
	kCameraDefault,		///< default camera selection
	kCameraFront,		///< front-facing camera selection
};

//==============================================================================
// Camera events and messages
//==============================================================================
#define CAMERA_EVENTS					\
	(kCaptureTimeOutEvent)				\
	(kCaptureQuotaHitEvent)				\
	(kCaptureStoppedEvent)				\
	(kCameraRemovedEvent)				\
	(kAudioTriggeredEvent)				\
	(kCaptureFrameEvent)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupCamera), CAMERA_EVENTS)

const tEventType 	kAllCameraEvents = AllEvents(kGroupCamera);

//------------------------------------------------------------------------------
//Use for both Video and Audio captures
struct tCaptureTimeoutMsg {
	union {
		tVidCapHndl		vhndl;
		tAudCapHndl		ahndl;
	};
	Boolean		saved;		// was the recording saved properly?
	U32			length;		// actual length of recording
};

struct tCaptureQuotaHitMsg {
	union {
		tVidCapHndl		vhndl;
		tAudCapHndl		ahndl;
	};
	Boolean		saved;
	U32			length;
};

struct tCaptureStoppedMsg {
	union {
		tVidCapHndl		vhndl;
		tAudCapHndl		ahndl;
	};
	Boolean		saved;
	U32			length;
};

struct tCameraRemovedMsg {
	union {
		tVidCapHndl		vhndl;
		tAudCapHndl		ahndl;
	};
	Boolean		saved;
	U32			length;
};

struct tAudioTriggeredMsg {
	tAudCapHndl	ahndl;
	Boolean		threshold;
	U32			timestamp;
};

struct tCaptureFrameMsg {
	tVidCapHndl	vhndl;
	U32			frame;
	struct timeVal {
		S32	seconds;
		S32	microSeconds;
	} timestamp;
};

union tCaptureMsg {
	tCaptureTimeoutMsg		timeOut;
	tCaptureQuotaHitMsg		quotaHit;
	tCaptureStoppedMsg		stopped;
	tCameraRemovedMsg		removed;
	tAudioTriggeredMsg		triggered;
	tCaptureFrameMsg		framed;
};

//------------------------------------------------------------------------------
class CCameraEventMessage : public IEventMessage {
public:
	CCameraEventMessage( const tCaptureTimeoutMsg& data );
	CCameraEventMessage( const tCaptureQuotaHitMsg& data );
	CCameraEventMessage( const tCaptureStoppedMsg& data );
	CCameraEventMessage( const tCameraRemovedMsg& data );
	CCameraEventMessage( const tAudioTriggeredMsg& data );
	CCameraEventMessage( const tCaptureFrameMsg& data );
	virtual U16	GetSizeInBytes() const;

	tCaptureMsg	data;
};

LF_END_BRIO_NAMESPACE()

#endif	// LF_BRIO_CAMERATYPES_H

// EOF
