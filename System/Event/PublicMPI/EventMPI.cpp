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
const CString* CEventMPI::GetMPIName() const
{
	return &kMPIName;

}

//----------------------------------------------------------------------------
tVersion CEventMPI::GetModuleVersion() const
{
	if(!pModule_)
		return kUndefinedVersion;
	return pModule_->GetModuleVersion();
}

//----------------------------------------------------------------------------
const CString* CEventMPI::GetModuleName() const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetModuleName();
}

//----------------------------------------------------------------------------
const CURI* CEventMPI::GetModuleOrigin() const
{
	if(!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
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
