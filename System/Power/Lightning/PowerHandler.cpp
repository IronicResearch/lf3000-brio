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
#include <sys/ioctl.h>

#include <PowerTypes.h>

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// Local state and utility functions
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	U32					gLastState;
	int 				power_fd;
	struct tPowerData   current_pe;
	tTaskHndl			handlePowerTask;
	struct tPowerData	data;
}

enum tPowerState GetPowerState(void)
{
		CDebugMPI dbg(kGroupPower);
		char buf[2];
		int size = read(power_fd, &buf, 2);
		dbg.Assert(size >= 0, "CPowerModule::LightningPowerTask: read failed");

		switch(buf[0]) {
			case '1':
			return kPowerExternal;
			case '2':
			return kPowerBattery;
			case '3':
			return kPowerLowBattery;
			case '4':
			return kPowerCritical;
		}
		return kPowerNull;
}

//============================================================================
// Asynchronous notifications
//============================================================================
//----------------------------------------------------------------------------
void *LightningPowerTask(void*)
{
	CEventMPI 	eventmgr;
	CDebugMPI	dbg(kGroupPower);
	CKernelMPI	kernel;
	struct sockaddr_un mon;
	socklen_t s_mon = sizeof(struct sockaddr_un);
	int ls, ms;
	int size;
	struct app_message msg;
	
	dbg.SetDebugLevel(kDbgLvlVerbose);
	
	dbg.DebugOut(kDbgLvlVerbose, "%s: Started\n", __FUNCTION__);

	ls = CreateListeningSocket(POWER_SOCK);
	dbg.Assert(ls >= 0, "can't open listening socket");

	while(1) {
		// get battery state
		current_pe.powerState = GetPowerState();

		// overwrite with power down, if one is pending
		ms = accept(ls, (struct sockaddr *)&mon, &s_mon);
		if(ms > 0) {
			while(1) {
				size = recv(ms, &msg, sizeof(msg), 0);
				if(size == sizeof(msg) && msg.type == APP_MSG_SET_POWER)
					current_pe.powerState = kPowerShutdown;
				else
					break;
			}
		}

		// Pace thread at time intervals relavant for power events
		kernel.TaskSleep(100);

		// report power state
		data.powerState = current_pe.powerState;
		CPowerMessage msg(data);
		eventmgr.PostEvent(msg, kPowerEventPriority);

	}
	return NULL;
}

//----------------------------------------------------------------------------

void CPowerModule::InitModule()
{
	tErrType	status = kModuleLoadFail;
	CKernelMPI	kernel;

	// instantiate event manager, needed to resolve symbol
	// '_ZN8LeapFrog4Brio63_GLOBAL__N__ZNK8LeapFrog4Brio12CEventModule16
	// GetModuleVersionEv5pinstE'
	
	CEventMPI	eventmgr;

	dbg_.DebugOut(kDbgLvlVerbose, "Power Init\n");

	data.powerState = kPowerNull;

	// Need valid file descriptor open before starting task thread 
	power_fd = open("/sys/devices/platform/lf1000-power/status", O_RDONLY);
	dbg_.Assert(power_fd >= 0, "CPowerModule::InitModule: cannot open status");

	if( kernel.IsValid() )
	{
		tTaskProperties	properties;
		properties.pTaskMainArgValues = NULL;
		properties.TaskMainFcn = LightningPowerTask;
		status = kernel.CreateTask(handlePowerTask, properties);
	}
	dbg_.Assert( status == kNoErr, 
				"CPowerModule::InitModule: background task creation failed" );
}

//----------------------------------------------------------------------------
void CPowerModule::DeinitModule()
{
	CKernelMPI	kernel;

	dbg_.DebugOut(kDbgLvlVerbose, "PowerModule::DeinitModule: Power Deinit\n");

	// Terminate power handler thread, and wait before closing driver
	kernel.CancelTask(handlePowerTask);
	kernel.TaskSleep(1);	
	close(power_fd);
}

//============================================================================
// Power state
//============================================================================
//----------------------------------------------------------------------------
enum tPowerState CPowerModule::GetPowerState() const
{
	return data.powerState;
}

//----------------------------------------------------------------------------
int CPowerModule::Shutdown() const
{
	//CDebugMPI	dbg(kGroupPower);
	//int status = ioctl(power_fd, POWER_IOCT_SHUTDOWN, 0);
	//dbg.Assert(status >= 0, "PowerModule::Shutdown: ioctl failed");

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
	//CDebugMPI	dbg(kGroupPower);
	//int status = ioctl(power_fd, POWER_IOCT_RESET, 0);
	//dbg.Assert(status >= 0, "PowerModule::Reset: ioctl failed");

	// Embedded version should never get here
	exit(kKernelExitReset);
	return kKernelExitError;
}
LF_END_BRIO_NAMESPACE()
// EOF
