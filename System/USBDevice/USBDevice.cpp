//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		USBDevice.cpp
//
// Description:
//		Implements the underlying USBDevice Manager
//
//============================================================================
#include <SystemTypes.h>
#include <USBDevicePriv.h>
#include <GroupEnumeration.h>
LF_BEGIN_BRIO_NAMESPACE()


//============================================================================
// Constants
//============================================================================
const CURI	kModuleURI	= "/LF/System/USBDevice";


//============================================================================
// CUSBDeviceModule: Informational functions
//============================================================================
//----------------------------------------------------------------------------
tVersion CUSBDeviceModule::GetModuleVersion() const
{
	return kUSBDeviceModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CUSBDeviceModule::GetModuleName() const
{
	return &kUSBDeviceModuleName;
}

//----------------------------------------------------------------------------
const CURI* CUSBDeviceModule::GetModuleOrigin() const
{
	return &kModuleURI;
}


//============================================================================
// Ctor & dtor
//============================================================================
CUSBDeviceModule::CUSBDeviceModule() : dbg_(kGroupUSBDevice)
{
	InitModule();	// delegate to platform or emulation initializer
}

//----------------------------------------------------------------------------
CUSBDeviceModule::~CUSBDeviceModule()
{
	DeinitModule();	// delegate to platform or emulation deinitializer
}

//----------------------------------------------------------------------------
Boolean	CUSBDeviceModule::IsValid() const
{
	return true;
}


LF_END_BRIO_NAMESPACE()


//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()

static CUSBDeviceModule*	sinst = NULL;

extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion /*version*/)
	{
		if( sinst == NULL )
			sinst = new CUSBDeviceModule;
		return sinst;
	}
		
	//------------------------------------------------------------------------
	void DestroyInstance(ICoreModule* /*ptr*/)
	{
	//		assert(ptr == sinst);
		delete sinst;
		sinst = NULL;
	}
}
#endif	// LF_MONOLITHIC_DEBUG


// eof
