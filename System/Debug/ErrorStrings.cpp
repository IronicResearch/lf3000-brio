//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		ErrorStrings.cpp
//
// Description:
//		Debug functions to turn error codes into strings.
//		Uses the boost preprocessor library (structured preprocessor 
//		programming) to take a preprocessor sequence of the form:
//		#define SEQ (a)(b)(c)
//		and create initializations of "ValueToString" struct elements
//		of the form:
//		{ a, "a" }, { b, "b" }, { c, "c" }, 
//
//==============================================================================

#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <StringTypes.h>
#include <SystemErrors.h>


//------------------------------------------------------------------------------
#define GEN_VALUE_TO_STRING(r, d, item)		\
		{ item, BOOST_PP_STRINGIZE( item ) },

//------------------------------------------------------------------------------
struct ValueToString
{
	tErrType	value;
	CString		string;
};

//------------------------------------------------------------------------------
CString ErrorToString( tErrType error )
{
	// TODO: Validate that list is ordered
	// TODO: Debug binary search
	static ValueToString g_errorLookup[] = {
		BOOST_PP_SEQ_FOR_EACH(GEN_VALUE_TO_STRING, , AUDIO_ERRORS)
		BOOST_PP_SEQ_FOR_EACH(GEN_VALUE_TO_STRING, , COMMON_ERRORS)
		BOOST_PP_SEQ_FOR_EACH(GEN_VALUE_TO_STRING, , EVENT_ERRORS)
		BOOST_PP_SEQ_FOR_EACH(GEN_VALUE_TO_STRING, , KERNEL_ERRORS)
		BOOST_PP_SEQ_FOR_EACH(GEN_VALUE_TO_STRING, , RESOURCE_ERRORS)
	};
	static U32 size = ArrayCount(g_errorLookup);
	
	CString	ret;
	U32	high, low, mid;
	for( low = 0, high = size-1, mid = size/2; low <= high; )
	{
		tErrType e = g_errorLookup[mid].value;
		if( e == error )
			return g_errorLookup[mid].string;
		if( e > error )
			high = mid - 1;
		else
			low = mid + 1;
		mid = (low + high) >> 1;
	}
	return "Unknown error value";
}


// EOF
