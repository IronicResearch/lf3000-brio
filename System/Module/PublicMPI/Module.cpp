//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		Module.cpp
//
// Description:
//		See the IncludePriv/Module.h file
//
//==============================================================================

#include <dlfcn.h>

#include <SystemTypes.h>
#include <Module.h>
#include <ModulePriv.h>
#include <CoreModule.h>
#include <SystemErrors.h>
#include <DebugMPI.h>
LF_BEGIN_BRIO_NAMESPACE()

const CString	kNullString;
const CURI		kNullURI;


//============================================================================
// Local Utility Functions
//============================================================================
namespace
{
	
	//------------------------------------------------------------------------
	void* gg_pModuleHandle = NULL;
	
	//------------------------------------------------------------------------
	tErrType LoadModuleManagerLib()
	{
//		CDebugMPI	dbg(kGroupModule);
		if( gg_pModuleHandle != NULL )
			return kNoErr;
		CPath path = GetModuleLibraryLocation();
		path = path + CPath("/libModule.so");
		const char* str = path.c_str();
//		dbg.DebugOut(kDbgLvlVerbose, "GetModuleLibraryLocation: %s\n", str);
		gg_pModuleHandle = dlopen(str, RTLD_LAZY);
//		dbg.Assert(gg_pModuleHandle != NULL, "LoadModuleManagerLib() failed:%s\n", dlerror());
	    dlerror();
	    return kNoErr;
	}
	
	//------------------------------------------------------------------------
	void UnloadModuldManagerLib()
	{
		dlclose(gg_pModuleHandle);
	}
	
	//------------------------------------------------------------------------
	inline void AbortOnError(const char* msg)
	{
		const char *dlsym_error = dlerror();
//		CDebugMPI	dbg(kGroupModule);
//		dbg.Assert(dlsym_error == NULL, msg);
	}
}

 
//============================================================================
// Module Interface
//============================================================================
namespace Module
{
	extern "C"
	{
		typedef tErrType (*tfnFindMod)();
		typedef tErrType (*tfnConnect)(void**, const char*, tVersion);
		typedef tErrType (*tfnDisconnect)(const ICoreModule*);
	}
	
	//------------------------------------------------------------------------
	tErrType FindModules()
	{
		tErrType status = LoadModuleManagerLib();
		if( status == kNoErr )
		{
			tfnFindMod funptr = reinterpret_cast<tfnFindMod>
										(dlsym(gg_pModuleHandle, "FindModules"));
			AbortOnError("FindModules lookup failure");
		  	status = (*funptr)();
		}
		return status;
	}
	
	//------------------------------------------------------------------------
	tErrType Connect(ICoreModule*& ptr, const CString& name, tVersion version)
	{
		static tErrType init = FindModules();	// FIXME/tp: temp startup, replace with real call from boot module

		ptr = NULL;
		tErrType status = LoadModuleManagerLib();
		if( status == kNoErr )
		{
			void* pModule = NULL;
			tfnConnect funptr = reinterpret_cast<tfnConnect>
										(dlsym(gg_pModuleHandle, "Connect"));
			AbortOnError("Connect lookup failure");
		  	status = (*funptr)(&pModule, name.c_str(), version);
		  	ptr = reinterpret_cast<ICoreModule*>(pModule);
		}
		return status;
	}
	
	//------------------------------------------------------------------------
	tErrType Disconnect(const ICoreModule* ptr)
	{
		tErrType status = LoadModuleManagerLib();
		if( status == kNoErr )
		{
			void* pModule;
			tfnDisconnect funptr = reinterpret_cast<tfnDisconnect>
										(dlsym(gg_pModuleHandle, "Disconnect"));
			AbortOnError("Disconnect lookup failure");
		  	status = (*funptr)(ptr);
		}
		return status;
	}
}


LF_END_BRIO_NAMESPACE()
// eof
