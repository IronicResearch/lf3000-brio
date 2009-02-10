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
#include <KernelTypes.h>
#include <Utility.h>

#include <sys/socket.h>
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
}

//----------------------------------------------------------------------------
CUSBDeviceMPI::~CUSBDeviceMPI()
{
}

//----------------------------------------------------------------------------
Boolean	CUSBDeviceMPI::IsValid() const
{
	return true;
}

//----------------------------------------------------------------------------
const CString* CUSBDeviceMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CUSBDeviceMPI::GetModuleVersion() const
{
	return kUSBDeviceModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CUSBDeviceMPI::GetModuleName() const
{
	return &kUSBDeviceModuleName;
}

//----------------------------------------------------------------------------
const CURI* CUSBDeviceMPI::GetModuleOrigin() const
{
	return &kModuleURI;
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
	return GetCurrentUSBDeviceState();
}

//----------------------------------------------------------------------------
tErrType CUSBDeviceMPI::EnableUSBDeviceDrivers(U32 drivers)
{
	// Not possible to enable USB mass storage device while Brio app running
	return kFunctionNotImplementedErr;
}

//----------------------------------------------------------------------------
tErrType CUSBDeviceMPI::DisableUSBDeviceDrivers(U32 drivers)
{
	// Not possible to disable USB mass storage while Brio app running
	return kFunctionNotImplementedErr;
}

//----------------------------------------------------------------------------
U32 CUSBDeviceMPI::GetUSBDeviceWatchdog(void)
{
#ifndef EMULATION
	struct app_message msg, resp;
	int size;
	int s = CreateReportSocket(MONITOR_SOCK);

	if(s < 0)
		return kUSBDeviceInvalidWatchdog;

	msg.type = APP_MSG_GET_USB;
	msg.payload = 0;

	if(send(s, &msg, sizeof(msg), 0) < 0) {
		close(s);
		return kUSBDeviceInvalidWatchdog;
	}

	size = recv(s, &resp, sizeof(struct app_message), 0);
	if(size != sizeof(struct app_message)) {
		close(s);
		return kUSBDeviceInvalidWatchdog;
	}

	close(s);
	return resp.payload;
#else
	return 0;
#endif
}

//----------------------------------------------------------------------------
void CUSBDeviceMPI::SetUSBDeviceWatchdog(U32 timerSec)
{
#ifndef EMULATION
	struct app_message msg;
	int s = CreateReportSocket(MONITOR_SOCK);
	
	if(s < 0)
		return;

	msg.type = APP_MSG_SET_USB;
	msg.payload = timerSec;

	send(s, &msg, sizeof(msg), MSG_NOSIGNAL);
	close(s);
#endif
}

LF_END_BRIO_NAMESPACE()
// EOF
