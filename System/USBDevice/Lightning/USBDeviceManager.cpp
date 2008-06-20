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

#define SYSFS_LUN0_PATH "/sys/devices/platform/lf1000-usbgadget/gadget/gadget-lun0/enabled"
#define SYSFS_LUN1_PATH "/sys/devices/platform/lf1000-usbgadget/gadget/gadget-lun1/enabled"
#define SYSFS_VBUS_PATH "/sys/devices/platform/lf1000-usbgadget/vbus"
#define SYSFS_WD_PATH "/sys/devices/platform/lf1000-usbgadget/watchdog_seconds"
#define FLAGS_VBUS_PATH "/flags/vbus"


//============================================================================
// Local state and utility functions
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	tTaskHndl		USBDeviceTask;
	tUSBDeviceData	data;
	int				mtd_num;
	CDebugMPI		*pDebugMPI_;
	tMutex dataMutex = PTHREAD_MUTEX_INITIALIZER;
	// Please don't access data without this mutex
}

static int get_vbus(void)
{
	FILE *f;
	int ret, vbus;
	char *env_vbus;
	char *end_ptr;

	/* use /flags/vbus setting if present */
	f = fopen(FLAGS_VBUS_PATH, "r");
	if (f) {	/* found file */
		ret = fscanf(f, "%d\n", &vbus);
		fclose(f);
		if (ret != 1)
			return -1;
		pDebugMPI_->DebugOut(kDbgLvlVerbose,
				"%s: %s=%d\n", __FUNCTION__, FLAGS_VBUS_PATH, vbus);
		return(vbus);
	}
			
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

//============================================================================
// Asynchronous notifications
//============================================================================
//----------------------------------------------------------------------------
void *LightningUSBDeviceTask(void*)
{
	CEventMPI	eventmgr;
	CKernelMPI	kernel;
	int vbus;
	int ls, ms;
	int size;
	struct sockaddr_un mon;
	socklen_t s_mon = sizeof(struct sockaddr_un);
	struct app_message msg;

	pDebugMPI_->DebugOut(kDbgLvlVerbose, "%s: Started\n", __FUNCTION__);

	ls = CreateListeningSocket(USB_SOCK);
	pDebugMPI_->Assert(ls >= 0, "can't open listening socket");

	while(1) {
		ms = accept(ls, (struct sockaddr *)&mon, &s_mon);
		if(ms > 0) { /* receiving a message */
			while(1) {
				size = recv(ms, &msg, sizeof(msg), 0);
				if(size == sizeof(msg) && msg.type == APP_MSG_SET_USB)
					vbus = !!msg.payload;
				else
					break;
			}
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
	int vbus, enabled;

	pDebugMPI_ = new CDebugMPI(kGroupUSBDevice);
	pDebugMPI_->SetDebugLevel(kDbgLvlVerbose);	

	pDebugMPI_->DebugOut(kDbgLvlVerbose, "USBDevice Init\n");

	data.USBDeviceSupports = kUSBDeviceIsMassStorage;
	data.USBDeviceDriver = 0;
	data.USBDeviceState = 0;

	/* On lightning, the app can't be running while USB is enabled! */
	enabled = (is_enabled() == 1);	// -1 does not count as enabled
	pDebugMPI_->Assert(!enabled, "Lightning can't run apps while USB enabled");

	// Check if we're connected
	vbus = get_vbus();
	if(vbus == -1) {
		// Perhaps the low level USB driver's not loaded?
		pDebugMPI_->DebugOut(kDbgLvlVerbose, "USBDevice initialization failed.\n");	  
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
	pDebugMPI_->Assert( status == kNoErr, 
				"CUSBDeviceModule::InitModule: background task creation failed" );
}

//----------------------------------------------------------------------------
void CUSBDeviceModule::DeinitModule()
{
	CKernelMPI	kernel;
	void* 		retval;

	pDebugMPI_->DebugOut(kDbgLvlVerbose, "USBDeviceModule::DeinitModule\n");

	// Terminate handler thread, and wait before closing driver
	kernel.CancelTask(USBDeviceTask);
	kernel.JoinTask(USBDeviceTask, retval);
	delete pDebugMPI_;
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

	CPowerMPI power;

	if(drivers & ~kUSBDeviceIsMassStorage)
		return kUSBDeviceUnsupportedDriver;

	/* This is a one-way ticket on Lightning */
	return power.Shutdown();
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
		
	/* On lightning, the app can't be running while USB is enabled! */
	pDebugMPI_->Assert(!enabled, "Lightning can't run apps while USB enabled");

	if(ret == kNoErr) {
		data.USBDeviceDriver = 0;
		data.USBDeviceState &= ~kUSBDeviceEnabled;
		return kNoErr;
	}
	
	return kUSBDeviceFailure;
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
