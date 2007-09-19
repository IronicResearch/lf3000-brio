//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		USBDeviceEmul.cpp
//
// Description: This is supposed to emulate USBDevice behavior on the host.
// However, this is not supported on emulation, so this is really just a place
// holder for future development.
//
//============================================================================
#include <USBDevicePriv.h>
#include <EmulationConfig.h>
#include <EventMPI.h>
#include <KernelMPI.h>
#include <KernelTypes.h>
#include <SystemErrors.h>
LF_BEGIN_BRIO_NAMESPACE()

//----------------------------------------------------------------------------
void CUSBDeviceModule::InitModule()
{

}

//----------------------------------------------------------------------------
void CUSBDeviceModule::DeinitModule()
{
}

//----------------------------------------------------------------------------
tUSBDeviceData CUSBDeviceModule::GetUSBDeviceState() const
{
	tUSBDeviceData	data = {0, 0, 0};

	return data;
}

tErrType CUSBDeviceModule::EnableUSBDeviceDrivers(U32 drivers)
{
	return kUSBDeviceUnsupportedDriver;
}

//----------------------------------------------------------------------------
tErrType CUSBDeviceModule::DisableUSBDeviceDrivers(U32 drivers)
{
	return kUSBDeviceUnsupportedDriver;
}


LF_END_BRIO_NAMESPACE()
// EOF
