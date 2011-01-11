//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		AccelerometerMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio 
//		Accelerometer module. Lightweight MPI has no underlying module lib.
//
//============================================================================

#include <AccelerometerTypes.h>
#include <AccelerometerMPI.h>
#include <EventMPI.h>
#include <Module.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <sys/stat.h>
#include <stdio.h>

LF_BEGIN_BRIO_NAMESPACE()

const CString	kMPIName 					= "AccelerometerMPI";
const CString	kAccelerometerModuleName	= "Accelerometer";
const CURI		kModuleURI					= "/LF/System/Accelerometer";
const tVersion	kAccelerometerModuleVersion	= 3;

//============================================================================
// CAccelerometerMessage
//============================================================================
//------------------------------------------------------------------------------
CAccelerometerMessage::CAccelerometerMessage( const tAccelerometerData& data )
	: IEventMessage(kAccelerometerDataChanged), mData(data)
{
}

//------------------------------------------------------------------------------
U16	CAccelerometerMessage::GetSizeInBytes() const
{
	return sizeof(CAccelerometerMessage);
}

//------------------------------------------------------------------------------
tAccelerometerData CAccelerometerMessage::GetAccelerometerData() const
{
	return mData;
}

//============================================================================
// CAccelerometerMPI
//============================================================================
//----------------------------------------------------------------------------
CAccelerometerMPI::CAccelerometerMPI() : pModule_(NULL)
{
}

//----------------------------------------------------------------------------
CAccelerometerMPI::~CAccelerometerMPI()
{
}

//----------------------------------------------------------------------------
Boolean	CAccelerometerMPI::IsValid() const
{
	return true;
}

//----------------------------------------------------------------------------
const CString* CAccelerometerMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CAccelerometerMPI::GetModuleVersion() const
{
	return kAccelerometerModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CAccelerometerMPI::GetModuleName() const
{
	return &kAccelerometerModuleName;
}

//----------------------------------------------------------------------------
const CURI* CAccelerometerMPI::GetModuleOrigin() const
{
	return &kModuleURI;
}


//============================================================================
//----------------------------------------------------------------------------
tErrType CAccelerometerMPI::RegisterEventListener(const IEventListener *pListener)
{
	CEventMPI evtmgr;
	if (GetAccelerometerMode() == kAccelerometerModeDisabled)
		SetAccelerometerMode(kAccelerometerModeContinuous);
	return evtmgr.RegisterEventListener(pListener);
}

//----------------------------------------------------------------------------
tErrType CAccelerometerMPI::UnregisterEventListener(const IEventListener *pListener)
{
	CEventMPI evtmgr;
	SetAccelerometerMode(kAccelerometerModeDisabled);
	return evtmgr.UnregisterEventListener(pListener);
}

//----------------------------------------------------------------------------
Boolean	CAccelerometerMPI::IsAccelerometerPresent()
{
	struct stat stbuf;
	int r = stat("/sys/devices/platform/lf1000-aclmtr", &stbuf);
	return (r == 0) ? true : false;
}

//----------------------------------------------------------------------------
tAccelerometerData CAccelerometerMPI::GetAccelerometerData() const
{
	const tAccelerometerData data = {0, 0, 0, {0, 0}};
	return data;
}

//----------------------------------------------------------------------------
U32	CAccelerometerMPI::GetAccelerometerRate() const
{
	U32 rate = 0;
	FILE* fd = fopen("/sys/devices/platform/lf1000-aclmtr/rate", "r");
	if (fd != NULL) {
		fscanf(fd, "%u\n", (unsigned int*)&rate);
		fclose(fd);
		return rate;
	}
	return 0;
}

//----------------------------------------------------------------------------
tErrType CAccelerometerMPI::SetAccelerometerRate(U32 rate)
{
	FILE* fd = fopen("/sys/devices/platform/lf1000-aclmtr/rate", "w");
	if (fd != NULL) {
		fprintf(fd, "%u\n", (unsigned int)rate);
		fclose(fd);
		return kNoErr;
	}
	return kNoImplErr;
}

//----------------------------------------------------------------------------
tAccelerometerMode CAccelerometerMPI::GetAccelerometerMode() const
{
	int enable = 0;
	FILE* fd = fopen("/sys/devices/platform/lf1000-aclmtr/enable", "r");
	if (fd != NULL) {
		fscanf(fd, "%u\n", &enable);
		fclose(fd);
		return (enable) ? kAccelerometerModeContinuous : kAccelerometerModeDisabled;
	}
	return static_cast<tAccelerometerMode>(0);
}

//----------------------------------------------------------------------------
tErrType CAccelerometerMPI::SetAccelerometerMode(tAccelerometerMode mode)
{
	int enable = 0;
	switch (mode) {
	case kAccelerometerModeDisabled:
		enable = 0;
		break;
	case kAccelerometerModeContinuous:
	case kAccelerometerModeOneShot:
	case kAccelerometerModeOrientation:
		enable = 1;
		break;
	}
	FILE* fd = fopen("/sys/devices/platform/lf1000-aclmtr/enable", "w");
	if (fd != NULL) {
		fprintf(fd, "%u\n", enable);
		fclose(fd);
		return kNoErr;
	}
	return kNoImplErr;
}

LF_END_BRIO_NAMESPACE()
// EOF
