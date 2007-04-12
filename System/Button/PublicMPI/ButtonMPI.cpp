//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		ButtonMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio 
//		Button Manager module.
//
//============================================================================

#include <ButtonMPI.h>
#include <ButtonPriv.h>
#include <Module.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
LF_BEGIN_BRIO_NAMESPACE()


const tVersion	kMPIVersion = MakeVersion(0,1);
const CString	kMPIName = "ButtonMPI";


//============================================================================
// CButtonMessage
//============================================================================
//------------------------------------------------------------------------------
CButtonMessage::CButtonMessage( const tButtonData& data ) 
	: IEventMessage(kButtonStateChanged, 0), mData(data)
{
}

//------------------------------------------------------------------------------
U16	CButtonMessage::GetSizeInBytes() const
{
	return sizeof(CButtonMessage);
}

//------------------------------------------------------------------------------
tButtonData CButtonMessage::GetButtonState() const
{
	return mData;
}


//============================================================================
// CButtonMPI
//============================================================================
//----------------------------------------------------------------------------
CButtonMPI::CButtonMPI() : pModule_(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kButtonModuleName, kButtonModuleVersion);
	pModule_ = reinterpret_cast<CButtonModule*>(pModule);
}

//----------------------------------------------------------------------------
CButtonMPI::~CButtonMPI()
{
	Module::Disconnect(pModule_);
}

//----------------------------------------------------------------------------
Boolean	CButtonMPI::IsValid() const
{
	return (pModule_ != NULL) ? true : false;
}

//----------------------------------------------------------------------------
tErrType CButtonMPI::GetMPIVersion(tVersion &version) const
{
	version = kMPIVersion;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CButtonMPI::GetMPIName(ConstPtrCString &pName) const
{
	pName = &kMPIName;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CButtonMPI::GetModuleVersion(tVersion &version) const
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetModuleVersion(version);
}

//----------------------------------------------------------------------------
tErrType CButtonMPI::GetModuleName(ConstPtrCString &pName) const
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetModuleName(pName);
}

//----------------------------------------------------------------------------
tErrType CButtonMPI::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetModuleOrigin(pURI);
}


//============================================================================
//----------------------------------------------------------------------------
tErrType CButtonMPI::GetButtonState(tButtonData& data) const
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetButtonState(data);
}


LF_END_BRIO_NAMESPACE()
// EOF
