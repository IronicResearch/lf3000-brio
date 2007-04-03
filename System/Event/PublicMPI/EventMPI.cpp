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

#include <StringTypes.h>
#include <SystemErrors.h>

#include <EventMPI.h>
#include <EventPriv.h>
#include <Module.h>


const tVersion	kMPIVersion = MakeVersion(0,1);
const CString	kMPIName = "EventMPI";


//============================================================================
//----------------------------------------------------------------------------
CEventMPI::CEventMPI() : mpModule(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kEventModuleName, kEventModuleVersion);
	mpModule = reinterpret_cast<CEventModule*>(pModule);
}

//----------------------------------------------------------------------------
CEventMPI::~CEventMPI()
{
	Module::Disconnect(mpModule);
}

//----------------------------------------------------------------------------
Boolean	CEventMPI::IsValid() const
{
	return (mpModule != NULL) ? true : false;
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
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetModuleVersion(version);
}

//----------------------------------------------------------------------------
tErrType CEventMPI::GetModuleName(ConstPtrCString &pName) const
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetModuleName(pName);
}

//----------------------------------------------------------------------------
tErrType CEventMPI::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->GetModuleOrigin(pURI);
}


//============================================================================
//----------------------------------------------------------------------------
tErrType CEventMPI::RegisterEventListener(const IEventListener *pListener,
											tEventRegistrationFlags flags)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->RegisterEventListener(pListener, flags);
}

//----------------------------------------------------------------------------
tErrType CEventMPI::UnregisterEventListener(const IEventListener *pListener)
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->UnregisterEventListener(pListener);
}



//============================================================================
//----------------------------------------------------------------------------
tErrType CEventMPI::PostEvent(const IEventMessage &msg, 
								tEventPriority priority, 
								const IEventListener *pListener) const
{
	if(!mpModule)
		return kMpiNotConnectedErr;
	return mpModule->PostEvent(msg, priority, pListener);
}

// EOF
