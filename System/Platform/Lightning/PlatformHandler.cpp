//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		PlatformHandler.cpp
//
// Description:
//		Implements the underlying Platform Manager module.
//		NOTE: Debug code at http://www.acm.vt.edu/~jmaxwell/programs/xspy/xspy.html
//
//============================================================================
#include <PlatformPriv.h>
#include <EventMPI.h>
#include <KernelMPI.h>
#include <KernelTypes.h>
#include <SystemErrors.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <PlatformTypes.h>

LF_BEGIN_BRIO_NAMESPACE()



//============================================================================
// Local state and utility functions
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	U32					gLastState;
	int 				platform_fd;
	struct tPlatformData  current_pe;
	tTaskHndl			handlePlatformTask;
	tPlatformData		data;
}



//============================================================================
// Asynchronous notifications
//============================================================================
//----------------------------------------------------------------------------
void *LightningPlatformTask(void*)
{
	CEventMPI 	eventmgr;
	CDebugMPI	dbg(kGroupPlatform);
	CKernelMPI	kernel;

	dbg.SetDebugLevel(kDbgLvlVerbose);
	dbg.DebugOut(kDbgLvlVerbose, "%s: Started\n", __FUNCTION__);
	
	while(1) {

		// Pace thread at time intervals relavant for platform events
		kernel.TaskSleep(1);

		int size = read(platform_fd, &current_pe, sizeof(struct tPlatformData));
		dbg.Assert( size >= 0, "CPlatformModule::LightningPlatformTask: platform read failed" );
		
		data.platformState = current_pe.platformState;
		CPlatformMessage msg(data);
		eventmgr.PostEvent(msg, kPlatformEventPriority);

	}
	return NULL;
}

//----------------------------------------------------------------------------

void CPlatformModule::InitModule()
{
	tErrType	status = kModuleLoadFail;
	CKernelMPI	kernel;

	// instantiate event manager, needed to resolve symbol
	// '_ZN8LeapFrog4Brio63_GLOBAL__N__ZNK8LeapFrog4Brio12CEventModule16
	// GetModuleVersionEv5pinstE'
	
	CEventMPI	eventmgr;

	
	dbg_.DebugOut(kDbgLvlVerbose, "Platform Init\n");

	data.platformState = 0;

	// Need valid file descriptor open before starting task thread 
	platform_fd = open( "/dev/platform", B_O_RDWR);
	dbg_.Assert(platform_fd != -1, "CPlatformModule::InitModule: cannot open /dev/platform");

	if( kernel.IsValid() )
	{
		tTaskProperties	properties;
		properties.pTaskMainArgValues = NULL;
		properties.TaskMainFcn = LightningPlatformTask;
		status = kernel.CreateTask(handlePlatformTask, properties);
	}
	dbg_.Assert( status == kNoErr, 
				"CPlatformModule::InitModule: background task creation failed" );
}

//----------------------------------------------------------------------------
void CPlatformModule::DeinitModule()
{
	CKernelMPI	kernel;

	dbg_.DebugOut(kDbgLvlVerbose, "PlatformModule::DeinitModule: Platform Deinit\n");

	// Terminate platform handler thread, and wait before closing driver
	kernel.CancelTask(handlePlatformTask);
	kernel.TaskSleep(1);	
	close(platform_fd);
}

//============================================================================
// Platform state
//============================================================================
//----------------------------------------------------------------------------
tPlatformData CPlatformModule::GetPlatformState() const
{
	return data;
}

LF_END_BRIO_NAMESPACE()
// EOF
