//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		DisplayMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio 
//		Display Manager module.
//
//==============================================================================

#include <DisplayMPI.h>
#include <DisplayPriv.h>
#include <Module.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "DisplaynMPI";


//============================================================================
// CDisplayMPI
//============================================================================
//----------------------------------------------------------------------------
CDisplayMPI::CDisplayMPI() : pModule_(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kDisplayModuleName, kDisplayModuleVersion);
	pModule_ = reinterpret_cast<CDisplayModule*>(pModule);
}

//----------------------------------------------------------------------------
CDisplayMPI::~CDisplayMPI()
{
	Module::Disconnect(pModule_);
}

//----------------------------------------------------------------------------
Boolean	CDisplayMPI::IsValid() const
{
	return (pModule_ != NULL) ? true : false;
}

//----------------------------------------------------------------------------
const CString* CDisplayMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CDisplayMPI::GetModuleVersion() const
{
	if(!pModule_)
		return kUndefinedVersion;
	return pModule_->GetModuleVersion();
}

//----------------------------------------------------------------------------
const CString* CDisplayMPI::GetModuleName() const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetModuleName();
}

//----------------------------------------------------------------------------
const CURI* CDisplayMPI::GetModuleOrigin() const
{
	if(!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
}


//============================================================================
//----------------------------------------------------------------------------
tErrType CDisplayMPI::GetDisplayDimensions(U16& width, U16& height) const
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetDisplayDimensions(width, height);
}


// EOF
