//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		USBDeviceManager.cpp
//
// Description:
//		Implements the underlying USBDevice Manager module.
//
//============================================================================
#include <USBDevicePriv.h>
#include <EventMPI.h>
#include <KernelMPI.h>
#include <KernelTypes.h>
#include <PowerMPI.h>
#include <SystemErrors.h>
#include <Utility.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// Platform-specific delegate functions
//============================================================================

//----------------------------------------------------------------------------

void CUSBDeviceModule::InitModule()
{
	// USB thread consolidated into Event manager
}

//----------------------------------------------------------------------------
void CUSBDeviceModule::DeinitModule()
{
}

//----------------------------------------------------------------------------
tUSBDeviceData CUSBDeviceModule::GetUSBDeviceState() const
{
	return GetCurrentUSBDeviceState();
}

//----------------------------------------------------------------------------
tErrType CUSBDeviceModule::EnableUSBDeviceDrivers(U32 drivers)
{
	// Not possible to enable USB mass storage device while Brio app running
	return kFunctionNotImplementedErr;
}

//----------------------------------------------------------------------------
tErrType CUSBDeviceModule::DisableUSBDeviceDrivers(U32 drivers)
{
	// Not possible to disable USB mass storage while Brio app running
	return kFunctionNotImplementedErr;
}

//----------------------------------------------------------------------------
U32 CUSBDeviceModule::GetUSBDeviceWatchdog(void)
{
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
}

//----------------------------------------------------------------------------
void CUSBDeviceModule::SetUSBDeviceWatchdog(U32 timerSec)
{
	struct app_message msg;
	int s = CreateReportSocket(MONITOR_SOCK);
	
	if(s < 0)
		return;

	msg.type = APP_MSG_SET_USB;
	msg.payload = timerSec;

	send(s, &msg, sizeof(msg), MSG_NOSIGNAL);
	close(s);
}

LF_END_BRIO_NAMESPACE()
// EOF
