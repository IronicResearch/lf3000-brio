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
#include <Utility.h>
#include <sys/stat.h>
#include <stdio.h>

LF_BEGIN_BRIO_NAMESPACE()

const CString	kMPIName 					= "AccelerometerMPI";
const CString	kAccelerometerModuleName	= "Accelerometer";
const CURI		kModuleURI					= "/LF/System/Accelerometer";
const tVersion	kAccelerometerModuleVersion	= 3;

const CString	SYSFS_ACLMTR_LF1000			= "/sys/devices/platform/lf1000-aclmtr/";
const CString	SYSFS_ACLMTR_LF2000			= "/sys/devices/platform/lf2000-aclmtr/";
static CString	SYSFS_ACLMTR_ROOT			= SYSFS_ACLMTR_LF2000;

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

	inline const char* SYSFS_ACLMTR_PATH(const char* path)
	{
		return CString(SYSFS_ACLMTR_ROOT + path).c_str();
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
		FILE* fd = fopen(SYSFS_ACLMTR_PATH("enable"), "w");
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
#ifdef LF1000
	SYSFS_ACLMTR_ROOT = (HasPlatformCapability(kCapsLF1000)) ? SYSFS_ACLMTR_LF1000 : SYSFS_ACLMTR_LF2000;
#endif
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
	int r = stat(SYSFS_ACLMTR_PATH("enable"), &stbuf);
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
	FILE* fd = fopen(SYSFS_ACLMTR_PATH("rate"), "r");
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
	FILE* fd = fopen(SYSFS_ACLMTR_PATH("rate"), "w");
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
	FILE* fd = fopen(SYSFS_ACLMTR_PATH("enable"), "r");
	if (fd != NULL) {
		fscanf(fd, "%u\n", &enable);
		fclose(fd);
	}
	fd = fopen(SYSFS_ACLMTR_PATH("orient"), "r");
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
	bool success = true;
	switch (mode) {
	case kAccelerometerModeDisabled:
		enable = 0;
		gbOneShot = false;
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
	
	// Set enable and orient state
	FILE* fd = fopen(SYSFS_ACLMTR_PATH("enable"), "w");
	if (fd != NULL) {
		fprintf(fd, "%u\n", enable);
		fclose(fd);
	}
	else
		success = false;
	
	fd = fopen(SYSFS_ACLMTR_PATH("orient"), "w");
	if (fd != NULL) {
		fprintf(fd, "%u\n", orient);
		fclose(fd);
	}
	else
		success = false;
	
	//Wait till driver updates data (at least one tick)
	U32 rate = GetAccelerometerRate();
	if (rate)
		usleep( 1000000 / rate );
	
	// Get initial x,y,z and orientation data if enable
	if (enable) {
		FILE* fd = fopen(SYSFS_ACLMTR_PATH("raw_xyz"), "r");
		if (fd != NULL) {
			int x = 0, y = 0, z = 0;
			fscanf(fd, "%d %d %d\n", &x, &y, &z);
			fclose(fd);
			gCachedData.accelX = x;
			gCachedData.accelY = y;
			gCachedData.accelZ = z;
		}
	}
	if (orient) {
		FILE* fd = fopen(SYSFS_ACLMTR_PATH("raw_phi"), "r");
		if (fd != NULL) {
			int phi = 0;
			fscanf(fd, "%d\n", &phi);
			fclose(fd);
			gCachedOrient = RawToOrient(phi);
		}
	}
	if( success )
		return kNoErr;
	return kNoImplErr;
}

tErrType CAccelerometerMPI::GetAccelerometerBias(S32& xoffset, S32& yoffset, S32& zoffset)
{
	//Read values directly from sysfs entry
	FILE* fd = fopen(SYSFS_ACLMTR_PATH("bias"), "r");
	if( fd != NULL ) {
		fscanf(fd, "%d %d %d", (int*)&xoffset, (int*)&yoffset, (int*)&zoffset);
		fclose(fd);
		return kNoErr;
	}
	return kNoImplErr;
}

tErrType CAccelerometerMPI::SetAccelerometerBias(S32 xoffset, S32 yoffset, S32 zoffset)
{
	//Write values directly to sysfs entry
	FILE* fd = fopen(SYSFS_ACLMTR_PATH("bias"), "w");
	if( fd != NULL ) {
		fprintf(fd, "%d %d %d", (int)xoffset, (int)yoffset, (int)zoffset);
		fclose(fd);
		return kNoErr;
	}
	return kNoImplErr;
}

LF_END_BRIO_NAMESPACE()
// EOF
