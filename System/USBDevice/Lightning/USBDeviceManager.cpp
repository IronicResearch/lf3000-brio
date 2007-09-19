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

LF_BEGIN_BRIO_NAMESPACE()

#define SYSFS_LUN0_PATH "/sys/devices/platform/lf1000-usbgadget/gadget/gadget-lun0/file"
#define SYSFS_LUN1_PATH "/sys/devices/platform/lf1000-usbgadget/gadget/gadget-lun1/file"
#define SYSFS_VBUS_PATH "/sys/devices/platform/lf1000-usbgadget/vbus"
#define UBI_VOL0_NAME "Brio_Base"
#define UBI_VOL1_NAME "Application_Area"

//============================================================================
// Local state and utility functions
//============================================================================
namespace
{
	//------------------------------------------------------------------------
	tTaskHndl		USBDeviceTask;
	tUSBDeviceData	data;
	int mtd_num0, mtd_num1;
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

static int is_enabled(int lun)
{
	FILE *f;
	char buf[100];
	int ret;

	f = fopen(lun == 0 ? SYSFS_LUN0_PATH : SYSFS_LUN1_PATH, "r");
	if(!f)
		return -1;
	
	ret = fscanf(f, "%s\n", buf);
	if(ret == 1)
		return 1;
	else
		return 0;
}

static void find_mtds(void)
{
	FILE *f;
	char buf1[256], buf2[256], name[256];
	char *prev, *cur, *tmp;
	int mtd_num, len;
	unsigned int size, erase;

	mtd_num0 = mtd_num1 = -1;

	f = fopen("/proc/mtd", "r");
	if(!f)
		return;
	
	// read off the first line.
	if(fgets(buf1, 256, f) == (char *)EOF)
		return;

	prev = buf1;
	prev[0] = 0;
	cur = buf2;
	while(fgets(cur, 256, f) != (char *)EOF) {

		//For some reason, sysfs files don't return EOF.  They just keep
		//returning the last line of the file.
		if(strcmp(cur, prev) == 0)
			break;
		sscanf(cur, "mtd%d: %x %x \"%s\"\n", &mtd_num, &size, &erase, name);
		//Chomp any trailing double quote
		len = strlen(name);
		if(name[len-1] == '"')
			name[len-1] = 0;

		if(strcmp(name, UBI_VOL0_NAME) == 0)
			mtd_num0 = mtd_num;
		else if(strcmp(name, UBI_VOL1_NAME) == 0)
			mtd_num1 = mtd_num;
		tmp = prev;
		prev = cur;
		cur = tmp;
		cur[0] = 0;
	}

	return;
}

static tErrType enable_lun(int lun)
{
	FILE *f;
	char buf[100];
	int ret, len;

	f = fopen(lun == 0 ? SYSFS_LUN0_PATH : SYSFS_LUN1_PATH, "w");
	if(!f)
		return kUSBDeviceFailure;

	sprintf(buf, "/dev/mtdblock%d", lun == 0 ? mtd_num0 : mtd_num1);
	len = strlen(buf);
	ret = fwrite(buf, 1, len, f);
	fclose(f);
	if(ret != len)
		return kUSBDeviceFailure;
	else
		return kNoErr;
}

static tErrType disable_lun(int lun)
{
	FILE *f;

	f = fopen(lun == 0 ? SYSFS_LUN0_PATH : SYSFS_LUN1_PATH, "w");
	if(!f)
		return kUSBDeviceFailure;
	fclose(f);
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

	dbg_.DebugOut(kDbgLvlVerbose, "USBDevice Init\n");

	data.USBDeviceSupports = kUSBDeviceIsMassStorage;
	data.USBDeviceDriver = 0;
	data.USBDeviceState = 0;

	if(is_enabled(0) && is_enabled(1)) {
		data.USBDeviceDriver |= kUSBDeviceIsMassStorage;
		data.USBDeviceState |= kUSBDeviceEnabled;
	}

	// Check if we're connected
	vbus = get_vbus();
	if(vbus == -1) {
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
	int lun0_enabled, lun1_enabled;
	int ret0 = kNoErr, ret1 = kNoErr;
	
	if(drivers & ~kUSBDeviceIsMassStorage)
		return kUSBDeviceUnsupportedDriver;

	find_mtds();
	if(mtd_num0 == -1 || mtd_num1 == -1)
		return kUSBDeviceFailure;

	lun0_enabled = is_enabled(0);
	lun1_enabled = is_enabled(1);
	if(lun0_enabled == -1 || lun1_enabled == -1)
		return kUSBDeviceFailure;
		
	if(lun0_enabled == 1 && lun1_enabled == 1) {
		data.USBDeviceDriver |= kUSBDeviceIsMassStorage;
		data.USBDeviceState |= kUSBDeviceEnabled;
		return kNoErr;
	}
	
	if(!lun0_enabled)
		ret0 = enable_lun(0);
	
	if(!lun1_enabled)
		ret1 = enable_lun(1);

	if(ret0 == kNoErr && ret1 == kNoErr) {
		data.USBDeviceDriver |= kUSBDeviceIsMassStorage;
		data.USBDeviceState |= kUSBDeviceEnabled;
		return kNoErr;
	}

	return kUSBDeviceFailure;
}

//----------------------------------------------------------------------------
tErrType CUSBDeviceModule::DisableUSBDeviceDrivers(U32 drivers)
{
	int lun0_enabled, lun1_enabled;
	int ret0 = kNoErr, ret1 = kNoErr;

	if(drivers & ~kUSBDeviceIsMassStorage)
		return kUSBDeviceUnsupportedDriver;

	lun0_enabled = is_enabled(0);
	lun1_enabled = is_enabled(1);
	if(lun0_enabled == -1 || lun1_enabled == -1)
		return kUSBDeviceFailure;
		
	if(lun0_enabled == 0 && lun1_enabled == 0)
			return kNoErr;
	
	if(lun0_enabled)
		ret0 = disable_lun(0);
	
	if(lun1_enabled)
		ret1 = disable_lun(1);

	if(ret0 == kNoErr && ret1 == kNoErr) {
		data.USBDeviceDriver = 0;
		data.USBDeviceState &= ~kUSBDeviceEnabled;
		return kNoErr;
	}
	
	return kUSBDeviceFailure;
}

LF_END_BRIO_NAMESPACE()
// EOF
