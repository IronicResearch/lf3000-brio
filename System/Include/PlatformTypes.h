#ifndef LF_BRIO_PLATFORMTYPES_H
#define LF_BRIO_PLATFORMTYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		PlatformTypes.h
//
// Description:
//		Defines types for the Platform Manager module. 
//
//==============================================================================

#include <EventMessage.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================	   
// Platform events
//==============================================================================
#define PLATFORM_EVENTS				\
	(kPlatformStateChanged)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupPlatform), PLATFORM_EVENTS)

const tEventType kAllPlatformEvents = AllEvents(kGroupPlatform);


//==============================================================================	   
// Platform errors
//==============================================================================
#define PLATFORM_ERRORS				\
	(kPlatformEmulationConfigErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupPlatform), PLATFORM_ERRORS)


//==============================================================================	   
// Platform types
//==============================================================================
//------------------------------------------------------------------------------

struct tPlatformData {
	U32 platformState;
};

const U32 kPlatformNull	         = 0;	/* invalid platform state */
const U32 kPlatformUsbConnect    = 1;	/* USB connected		  */
const U32 kPlatformUsbDisconnect = 2;	/* USB disconnected		  */


//------------------------------------------------------------------------------
class CPlatformMessage : public IEventMessage {
public:
	CPlatformMessage(const tPlatformData& data);
	virtual U16 GetSizeInBytes() const;
	tPlatformData  GetPlatformState() const;
	
private:
	tPlatformData mData;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_PLATFORMTYPES_H

// eof
