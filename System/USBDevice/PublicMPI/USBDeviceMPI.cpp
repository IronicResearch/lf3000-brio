//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		USBDeviceMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio 
//		USBDevice Manager module.
//
//============================================================================

#include <USBDeviceMPI.h>
#include <USBDevicePriv.h>
#include <EventMPI.h>
#include <Module.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "USBDeviceMPI";


//============================================================================
// CUSBDeviceMessage
//============================================================================
//------------------------------------------------------------------------------
CUSBDeviceMessage::CUSBDeviceMessage( const tUSBDeviceData& data ) 
	: IEventMessage(kUSBDeviceStateChange), mData(data)
{
}

//------------------------------------------------------------------------------
U16	CUSBDeviceMessage::GetSizeInBytes() const
{
	return sizeof(CUSBDeviceMessage);
}

//------------------------------------------------------------------------------
tUSBDeviceData CUSBDeviceMessage::GetUSBDeviceState() const
{
	return mData;
}


//============================================================================
// CUSBDeviceMPI
//============================================================================
//----------------------------------------------------------------------------
CUSBDeviceMPI::CUSBDeviceMPI() : pModule_(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kUSBDeviceModuleName, kUSBDeviceModuleVersion);
	pModule_ = reinterpret_cast<CUSBDeviceModule*>(pModule);
}

//----------------------------------------------------------------------------
CUSBDeviceMPI::~CUSBDeviceMPI()
{
	Module::Disconnect(pModule_);
}

//----------------------------------------------------------------------------
Boolean	CUSBDeviceMPI::IsValid() const
{
	return (pModule_ != NULL) ? true : false;
}

//----------------------------------------------------------------------------
const CString* CUSBDeviceMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CUSBDeviceMPI::GetModuleVersion() const
{
	if(!pModule_)
		return kUndefinedVersion;
	return pModule_->GetModuleVersion();
}

//----------------------------------------------------------------------------
const CString* CUSBDeviceMPI::GetModuleName() const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetModuleName();
}

//----------------------------------------------------------------------------
const CURI* CUSBDeviceMPI::GetModuleOrigin() const
{
	if(!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
}


//============================================================================
//----------------------------------------------------------------------------
tErrType CUSBDeviceMPI::RegisterEventListener(const IEventListener *pListener)
{
	CEventMPI eventmgr;
	return eventmgr.RegisterEventListener(pListener);
}

//----------------------------------------------------------------------------
tErrType CUSBDeviceMPI::UnregisterEventListener(const IEventListener *pListener)
{
	CEventMPI eventmgr;
	return eventmgr.UnregisterEventListener(pListener);
}

//----------------------------------------------------------------------------
tUSBDeviceData CUSBDeviceMPI::GetUSBDeviceState() const
{
	if(!pModule_)
	{
		tUSBDeviceData	data = { 0, 0, 0 };
		return data;
	}
	return pModule_->GetUSBDeviceState();
}

//----------------------------------------------------------------------------
tErrType CUSBDeviceMPI::EnableUSBDeviceDrivers(U32 drivers)
{
	if(!pModule_)
	{
		if(drivers)
			return kUSBDeviceTooManyDrivers;
		return kNoErr;
	}
	return pModule_->EnableUSBDeviceDrivers(drivers);
}

//----------------------------------------------------------------------------
tErrType CUSBDeviceMPI::DisableUSBDeviceDrivers(U32 drivers)
{
	if(!pModule_)
	{
		return kNoErr;
	}
	return pModule_->DisableUSBDeviceDrivers(drivers);
}

//----------------------------------------------------------------------------
U32 CUSBDeviceMPI::GetUSBDeviceWatchdog(void)
{
	if(!pModule_)
	{
		return kUSBDeviceInvalidWatchdog;
	}
	return pModule_->GetUSBDeviceWatchdog();
}

//----------------------------------------------------------------------------
void CUSBDeviceMPI::SetUSBDeviceWatchdog(U32 timerSec)
{
	if(!pModule_)
	{
		return;
	}
	pModule_->SetUSBDeviceWatchdog(timerSec);
}

LF_END_BRIO_NAMESPACE()
// EOF
