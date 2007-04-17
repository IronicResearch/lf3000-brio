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

const U32 kButtonUpKey		= (1 << 0);
const U32 kButtonDownKey	= (1 << 1);
const U32 kButtonRightKey	= (1 << 2);
const U32 kButtonLeftKey	= (1 << 3);
const U32 kButtonAKey		= (1 << 4);
const U32 kButtonBKey		= (1 << 5);
const U32 kButtonVolumeUp	= (1 << 6);
const U32 kButtonVolumeDown	= (1 << 7);

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
