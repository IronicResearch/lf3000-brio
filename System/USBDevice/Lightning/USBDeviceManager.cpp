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
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

LF_BEGIN_BRIO_NAMESPACE()

#define SYSFS_LUN0_PATH "/sys/devices/platform/lf1000-usbgadget/gadget/gadget-lun0/enabled"
#define SYSFS_LUN1_PATH "/sys/devices/platform/lf1000-usbgadget/gadget/gadget-lun1/enabled"
#define SYSFS_VBUS_PATH "/sys/devices/platform/lf1000-usbgadget/vbus"

//============================================================================
// Local state and utility functions
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	tTaskHndl		USBDeviceTask;
	tUSBDeviceData	data;
	int mtd_num;
	tMutex dataMutex = PTHREAD_MUTEX_INITIALIZER;
	// Please don't access data without this mutex
}

static int get_vbus(void)
{
	FILE *f;
	int ret, vbus;

	f = fopen(SYSFS_VBUS_PATH, "r");
	if(!f)
		return -1;
	
	ret = fscanf(f, "%d\n", &vbus);
	if(ret != 1)
		return -1;
	fclose(f);
	return vbus;
}

static int is_enabled()
{
	FILE *f;
	int enabled0 = 0, enabled1 = 0;
	int ret = 0;
	
	/* First check LUN0 */
	f = fopen(SYSFS_LUN0_PATH, "r");
	if(!f)
		return -1;
	
	ret = fscanf(f, "%d\n", &enabled0);
	fclose(f);
	if(ret != 1)
		return -1;

	/* LUN1 may or may not exist */
	ret = 0;
	f = fopen(SYSFS_LUN1_PATH, "r");
	if(!f && errno == ENOENT)
		return enabled0;
	else if(!f)
		return -1;
	else
		ret = fscanf(f, "%d\n", &enabled1);
	fclose(f);
	if (ret != 1)
		return -1;

	if(enabled0 == 1 && enabled1 == 1)
		return 1;
	else
		return 0;
}

static tErrType enable()
{
	FILE *f;
	int ret, len;
	char *buf = "1";

	f = fopen(SYSFS_LUN0_PATH, "w");
	if(!f)
		return kUSBDeviceFailure;

	len = strlen(buf);
	ret = fwrite(buf, 1, len, f);
	fclose(f);
	if(ret != len)
		return kUSBDeviceFailure;

	/* LUN1 may not exist */
	f = fopen(SYSFS_LUN1_PATH, "w");
	if(!f && errno == ENOENT)
		return kNoErr;
	else if(!f)
		return kUSBDeviceFailure;
	else
		ret = fwrite(buf, 1, len, f);
	fclose(f);

	if(ret != len)
		return kUSBDeviceFailure;

	return kNoErr;
}

static tErrType disable()
{
	FILE *f;
	int ret, len;
	char *buf = "0";

	f = fopen(SYSFS_LUN0_PATH, "w");
	if(!f)
		return kUSBDeviceFailure;

	len = strlen(buf);
	ret = fwrite(buf, 1, len, f);
	fclose(f);
	if(ret != len)
		return kUSBDeviceFailure;

	/* LUN1 may not exist */
	f = fopen(SYSFS_LUN1_PATH, "w");
	if(!f && errno == ENOENT)
		return kNoErr;
	else if(!f)
		return kUSBDeviceFailure;
	else
		ret = fwrite(buf, 1, len, f);
	fclose(f);

	if(ret != len)
		return kUSBDeviceFailure;

	return kNoErr;
}

//============================================================================
// Asynchronous notifications
//============================================================================
//----------------------------------------------------------------------------
void *LightningUSBDeviceTask(void*)
{
	CEventMPI	eventmgr;
	CDebugMPI	dbg(kGroupUSBDevice);
	CKernelMPI	kernel;
	int vbus;

	dbg.DebugOut(kDbgLvlVerbose, "%s: Started\n", __FUNCTION__);
	
	while(1) {
		// Just check the vbus status every so often
		vbus = get_vbus();
		if(vbus == -1) {
			dbg.DebugOut(kDbgLvlVerbose, "USBDevice can't get VBUS state!\n");
			continue;
		}
		
		// Did the connection state change?
		kernel.LockMutex(dataMutex);
		if((vbus == 1) && !(data.USBDeviceState & kUSBDeviceConnected)) {
			data.USBDeviceState |= kUSBDeviceConnected;
			CUSBDeviceMessage msg(data);
			eventmgr.PostEvent(msg, kUSBDeviceEventPriority);
		} else if((vbus == 0) && (data.USBDeviceState & kUSBDeviceConnected)) {
			data.USBDeviceState &= ~(kUSBDeviceConnected);
			CUSBDeviceMessage msg(data);
			eventmgr.PostEvent(msg, kUSBDeviceEventPriority);
		}
		kernel.UnlockMutex(dataMutex);

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
	int vbus;

	pDbg_->DebugOut(kDbgLvlVerbose, "USBDevice Init\n");

	data.USBDeviceSupports = kUSBDeviceIsMassStorage;
	data.USBDeviceDriver = 0;
	data.USBDeviceState = 0;

	if(is_enabled()) {
		data.USBDeviceDriver |= kUSBDeviceIsMassStorage;
		data.USBDeviceState |= kUSBDeviceEnabled;
	}

	// Check if we're connected
	vbus = get_vbus();
	if(vbus == -1) {
		// Perhaps the low level USB driver's not loaded?
		pDbg_->DebugOut(kDbgLvlVerbose, "USBDevice initialization failed.\n");	  
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
	pDbg_->Assert( status == kNoErr, 
				"CUSBDeviceModule::InitModule: background task creation failed" );
}

//----------------------------------------------------------------------------
void CUSBDeviceModule::DeinitModule()
{
	CKernelMPI	kernel;

	pDbg_->DebugOut(kDbgLvlVerbose, "USBDeviceModule::DeinitModule\n");

	// Terminate handler thread, and wait before closing driver
	kernel.CancelTask(USBDeviceTask);
	kernel.TaskSleep(1);

	kernel.LockMutex(dataMutex);
	kernel.DeInitMutex(dataMutex);
}

//----------------------------------------------------------------------------
tUSBDeviceData CUSBDeviceModule::GetUSBDeviceState() const
{
	tUSBDeviceData ret;
	CKernelMPI	kernel;

	kernel.LockMutex(dataMutex);
	ret = data;
	kernel.UnlockMutex(dataMutex);

	return ret;
}

//----------------------------------------------------------------------------
tErrType CUSBDeviceModule::EnableUSBDeviceDrivers(U32 drivers)
{
	int enabled;
	int ret = kNoErr;
	
	if(drivers & ~kUSBDeviceIsMassStorage)
		return kUSBDeviceUnsupportedDriver;

	enabled = is_enabled();
	if(enabled == -1)
		return kUSBDeviceFailure;
		
	if(!enabled)
		ret = enable();
	
	if(ret == kNoErr) {
		data.USBDeviceDriver |= kUSBDeviceIsMassStorage;
		data.USBDeviceState |= kUSBDeviceEnabled;
		return kNoErr;
	}

	return kUSBDeviceFailure;
}

//----------------------------------------------------------------------------
tErrType CUSBDeviceModule::DisableUSBDeviceDrivers(U32 drivers)
{
	int enabled;
	int ret = kNoErr;

	if(drivers & ~kUSBDeviceIsMassStorage)
		return kUSBDeviceUnsupportedDriver;

	enabled = is_enabled();
	if(enabled == -1)
		return kUSBDeviceFailure;
		
	if(enabled)
		ret = disable();
	
	if(ret == kNoErr) {
		data.USBDeviceDriver = 0;
		data.USBDeviceState &= ~kUSBDeviceEnabled;
		return kNoErr;
	}
	
	return kUSBDeviceFailure;
}

LF_END_BRIO_NAMESPACE()
// EOF
