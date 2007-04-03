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

#include <Module.h>
#include <CoreModule.h>
#include <SystemErrors.h>

//============================================================================
// Local Utility Functions
//============================================================================
namespace
{
	
	//------------------------------------------------------------------------
	void* gg_pModuleHandle = NULL;
	
	const char* fixme_startup = "Build/Output_emulation/LightningGCC/Module/libModule.so";
	//------------------------------------------------------------------------
	tErrType LoadModuleManagerLib()
	{
		if( gg_pModuleHandle != NULL )
			return kNoErr;
		gg_pModuleHandle = dlopen(fixme_startup, RTLD_LAZY);
		if( !gg_pModuleHandle )
		{
			//TODO: DebugMPI message (using dlerror?)
	    	return kModuleLoadFail;
		}		
	    dlerror();
	    return kNoErr;
	}
	
	//------------------------------------------------------------------------
	void UnloadModuldManagerLib()
	{
		dlclose(gg_pModuleHandle);
	}
	
	//------------------------------------------------------------------------
	tErrType CheckError()
	{
		const char *dlsym_error = dlerror();
	    if( dlsym_error != NULL )
	    {
			//TODO: DebugMPI message
	    	return kModuleLoadFail;
	    }
	    return kNoErr;
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
			if( kNoErr == (status = CheckError()) )
		  		status = (*funptr)();
		}
		return status;
	}
	
	//------------------------------------------------------------------------
	tErrType Connect(ICoreModule*& ptr, const CString& name, tVersion version)
	{
		static tErrType init = FindModules();	// FIXME/tp: temp startup, replace with real call from boot module

		tErrType status = LoadModuleManagerLib();
		if( status == kNoErr )
		{
			void* pModule = NULL;
			tfnConnect funptr = reinterpret_cast<tfnConnect>
										(dlsym(gg_pModuleHandle, "Connect"));
			if( kNoErr == (status = CheckError()) )
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
			if( kNoErr == (status = CheckError()) )
		  		status = (*funptr)(ptr);
		}
		return status;
	}
}


// eof
