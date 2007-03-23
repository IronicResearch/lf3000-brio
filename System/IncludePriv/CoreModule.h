#ifndef LF_BRIO_COREMODULE_H
#define LF_BRIO_COREMODULE_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		CoreModule.h
//
// Description:
//		Defines the ICoreModule interface class 
//		from which all System, Product & Cartridge modules are derived.
//
//		Each module can have only a single instanciated instance, since
//		modules touch hardware, and multiple objects accessing a single
//		hardware subsystem would be extremely error prone.
//
//		The boot system ModuleMgr manages the instanciation and destruction
//		of modules to ensure only a single instance of a particular module 
//		is ever instanciated.
//
//		Each shared object that implements a module must export two extern "C"
//		functions, "CreateInstance()" and "DestroyInstance()", used by the ModuleMgr
//		to manage the lifetime of a new ICoreModule-derived object.
//		"CreateInstance()" returns a ICoreModule*, which it in turn provides to
//		an ICoreMPI-derived object to connect the lightweight interface
//		object to the underlying system module.  The ICoreMPI object can
//		then cast the ICoreModule* to an appropriate ICoreModule-derived
//		class pointer to access the module's functionality.
//
//		See the ModuleMgr header for more details on how modules are 
//		instanciated and destroyed.
//
//==============================================================================
// NOTE: There are many subtle issues with module versioning that are not yet worked out.

#include <SystemTypes.h>


//==============================================================================
// ICoreModule
//==============================================================================
class ICoreModule {
public:
	ICoreModule() 	{};
	virtual ~ICoreModule()	{};

	virtual Boolean		IsValid() const = 0;

	virtual tErrType	GetModuleVersion(tVersion &version) const = 0;
	virtual tErrType	GetModuleName(ConstPtrCString &pName) const = 0;	
	virtual tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const = 0;
};

//==============================================================================
// Instance management interface for the Module Manager
//==============================================================================
extern "C" ICoreModule* CreateInstance(tVersion version);
extern "C" void			DestroyInstance(ICoreModule*);

extern "C"
{
	typedef ICoreModule* (*pFnCreateInstance)(tVersion);
	typedef void	(*pFnDestroyInstance)(ICoreModule*);
}

#define kCreateInstanceFnName	"CreateInstance"
#define kDestroyInstanceFnName	"DestroyInstance"


#endif // LF_BRIO_COREMODULE_H

// eof
