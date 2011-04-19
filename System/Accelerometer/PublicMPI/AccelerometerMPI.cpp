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
#include <DebugMPI.h>
#include <Module.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <DisplayTypes.h>
#include <sys/stat.h>
#include <stdio.h>

LF_BEGIN_BRIO_NAMESPACE()

const CString	kMPIName 					= "AccelerometerMPI";
const CString	kAccelerometerModuleName	= "Accelerometer";
const CURI		kModuleURI					= "/LF/System/Accelerometer";
const tVersion	kAccelerometerModuleVersion	= 3;

static tAccelerometerData 	gCachedData 	= {0, 0, 0, {0, 0}};
static S32					gCachedOrient	= 0;
static bool					gbOneShot		= false;

//============================================================================
namespace
{
	inline int RawToOrient(int raw)
	{
		switch (raw)
		{
			case 6:
			case 2:
				return kOrientationLandscape;
			case 4:
			case 0:
				return kOrientationPortrait;
			case 7:
			case 3:
				return kOrientationLandscapeUpsideDown;
			case 5:
			case 1:
				return kOrientationPortraitUpsideDown;
		}
		return 0;
	}
}

//============================================================================
// CAccelerometerMessage
//============================================================================
//------------------------------------------------------------------------------
CAccelerometerMessage::CAccelerometerMessage( const tAccelerometerData& data )
	: IEventMessage(kAccelerometerDataChanged), mData(data)
{
	gCachedData = data;
	if (gbOneShot) {
		FILE* fd = fopen("/sys/devices/platform/lf1000-aclmtr/enable", "w");
		if (fd != NULL) {
			fprintf(fd, "%u\n", 0);
			fclose(fd);
		}
	}
}

//------------------------------------------------------------------------------
CAccelerometerMessage::CAccelerometerMessage( const S32& data )
	: IEventMessage(kOrientationChanged), mOrient(data)
{
	gCachedOrient = mOrient = RawToOrient(data);
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

//------------------------------------------------------------------------------
S32 CAccelerometerMessage::GetOrientation() const
{
	return mOrient;
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
	CDebugMPI dbg(kGroupAccelerometer);
	struct stat stbuf;
	int r = stat("/sys/devices/platform/lf1000-aclmtr", &stbuf);
	dbg.DebugOut(kDbgLvlImportant, "IsAccelerometerPresent: %d\n", (r == 0));
	return (r == 0) ? true : false;
}

//----------------------------------------------------------------------------
tAccelerometerData CAccelerometerMPI::GetAccelerometerData() const
{
	return gCachedData;
}

//----------------------------------------------------------------------------
S32 CAccelerometerMPI::GetOrientation() const
{
	return gCachedOrient;
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
	int orient = 0;
	FILE* fd = fopen("/sys/devices/platform/lf1000-aclmtr/enable", "r");
	if (fd != NULL) {
		fscanf(fd, "%u\n", &enable);
		fclose(fd);
	}
	fd = fopen("/sys/devices/platform/lf1000-aclmtr/orient", "r");
	if (fd != NULL) {
		fscanf(fd, "%u\n", &orient);
		fclose(fd);
	}
	if (gbOneShot)
		return kAccelerometerModeOneShot;
	if (enable && orient)
		return kAccelerometerModeOrientation;
	if (enable)
		return kAccelerometerModeContinuous;
	return kAccelerometerModeDisabled;
}

//----------------------------------------------------------------------------
tErrType CAccelerometerMPI::SetAccelerometerMode(tAccelerometerMode mode)
{
	int enable = 0;
	int orient = 0;
	switch (mode) {
	case kAccelerometerModeDisabled:
		enable = 0;
		break;
	case kAccelerometerModeOrientation:
		orient = 1;
	case kAccelerometerModeContinuous:
		enable = 1;
		gbOneShot = false;
		break;
	case kAccelerometerModeOneShot:
		enable = 1;
		gbOneShot = true;
		break;
	}
	FILE* fd = fopen("/sys/devices/platform/lf1000-aclmtr/enable", "w");
	if (fd != NULL) {
		fprintf(fd, "%u\n", enable);
		fclose(fd);
	}
	fd = fopen("/sys/devices/platform/lf1000-aclmtr/orient", "w");
	if (fd != NULL) {
		fprintf(fd, "%u\n", orient);
		fclose(fd);
		return kNoErr;
	}
	return kNoImplErr;
}

LF_END_BRIO_NAMESPACE()
// EOF
