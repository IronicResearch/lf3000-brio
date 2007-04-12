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
IEventListener::IEventListener(const tEventType* pTypes, U32 count)
	: pimpl_(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kEventModuleName, kEventModuleVersion);
	CEventModule* pEvent = reinterpret_cast<CEventModule*>(pModule);
	if( pEvent )
	{
		pimpl_ = pEvent->GenerateEventListenerImpl(pTypes, count);
		Module::Disconnect(pModule);
	}
}

//----------------------------------------------------------------------------
IEventListener::~IEventListener()
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kEventModuleName, kEventModuleVersion);
	CEventModule* pEvent = reinterpret_cast<CEventModule*>(pModule);
	if( pEvent )
	{
		pEvent->UnregisterEventListener(this);
		Module::Disconnect(pModule);
		delete pimpl_;
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
