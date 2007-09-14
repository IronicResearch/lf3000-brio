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

#include <AudioTypes.h>
#include <ButtonTypes.h>
#include <DisplayTypes.h>
#include <EventTypes.h>
#include <KernelTypes.h>
#include <CoreModule.h>
//#include <ResourceTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//------------------------------------------------------------------------------
#define GEN_VALUE_TO_STRING(r, d, item)		\
		{ item, BOOST_PP_STRINGIZE( item ) },

//------------------------------------------------------------------------------
struct ValueToString
{
	tErrType	value;
	char* 		string;
};

//------------------------------------------------------------------------------
const char* const ErrToStr( tErrType error )
{
	// TODO: Keeping the following BRIO_STATIC_ASSERT in sync with the
	//		BOOST_PP_SEQ_FOR_EACH lines will validate that the g_errorLookup 
	//		list is ordered.
	BRIO_STATIC_ASSERT(kGroupAudio		< kGroupCommon
					&&	kGroupCommon	< kGroupDisplay
					&&	kGroupDisplay	< kGroupEvent
					&&	kGroupEvent		< kGroupKernel
					&&	kGroupKernel	< kGroupModule
//					&&	kGroupModule 	< kGroupResource
					);
	static ValueToString g_errorLookup[] = {
		BOOST_PP_SEQ_FOR_EACH(GEN_VALUE_TO_STRING, , AUDIO_ERRORS)
		BOOST_PP_SEQ_FOR_EACH(GEN_VALUE_TO_STRING, , BUTTON_ERRORS)
		BOOST_PP_SEQ_FOR_EACH(GEN_VALUE_TO_STRING, , COMMON_ERRORS)
		BOOST_PP_SEQ_FOR_EACH(GEN_VALUE_TO_STRING, , DISPLAY_ERRORS)
		BOOST_PP_SEQ_FOR_EACH(GEN_VALUE_TO_STRING, , EVENT_ERRORS)
		BOOST_PP_SEQ_FOR_EACH(GEN_VALUE_TO_STRING, , KERNEL_ERRORS)
		BOOST_PP_SEQ_FOR_EACH(GEN_VALUE_TO_STRING, , MODULE_ERRORS)
//		BOOST_PP_SEQ_FOR_EACH(GEN_VALUE_TO_STRING, , RESOURCE_ERRORS)
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


LF_END_BRIO_NAMESPACE()
// EOF
