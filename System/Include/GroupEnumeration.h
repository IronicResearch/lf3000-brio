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

//----------------------------------------------------------------------------
enum eGroupEnum { 
	// Brio v1.0	 
	kGroupAudio = kFirstNumSpaceGroup,
	kGroupCommon,
	kGroupDisplay,
	kGroupEvent,
	kGroupKernel,
	kGroupHardware,
	kGroupResource,
	kGroupUnitTests,
	
	// Brio v1.1
		 
	// WARNING: After GM, values can only be APPENDED to this enumeration!!!
	// WARNING: Inserting values will break backwards compatability!!!
};

#endif // LF_BRIO_GROUPENUMERATION_H

// EOF
