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
	kMicrophoneRate,
	kMicrophoneBlockSize,
};

//==============================================================================
// Camera events and messages
//==============================================================================
#define MICROPHONE_EVENTS					\
	(kMicrophoneCaptureTimeOutEvent)				\
	(kMicrophoneCaptureQuotaHitEvent)				\
	(kMicrophoneCaptureStoppedEvent)				\
	(kMicrophoneRemovedEvent)				\
	(kMicrophoneAudioTriggeredEvent)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupMicrophone), MICROPHONE_EVENTS)

const tEventType 	kAllMicrophoneEvents = AllEvents(kGroupMicrophone);

//------------------------------------------------------------------------------
//Use for both Video and Audio captures
struct tMicrophoneCaptureTimeoutMsg {
	tAudCapHndl		ahndl;
	Boolean		saved;		// was the recording saved properly?
	U32			length;		// actual length of recording
};

struct tMicrophoneCaptureQuotaHitMsg {
	tAudCapHndl		ahndl;
	Boolean		saved;
	U32			length;
};

struct tMicrophoneCaptureStoppedMsg {
	tAudCapHndl		ahndl;
	Boolean		saved;
	U32			length;
};

struct tMicrophoneRemovedMsg {
	tAudCapHndl		ahndl;
	Boolean		saved;
	U32			length;
};

struct tMicrophoneAudioTriggeredMsg {
	tAudCapHndl	ahndl;
	Boolean		threshold;
	U32			timestamp;
};

union tMicrophoneCaptureMsg {
	tMicrophoneCaptureTimeoutMsg		timeOut;
	tMicrophoneCaptureQuotaHitMsg		quotaHit;
	tMicrophoneCaptureStoppedMsg		stopped;
	tMicrophoneRemovedMsg		removed;
	tMicrophoneAudioTriggeredMsg		triggered;
};

//------------------------------------------------------------------------------
class CMicrophoneEventMessage : public IEventMessage {
public:
	CMicrophoneEventMessage( const tMicrophoneCaptureTimeoutMsg& data );
	CMicrophoneEventMessage( const tMicrophoneCaptureQuotaHitMsg& data );
	CMicrophoneEventMessage( const tMicrophoneCaptureStoppedMsg& data );
	CMicrophoneEventMessage( const tMicrophoneRemovedMsg& data );
	CMicrophoneEventMessage( const tMicrophoneAudioTriggeredMsg& data );
	virtual U16	GetSizeInBytes() const;

	tMicrophoneCaptureMsg	data;
};

LF_END_BRIO_NAMESPACE()

#endif	// LF_BRIO_MICROPHONETYPES_H

// EOF
