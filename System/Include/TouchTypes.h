#ifndef LF_BRIO_TOUCHTYPES_H
#define LF_BRIO_TOUCHTYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		TouchTypes.h
//
// Description:
//		Defines types for the Touch Manager module. 
//
//==============================================================================

#include <EventMessage.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================	   
// Touch events
//==============================================================================
#define TOUCH_EVENTS					\
	(kTouchStateChanged)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupTouch), TOUCH_EVENTS)

const tEventType kAllTouchEvents = AllEvents(kGroupTouch);


//==============================================================================	   
// Touch errors
//==============================================================================
#define TOUCH_ERRORS				\
	(kTouchEmulationConfigErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupTouch), TOUCH_ERRORS)


//==============================================================================	   
// Touch types
//==============================================================================
//------------------------------------------------------------------------------
struct tTouchData {
	U16 touchState;// Boolean really: 0=no touch, 1=touch
	S16 touchX, touchY;// Position in screen coords
	struct timeVal {
		S32 seconds;
		S32 microSeconds;
	} time;
};

//------------------------------------------------------------------------------
class CTouchMessage : public IEventMessage {
public:
	CTouchMessage( const tTouchData& data );
	virtual U16	GetSizeInBytes() const;
	tTouchData GetTouchState() const;
private:
	tTouchData	mData;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_TOUCHTYPES_H

// eof
