#ifndef LF_BRIO_SYSTEMTYPES_H
#define LF_BRIO_SYSTEMTYPES_H
//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
// File:
//		SystemTypes.h
//
// Description:
//		Defines the types which are specific to the system. 
//============================================================================

#include <CoreTypes.h>

//============================================================================
// Basic Types
//============================================================================

typedef U32			tOptionFlags;	// std call options bitmask type

const tOptionFlags	kNoOptionFlags	= (tOptionFlags)(0);

typedef tOpaqueType	tHndl;			// generic handle

const tHndl			kUndefinedHndl	= (tHndl)(0);


//----------------------------------------------------------------------------
// Type:
//		Standard enumerated number spaces
//
// Description:
//		By convention, the numeric ranges of various enumerated types defined
//		by the System are segmented into "number spaces" at the highest level 
//		by a common 4-bit domain value.  The domain value separates System, 
//		product and application-defined values of the same type, providing a 
//		simple mechanism for developers in each of these spaces to independently 
//		define unique enumerated values without worrying about overlap.
//
//		Enumerated number spaces are typically subdivided into sub-spaces by
//		a "group" value, and into sub-sub-spaces by a "tag" value.
//----------------------------------------------------------------------------

enum {
	kUndefinedNumSpaceDomain	= 0,

	kSystemNumSpaceDomain		= 1,	// values defined by System code
	kProductNumSpaceDomain		= 2,	// values defined by product/product-family code
	kApplicationNumSpaceDomain	= 8		// values defined by application code

	// (all other values are reserved for System use)
};

const U32 kFirstNumSpaceGroup	= 1;
const U32 kFirstNumSpaceTag		= 1;



//----------------------------------------------------------------------------
// Type:
//		Standard U32 number space format
//
//	Description:
//		One of the most common divisions of a 32-bit number space is with the 
//		following format:
//
//		MSBit: [4 bit domain][8 bits reserved][10 bit group][10 bit tag] :LSBit
//----------------------------------------------------------------------------

typedef U32 tU32NumSpace;

#define kU32NumSpaceDomainBShift	(28)
#define kU32NumSpaceGroupBShift		(10)
#define kU32NumSpaceTagBShift		(0)

#define kU32NumSpaceDomainBMask		(0xF)
#define kU32NumSpaceGroupBMask		(0x3FF)
#define kU32NumSpaceTagBMask		(0x3FF)
#define kU32LastNumSpaceTag			kU32NumSpaceTagBMask
#define kWildcardNumSpaceTag		kU32NumSpaceTagBMask

#define MakeU32NumSpace(domain, group, tag) ((tU32NumSpace)(			\
				((domain & kU32NumSpaceDomainBMask) << kU32NumSpaceDomainBShift) |	\
				((group & kU32NumSpaceGroupBMask) << kU32NumSpaceGroupBShift) |		\
				((tag & kU32NumSpaceTagBMask) << kU32NumSpaceTagBShift)				\
				))

//----------------------------------------------------------------------------
// Type:
//		Standard U16 number space format
//
//	Description:
//		One of the most common divisions of a 16-bit number space is with the 
//		following format:
//
//		MSBit: [4 bit domain][2 bits reserved][5 bit group][5 bit tag] :LSBit
//----------------------------------------------------------------------------

typedef U16 tU16NumSpace;

#define kU16NumSpaceDomainBShift	(12)
#define kU16NumSpaceGroupBShift	(5)
#define kU16NumSpaceTagBShift		(0)

#define kU16NumSpaceDomainBMask	(0xF)
#define kU16NumSpaceGroupBMask		(0x1F)
#define kU16NumSpaceTagBMask		(0x1F)

#define MakeU16NumSpace(domain, group, tag) ((tU16NumSpace)(			\
				((domain & kU16NumSpaceDomainBMask) << kU16NumSpaceDomainBShift) |	\
				((group & kU16NumSpaceGroupBMask) << kU16NumSpaceGroupBShift) |		\
				((tag & kU16NumSpaceTagBMask) << kU16NumSpaceTagBShift)				\
				))

//============================================================================
// Versioning
//============================================================================

//----------------------------------------------------------------------------
// Type:
//		tSimpleVersion
//
// Description:
//		Version as simple counting index: stored as U16 index
//----------------------------------------------------------------------------

typedef U16	tSimpleVersion;

#define MakeSimpleVersion(version) 	((tSimpleVersion)version)

#define kUndefinedSimpleVersion		MakeSimpleVersion(0)
#define kMaxSimpleVersion				MakeSimpleVersion(kU16Max)

//----------------------------------------------------------------------------
// Type:
//		tVersion
//
// Description:
//		Standard major.minor version:  
//			stored as [MSB][U8 major][U8 minor][LSB]
//----------------------------------------------------------------------------

typedef U16	tVersion;

#define MakeVersion(major, minor) 		((tVersion)ToU16(major, minor))

#define kUndefinedVersion				MakeVersion(0, 0)
#define kVersion1						MakeVersion(1, 0)
#define kVersion2						MakeVersion(2, 0)
#define kVersion3						MakeVersion(3, 0)
#define kMaxVersion					MakeVersion(kU8Max, kU8Max)

#define GetMajorVersion(version)  		GetHighU8(version)
#define GetMinorVersion(version) 		GetLowU8(version)

#define NextMajorVersion(version)		((tVersion)((version & 0xFF00) + MakeVersion(1, 0)))
#define NextMinorVersion(version)		((tVersion)(version + MakeVersion(0, 1)))

//----------------------------------------------------------------------------
// Type:
//		tBuildVersion
//
// Description:
//		Standard major.minor.build version:  
//			stored as [MSB][U8 major][U8 minor][U16 build][LSB]
//----------------------------------------------------------------------------

typedef U32	tBuildVersion;

#define MakeBuildVersion(major, minor, build) 	((tBuildVersion)ToU32(ToU16(major, minor), build))

#define kUndefinedBuildVersion			MakeBuildVersion(0, 0, 0)
#define kMaxBuildVersion				MakeBuildVersion(kU8Max, kU8Max, kU16Max)

#define GetBuildMajorVersion(buildVersion) GetHighU8(GetHighU16(buildVersion))
#define GetBuildMinorVersion(buildVersion) GetLowU8(GetHighU16(buildVersion))
#define GetBuildBuildVersion(buildVersion) GetLowU16(buildVersion)

//----------------------------------------------------------------------------
// Type:
//		tDataFmtVersion
//
// Description:
//		Defines a data structure format version:
//			stored as [MSB][6-bits simple version][10-bits struct size][LSB]
//----------------------------------------------------------------------------

typedef U16	tDataFmtVersion;

enum {
	kDataFmtSimpleVersionBShift	= 10,
	kDataFmtSimpleVersionBMask 	= 0x003F,
	kDataFmtSizeBShift			= 0,
	kDataFmtSizeBMask 			= 0x03FF
};

#define MakeDataFmtVersion(fmtSimpleVersion, fmtSize) 	((tDataFmtVersion) \
					((U16)((fmtSimpleVersion & kDataFmtSimpleVersionBMask) << kDataFmtSimpleVersionBShift) | 	\
					((U16)((fmtSize & kDataFmtSizeBMask) << kDataFmtSizeBShift))

#define kUndefinedDataFmtVersion		MakeDataFmtVersion(0, 0)
#define kMaxDataFmtVersion				MakeDataFmtVersion(kU16Max, kU16Max)

#define GetDataFmtVersion(dataFmtVersion) ((U16)((dataFmtVersion >> kDataFmtSimpleVersionBShift) & \
													kDataFmtSimpleVersionBMask))
#define GetDataFmtSize(dataFmtVersion)    ((U16)((dataFmtVersion >> kDataFmtSizeBShift) & \
													kDataFmtSizeBMask))

//----------------------------------------------------------------------------
// Type:
//		tObjTypeID
//
//	Description:
//		Storage for an object type identifier, comprised of three elements:
//
//			MSBit: [4 bit domain][8 bits reserved][10 bit group][10 bit tag] :LSBit
//
//		- the domain, which segments system, product and application object types 
//		- the group, which specifies the group to which the object type belongs
//		- the tag - a key number that identifies the specific object sub-type 
//			within its group
//
//		Application code may use any product or System-defined types, but 
//		can only define new application types -- not product or System
//		types.  Similarly, product code may use any System-defined types,
//		but can only define new product types -- not application or System types.
//----------------------------------------------------------------------------
/*
typedef tU32NumSpace 	tObjTypeID;

#define MakeObjType(domain, group, tag) ((tObjTypeID)MakeU32NumSpace(domain, group, tag))

#define kUndefinedObjType		((tObjTypeID)0)
*/

//----------------------------------------------------------------------------
// Type:
//		tRsrcType
//
//	Description:
//		Storage for a resource type, comprised of three elements:
//
//			MSBit: [4 bit domain][8 bits reserved][10 bit group][10 bit tag] :LSBit
//
//		- the domain, which segments system, product and application resource types 
//		- the group, which specifies the group to which the resource type belongs
//		- the tag - a key number that identifies the specific resource sub-type 
//			within its group
//
//		Application code may use any product or System-defined types, but 
//		can only define new application types -- not product or System
//		types.  Similarly, product code may use any System-defined types,
//		but can only define new product types -- not application or System types.
//----------------------------------------------------------------------------

typedef tU32NumSpace 	tRsrcType;

#define MakeRsrcType(domain, group, tag) ((tRsrcType)MakeU32NumSpace(domain, group, tag))

#define kUndefinedRsrcType		((tRsrcType)0)

//----------------------------------------------------------------------------
// Type:
//		tEventType
//
//	Description:
//		Storage for an event type/code, comprised of three elements:
//
//			MSBit: [4 bit domain][8 bits reserved][10 bit group][10 bit tag] :LSBit
//
//		- the domain, which segments system, product and application event types 
//		- the group, which specifies the group to which the event type belongs
//		- the tag - a key number that identifies the specific event sub-type 
//			within its group
//
//		Application code may use any product or System-defined types, but 
//		can only define new application types -- not product or System
//		types.  Similarly, product code may use any System-defined types,
//		but can only define new product types -- not application or System types.
//----------------------------------------------------------------------------

typedef tU32NumSpace 	tEventType;

#define MakeEventType(domain, group, tag) ((tEventType)MakeU32NumSpace(domain, group, tag))

#define kUndefinedEventType	((tEventType)0)

//----------------------------------------------------------------------------
// Type:
//		tErrType
//
//	Description:
//		Storage for an error type/code, comprised of three elements:
//
//			MSBit: [4 bit domain][8 bits reserved][10 bit group][10 bit tag] :LSBit
//
//		- the domain, which segments system, product and application error codes 
//		- the group, which specifies the group to which the error code belongs
//		- the tag - a key number that identifies the specific error sub-code 
//			within its group
//
//		Application code may use any product or System-defined types, but 
//		can only define new application types -- not product or System
//		types.  Similarly, product code may use any System-defined types,
//		but can only define new product types -- not application or System types.
//----------------------------------------------------------------------------

typedef tU32NumSpace	tErrType;

#define MakeErrType(domain, group, tag) ((tErrType)MakeU32NumSpace(domain, group, tag))

#define kNoErr			((tErrType)0)


#endif // LF_BRIO_SYSTEMTYPES_H

// EOF
