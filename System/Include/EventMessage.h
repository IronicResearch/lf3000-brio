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

#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//============================================================================
class IEventMessage { 
public:
	IEventMessage(tEventType type)
		: eventType_(type){ };
	virtual ~IEventMessage() { };

	tEventType	GetEventType() const	{ return eventType_; }
	virtual U16	GetSizeInBytes() const	= 0;

private:
	tEventType	eventType_;			// Event type
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_EVENTMESSAGE_H

// EOF
