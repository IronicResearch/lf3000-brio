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

//------------------------------------------------------------------------------
class CButtonMessage : public IEventMessage {
public:
	CButtonMessage( const tButtonData& data );
	virtual U16	GetSizeInBytes() const;
	tButtonData GetButtonState() const;
private:
	tButtonData	mData;
};


#endif // LF_BRIO_BUTTONTYPES_H

// eof
