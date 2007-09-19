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
#include <sys/types.h>
#include <sys/wait.h>

LF_BEGIN_BRIO_NAMESPACE()

#define SYSFS_LUN0_PATH "/sys/devices/platform/lf1000-usbgadget/gadget/gadget-lun0/file"
#define SYSFS_LUN1_PATH "/sys/devices/platform/lf1000-usbgadget/gadget/gadget-lun1/file"
#define SYSFS_VBUS_PATH "/sys/devices/platform/lf1000-usbgadget/vbus"

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
	FILE *f;
	int vbus, ret;

	dbg.DebugOut(kDbgLvlVerbose, "%s: Started\n", __FUNCTION__);
	
	while(1) {
		// Just check the vbus status every so often
		f = fopen(SYSFS_VBUS_PATH, "r");
		if(!f) {
			dbg.DebugOut(kDbgLvlVerbose, "USBDevice can't get VBUS state!\n");
			continue;
		}
		
		ret = fscanf(f, "%d\n", &vbus);
		fclose(f);
		if(ret != 1) {
			dbg.DebugOut(kDbgLvlVerbose, "USBDevice can't get VBUS state!\n");
			continue;
		}
		
		// Did the connection state change?
		if((vbus == 1) && !(data.USBDeviceState & kUSBDeviceConnected)) {
			data.USBDeviceState |= kUSBDeviceConnected;
			CUSBDeviceMessage msg(data);
			eventmgr.PostEvent(msg, kUSBDeviceEventPriority);
		} else if((vbus == 0) && (data.USBDeviceState & kUSBDeviceConnected)) {
			data.USBDeviceState &= ~(kUSBDeviceConnected);
			CUSBDeviceMessage msg(data);
			eventmgr.PostEvent(msg, kUSBDeviceEventPriority);
		}
		kernel.TaskSleep(100);
		
	}
	return NULL;
}

//----------------------------------------------------------------------------

void CUSBDeviceModule::InitModule()
{
	tErrType	status = kModuleLoadFail;
	CKernelMPI	kernel;
	CEventMPI	eventmgr;
	FILE *f;
    char buf[100];
    int ret, vbus;

	dbg_.DebugOut(kDbgLvlVerbose, "USBDevice Init\n");

	data.USBDeviceSupports = kUSBDeviceIsMassStorage;
	data.USBDeviceDriver = 0;
	data.USBDeviceState = 0;

	// Check to see if the driver is enabled or not
	f = fopen(SYSFS_LUN0_PATH, "r");
	if(!f) {
		dbg_.DebugOut(kDbgLvlVerbose, "USBDevice initialization failed.\n");	  
		return;
	}
    
	ret = fscanf(f, "%s\n", buf);
	fclose(f);
	if(ret == 1) {
		// There's one string in the file.  It's the name of the mounted block
		// device.
		data.USBDeviceDriver |= kUSBDeviceIsMassStorage;
		data.USBDeviceState |= kUSBDeviceEnabled;
	}

	// Check if we're connected
	f = fopen(SYSFS_VBUS_PATH, "r");
	if(!f) {
		dbg_.DebugOut(kDbgLvlVerbose, "USBDevice initialization failed.\n");	  
		return;
	}
    
	ret = fscanf(f, "%d\n", &vbus);
	fclose(f);
	if(ret != 1) {
		// Perhaps the low level USB driver's not loaded?
		dbg_.DebugOut(kDbgLvlVerbose, "USBDevice initialization failed.\n");	  
		return;
	}
	if(vbus == 1)
		data.USBDeviceState |= kUSBDeviceConnected;

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
extern "C" char **environ;
char *enable_args[] = {"usbctl", "-d", "mass_storage", "-a", "enable", 0};

tErrType CUSBDeviceModule::EnableUSBDeviceDrivers(U32 drivers)
{
	pid_t c, ws;
	int ret;

	if(drivers & ~kUSBDeviceIsMassStorage)
		return kUSBDeviceUnsupportedDriver;

	c = fork();
	if(c == 0)
		execve("/usr/bin/usbctl", enable_args, environ);
	else if(c == -1)
		return kUSBDeviceFailure;
	else
		ws = waitpid(c, &ret, 0);

	if(WIFEXITED(ret) && !WEXITSTATUS(ret)) {
		data.USBDeviceDriver |= kUSBDeviceIsMassStorage;
		data.USBDeviceState |= kUSBDeviceEnabled;
		return kNoErr;
	} else {
		return kUSBDeviceFailure;
	}
}

//----------------------------------------------------------------------------
char *disable_args[] = {"usbctl", "-d", "mass_storage", "-a", "disable", 0};
tErrType CUSBDeviceModule::DisableUSBDeviceDrivers(U32 drivers)
{
	pid_t c, ws;
	int ret;

	if(drivers & ~kUSBDeviceIsMassStorage)
		return kUSBDeviceUnsupportedDriver;

	c = fork();
	if(c == 0)
		execve("/usr/bin/usbctl", disable_args, environ);
	else if(c == -1)
		return kUSBDeviceFailure;
	else
		ws = waitpid(c, &ret, 0);

	if(WIFEXITED(ret) && !WEXITSTATUS(ret)) {
		data.USBDeviceDriver = 0;
		data.USBDeviceState &= ~kUSBDeviceEnabled;
		return kNoErr;
	} else {
		return kUSBDeviceFailure;
	}
}

LF_END_BRIO_NAMESPACE()
// EOF
