//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		Platform.cpp
//
// Description:
//		Implements the underlying Platform Manager module.
//
//============================================================================
#include <SystemTypes.h>
#include <PlatformPriv.h>
#include <GroupEnumeration.h>
LF_BEGIN_BRIO_NAMESPACE()


//============================================================================
// Constants
//============================================================================
const CURI	kModuleURI	= "/LF/System/Platform";


//============================================================================
// CPlatformModule: Informational functions
//============================================================================
//----------------------------------------------------------------------------
tVersion CPlatformModule::GetModuleVersion() const
{
	return kPlatformModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CPlatformModule::GetModuleName() const
{
	return &kPlatformModuleName;
}

//----------------------------------------------------------------------------
const CURI* CPlatformModule::GetModuleOrigin() const
{
	return &kModuleURI;
}


//============================================================================
// Ctor & dtor
//============================================================================
CPlatformModule::CPlatformModule() : dbg_(kGroupPlatform)
{
	InitModule();	// delegate to platform or emulation initializer
}

//----------------------------------------------------------------------------
CPlatformModule::~CPlatformModule()
{
	DeinitModule();	// delegate to platform or emulation deinitializer
}

//----------------------------------------------------------------------------
Boolean	CPlatformModule::IsValid() const
{
	return true;
}


LF_END_BRIO_NAMESPACE()


//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()

static CPlatformModule*	sinst = NULL;

extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion /*version*/)
	{
		if( sinst == NULL )
			sinst = new CPlatformModule;
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
