//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		Power.cpp
//
// Description:
//		Implements the underlying Power Manager module.
//
//============================================================================
#include <SystemTypes.h>
#include <PowerPriv.h>
#include <GroupEnumeration.h>
LF_BEGIN_BRIO_NAMESPACE()


//============================================================================
// Constants
//============================================================================
const CURI	kModuleURI	= "/LF/System/Power";


//============================================================================
// CPowerModule: Informational functions
//============================================================================
//----------------------------------------------------------------------------
tVersion CPowerModule::GetModuleVersion() const
{
	return kPowerModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CPowerModule::GetModuleName() const
{
	return &kPowerModuleName;
}

//----------------------------------------------------------------------------
const CURI* CPowerModule::GetModuleOrigin() const
{
	return &kModuleURI;
}


//============================================================================
// Ctor & dtor
//============================================================================
CPowerModule::CPowerModule() : dbg_(kGroupPower)
{
	InitModule();	// delegate to platform or emulation initializer
}

//----------------------------------------------------------------------------
CPowerModule::~CPowerModule()
{
	DeinitModule();	// delegate to platform or emulation deinitializer
}

//----------------------------------------------------------------------------
Boolean	CPowerModule::IsValid() const
{
	return true;
}


LF_END_BRIO_NAMESPACE()


//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()

static CPowerModule*	sinst = NULL;

extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion /*version*/)
	{
		if( sinst == NULL )
			sinst = new CPowerModule;
		return sinst;
	}
		
	//------------------------------------------------------------------------
	void DestroyInstance(ICoreModule* /*ptr*/)
	{
	//		assert(ptr == sinst);
		delete sinst;
		sinst = NULL;
	}
}
#endif	// LF_MONOLITHIC_DEBUG


// eof
