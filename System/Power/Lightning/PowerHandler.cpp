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

#include <PowerTypes.h>

/* events we may receive */
#define EVENT_POWER		1
#define EVENT_BATTERY 	2

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// Local state and utility functions
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	U32					gLastState;
	struct tPowerData   current_pe;
	tTaskHndl			handlePowerTask;
	struct tPowerData	data;
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
		current_pe.powerState = GetCurrentPowerState();

		// overwrite with power down, if one is pending
		ms = accept(ls, (struct sockaddr *)&mon, &s_mon);
		if(ms > 0) {
			while(1) {
				size = recv(ms, &msg, sizeof(msg), 0);

				if(size == sizeof(msg) && msg.type == APP_MSG_SET_POWER) {
						switch(msg.payload) {
								case EVENT_BATTERY:
								current_pe.powerState = kPowerCritical;
								dbg.DebugOut(kDbgLvlVerbose,
									"%s.%d: state = kPowerCritical\n",
									__FUNCTION__, __LINE__);
								break;
								
								default:
								case EVENT_POWER:
								current_pe.powerState = kPowerShutdown;
								dbg.DebugOut(kDbgLvlVerbose, 
									 "%s.%d: state = kPowerShutdown\n",
									 __FUNCTION__, __LINE__);
								break;
						} 
				} 
				else {
					break;
				}
			}
		}

		// Pace thread at time intervals relavant for power events
		kernel.TaskSleep(250);

		// report power state if changed unless kPowerShutdown already sent
		if (data.powerState != current_pe.powerState &&
		    data.powerState != kPowerShutdown) {
			data.powerState = current_pe.powerState;
			CPowerMessage msg(data);
			eventmgr.PostEvent(msg, kPowerEventPriority);
		}

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

	data.powerState = GetCurrentPowerState();

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
	void* 		retval;

	dbg_.DebugOut(kDbgLvlVerbose, "PowerModule::DeinitModule: Power Deinit\n");

	// Terminate power handler thread, and wait before closing driver
	kernel.CancelTask(handlePowerTask);
	kernel.JoinTask(handlePowerTask, retval);
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
