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
#include <SystemTypes.h>
#include <GroupEnumeration.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================	   
// Preprocessor macro to generate type values
//==============================================================================
#define GEN_TYPE_VALUE(r, initval, count, name)		\
	const tEventType name = initval + count;
	
inline tEventType FirstEvent( eGroupEnum group )
{
	return MakeEventType(kSystemNumSpaceDomain, group, kFirstNumSpaceTag);
}

inline tEventType AllEvents( eGroupEnum group )
{
	return MakeEventType(kSystemNumSpaceDomain, group, kWildcardNumSpaceTag);
}


//==============================================================================	   
// Common events
//==============================================================================
#define COMMON_EVENTS					\
	(kCommonEvent1)						\
	(kCommonEvent2)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupCommon), COMMON_EVENTS)

const tEventType kAllCommonEvents = AllEvents(kGroupCommon);



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


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_SYSTEMEVENTS_H

// EOF
