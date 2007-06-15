#ifndef LF_BRIO_BUTTONTYPES_H
#define LF_BRIO_BUTTONTYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		ButtonTypes.h
//
// Description:
//		Defines types for the Button Manager module. 
//
//==============================================================================

#include <EventMessage.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================	   
// Button events
//==============================================================================
#define BUTTON_EVENTS					\
	(kButtonStateChanged)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupButton), BUTTON_EVENTS)

const tEventType kAllButtonEvents = AllEvents(kGroupButton);


//==============================================================================	   
// Button errors
//==============================================================================
#define BUTTON_ERRORS				\
	(kButtonEmulationConfigErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupButton), BUTTON_ERRORS)


//==============================================================================	   
// Button types
//==============================================================================
//------------------------------------------------------------------------------
struct tButtonData {
	U32	buttonState;
	U32	buttonTransition;
};

const U32 kButtonUp				= (1 << 0);
const U32 kButtonDown			= (1 << 1);
const U32 kButtonRight			= (1 << 2);
const U32 kButtonLeft			= (1 << 3);
const U32 kButtonA				= (1 << 4);
const U32 kButtonB				= (1 << 5);
const U32 kButtonLeftShoulder	= (1 << 6);
const U32 kButtonRightShoulder	= (1 << 7);
const U32 kButtonMenu			= (1 << 8);
const U32 kButtonHint			= (1 << 9);
const U32 kButtonPause			= (1 << 10);
const U32 kButtonBrightness		= (1 << 11);


//------------------------------------------------------------------------------
class CButtonMessage : public IEventMessage {
public:
	CButtonMessage( const tButtonData& data );
	virtual U16	GetSizeInBytes() const;
	tButtonData GetButtonState() const;
private:
	tButtonData	mData;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_BUTTONTYPES_H

// eof
