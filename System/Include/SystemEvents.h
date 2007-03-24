#ifndef LF_BRIO_SYSTEMEVENTS_H
#define LF_BRIO_SYSTEMEVENTS_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
// File:
//		SystemEvents.h
//
// Description:
//		Enumerate all of the system-defined events.
//		Uses the boost preprocessor library (structured preprocessor 
//		programming) to take a preprocessor sequence of the form:
//		#define SEQ (a)(b)(c)
//		and create individual defintions of the form:
//		const tEventType a = <enumerated value>; 
//		const tEventType b = <enumerated value>; 
//		const tEventType c = <enumerated value>; 
//==============================================================================

#include <boost/preprocessor/seq/for_each_i.hpp>
#include <GroupEnumeration.h>


//==============================================================================	   
// Preprocessor macro to generate type values
//==============================================================================
#define GEN_TYPE_VALUE(r, initval, count, name)		\
	const tEventType name = initval + count;
	
#define FirstEvent(group) MakeEventType(kSystemNumSpaceDomain, group, kFirstNumSpaceTag)
#define AllEvents(group)  MakeEventType(kSystemNumSpaceDomain, group, kWildcardNumSpaceTag)


//==============================================================================	   
// Audio events
//==============================================================================
#define AUDIO_EVENTS					\
	(kAudioCompletedEvent)				\
	(kAudioCuePointEvent)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupAudio), AUDIO_EVENTS)

const tEventType kAllAudioEvents = AllEvents(kGroupAudio);


//==============================================================================	   
// Common events
//==============================================================================
#define COMMON_EVENTS					\
	(kCommonEvent1)						\
	(kCommonEvent2)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupCommon), COMMON_EVENTS)

const tEventType kAllCommonEvents = AllEvents(kGroupCommon);


//==============================================================================	   
// Display events
//==============================================================================
#define DISPLAY_EVENTS					\
	(kDisplayEvent1)					\
	(kDisplayEvent2)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupDisplay), DISPLAY_EVENTS)

const tEventType kAllDisplayEvents = AllEvents(kGroupDisplay);


//==============================================================================	   
// Resource Manager events
//==============================================================================	   
#define RESOURCE_EVENTS					\
	(kResourceDeviceOpenedEvent)		\
	(kResourceAllDevicesOpenedEvent)	\
	(kResourcePackageOpenedEvent)		\
	(kResourcePackageLoadedEvent)		\
	(kResourcePackageUnloadedEvent)		\
	(kResourceOpenedEvent)				\
	(kResourceReadDoneEvent)			\
	(kResourceWriteDoneEvent)			\
	(kResourceLoadedEvent)				\
	(kResourceUnloadedEvent)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupResource), RESOURCE_EVENTS)

const tEventType kAllResourceEvents = AllEvents(kGroupResource);


#ifdef UNIT_TESTING
//==============================================================================	   
// Unit Test events
//==============================================================================	   
#define UNIT_TEST_EVENTS				\
	(kUnitTestEvent1)					\
	(kUnitTestEvent2)					\
	(kUnitTestEvent3)					\
	(kUnitTestEvent4)					\
	(kUnitTestEvent5)					\
	(kUnitTestEvent6)					\
	(kUnitTestEvent7)					\
	(kUnitTestEvent8)					\
	(kUnitTestEvent9)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupUnitTests), UNIT_TEST_EVENTS)

const tEventType kAllUnitTestEvents = AllEvents(kGroupUnitTests);
#endif //UNIT_TESTING

//==============================================================================	   
#undef GENERATE_TYPE_VALUE
#undef FirstEvent
#undef AllEvents

#endif // LF_BRIO_SYSTEMEVENTS_H

// EOF
