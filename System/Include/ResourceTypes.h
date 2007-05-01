#ifndef LF_BRIO_RSRCMGRTYPES_H
#define LF_BRIO_RSRCMGRTYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
// File:
//		ResourceTypes.h
//
// Description:
//		Defines Resource Manager module's basic types. 
//==============================================================================

#include <CoreTypes.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
LF_BEGIN_BRIO_NAMESPACE()


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


//==============================================================================	   
// Resource Manager errors
//==============================================================================	   
#define RESOURCE_ERRORS			\
	(kResourceInvalidErr)		\
	(kResourceNotFoundErr)		\
	(kResourceNotLoadedErr)		\
	(kResourceNotOpenErr)		\
	(kResourceInvalidMPIIdErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupResource), RESOURCE_ERRORS)


//==============================================================================	   
// Resource Manager types
//==============================================================================	   
enum {
	kRsrcDeviceTypeUndefined = 0,
	kRsrcDeviceTypeCDROM,
	kRsrcDeviceTypeUSBMassStorage,
	kRsrcDeviceTypeSDCard
};

typedef U16	tDeviceType;

enum {
	kRsrcPackageTypeUndefined = 0,
	kRsrcPackageTypeGeneric,
	kRsrcPackageTypeSystem,
	kRsrcPackageTypeApp,
};

typedef U16	tRsrcPackageType;

enum {
	kRsrcSearchTypeUndefined = 0,
	kRsrcSearchTypeByURI,
	kRsrcSearchTypeByID,
	kRsrcSearchTypeByHandle,
	kRsrcSearchTypeByType,
};

typedef U16 tRsrcSearchType;

enum {				// bitmask options for OpenRsrc(), unspecified handled as read-only
	kOpenRsrcOptionRead		= 0x00000001,
	kOpenRsrcOptionWrite	= 0x00000002
};

enum {
	kSeekRsrcOptionSet		= 0,
	kSeekRsrcOptionCur		= 1,
	kSeekRsrcOptionEnd		= 2
};

enum {
	kLoadRsrcOptionUndefined = 0,
	kLoadRsrcOptionAsync
};

enum {
	kReadRsrcOptionUndefined = 0,
	kReadRsrcOptionAsync
};

typedef U32		tRsrcID;
typedef tHndl 	tDeviceHndl;
typedef tHndl 	tRsrcPackageHndl;
typedef U32		tRsrcType;
typedef tHndl 	tRsrcHndl;

typedef	U32		tResourceMsgDat;

enum eSearchScope {
	kUndefinedSearchScope = 0,
	kOpenPackages,
	kOpenPackagesAndDevices
};

const tDeviceHndl		kInvalidDeviceHndl = static_cast<tDeviceHndl>(0);
const tRsrcPackageHndl	kInvalidRsrcPackageHndl = static_cast<tRsrcPackageHndl>(0);
const tRsrcHndl			kInvalidRsrcHndl = static_cast<tRsrcHndl>(0);
const tRsrcType			kInvalidRsrcType = static_cast<tRsrcType>(0);


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_RSRCMGRTYPES_H

// EOF
