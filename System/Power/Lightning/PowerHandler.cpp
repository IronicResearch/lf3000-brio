//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		PowerHandler.cpp
//
// Description:
//		Implements the underlying Power Manager module.
//		NOTE: Debug code at http://www.acm.vt.edu/~jmaxwell/programs/xspy/xspy.html
//
//============================================================================
#include <PowerPriv.h>
#include <EventMPI.h>
#include <KernelMPI.h>
#include <KernelTypes.h>
#include <SystemErrors.h>
#include <Utility.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <fcntl.h>

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// Platform-specific delegate functions
//============================================================================

//----------------------------------------------------------------------------

void CPowerModule::InitModule()
{
	// Power thread consolidated into Event manager
}

//----------------------------------------------------------------------------
void CPowerModule::DeinitModule()
{
}

//============================================================================
// Power state
//============================================================================
//----------------------------------------------------------------------------
enum tPowerState CPowerModule::GetPowerState() const
{
	return GetCurrentPowerState();
}

//----------------------------------------------------------------------------
int CPowerModule::Shutdown() const
{
	// Embedded version should never get here
	exit(kKernelExitShutdown);
	return kKernelExitError;
}

//----------------------------------------------------------------------------
int CPowerModule::GetShutdownTimeMS() const
{
	struct app_message msg, resp;
	int size;
	int s = CreateReportSocket(MONITOR_SOCK);

	msg.type = APP_MSG_GET_POWER;
	msg.payload = 0;

	if(send(s, &msg, sizeof(msg), 0) < 0) {
		close(s);
		return -1;
	}

	size = recv(s, &resp, sizeof(struct app_message), 0);
	if(size != sizeof(struct app_message)) {
		close(s);
		return -1;
	}

	close(s);
	return resp.payload*1000;
}

//----------------------------------------------------------------------------
int CPowerModule::SetShutdownTimeMS(int iMilliseconds) const
{
	struct app_message msg;
	int s = CreateReportSocket(MONITOR_SOCK);

	msg.type = APP_MSG_SET_POWER;
	if(iMilliseconds == 0)
		msg.payload = 0;
	else
		msg.payload = iMilliseconds >= 1000 ? iMilliseconds/1000 : 1;

	send(s, &msg, sizeof(msg), MSG_NOSIGNAL);
	close(s);
	return 0;
}

//----------------------------------------------------------------------------
int CPowerModule::Reset() const
{
	// Embedded version should never get here
	exit(kKernelExitReset);
	return kKernelExitError;
}
LF_END_BRIO_NAMESPACE()
// EOF
