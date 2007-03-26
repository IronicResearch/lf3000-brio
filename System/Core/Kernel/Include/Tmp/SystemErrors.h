#ifndef SYSTEMERRORS_H
#define SYSTEMERRORS_H
//==============================================================================
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
// File:
//		SystemErrors.h
//
// Description:
//		System-defined errors.
//==============================================================================

 #include <SystemTypes.h>

//==============================================================================	   
// System error groups
//==============================================================================

enum {   
	kSystemErrTypeGroupGeneral = kFirstNumSpaceGroup,	   // FIXME/dg: general group (temporary?) for bringup
	kSystemErrTypeGroupHardware,
	kSystemErrTypeGroupKernel,
	kSystemErrTypeGroupAudioMgr,
	kSystemErrTypeGroupRsrcMgr,	      	
	kSystemErrTypeGroupDisplayMgr,	 
};

#define MakeFirstSystemGroupErrType(group)  MakeErrType(kSystemNumSpaceDomain, group, kFirstNumSpaceTag)

//==============================================================================	   
// System errors
//==============================================================================

enum {
	//------------------------------------------------------------------------------
	// General errors
	//------------------------------------------------------------------------------
	kUnspecifiedErr = MakeFirstSystemGroupErrType(kSystemErrTypeGroupGeneral),   // FIXME/dg: temp(?) for bringup
	kInvalidParamErr,
	kNoImplErr,
	kAllocMPIErr,

	//------------------------------------------------------------------------------
	// Hardware errors
	//------------------------------------------------------------------------------
	// <firstGroupErr> = MakeFirstSystemGroupErrType(kSystemErrTypeGroupHardware),  

	//------------------------------------------------------------------------------
	// Kernel errors
	//------------------------------------------------------------------------------
	kMemoryAllocErr = MakeFirstSystemGroupErrType(kSystemErrTypeGroupKernel),  

	//------------------------------------------------------------------------------
	// AudioMgr errors
	//------------------------------------------------------------------------------
	// <firstGroupErr> = MakeFirstSystemGroupErrType(kSystemErrTypeGroupAudioMgr),  
};

#endif // SYSTEMERRORS_H

// EOF

