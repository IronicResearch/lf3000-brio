#ifndef LF_BRIO_EVENTMESSAGE_H
#define LF_BRIO_EVENTMESSAGE_H
//============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		EventMessage.h
//
// Description:
//		Base class for all events.  The EventManager class posts objects
//		derived from this class, EventListner-derived objects receive them.
//
//============================================================================

#include <ResourceTypes.h>


//============================================================================
class IEventMessage { 
public:
	IEventMessage(tEventType type, tRsrcHndl handle)
		: mEventType(type), mEventSource(handle) { };
	virtual ~IEventMessage() { };

	tEventType	GetEventType() const	{ return mEventType; }
	tRsrcHndl	GetEventSource() const	{ return mEventSource; }
	virtual U16	GetSizeInBytes() const	= 0;

private:
	tEventType	mEventType;			// Event type
	tRsrcHndl	mEventSource;		// Source of the message FIXME/tp: needed?
};

#endif // LF_BRIO_EVENTMESSAGE_H

// EOF
