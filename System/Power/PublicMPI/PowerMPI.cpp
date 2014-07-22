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
// CKeepAliveDelegate
//============================================================================
//------------------------------------------------------------------------------	
struct CKeepAliveDelegate {
	CKeepAliveDelegate(class CPowerMPI* pPowerMPI);
	~CKeepAliveDelegate();

	static void KeepAlive();
	static void GetInactivityTimeouts(int& firstInterval, int& secondInterval);

private:
	void InitializeKeepAlive();
	static void GetActivityTimeoutIntervalValues(int& firstInterval, int& secondInterval);
			
private:
	static const int ONE_SECOND_IN_MS = 1*1000;
	static const int ONE_MINUTE_IN_MS = 1*60*1000;
	static const int DISABLED_INTERVAL = 0;

	static long int	mLastEventPostTime;
	static long int	mKeepAliveLoopTime; 
	
	static int mFirstInterval;
	static int mSecondInterval;
};

//------------------------------------------------------------------------------	
long int CKeepAliveDelegate::mLastEventPostTime = 0;
long int CKeepAliveDelegate::mKeepAliveLoopTime = 0; 
int CKeepAliveDelegate::mFirstInterval = 0;
int CKeepAliveDelegate::mSecondInterval = 0; 
	
//------------------------------------------------------------------------------	
CKeepAliveDelegate::CKeepAliveDelegate(CPowerMPI* powerMPI)
{
	InitializeKeepAlive();
}

//------------------------------------------------------------------------------
CKeepAliveDelegate::~CKeepAliveDelegate()
{
}

//------------------------------------------------------------------------------
void CKeepAliveDelegate::InitializeKeepAlive()
{
	mLastEventPostTime = time(NULL)*1000; // Current time in ms

	GetActivityTimeoutIntervalValues(mFirstInterval, mSecondInterval);
	if (mFirstInterval <= DISABLED_INTERVAL && mSecondInterval <= DISABLED_INTERVAL) {
		mKeepAliveLoopTime = DISABLED_INTERVAL;
	}
	else {
		mKeepAliveLoopTime = ((mFirstInterval < ONE_MINUTE_IN_MS) ? ONE_SECOND_IN_MS : ONE_MINUTE_IN_MS);
	}
	
	// Set minimum second interval to 1 minute
	if (mSecondInterval < ONE_MINUTE_IN_MS) 
		mSecondInterval = ONE_MINUTE_IN_MS;
}

//------------------------------------------------------------------------------
void CKeepAliveDelegate::KeepAlive()
{
	// If timeout interval values in /flags/idle_timeouts is 0, timer is disabled, 
	// no kPowerKeepAlive event will be posted.
	if (mKeepAliveLoopTime == DISABLED_INTERVAL) return;
		
	// Post kPowerKeepAlive event at most once a second or once a minute, 
	// depending on timeout interval values in /flags/idle_timeouts file
	if (time(NULL)*1000 - mLastEventPostTime > mKeepAliveLoopTime)
	{
		CDebugMPI debugMPI(kGroupPower);
		debugMPI.DebugOut(kDbgLvlImportant, "\nPost kPowerKeepAlive event\n");	
		
		CPowerKeepAliveMessage msg(kPowerKeepAlive);
		LeapFrog::Brio::CEventMPI eventMPI;
		eventMPI.PostEvent(msg, 128);
		
		mLastEventPostTime = time(NULL)*1000;
	}
}

//------------------------------------------------------------------------------
void CKeepAliveDelegate::GetInactivityTimeouts(int& firstInterval, int& secondInterval)
{
	firstInterval = mFirstInterval;
	secondInterval = mSecondInterval;
}
	
//------------------------------------------------------------------------------
void CKeepAliveDelegate::GetActivityTimeoutIntervalValues(int& firstInterval, int& secondInterval)
{
	// Setting default timer intervals to (30 mins / 60 mins) in ms.
	firstInterval = 30*ONE_MINUTE_IN_MS; 
	secondInterval = 60*ONE_MINUTE_IN_MS;
	
	FILE *flag = fopen("/flags/idle_timeouts", "r");
	if(flag)
	{
		fscanf(flag, "%d %d", &firstInterval, &secondInterval);
		fclose(flag);
	}
}

//============================================================================
// CPowerKeepAliveMessage
//============================================================================
//------------------------------------------------------------------------------
CPowerKeepAliveMessage::CPowerKeepAliveMessage(tEventType type) 
	: IEventMessage(type)
{
}

//------------------------------------------------------------------------------
U16	CPowerKeepAliveMessage::GetSizeInBytes() const
{
	return sizeof(CPowerKeepAliveMessage);
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
CKeepAliveDelegate* CPowerMPI::pKeepAliveDelegate = 0;

//----------------------------------------------------------------------------
CPowerMPI::CPowerMPI() : pModule_(NULL)
{
	pKeepAliveDelegate = new CKeepAliveDelegate(this);
}

//----------------------------------------------------------------------------
CPowerMPI::~CPowerMPI()
{
	delete pKeepAliveDelegate;
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
void CPowerMPI::KeepAlive()
{	
	if (pKeepAliveDelegate)
		pKeepAliveDelegate->KeepAlive();
}

//------------------------------------------------------------------------------
void CPowerMPI::GetInactivityTimeouts(int& firstInterval, int& secondInterval)
{
	if (pKeepAliveDelegate)
		pKeepAliveDelegate->GetInactivityTimeouts(firstInterval, secondInterval);
}

//----------------------------------------------------------------------------
LF_END_BRIO_NAMESPACE()

// EOF
