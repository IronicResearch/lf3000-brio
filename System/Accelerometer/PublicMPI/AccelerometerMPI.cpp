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
//		Accelerometer Manager module.
//
//============================================================================

#include <AccelerometerTypes.h>
#include <AccelerometerMPI.h>
#include <Module.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "AccelerometerMPI";


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
//	pModule_ = new CAccelerometerModule();
}

//----------------------------------------------------------------------------
CAccelerometerMPI::~CAccelerometerMPI()
{
//	delete pModule_;
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
//	return kAccelerometerModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CAccelerometerMPI::GetModuleName() const
{
//	return &kAccelerometerModuleName;
}

//----------------------------------------------------------------------------
const CURI* CAccelerometerMPI::GetModuleOrigin() const
{
//	return &kModuleURI;
}


//============================================================================
//----------------------------------------------------------------------------
tErrType CAccelerometerMPI::RegisterEventListener(const IEventListener *pListener)
{
	return kNoImplErr;
}

//----------------------------------------------------------------------------
tErrType CAccelerometerMPI::UnregisterEventListener(const IEventListener *pListener)
{
	return kNoImplErr;
}

//----------------------------------------------------------------------------
Boolean	CAccelerometerMPI::IsAccelerometerPresent()
{
	return false;
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
	return 0;
}

//----------------------------------------------------------------------------
tErrType CAccelerometerMPI::SetAccelerometerRate(U32 rate)
{
	return kNoImplErr;
}

//----------------------------------------------------------------------------
tAccelerometerMode CAccelerometerMPI::GetAccelerometerMode() const
{
	return static_cast<tAccelerometerMode>(0);
}

//----------------------------------------------------------------------------
tErrType CAccelerometerMPI::SetAccelerometerMode(tAccelerometerMode mode)
{
	return kNoImplErr;
}

LF_END_BRIO_NAMESPACE()
// EOF
