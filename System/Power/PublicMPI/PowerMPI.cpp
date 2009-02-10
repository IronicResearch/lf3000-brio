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

#include <sys/socket.h>

LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "PowerMPI";


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
	// Embedded version should never get here
	exit(kKernelExitShutdown);
	return kKernelExitError;
}

//----------------------------------------------------------------------------
int CPowerMPI::GetShutdownTimeMS() const
{
#ifndef EMULATION
	struct app_message msg, resp;
	int size;
	int s = CreateReportSocket(MONITOR_SOCK);

	msg.type = APP_MSG_GET_POWER;
	msg.payload = 0;

	if(send(s, &msg, sizeof(msg), 0) < 0) {
		close(s);
		return -1;
	}

	size = recv(s, &resp, sizeof(struct app_message), 0);
	if(size != sizeof(struct app_message)) {
		close(s);
		return -1;
	}

	close(s);
	return resp.payload*1000;
#else
	return 0;
#endif
}

//----------------------------------------------------------------------------
int CPowerMPI::SetShutdownTimeMS(int iMilliseconds) const
{
#ifndef EMULATION
	struct app_message msg;
	int s = CreateReportSocket(MONITOR_SOCK);

	msg.type = APP_MSG_SET_POWER;
	if(iMilliseconds == 0)
		msg.payload = 0;
	else
		msg.payload = iMilliseconds >= 1000 ? iMilliseconds/1000 : 1;

	send(s, &msg, sizeof(msg), MSG_NOSIGNAL);
	close(s);
#endif
	return 0;
}

//----------------------------------------------------------------------------
int CPowerMPI::Reset() const
{
	// Embedded version should never get here
	exit(kKernelExitReset);
	return kKernelExitError;
}

LF_END_BRIO_NAMESPACE()

// EOF
