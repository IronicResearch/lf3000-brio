#ifndef LF_BRIO_CORETYPES_H
#define LF_BRIO_CORETYPES_H
//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		CoreTypes.h
//
// Description:
//		Defines the core types of the Brio system. 
//
//============================================================================
#include <boost/static_assert.hpp>

#ifdef LF_USE_CPP_NAMESPACES
	#define LF_BEGIN_BRIO_NAMESPACE()	namespace LeapFrog { namespace Brio {
	#define LF_END_BRIO_NAMESPACE()		} }
	#define LF_USING_BRIO_NAMESPACE()	using namespace LeapFrog::Brio;
#else
	#define LF_BEGIN_BRIO_NAMESPACE() 
	#define LF_END_BRIO_NAMESPACE()
	#define LF_USING_BRIO_NAMESPACE()
#endif

LF_BEGIN_BRIO_NAMESPACE()

#undef NULL
#define NULL	0
#define kNull 	0

#define BRIO_STATIC_ASSERT	BOOST_STATIC_ASSERT

//---------------------------------------------------------------------
// Basic storage types
//---------------------------------------------------------------------
typedef unsigned long long	U64;
typedef signed long long	S64;
typedef unsigned long		U32;
typedef signed long			S32;
typedef unsigned short		U16;
typedef signed short		S16;
typedef unsigned char		U8;
typedef signed char			S8;

typedef void*				tPtr;			// generic pointer 
typedef U8*					tDataPtr;		// pointer to data struct/buffer
typedef U32					tAddr;			// generic memory address

typedef U32					tOpaqueType;	// generic opaque simple type 

// logic types
typedef unsigned char		Boolean;

//---------------------------------------------------------------------
// Basic values
//---------------------------------------------------------------------
#define kU64Max	0xFFFFFFFFFFFFFFFFULL	// Max unsigned 64-bit value
#define kU64Min	0						// Min unsigned 64-bit value

#define kU32Max	0xFFFFFFFF    			// Max unsigned 32-bit value
#define kU32Min	0	    		   		// Min unsigned 32-bit value
#define kS32Max	2147483647     			// Max signed 32-bit value
#define kS32Min	(-2147483647)  			// Min signed 32-bit value

#define kU16Max	0xFFFF   		     	// Max unsigned 16-bit value
#define kU16Min	0						// Min unsigned 16-bit value
#define kS16Max	32767         			// Max signed 16-bit value
#define kS16Min	(-32767)      			// Min signed 16-bit value

#define kU8Max	0xFF           			// Max unsigned 8-bit value
#define kU8Min	0	          			// Min unsigned 8-bit value
#define kS8Max	127           			// Max signed 8-bit value
#define kS8Min	(-127)        			// Min signed 8-bit value

#define kBit0	0x01
#define kBit1	0x02
#define kBit2	0x04
#define kBit3	0x08
#define kBit4	0x10
#define kBit5	0x20
#define kBit6	0x40
#define kBit7	0x80

//---------------------------------------------------------------------
// Basic type accessors and casts
//---------------------------------------------------------------------
#define GetLowU16(aLong)			((U16)(U32)(aLong))
#define GetHighU16(aLong)			((U16)((U32)(aLong) >> 16))
#define GetLowU8(aWord)				((U8)(aWord))
#define GetHighU8(aWord)			((U8)((U16)(aWord) >> 8))

#define ToU16(highByte, lowByte)	(((U16)(highByte) << 8)		\
									| (U16)(lowByte & 0xFF))
#define ToU32(highWord, lowWord)	(((U32)(highWord) << 16)	\
									| (U32)(lowWord & 0xFFFF))

// common casts
#define ToPtr(a) 		(reinterpret_cast<tPtr>(a))
#define ToDataPtr(a) 	(reinterpret_cast<tDataPtr>(a))
#define ToAddr(a) 		(reinterpret_cast<tAddr>(a))

//---------------------------------------------------------------------
// Basic macros
//---------------------------------------------------------------------
#define Min(a, b) 	((a) < (b) ? (a) : (b))
#define Max(a, b) 	((a) > (b) ? (a) : (b))
#define Abs(a)  	((a < 0) ? (-(a)) : (a))

#define TableCount(tableType, elementsType) 	\
			(sizeof(tableType) / sizeof(elementsType))
#define PtrTableCount(tableType)				\
			TableCount(tableType, void*)
#define FcnTableCount(tableType)				\
			PtrTableCount(tableType)
#define ArrayCount(array) 						\
			(sizeof(array) / sizeof(array[0]))


LF_END_BRIO_NAMESPACE()
#endif // LF_BRIO_CORETYPES_H

// eof
