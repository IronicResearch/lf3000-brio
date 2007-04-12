#ifndef LF_BRIO_MODULEPRIV_H_
#define LF_BRIO_MODULEPRIV_H_
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		ModulePriv.h
//
// Description:
//		Private definitions for the module manager. 
//
//==============================================================================
#include <SystemTypes.h>
#include <EmulationConfig.h>	//FIXME/tp: Remove

LF_BEGIN_BRIO_NAMESPACE()


#ifndef EMULATION

	inline CPath GetModuleLibraryLocation()
	{
		return "/System/Modules";
	}
	
#else // !EMULATION

	#include <EmulationConfig.h>

	inline CPath GetModuleLibraryLocation()
	{
		return EmulationConfig::Instance().GetModuleSearchPath();
	}
	
#endif // !EMULATION

LF_END_BRIO_NAMESPACE()	
#endif //LF_BRIO_MODULEPRIV_H_
