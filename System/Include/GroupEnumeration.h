#ifndef LF_BRIO_GROUPENUMERATION_H
#define LF_BRIO_GROUPENUMERATION_H
//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
// File:
//		GroupEnumeration.h
//
// Description:
//		Enumerate group spaces 
//============================================================================

#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()

//----------------------------------------------------------------------------
enum eGroupEnum { 
	// Brio v1.0	 
	// Make sure any changes here get reflected in LightningUtils.py's SetupTypeConversionMap()
	kGroupAudio = kFirstNumSpaceGroup,
	kGroupBoot,
	kGroupButton,
	kGroupCommon,
	kGroupDisplay,
	kGroupEvent,
	kGroupFont,
	kGroupKernel,
	kGroupHardware,
	kGroupModule,
//	kGroupResource,
	kGroupTimer,
	kGroupUnitTests,
	kGroupVideo,
	kGroupPower,
	kGroupPlatform,
	kGroupUSBDevice,
	kGroupMfgTest,
	kGroupTouch

	// Brio v1.1
		 
	// WARNING: After GM, values can only be APPENDED to this enumeration!!!
	// WARNING: Inserting values will break backwards compatability!!!
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_GROUPENUMERATION_H

// EOF
