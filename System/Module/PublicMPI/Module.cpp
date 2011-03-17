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

#include <SystemTypes.h>
#include <Module.h>
#include <ModulePriv.h>
#include <CoreModule.h>
#include <SystemErrors.h>
#include <DebugMPI.h>
#include <BootSafeKernelMPI.h>

LF_BEGIN_BRIO_NAMESPACE()

const CString	kNullString = "";
const CURI		kNullURI = "";

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
	tHndl gg_pModuleHandle = kInvalidHndl;
	
	//------------------------------------------------------------------------
	tErrType LoadModuleManagerLib()
	{
		CBootSafeKernelMPI	kernel;
		if( gg_pModuleHandle != kInvalidHndl )
			return kNoErr;
		// Attempt to load libModule.so in local search path
		gg_pModuleHandle = kernel.LoadModule("libModule.so");
		if( gg_pModuleHandle != kInvalidHndl )
			return kNoErr;
		CPath path = GetModuleLibraryLocation();
		path = path + "libModule.so";
		gg_pModuleHandle = kernel.LoadModule(path);
		if( gg_pModuleHandle == kInvalidHndl )
		{
			kernel.Printf("BOOTFAIL: Failed to load found module at sopath: %s\n", path.c_str());
			kernel.PowerDown();
		}
	    return kNoErr;
	}
	
	//------------------------------------------------------------------------
	void UnloadModuldManagerLib()
	{
		CBootSafeKernelMPI	kernel;
		kernel.UnloadModule(gg_pModuleHandle);
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
			CBootSafeKernelMPI	kernel;
		    fp.voidptr = kernel.RetrieveSymbolFromModule(gg_pModuleHandle, "FindModules");
		    if( fp.voidptr == NULL )
		    {
				kernel.Printf("BOOTFAIL: Failed to find FindModules()\n");
				kernel.PowerDown();
		    }
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
			CBootSafeKernelMPI	kernel;
		    fp.voidptr = kernel.RetrieveSymbolFromModule(gg_pModuleHandle, "Connect");
		    if( fp.voidptr == NULL )
		    {
				kernel.Printf("BOOTFAIL: Failed to find Connect()\n");
				kernel.PowerDown();
		    }
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
			CBootSafeKernelMPI	kernel;
		    fp.voidptr = kernel.RetrieveSymbolFromModule(gg_pModuleHandle, "Disconnect");
		    if( fp.voidptr == NULL )
		    {
				kernel.Printf("BOOTFAIL: Failed to find Disconnect()\n");
				kernel.PowerDown();
		    }
		  	status = (*(fp.pfnDisconnect))(ptr);
		}
		return status;
	}
}

#else	// LF_MONOLITHIC_DEBUG

//------------------------------------------------------------------------
// For LF_MONOLITHIC_DEBUG builds, create instances of all of the modules and
//------------------------------------------------------------------------

LF_END_BRIO_NAMESPACE()

LF_USING_BRIO_NAMESPACE()
#include <Audio/Include/AudioPriv.h>
#include <Button/Include/ButtonPriv.h>
#include <Debug/Include/DebugPriv.h>
#include <Display/Include/DisplayPriv.h>
#include <Event/Include/EventPriv.h>
#include <Font/Include/FontPriv.h>
#include <Kernel/Include/KernelPriv.h>
#include <Power/Include/PowerPriv.h>
#include <USBDevice/Include/USBDevicePriv.h>
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
	else if (g_requestedName == kPowerModuleName)
		g_map[kPowerModuleName]		= new CPowerModule;
	else if (g_requestedName == kUSBDeviceModuleName)
		g_map[kUSBDeviceModuleName]	= new CUSBDeviceModule;
	else if (g_requestedName == kVideoModuleName)
		g_map[kVideoModuleName]		= new CVideoModule;

	return NULL;		
}
	
LF_BEGIN_BRIO_NAMESPACE()

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
