//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		EventMgrMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio 
//		Event Manager module.
//
//============================================================================

#include <StringTypes.h>
#include <SystemErrors.h>

#include <EventListener.h>
#include <EventMessage.h>
#include <EventMgrMPI.h>
#include <EventMgrModulePriv.h>
#include <ModuleMgr.h>


const tVersion	kMPIVersion = MakeVersion(0,1);
const CString	kMpiName = "EventMgrMPI";


//============================================================================
//----------------------------------------------------------------------------
CEventMgrMPI::CEventMgrMPI() : mpModule(NULL)
{
	ICoreModule*	pModule;
	CModuleMgr::Instance()->Connect(pModule, kEventMgrModuleName, 
									kEventMgrModuleVersion);
	mpModule = reinterpret_cast<CEventMgrModule*>(pModule);
}

//----------------------------------------------------------------------------
CEventMgrMPI::~CEventMgrMPI()
{
	CModuleMgr::Instance()->Disconnect(kEventMgrModuleName);
}

//----------------------------------------------------------------------------
Boolean	CEventMgrMPI::IsValid() const
{
	return (mpModule != NULL) ? true : false;
}

//----------------------------------------------------------------------------
tErrType CEventMgrMPI::GetMPIVersion(tVersion &version) const
{
	version = kMPIVersion;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CEventMgrMPI::GetMPIName(ConstPtrCString &pName) const
{
	pName = &kMpiName;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CEventMgrMPI::GetModuleVersion(tVersion &version) const
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetModuleVersion(version);
}

//----------------------------------------------------------------------------
tErrType CEventMgrMPI::GetModuleName(ConstPtrCString &pName) const
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetModuleName(pName);
}

//----------------------------------------------------------------------------
tErrType CEventMgrMPI::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetModuleOrigin(pURI);
}


//============================================================================
//----------------------------------------------------------------------------
tErrType CEventMgrMPI::RegisterEventListener(const IEventListener *pListener,
											tEventRegistrationFlags flags)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->RegisterEventListener(pListener, flags);
}

//----------------------------------------------------------------------------
tErrType CEventMgrMPI::RegisterResponseEventListener(tListenerId &id, 
											const IEventListener *pListener,
											tEventRegistrationFlags flags)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->RegisterResponseEventListener(id, pListener, flags);
}

//----------------------------------------------------------------------------
tErrType CEventMgrMPI::UnregisterEventListener(const IEventListener *pListener)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->UnregisterEventListener(pListener);
}



//============================================================================
//----------------------------------------------------------------------------
tErrType CEventMgrMPI::PostEvent(const IEventMessage &msg, 
								tEventPriority priority, 
								const IEventListener *pListener,
								tListenerId id) const
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->PostEvent(msg, priority, pListener, id);
}

// EOF
