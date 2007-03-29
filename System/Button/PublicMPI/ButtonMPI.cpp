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
CButtonMPI::CButtonMPI() : mpModule(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kButtonModuleName, kButtonModuleVersion);
	mpModule = reinterpret_cast<CButtonModule*>(pModule);
}

//----------------------------------------------------------------------------
CButtonMPI::~CButtonMPI()
{
	Module::Disconnect(kButtonModuleName);
}

//----------------------------------------------------------------------------
Boolean	CButtonMPI::IsValid() const
{
	return (mpModule != NULL) ? true : false;
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
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetModuleVersion(version);
}

//----------------------------------------------------------------------------
tErrType CButtonMPI::GetModuleName(ConstPtrCString &pName) const
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetModuleName(pName);
}

//----------------------------------------------------------------------------
tErrType CButtonMPI::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetModuleOrigin(pURI);
}


//============================================================================
//----------------------------------------------------------------------------
tErrType CButtonMPI::GetButtonState(tButtonData& data)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetButtonState(data);
}

// EOF
