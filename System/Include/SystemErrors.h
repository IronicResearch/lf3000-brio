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
#include <SystemTypes.h>
#include <GroupEnumeration.h>
LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================	   
// System resource types (FIXME/tp: Move to separate file?
//==============================================================================
const tRsrcType kCommonRsrcText = MakeRsrcType(kUndefinedNumSpaceDomain, kGroupCommon, 1);
const tRsrcType kCommonRsrcBin  = MakeRsrcType(kUndefinedNumSpaceDomain, kGroupCommon, 2);
const tRsrcType kCommonRsrcJson = MakeRsrcType(kUndefinedNumSpaceDomain, kGroupCommon, 3);
const tRsrcType kCommonRsrcXml  = MakeRsrcType(kUndefinedNumSpaceDomain, kGroupCommon, 4);


//==============================================================================	   
// Preprocessor macro to generate type values
//==============================================================================
#define GEN_ERR_VALUE(r, initval, count, name)		\
	const tErrType name = initval + count;

inline tErrType FirstErr( eGroupEnum group )
{
	return MakeErrType(kSystemNumSpaceDomain, group, kFirstNumSpaceTag);
}


//==============================================================================	   
// Common errors
//==============================================================================
#define COMMON_ERRORS			\
	(kUnspecifiedErr)			\
	(kInvalidParamErr)			\
	(kNoImplErr)				\
	(kAllocMPIErr)				\
	(kPermissionsErr)			\
	(kMPINotConnectedErr)		\
	(kMemoryAllocationErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupCommon), COMMON_ERRORS)


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_SYSTEMERRORS_H

// EOF

