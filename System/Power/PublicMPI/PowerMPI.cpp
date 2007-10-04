//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		PowerMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio 
//		Power Manager module.
//
//============================================================================

#include <PowerMPI.h>
#include <PowerPriv.h>
#include <EventMPI.h>
#include <Module.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "PowerMPI";


//============================================================================
// CPowerMessage
//============================================================================
//------------------------------------------------------------------------------
CPowerMessage::CPowerMessage( const tPowerData& data ) 
	: IEventMessage(kPowerStateChanged), mData(data)
{
}

//------------------------------------------------------------------------------
U16	CPowerMessage::GetSizeInBytes() const
{
	return sizeof(CPowerMessage);
}

//------------------------------------------------------------------------------
enum tPowerState CPowerMessage::GetPowerState() const
{
	return mData.powerState;
}


//============================================================================
// CPowerMPI
//============================================================================
//----------------------------------------------------------------------------
CPowerMPI::CPowerMPI() : pModule_(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kPowerModuleName, kPowerModuleVersion);
	pModule_ = reinterpret_cast<CPowerModule*>(pModule);
}

//----------------------------------------------------------------------------
CPowerMPI::~CPowerMPI()
{
	Module::Disconnect(pModule_);
}

//----------------------------------------------------------------------------
Boolean	CPowerMPI::IsValid() const
{
	return (pModule_ != NULL) ? true : false;
}

//----------------------------------------------------------------------------
const CString* CPowerMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CPowerMPI::GetModuleVersion() const
{
	if(!pModule_)
		return kUndefinedVersion;
	return pModule_->GetModuleVersion();
}

//----------------------------------------------------------------------------
const CString* CPowerMPI::GetModuleName() const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetModuleName();
}

//----------------------------------------------------------------------------
const CURI* CPowerMPI::GetModuleOrigin() const
{
	if(!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
}


//============================================================================
//----------------------------------------------------------------------------
tErrType CPowerMPI::RegisterEventListener(const IEventListener *pListener)
{
	CEventMPI eventmgr;
	return eventmgr.RegisterEventListener(pListener);
}

//----------------------------------------------------------------------------
tErrType CPowerMPI::UnregisterEventListener(const IEventListener *pListener)
{
	CEventMPI eventmgr;
	return eventmgr.UnregisterEventListener(pListener);
}

//----------------------------------------------------------------------------
enum tPowerState CPowerMPI::GetPowerState() const
{
	if(!pModule_)
	{
		return kPowerNull;
	}
	return pModule_->GetPowerState();
}

//----------------------------------------------------------------------------
int CPowerMPI::Shutdown() const
{
	if(!pModule_)
	{
		return kPowerNull;
	}
	return pModule_->Shutdown();
}

//----------------------------------------------------------------------------
int CPowerMPI::GetShutdownTimeMS() const
{
	if(!pModule_)
	{
		return kPowerNull;
	}
	return pModule_->GetShutdownTimeMS();
}

//----------------------------------------------------------------------------
int CPowerMPI::SetShutdownTimeMS(int iMilliseconds) const
{
	if(!pModule_)
	{
		return kPowerNull;
	}
	return pModule_->SetShutdownTimeMS(iMilliseconds);
}

//----------------------------------------------------------------------------
int CPowerMPI::Reset() const
{
	if(!pModule_)
	{
		return kPowerNull;
	}
	return pModule_->Reset();
}

LF_END_BRIO_NAMESPACE()

// EOF
