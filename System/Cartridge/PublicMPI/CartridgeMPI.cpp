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
//		Cartridge module.
//
//============================================================================

#include <CartridgeMPI.h>
#include <CartridgePriv.h>
#include <EventMPI.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <KernelTypes.h>
#include <Utility.h>


LF_BEGIN_BRIO_NAMESPACE()

const CString	kMPIName = "CartridgeMPI";

//============================================================================
// CCartridgeMessage
//============================================================================
//------------------------------------------------------------------------------
CCartridgeMessage::CCartridgeMessage( const tCartridgeData& data ) 
	: IEventMessage(kCartridgeStateChanged), mData(data)
{
}

//------------------------------------------------------------------------------
U16 CCartridgeMessage::GetSizeInBytes() const
{
	return sizeof(CCartridgeMessage);
}

//------------------------------------------------------------------------------
tCartridgeData CCartridgeMessage::GetCartridgeState() const
{
	return mData;
}


//============================================================================
// CPowerMPI
//============================================================================
//----------------------------------------------------------------------------
CCartridgeMPI::CCartridgeMPI() : pModule_(NULL)
{
}

//----------------------------------------------------------------------------
CCartridgeMPI::~CCartridgeMPI()
{
}

//----------------------------------------------------------------------------
Boolean	CCartridgeMPI::IsValid() const
{
	return true;
}

//----------------------------------------------------------------------------
const CString* CCartridgeMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CCartridgeMPI::GetModuleVersion() const
{
	return kCartridgeModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CCartridgeMPI::GetModuleName() const
{
	return &kCartridgeModuleName;
}

//----------------------------------------------------------------------------
const CURI* CCartridgeMPI::GetModuleOrigin() const
{
	return &kModuleURI;
}


//============================================================================
//----------------------------------------------------------------------------
tErrType CCartridgeMPI::RegisterEventListener(const IEventListener *pListener)
{
	CEventMPI eventmgr;
	return eventmgr.RegisterEventListener(pListener);
}

//----------------------------------------------------------------------------
tErrType CCartridgeMPI::UnregisterEventListener(const IEventListener *pListener)
{
	CEventMPI eventmgr;
	return eventmgr.UnregisterEventListener(pListener);
}

//----------------------------------------------------------------------------
tCartridgeData CCartridgeMPI::GetCartridgeState() const
{
	return LeapFrog::Brio::GetCartridgeState();
}


LF_END_BRIO_NAMESPACE()

// EOF
