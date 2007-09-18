#ifndef LF_BRIO_POWERTYPES_H
#define LF_BRIO_POWERTYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		PowerTypes.h
//
// Description:
//		Defines types for the Power Manager module. 
//
//==============================================================================

#include <EventMessage.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================	   
// Power events
//==============================================================================
#define POWER_EVENTS				\
	(kPowerStateChanged)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupPower), POWER_EVENTS)

const tEventType kAllPowerEvents = AllEvents(kGroupPower);


//==============================================================================	   
// Power errors
//==============================================================================
#define POWER_ERRORS				\
	(kPowerEmulationConfigErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupPower), POWER_ERRORS)


//==============================================================================	   
// Power types
//==============================================================================
//------------------------------------------------------------------------------

struct tPowerData {
	U32 powerState;
};

const U32 kPowerNull	   = 0;	/* invalid power state	*/
const U32 kPowerExternal   = 1;	/* on external power	*/
const U32 kPowerBattery    = 2;	/* battery powered		*/
const U32 kPowerLowBattery = 3;	/* low battery			*/
const U32 kPowerConserve   = 4;	/* conserving power		*/
const U32 kPowerShutdown   = 5;	/* shutdown requested	*/


//------------------------------------------------------------------------------
class CPowerMessage : public IEventMessage {
public:
	CPowerMessage(const tPowerData& data);
	virtual U16 GetSizeInBytes() const;
	
	// Get current power state which is the same as the last message passing
	// through the event manager
	tPowerData  GetPowerState() const;
	
	// Conserve power.  Passing a non-zero value places the system
	// in power conservation mode.  Call with zero to exit power conserve
	// mode.
	tPowerData	SetConservePower(bool) const;
	
	// Complete system shutdown.  This is the application's response to the kPowerShutdown
	// message.  It indicates shutdown can proceed immediately, overriding the shutdown
	// watchdog timer.
	U32			Shutdown() const;
	
	// get shutdown time in milliseconds
	U32			GetShutdownTime() const;
	
	// set shutdown time in milliseconds.  Returns actual millisecond shutdown time.
	U32			SetShutdownTime(U32) const;
	
	
private:
	tPowerData mData;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_POWERTYPES_H

// eof
