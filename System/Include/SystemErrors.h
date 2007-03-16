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
	kSystemErrTypeGroupEventMgr,	 
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

#endif // LF_BRIO_SYSTEMERRORS_H

// EOF

