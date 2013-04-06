#ifndef LF_BRIO_KEYPADTYPES_H
#define LF_BRIO_KEYPADTYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		KeypadTypes.h
//
// Description:
//		Defines types for Keypad events and messages.
//
//==============================================================================

#include <EventMessage.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================	   
// Keypad events
//==============================================================================
#define KEYPAD_EVENTS					\
	(kKeypadStateChanged)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupKeypad), KEYPAD_EVENTS)

const tEventType kAllKeypadEvents = AllEvents(kGroupKeypad);


//==============================================================================	   
// Keypad errors
//==============================================================================
#define KEYPAD_ERRORS				\
	(kKeypadEmulationConfigErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupKeypad), KEYPAD_ERRORS)


//==============================================================================	   
// Keypad types
//==============================================================================
//------------------------------------------------------------------------------
struct tKeypadData {
	U32	keypadState;		///< 0=up, 1=down, 2=repeat
	U32	keypadTransition;	///< 0=none, 1=transition
	struct timeVal {		///< timestamp per input event system
		S32	seconds;
		S32	microSeconds;
	} time;
};

//------------------------------------------------------------------------------
class CKeypadMessage : public IEventMessage {
public:
	CKeypadMessage( const tKeypadData& data );
	virtual U16	GetSizeInBytes() const;
	tKeypadData GetKeypadState() const;
private:
	tKeypadData	mData;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_KEYPADTYPES_H

// eof
