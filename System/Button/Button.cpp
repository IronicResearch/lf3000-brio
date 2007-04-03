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

//#include <KernelMPI.h>
#include <ButtonMPI.h>
#include <ButtonPriv.h>
#include <EventMPI.h>

#include <stdlib.h>	// FIXME: remove when include KernelMPI

const CURI	kModuleURI	= "Button FIXME";


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
//----------------------------------------------------------------------------
CButtonModule::CButtonModule()
{
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


//============================================================================
// Button state
//============================================================================
//----------------------------------------------------------------------------
tErrType CButtonModule::GetButtonState(tButtonData& data)
{
	data.buttonState = 0;
	data.buttonTransition = 0;
	return kNoErr;
}

/*
 * 	// FIXME/tp:
	CEventMPI	em;
	CButtonMessage	msg(data);
	if( em.IsValid() )
		em.PostEvent(msg, 0);
	
 */

//============================================================================
// Instance management interface for the Module Manager
//============================================================================

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
