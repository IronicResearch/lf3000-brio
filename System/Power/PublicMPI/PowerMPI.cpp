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

LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "PowerMPI";

static int		gTimeoutMSec = 5000;

//============================================================================
// CPowerMessage
//============================================================================
//------------------------------------------------------------------------------
CPowerMessage::CPowerMessage( const tPowerData& data ) 
	: IEventMessage(kPowerStateChanged), mData(data)
{
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
#endif
	// Embedded version should never get here
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
	execl("/usr/bin/sudo", "sudo", "/sbin/reboot", "-f", NULL);
#endif
	// Embedded version should never get here
	_exit(kKernelExitShutdown);
	return kKernelExitError;
}

LF_END_BRIO_NAMESPACE()

// EOF
