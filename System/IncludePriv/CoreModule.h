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
#include <StringTypes.h>
#include <SystemErrors.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================	   
// Constants
//==============================================================================
// VTABLE_EXPORT is used to indicate that the member function is not intended
// for derivation, rather, the keyword "virtual" is being applied because
// clients using the dlopen()/dlsym() class creation mechanism can only  
// resolve member function addresses if they are in the vtable.

#define VTABLE_EXPORT	virtual

#ifdef UNIT_TESTING
	const bool kInUnitTest = true;
#else
	const bool kInUnitTest = false;
#endif


//==============================================================================	   
// Module errors
//==============================================================================
#define MODULE_ERRORS			\
	(kModuleNotFound)			\
	(kModuleOpenFail)			\
	(kModuleLoadFail)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupModule), MODULE_ERRORS)



//==============================================================================
// ICoreModule
//==============================================================================
class ICoreModule {
public:
	ICoreModule() 	{};
	virtual ~ICoreModule()	{};

	virtual Boolean			IsValid() const = 0;
	virtual tVersion		GetModuleVersion() const = 0;
	virtual const CString*	GetModuleName() const = 0;	
	virtual const CURI*		GetModuleOrigin() const = 0;

private:
	// Disable copy semantics
	ICoreModule(const ICoreModule&);
	ICoreModule& operator=(const ICoreModule&);
};

LF_END_BRIO_NAMESPACE()


//==============================================================================
// Instance management interface for the Module Manager
//==============================================================================
LF_USING_BRIO_NAMESPACE()
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
