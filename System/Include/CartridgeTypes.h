#ifndef LF_BRIO_CARTRIDGETYPES_H
#define LF_BRIO_CARTRIDGETYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		CartridgeTypes.h
//
// Description:
//		Defines types for the Cartridge. 
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
#define CARTRIDGE_EVENTS					\
	(kCartridgeStateChanged)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupCartridge), CARTRIDGE_EVENTS)

const tEventType kAllCartridgeEvents = AllEvents(kGroupCartridge);


//==============================================================================	   
// Button errors
//==============================================================================
#define CARTRIDGE_ERRORS				\
	(kCartridgeEmulationConfigErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupCartridge), CARTRIDGE_ERRORS)


//==============================================================================	   
// Cartridge types
//==============================================================================

enum eCartridgeState_ {
	CARTRIDGE_STATE_NONE,
	CARTRIDGE_STATE_INSERTED,
	CARTRIDGE_STATE_DRIVER_READY,	/* Internal state: driver is fully initialized */
	CARTRIDGE_STATE_READY,
	CARTRIDGE_STATE_REMOVED,
	CARTRIDGE_STATE_FS_CLEAN,		/* Internal state: File system is fully unmounted */
	CARTRIDGE_STATE_CLEAN,
	CARTRIDGE_STATE_REINSERT,
	CARTRIDGE_STATE_UNKOWN
}; 


enum eCartridgeEvent_ {
	CARTRIDGE_EVENT_NONE,
	CARTRIDGE_EVENT_INSERT,
	CARTRIDGE_EVENT_REMOVE,
}; 


//------------------------------------------------------------------------------
struct tCartridgeData {
	enum eCartridgeState_ cartridgeState;
};



//------------------------------------------------------------------------------
class CCartridgeMessage : public IEventMessage {
public:
	CCartridgeMessage( const tCartridgeData& data );
	virtual U16	GetSizeInBytes() const;
	tCartridgeData GetCartridgeState() const;
private:
	tCartridgeData	mData;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_CARTRIDGETYPES_H

// eof
