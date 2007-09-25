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
#include <SystemErrors.h>
LF_BEGIN_BRIO_NAMESPACE()


//============================================================================
// IEventListener
//============================================================================
//----------------------------------------------------------------------------
IEventListener::IEventListener(const tEventType* pTypes, U32 count) : pimpl_(NULL)
{
	ICoreModule*	pModule;
	CEventModule*	pEvent;
	
	// Connect to the EventMgr Module, creating one if it doesn't exist.
	Module::Connect(pModule, kEventModuleName, kEventModuleVersion);

	pEvent = reinterpret_cast<CEventModule*>(pModule);

	// Use EventMgr Module to create/register an EventListener.
	pimpl_ = pEvent->GenerateEventListenerImpl(pTypes, count);
	
	// Stash the pointer to the EventModule so we can use it to un-register later.
	pimpl_->pEvent_ = pEvent;
}

//----------------------------------------------------------------------------
IEventListener::~IEventListener()
{
	// Get pointer to the EventMgr Module.
	CEventModule*	pEvent = pimpl_->pEvent_;
	ICoreModule*	pModule;

	// This should be an assert, we should ALWAYS be able to get a module ptr!
	if( pEvent )
	{
		// unregister ourselves from the event module.
		pEvent->UnregisterEventListener(this);
		
		// Cleanup our implementation.
		// TODO: figure out why we have to delete the pimpl_
		// before we can disconnect from the module...
		delete pimpl_;

		// Disconnect ourselves from the EventMgr Module (which should get destructed
		// if we're the last connection.
		pModule = reinterpret_cast<ICoreModule*>(pEvent);
		Module::Disconnect(pModule);
	}
}

//----------------------------------------------------------------------------
const IEventListener* IEventListener::GetNextListener() const
{
	return pimpl_->GetNextListener();
}

//----------------------------------------------------------------------------
tErrType IEventListener::SetNextListener(const IEventListener* pListener)
{
	return pimpl_->SetNextListener(pListener);
}

//----------------------------------------------------------------------------
tErrType IEventListener::DisableNotifyForEventType(tEventType type)
{
	return pimpl_->DisableNotifyForEventType(type);
}

//----------------------------------------------------------------------------
tErrType IEventListener::ReenableNotifyForEventType(tEventType type)
{
	return pimpl_->ReenableNotifyForEventType(type);
}


LF_END_BRIO_NAMESPACE()
// EOF
