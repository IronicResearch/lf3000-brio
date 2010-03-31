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
const tVidCapHndl	kInvalidAudCapHndl = static_cast<tAudCapHndl>(0);


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

//==============================================================================
// Camera events and messages
//==============================================================================
#define CAMERA_EVENTS					\
	(kCaptureTimeOutEvent)				\
	(kCaptureQuotaHitEvent)				\
	(kCaptureStoppedEvent)

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

union tCaptureMsg {
	tCaptureTimeoutMsg		timeOut;
	tCaptureQuotaHitMsg		quotaHit;
	tCaptureStoppedMsg		stopped;
};

//------------------------------------------------------------------------------
class CCameraEventMessage : public IEventMessage {
public:
	CCameraEventMessage( const tCaptureTimeoutMsg& data );
	CCameraEventMessage( const tCaptureQuotaHitMsg& data );
	CCameraEventMessage( const tCaptureStoppedMsg& data );
	virtual U16	GetSizeInBytes() const;

	tCaptureMsg	data;
};

LF_END_BRIO_NAMESPACE()

#endif	// LF_BRIO_CAMERATYPES_H

// EOF
