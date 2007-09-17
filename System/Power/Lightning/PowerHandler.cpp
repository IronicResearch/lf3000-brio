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

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

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
	tPowerData			data;
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

	dbg.SetDebugLevel(kDbgLvlVerbose);
	dbg.DebugOut(kDbgLvlVerbose, "%s: Started\n", __FUNCTION__);
	
	while(1) {

		// Pace thread at time intervals relavant for power events
		kernel.TaskSleep(1);

		int size = read(power_fd, &current_pe, sizeof(struct tPowerData));
		dbg.Assert( size >= 0, "CPowerModule::LightningPowerTask: power read failed" );
		
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

	data.powerState = 0;

	// Need valid file descriptor open before starting task thread 
	power_fd = open( "/dev/power", B_O_RDWR);
	dbg_.Assert(power_fd != -1, "CPowerModule::InitModule: cannot open /dev/power");

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
tPowerData CPowerModule::GetPowerState() const
{
	return data;
}

LF_END_BRIO_NAMESPACE()
// EOF
