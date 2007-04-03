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
