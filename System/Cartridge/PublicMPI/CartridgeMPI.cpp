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
#include <DebugMPI.h>
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


//----------------------------------------------------------------------------
// The Emerald OTP cart ID is 3 verus Didj OTP ID 7
#define CART_TYPE_INFO  "/sys/devices/platform/lf2000-nand/cart_hotswap"
enum eCartridgeType_ CCartridgeMPI::GetCartridgeType() const
{
	FILE *fp;
	CDebugMPI debug(kGroupCartridge);
	int ret, ready, option;
	enum eCartridgeType_	type = CARTRIDGE_TYPE_UNKNOWN;
	
	fp = fopen(CART_TYPE_INFO, "r");
	if(fp == NULL) {
		debug.DebugOut(kDbgLvlCritical, "Error: can't open %s for read !\n", CART_TYPE_INFO);
		return CARTRIDGE_TYPE_UNKNOWN;
	}

	ret = fscanf(fp, "%d %d", &ready, &option);
	if((ret == 0) || (ret == EOF)) {
		debug.DebugOut(kDbgLvlCritical, "Error: data at %s: Format Wrong \n", CART_TYPE_INFO);
	} 
	fclose(fp);
	
	if(ready == 1) {
		if(option==1) {
			type = CARTRIDGE_TYPE_DIDJ;
		}else if(option==0) {
			type = CARTRIDGE_TYPE_EMERALD;
		}
	} else {
		type = CARTRIDGE_TYPE_UNKNOWN;		
	}
	
	return type;
}

LF_END_BRIO_NAMESPACE()

// EOF
