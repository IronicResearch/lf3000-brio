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
#include <SystemErrors.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

LF_BEGIN_BRIO_NAMESPACE()



//============================================================================
// Local state and utility functions
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	tTaskHndl		USBDeviceTask;
	tUSBDeviceData	data;
}



//============================================================================
// Asynchronous notifications
//============================================================================
//----------------------------------------------------------------------------
void *LightningUSBDeviceTask(void*)
{
	CEventMPI 	eventmgr;
	CDebugMPI	dbg(kGroupUSBDevice);
	CKernelMPI	kernel;

	dbg.SetDebugLevel(kDbgLvlVerbose);
	dbg.DebugOut(kDbgLvlVerbose, "%s: Started\n", __FUNCTION__);
	
	while(1) {

		// This is currently unimplemented.  Just sleep.
		kernel.TaskSleep(1000);

	}
	return NULL;
}

//----------------------------------------------------------------------------

void CUSBDeviceModule::InitModule()
{
	tErrType	status = kModuleLoadFail;
	CKernelMPI	kernel;
	CEventMPI	eventmgr;
	
	dbg_.DebugOut(kDbgLvlVerbose, "USBDevice Init\n");

	data.USBDeviceState = 0;
	data.USBDeviceDriver = 0;

	if( kernel.IsValid() )
	{
		tTaskProperties	properties;
		properties.pTaskMainArgValues = NULL;
		properties.TaskMainFcn = LightningUSBDeviceTask;
		status = kernel.CreateTask(USBDeviceTask, properties);
	}
	dbg_.Assert( status == kNoErr, 
				"CUSBDeviceModule::InitModule: background task creation failed" );
}

//----------------------------------------------------------------------------
void CUSBDeviceModule::DeinitModule()
{
	CKernelMPI	kernel;

	dbg_.DebugOut(kDbgLvlVerbose, "USBDeviceModule::DeinitModule\n");

	// Terminate handler thread, and wait before closing driver
	kernel.CancelTask(USBDeviceTask);
	kernel.TaskSleep(1);	
}

//----------------------------------------------------------------------------
tUSBDeviceData CUSBDeviceModule::GetUSBDeviceState() const
{
	return data;
}

//----------------------------------------------------------------------------
tErrType CUSBDeviceModule::ActivateUSBDeviceDrivers(U32 drivers)
{
	return kUSBDeviceUnsupportedDriver;
}

//----------------------------------------------------------------------------
tErrType CUSBDeviceModule::DeactivateUSBDeviceDrivers(U32 drivers)
{
	return kUSBDeviceUnsupportedDriver;
}

LF_END_BRIO_NAMESPACE()
// EOF
