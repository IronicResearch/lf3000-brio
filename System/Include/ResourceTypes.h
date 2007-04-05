#ifndef LF_BRIO_RSRCMGRTYPES_H
#define LF_BRIO_RSRCMGRTYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
// File:
//		RsrcMgrTypes.h
//
// Description:
//		Defines RsrcMgr module's basic types. 
//==============================================================================

#include <SystemTypes.h>


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


#endif // LF_BRIO_RSRCMGRTYPES_H

// EOF
