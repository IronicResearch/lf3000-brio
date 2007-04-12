//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		Event.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio 
//		Event Manager module.
//
//============================================================================

#include <SystemTypes.h>
#include <StringTypes.h>
#include <SystemErrors.h>

#include <EventMPI.h>
#include <EventPriv.h>
#include <Module.h>
LF_BEGIN_BRIO_NAMESPACE()


const tVersion	kMPIVersion = MakeVersion(0,1);
const CString	kMPIName = "EventMPI";


//============================================================================
//----------------------------------------------------------------------------
CEventMPI::CEventMPI() : pModule_(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kEventModuleName, kEventModuleVersion);
	pModule_ = reinterpret_cast<CEventModule*>(pModule);
}

//----------------------------------------------------------------------------
CEventMPI::~CEventMPI()
{
	Module::Disconnect(pModule_);
}

//----------------------------------------------------------------------------
Boolean	CEventMPI::IsValid() const
{
	return (pModule_ != NULL) ? true : false;
}

//----------------------------------------------------------------------------
tErrType CEventMPI::GetMPIVersion(tVersion &version) const
{
	version = kMPIVersion;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CEventMPI::GetMPIName(ConstPtrCString &pName) const
{
	pName = &kMPIName;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CEventMPI::GetModuleVersion(tVersion &version) const
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetModuleVersion(version);
}

//----------------------------------------------------------------------------
tErrType CEventMPI::GetModuleName(ConstPtrCString &pName) const
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetModuleName(pName);
}

//----------------------------------------------------------------------------
tErrType CEventMPI::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetModuleOrigin(pURI);
}


//============================================================================
//----------------------------------------------------------------------------
tErrType CEventMPI::RegisterEventListener(const IEventListener *pListener,
											tEventRegistrationFlags flags)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->RegisterEventListener(pListener, flags);
}

//----------------------------------------------------------------------------
tErrType CEventMPI::UnregisterEventListener(const IEventListener *pListener)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->UnregisterEventListener(pListener);
}



//============================================================================
//----------------------------------------------------------------------------
tErrType CEventMPI::PostEvent(const IEventMessage &msg, 
								tEventPriority priority, 
								const IEventListener *pListener) const
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->PostEvent(msg, priority, pListener);
}


LF_END_BRIO_NAMESPACE()
// EOF
