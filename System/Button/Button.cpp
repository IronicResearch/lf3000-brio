//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		Button.cpp
//
// Description:
//		Implements the underlying Button Manager module.
//
//============================================================================
#include <SystemTypes.h>
#include <ButtonPriv.h>
#include <GroupEnumeration.h>
LF_BEGIN_BRIO_NAMESPACE()


//============================================================================
// Constants
//============================================================================
const CURI	kModuleURI	= "/LF/System/Button";


//============================================================================
// CButtonModule: Informational functions
//============================================================================
//----------------------------------------------------------------------------
tVersion CButtonModule::GetModuleVersion() const
{
	return kButtonModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CButtonModule::GetModuleName() const
{
	return &kButtonModuleName;
}

//----------------------------------------------------------------------------
const CURI* CButtonModule::GetModuleOrigin() const
{
	return &kModuleURI;
}


//============================================================================
// Ctor & dtor
//============================================================================
CButtonModule::CButtonModule() : dbg_(kGroupButton)
{
	InitModule();	// delegate to platform or emulation initializer
}

//----------------------------------------------------------------------------
CButtonModule::~CButtonModule()
{
	DeinitModule();	// delegate to platform or emulation deinitializer
}

//----------------------------------------------------------------------------
Boolean	CButtonModule::IsValid() const
{
	return true;
}


LF_END_BRIO_NAMESPACE()


//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()

static CButtonModule*	sinst = NULL;

extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion /*version*/)
	{
		if( sinst == NULL )
			sinst = new CButtonModule;
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
