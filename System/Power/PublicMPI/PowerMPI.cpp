//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		PowerMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio 
//		Power Manager module.
//
//============================================================================

#include <PowerMPI.h>
#include <PowerPriv.h>
#include <EventMPI.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <KernelTypes.h>
#include <Utility.h>
#include <DebugMPI.h>

#include <sys/socket.h>
#include <unistd.h>
#include <sys/reboot.h>

LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "PowerMPI";

static int		gTimeoutMSec = 5000;

const CString	SYSFS_POWER_LF1000			= "/sys/devices/platform/lf1000-power/";
const CString	SYSFS_POWER_LF2000			= "/sys/devices/platform/lf2000-power/";
static CString	SYSFS_POWER_ROOT			= SYSFS_POWER_LF2000;

inline const char* SYSFS_POWER_PATH(const char* path)
{
	return CString(SYSFS_POWER_ROOT + path).c_str();
}

//============================================================================
// CPowerMessage
//============================================================================
//------------------------------------------------------------------------------
CPowerMessage::CPowerMessage( const tPowerData& data ) 
	: IEventMessage(kPowerStateChanged), mData(data)
{
	if (mData.powerState == kPowerShutdown) {
		int fd = CreateReportSocket("/tmp/video_events_socket");
		if (fd > 0) {
			struct app_message msg = {1, 4};
			send(fd, &msg, sizeof(msg), 0);
			close(fd);
		}
	}
}

//------------------------------------------------------------------------------
U16	CPowerMessage::GetSizeInBytes() const
{
	return sizeof(CPowerMessage);
}

//------------------------------------------------------------------------------
enum tPowerState CPowerMessage::GetPowerState() const
{
	return mData.powerState;
}


//============================================================================
// CPowerMPI
//============================================================================
//----------------------------------------------------------------------------
CPowerMPI::CPowerMPI() : pModule_(NULL)
{
}

//----------------------------------------------------------------------------
CPowerMPI::~CPowerMPI()
{
}

//----------------------------------------------------------------------------
Boolean	CPowerMPI::IsValid() const
{
	return true;
}

//----------------------------------------------------------------------------
const CString* CPowerMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CPowerMPI::GetModuleVersion() const
{
	return kPowerModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CPowerMPI::GetModuleName() const
{
	return &kPowerModuleName;
}

//----------------------------------------------------------------------------
const CURI* CPowerMPI::GetModuleOrigin() const
{
	return &kModuleURI;
}


//============================================================================
//----------------------------------------------------------------------------
tErrType CPowerMPI::RegisterEventListener(const IEventListener *pListener)
{
	CEventMPI eventmgr;
	return eventmgr.RegisterEventListener(pListener);
}

//----------------------------------------------------------------------------
tErrType CPowerMPI::UnregisterEventListener(const IEventListener *pListener)
{
	CEventMPI eventmgr;
	return eventmgr.UnregisterEventListener(pListener);
}

//----------------------------------------------------------------------------
enum tPowerState CPowerMPI::GetPowerState() const
{
	return GetCurrentPowerState();;
}

//----------------------------------------------------------------------------
int CPowerMPI::Shutdown() const
{
#ifndef EMULATION
	CDebugMPI debug(kGroupPower);
	debug.DebugOut(kDbgLvlCritical, "PowerMPI::Shutdown poweroff\n");
	// system("sudo /sbin/poweroff &");
	//execl("/usr/bin/sudo", "sudo", "/sbin/poweroff", NULL);
	int fd = CreateReportSocket("/tmp/video_events_socket");
	if (fd > 0) {
		struct app_message msg = {1, 4};
		send(fd, &msg, sizeof(msg), 0);
		close(fd);
	}
	//reboot(RB_POWER_OFF);
#endif
	_exit(kKernelExitShutdown);
	return kKernelExitError;
}

//----------------------------------------------------------------------------
int CPowerMPI::GetShutdownTimeMS() const
{
	return gTimeoutMSec;
}

//----------------------------------------------------------------------------
int CPowerMPI::SetShutdownTimeMS(int iMilliseconds) const
{
	gTimeoutMSec = iMilliseconds;
	return 0;
}

//----------------------------------------------------------------------------
int CPowerMPI::Reset() const
{
#ifndef EMULATION
	CDebugMPI debug(kGroupPower);
	debug.DebugOut(kDbgLvlCritical, "PowerMPI::Reset reboot\n");
	// system("sudo /sbin/reboot -f");
	// execl("/usr/bin/sudo", "sudo", "/sbin/reboot", "-f", NULL);
	//reboot(RB_AUTOBOOT);
#endif
	_exit(kKernelExitReset);
	return kKernelExitError;
}

//----------------------------------------------------------------------------
S32	CPowerMPI::GetPowerParam(tPowerParam param)
{
	FILE* fd;
	int value = 0;

	switch (param) {
	case kPowerParamBatteryMV:
		fd = fopen(SYSFS_POWER_PATH("voltage"),"r");
		if (fd) {
			fscanf(fd, "%d", &value);
			fclose(fd);
			return value;
		}
		break;
	case kPowerParamBatteryGuage:
		fd = fopen("/tmp/battery_level","r");
		if (fd) {
			fscanf(fd, "%d", &value);
			fclose(fd);
			return value;
		}
		break;
	case kPowerParamMaxBatteryMV:
		fd = fopen(SYSFS_POWER_PATH("max_battery_mv"),"r");
		if (fd) {
			fscanf(fd, "%d", &value);
			fclose(fd);
			return value;
		}
		break;
	case kPowerParamNormalBatteryMV:
		fd = fopen(SYSFS_POWER_PATH("normal_battery_mv"),"r");
		if (fd) {
			fscanf(fd, "%d", &value);
			fclose(fd);
			return value;
		}
		break;
	case kPowerParamLowBatteryMV:
		fd = fopen(SYSFS_POWER_PATH("low_battery_mv"),"r");
		if (fd) {
			fscanf(fd, "%d", &value);
			fclose(fd);
			return value;
		}
		break;
	case kPowerParamCriticalBatteryMV:
		fd = fopen(SYSFS_POWER_PATH("critical_battery_mv"),"r");
		if (fd) {
			fscanf(fd, "%d", &value);
			fclose(fd);
			return value;
		}
		break;
	}
	return 0;
}

//----------------------------------------------------------------------------
Boolean CPowerMPI::SetPowerParam(tPowerParam param, S32 value)
{
	return false;
}

//----------------------------------------------------------------------------
LF_END_BRIO_NAMESPACE()

// EOF
