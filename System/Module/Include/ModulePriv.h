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

#ifndef EMULATION

	inline const char* GetModuleLibraryLocation()
	{
		return "/System/Modules";
	}
	
#else // !EMULATION

	#include <EmulationConfig.h>

	inline const char* GetModuleLibraryLocation()
	{
		return LeapFrog::Brio::EmulationConfig::Instance().GetModuleSearchPath().c_str();
	}
	
#endif // !EMULATION

#endif //LF_BRIO_MODULEPRIV_H_
