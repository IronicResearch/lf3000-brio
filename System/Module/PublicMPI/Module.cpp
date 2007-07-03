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
extern "C"
{
	typedef tErrType (*tfnFindMod)();
	typedef tErrType (*tfnConnect)(void**, const char*, tVersion);
	typedef tErrType (*tfnDisconnect)(const ICoreModule*);
}


//============================================================================
// Local Utility Functions
//============================================================================
namespace
{
	//========================================================================
	// This union is some syntactic grease that allows us to take the void*
	// returned by dlsym() and "cast" it to an appropriate function pointer
	// when the "-pedantic-error" compiler switch is enabled.
	// Directly casting an object* to a function* causes a compiler error when
	// "-pedantic-error" is enabled.
	//========================================================================	   
	union ObjPtrToFunPtrConverter
	{
		void* 			voidptr;
		tfnFindMod		pfnFind;
		tfnConnect		pfnConnect;
		tfnDisconnect	pfnDisconnect;
	};
		
	//------------------------------------------------------------------------
	void* gg_pModuleHandle = NULL;
	
	//------------------------------------------------------------------------
	tErrType LoadModuleManagerLib()
	{
//		CDebugMPI	dbg(kGroupModule);
		if( gg_pModuleHandle != NULL )
			return kNoErr;
		CPath path = GetModuleLibraryLocation();
		path = path + "libModule.so";
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
	inline void AbortOnError(const char* /*msg*/)
	{
		// FIXME/tp: Implement
//		const char *dlsym_error = dlerror();
//		CDebugMPI	dbg(kGroupModule);
//		dbg.Assert(dlsym_error == NULL, msg);
	}
}

 
//============================================================================
// Module Interface
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
namespace Module
{
	//------------------------------------------------------------------------
	// For normal, non-LF_MONOLITHIC_DEBUG builds, call through to the libModule.so
	// replacable shared object for the implementation.
	//------------------------------------------------------------------------
		//------------------------------------------------------------------------
	tErrType FindModules()
	{
		tErrType status = LoadModuleManagerLib();
		if( status == kNoErr )
		{
		    ObjPtrToFunPtrConverter fp;
		    fp.voidptr = dlsym(gg_pModuleHandle, "FindModules");
			AbortOnError("FindModules lookup failure");
		  	status = (*(fp.pfnFind))();
		}
		return status;
	}
	
	//------------------------------------------------------------------------
	tErrType Connect(ICoreModule*& ptr, const CString& name, tVersion version)
	{
		static tErrType init = FindModules();	// FIXME/tp: temp startup, replace with real call from boot module
		init = !!init;		// bogus line to revmove "unused variable 'init'" warning

		ptr = NULL;
		tErrType status = LoadModuleManagerLib();
		if( status == kNoErr )
		{
			void* pModule = NULL;
			ObjPtrToFunPtrConverter fp;
			fp.voidptr = dlsym(gg_pModuleHandle, "Connect");
			AbortOnError("Connect lookup failure");
		  	status = (*(fp.pfnConnect))(&pModule, name.c_str(), version);
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
			ObjPtrToFunPtrConverter fp;
			fp.voidptr = dlsym(gg_pModuleHandle, "Disconnect");
			AbortOnError("Disconnect lookup failure");
		  	status = (*(fp.pfnDisconnect))(ptr);
		}
		return status;
	}
}

#else	// LF_MONOLITHIC_DEBUG

//------------------------------------------------------------------------
// For LF_MONOLITHIC_DEBUG builds, create instances of all of the modules and
//------------------------------------------------------------------------

#include <Audio/Include/AudioPriv.h>
#include <Button/Include/ButtonPriv.h>
#include <Debug/Include/DebugPriv.h>
#include <Display/Include/DisplayPriv.h>
#include <Event/Include/EventPriv.h>
#include <Font/Include/FontPriv.h>
#include <Kernel/Include/KernelPriv.h>
#include <Resource/Include/ResourcePriv.h>
#include <Video/Include/VideoPriv.h>
#include <map>
#include <vector>

typedef std::map<CString, ICoreModule*>	ModuleMapX;
static ModuleMapX	g_map;
static const char*		g_requestedName = NULL;

//------------------------------------------------------------------------
extern "C" ICoreModule* CreateInstance(tVersion)
{
	// Funky funky trick because the Module ctors and dtors are declared
	// private to prevent accidental instanciation outside fo the 
	// CreateInstance()/DestroyInstance() functions.
	//
	if (g_requestedName == kAudioModuleName)
		g_map.insert(ModuleMapX::value_type(kAudioModuleName, new CAudioModule));
	else if (g_requestedName == kButtonModuleName)
		g_map[kButtonModuleName]	= new CButtonModule;
	else if (g_requestedName == kDebugModuleName)
		g_map[kDebugModuleName]		= new CDebugModule;
	else if (g_requestedName == kDisplayModuleName)
		g_map[kDisplayModuleName]	= new CDisplayModule;
	else if (g_requestedName == kEventModuleName)
		g_map[kEventModuleName]		= new CEventModule;
	else if (g_requestedName == kFontModuleName)
		g_map[kFontModuleName]		= new CFontModule;
	else if (g_requestedName == kKernelModuleName)
		g_map[kKernelModuleName]	= new CKernelModule;
	else if (g_requestedName == kResourceModuleName)
		g_map[kResourceModuleName]	= new CResourceModule;
	else if (g_requestedName == kVideoModuleName)
		g_map[kVideoModuleName]		= new CVideoModule;

	return NULL;		
}
	
namespace Module
{	
	//------------------------------------------------------------------------
	tErrType FindModules()
	{
		return kNoErr;
	}
	
	//------------------------------------------------------------------------
	tErrType Connect(ICoreModule*& ptr, const CString& name, tVersion version)
	{
		if (g_map.count(name) == 0)
		{
			g_requestedName = name.c_str();
			CreateInstance(kUndefinedVersion);
			if (g_map.count(name) == 0)
				return kModuleNotFound;
		}
		ptr = g_map[name];
		return kNoErr;
	}
	
	//------------------------------------------------------------------------
	tErrType Disconnect(const ICoreModule* ptr)
	{
		return kNoErr;
	}
}
#endif	// LF_MONOLITHIC_DEBUG



LF_END_BRIO_NAMESPACE()
// eof
