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

enum tPowerState {
	kPowerNull	   	 = 0,	/* invalid power state	*/
	kPowerExternal   = 1,	/* on external power	*/
	kPowerBattery    = 2,	/* battery powered		*/
	kPowerLowBattery = 3,	/* low battery			*/
	kPowerShutdown   = 4,	/* shutdown requested	*/
	kPowerCritical   = 5,	/* critical battery     */
	kPowerRechargeable = 6,	/* battery rechargeable */
	kPowerCharging   = 7	/* battery charging     */
};

struct tPowerData {
	enum tPowerState powerState;
};

//------------------------------------------------------------------------------
class CPowerMessage : public IEventMessage {
public:
	CPowerMessage(const tPowerData& data);
	virtual U16 GetSizeInBytes() const;

	// Get current power state which is the same as the last message passing
	// through the event manager
	enum tPowerState GetPowerState() const;

private:
	tPowerData mData;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_POWERTYPES_H

// eof
