//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		EventListener.cpp
//
// Description:
//		Implements the underlying Event Manager module.
//
//============================================================================

#include <Module.h>
#include <EventListener.h>
#include <EventListenerPriv.h>
#include <EventPriv.h>


//============================================================================
// IEventListener
//============================================================================
//----------------------------------------------------------------------------
IEventListener::IEventListener(const tEventType* pTypes, U32 count)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kEventMgrModuleName, 
									kEventMgrModuleVersion);
	CEventModule* pEvent = reinterpret_cast<CEventModule*>(pModule);
	mpimpl = pEvent->GenerateEventListenerImpl(pTypes, count);
	Module::Disconnect(kEventMgrModuleName);
}

//----------------------------------------------------------------------------
IEventListener::~IEventListener()
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kEventMgrModuleName, 
									kEventMgrModuleVersion);
	CEventModule* eventmgr = reinterpret_cast<CEventModule*>(pModule);
	eventmgr->UnregisterEventListener(this);
	Module::Disconnect(kEventMgrModuleName);
	delete mpimpl;
}

//----------------------------------------------------------------------------
const IEventListener* IEventListener::GetNextListener() const
{
	return mpimpl->GetNextListener();
}

//----------------------------------------------------------------------------
tErrType IEventListener::SetNextListener(const IEventListener* pListener)
{
	return mpimpl->SetNextListener(pListener);
}

//----------------------------------------------------------------------------
tErrType IEventListener::DisableNotifyForEventType(tEventType type)
{
	return mpimpl->DisableNotifyForEventType(type);
}

//----------------------------------------------------------------------------
tErrType IEventListener::ReenableNotifyForEventType(tEventType type)
{
	return mpimpl->ReenableNotifyForEventType(type);
}

// EOF
