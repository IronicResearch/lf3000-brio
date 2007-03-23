#ifndef LF_BRIO_SYSTEMERRORS_H
#define LF_BRIO_SYSTEMERRORS_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
// File:
//		SystemErrors.h
//
// Description:
//		Enumerate all of the system-defined errors.
//		Uses the boost preprocessor library (structured preprocessor 
//		programming) to take a preprocessor sequence of the form:
//		#define SEQ (a)(b)(c)
//		and create individual defintions of the form:
//		const tErrType a = <enumerated value>; 
//		const tErrType b = <enumerated value>; 
//		const tErrType c = <enumerated value>; 
//==============================================================================

#include <boost/preprocessor/seq/for_each_i.hpp>
#include <GroupEnumeration.h>


//==============================================================================	   
// Preprocessor macro to generate type values
//==============================================================================
#define GEN_ERR_VALUE(r, initval, count, name)		\
	const tErrType name = initval + count;

#define FirstErr(group)  	\
	MakeErrType(kSystemNumSpaceDomain, group, kFirstNumSpaceTag)


//==============================================================================	   
// Audio errors
//==============================================================================
#define AUDIO_ERRORS			\
	(kAudioCreateTaskErr)		\
	(kAudioCreateEventErr)		\
	(kAudioCreateQueueErr)		\
	(kAudioNullContextErr)		\
	(kAudioNoDataAvailErr)		\
	(kAudioNoMoreDataErr)		\
	(kAudioInvalid)				\
	(kAudioMidiErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupAudio), AUDIO_ERRORS)


//==============================================================================	   
// Common errors
//==============================================================================
#define COMMON_ERRORS			\
	(kUnspecifiedErr)			\
	(kInvalidParamErr)			\
	(kNoImplErr)				\
	(kAllocMPIErr)				\
	(kMpiNotConnectedErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupCommon), COMMON_ERRORS)


//==============================================================================	   
// Event Manager errors (FIXME/tp: Move to common?)
//==============================================================================
#define EVENT_ERRORS					\
	(kEventNotFoundErr)					\
	(kEventTypeNotFoundErr)				\
	(kEventListenerNotRegisteredErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupEvent), EVENT_ERRORS)


//==============================================================================	   
// Kernel errors
//==============================================================================
#define KERNEL_ERRORS			\
	(kMemoryAllocErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupKernel), KERNEL_ERRORS)


//==============================================================================	   
// Module errors
//==============================================================================
#define MODULE_ERRORS			\
	(kModuleNotFound)			\
	(kModuleOpenFail)			\
	(kModuleLoadFail)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupModule), MODULE_ERRORS)


//==============================================================================	   
// Resource Manager errors
//==============================================================================	   
#define RESOURCE_ERRORS			\
	(kResourceInvalidErr)		\
	(kResourceNotFoundErr)		\
	(kResourceNotLoadedErr)		\
	(kResourceNotOpenErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupResource), RESOURCE_ERRORS)


//==============================================================================	   
#undef GEN_ERR_VALUE
#undef FirstErr

#endif // LF_BRIO_SYSTEMERRORS_H

// EOF

