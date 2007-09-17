//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		PlatformMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio 
//		Platform Manager module.
//
//============================================================================

#include <PlatformMPI.h>
#include <PlatformPriv.h>
#include <EventMPI.h>
#include <Module.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "PlatformMPI";


//============================================================================
// CPlatformMessage
//============================================================================
//------------------------------------------------------------------------------
CPlatformMessage::CPlatformMessage( const tPlatformData& data ) 
	: IEventMessage(kPlatformStateChanged), mData(data)
{
}

//------------------------------------------------------------------------------
U16	CPlatformMessage::GetSizeInBytes() const
{
	return sizeof(CPlatformMessage);
}

//------------------------------------------------------------------------------
tPlatformData CPlatformMessage::GetPlatformState() const
{
	return mData;
}


//============================================================================
// CPlatformMPI
//============================================================================
//----------------------------------------------------------------------------
CPlatformMPI::CPlatformMPI() : pModule_(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kPlatformModuleName, kPlatformModuleVersion);
	pModule_ = reinterpret_cast<CPlatformModule*>(pModule);
}

//----------------------------------------------------------------------------
CPlatformMPI::~CPlatformMPI()
{
	Module::Disconnect(pModule_);
}

//----------------------------------------------------------------------------
Boolean	CPlatformMPI::IsValid() const
{
	return (pModule_ != NULL) ? true : false;
}

//----------------------------------------------------------------------------
const CString* CPlatformMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CPlatformMPI::GetModuleVersion() const
{
	if(!pModule_)
		return kUndefinedVersion;
	return pModule_->GetModuleVersion();
}

//----------------------------------------------------------------------------
const CString* CPlatformMPI::GetModuleName() const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetModuleName();
}

//----------------------------------------------------------------------------
const CURI* CPlatformMPI::GetModuleOrigin() const
{
	if(!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
}


//============================================================================
//----------------------------------------------------------------------------
tErrType CPlatformMPI::RegisterEventListener(const IEventListener *pListener)
{
	CEventMPI eventmgr;
	return eventmgr.RegisterEventListener(pListener);
}

//----------------------------------------------------------------------------
tErrType CPlatformMPI::UnregisterEventListener(const IEventListener *pListener)
{
	CEventMPI eventmgr;
	return eventmgr.UnregisterEventListener(pListener);
}

//----------------------------------------------------------------------------
tPlatformData CPlatformMPI::GetPlatformState() const
{
	if(!pModule_)
	{
		tPlatformData data = { kPlatformNull };
		return data;
	}
	return pModule_->GetPlatformState();
}


LF_END_BRIO_NAMESPACE()
// EOF
