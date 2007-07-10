#ifndef LF_BRIO_EVENTTYPES_H
#define LF_BRIO_EVENTTYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		EventMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the Event module. 
//
//==============================================================================

#include <SystemTypes.h>
#include <SystemErrors.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================	   
// Event Manager errors 
//==============================================================================
#define EVENT_ERRORS					\
	(kEventNotFoundErr)					\
	(kEventTypeNotFoundErr)				\
	(kEventListenerNotRegisteredErr)	\
	(kEventListenerCycleErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupEvent), EVENT_ERRORS)


//==============================================================================	   
// Event Manager types 
//==============================================================================
typedef U32		tListenerId;
typedef U32		tEventRegistrationFlags;
typedef U8		tEventPriority;

const tListenerId kNoListener = static_cast<tListenerId>(0);


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_EVENTTYPES_H

// eof
