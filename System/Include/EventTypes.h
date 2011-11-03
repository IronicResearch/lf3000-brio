#ifndef LF_BRIO_EVENTTYPES_H
#define LF_BRIO_EVENTTYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		EventTypes.h
//
// Description:
//		Defines enums and types for EventMgr. 
//
//==============================================================================

#include <EventMessage.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
// EventListener events
//==============================================================================
#define EVENT_LISTENER_EVENTS	\
	(kEventListenerRegistered)	\
	(kEventListenerUnregistered)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupEvent), EVENT_LISTENER_EVENTS)

const tEventType kAllEvenListenerEvents = AllEvents(kGroupEvent);

//==============================================================================	   
// Event Manager errors 
//==============================================================================
#define EVENT_ERRORS					\
	(kEventNotFoundErr)					\
	(kEventTypeNotFoundErr)				\
	(kEventListenerNotRegisteredErr)	\
	(kEventListenerCycleErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupEvent), EVENT_ERRORS)

#define kEventRegistrationStayLastBitMask 0x1

//==============================================================================	   
// Event Manager types 
//==============================================================================
typedef U32		tListenerId;
typedef U32		tEventRegistrationFlags;
typedef U8		tEventPriority;

const tListenerId kNoListener = static_cast<tListenerId>(0);
//------------------------------------------------------------------------------

class IEventListener;
struct tEventListenerData {
	const IEventListener	*eventListenerId;
};
//------------------------------------------------------------------------------
class CEventListenerMessage : public IEventMessage {
public:
	CEventListenerMessage( U32 event_type, const struct tEventListenerData& data );
	virtual U16	GetSizeInBytes() const;
	tEventListenerData GetEventListenerData() const;
private:
	tEventListenerData	mData;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_EVENTTYPES_H

// eof
