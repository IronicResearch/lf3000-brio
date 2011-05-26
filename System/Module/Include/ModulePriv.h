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
#include <EmulationConfig.h>
#include <sys/stat.h>

LF_BEGIN_BRIO_NAMESPACE()


#ifndef EMULATION

	inline CPath GetLocalLibrary(CPath libname)
	{
		CPath libpath = getenv("LD_LIBRARY_PATH");
		CPath modpath = libpath.substr(0, libpath.find(":")) + "/" + libname;
		struct stat s;
		if (0 == stat(modpath.c_str(), &s))
			return modpath;
		return "";
	}

	inline CPath GetModuleLibraryLocation()
	{
		CPath libpath = getenv("LD_LIBRARY_PATH");
		CPath modpath = libpath.substr(0, libpath.find(":")) + "/Module/";
		struct stat s;
		if (0 == stat(modpath.c_str(), &s) && S_ISDIR(s.st_mode))
			return modpath;
		return "/LF/Base/Brio/Module/";
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
