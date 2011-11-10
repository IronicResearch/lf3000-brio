#ifndef LF_BRIO_MICROPHONETYPES_H
#define LF_BRIO_MICROPHONETYPES_H
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		MicrophoneTypes.h
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
const tAudCapHndl	kInvalidAudCapHndl = static_cast<tAudCapHndl>(0);

enum tMicrophoneParam {
	kMicrophoneThreshold,
	kMicrophoneDuration,
	kMicrophoneClipCount,
	kMicrophoneRateAdjust,
};

//==============================================================================
// Camera events and messages
//==============================================================================
#define MICROPHONE_EVENTS					\
	(kCaptureTimeOutEvent)				\
	(kCaptureQuotaHitEvent)				\
	(kCaptureStoppedEvent)				\
	(kMicrophoneRemovedEvent)				\
	(kAudioTriggeredEvent)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupMicrophone), MICROPHONE_EVENTS)

const tEventType 	kAllCameraEvents = AllEvents(kGroupMicrophone);

//------------------------------------------------------------------------------
//Use for both Video and Audio captures
struct tCaptureTimeoutMsg {
	tAudCapHndl		ahndl;
	Boolean		saved;		// was the recording saved properly?
	U32			length;		// actual length of recording
};

struct tCaptureQuotaHitMsg {
	tAudCapHndl		ahndl;
	Boolean		saved;
	U32			length;
};

struct tCaptureStoppedMsg {
	tAudCapHndl		ahndl;
	Boolean		saved;
	U32			length;
};

struct tMicrophoneRemovedMsg {
	tAudCapHndl		ahndl;
	Boolean		saved;
	U32			length;
};

struct tAudioTriggeredMsg {
	tAudCapHndl	ahndl;
	Boolean		threshold;
	U32			timestamp;
};

union tCaptureMsg {
	tCaptureTimeoutMsg		timeOut;
	tCaptureQuotaHitMsg		quotaHit;
	tCaptureStoppedMsg		stopped;
	tMicrophoneRemovedMsg		removed;
	tAudioTriggeredMsg		triggered;
};

//------------------------------------------------------------------------------
class CMicrophoneEventMessage : public IEventMessage {
public:
	CMicrophoneEventMessage( const tCaptureTimeoutMsg& data );
	CMicrophoneEventMessage( const tCaptureQuotaHitMsg& data );
	CMicrophoneEventMessage( const tCaptureStoppedMsg& data );
	CMicrophoneEventMessage( const tMicrophoneRemovedMsg& data );
	CMicrophoneEventMessage( const tAudioTriggeredMsg& data );
	virtual U16	GetSizeInBytes() const;

	tCaptureMsg	data;
};

LF_END_BRIO_NAMESPACE()

#endif	// LF_BRIO_MICROPHONETYPES_H

// EOF
