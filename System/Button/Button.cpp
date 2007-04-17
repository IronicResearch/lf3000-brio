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
const CURI	kModuleURI	= "Button URI";


//============================================================================
// CButtonModule: Informational functions
//============================================================================
//----------------------------------------------------------------------------
tErrType CButtonModule::GetModuleVersion(tVersion &version) const
{
	version = kButtonModuleVersion;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CButtonModule::GetModuleName(ConstPtrCString &pName) const
{
	pName = &kButtonModuleName;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CButtonModule::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	pURI = &kModuleURI;
	return kNoErr;
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

LF_USING_BRIO_NAMESPACE()

static CButtonModule*	sinst = NULL;
//------------------------------------------------------------------------
extern "C" ICoreModule* CreateInstance(tVersion version)
{
	if( sinst == NULL )
		sinst = new CButtonModule;
	return sinst;
}
	
//------------------------------------------------------------------------
extern "C" void DestroyInstance(ICoreModule* ptr)
{
//		assert(ptr == sinst);
	delete sinst;
	sinst = NULL;
}


// EOF
