//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		WirelessMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio 
//		Wireless Networking module.
//
//============================================================================

#include <WirelessMPI.h>
#include <WirelessPriv.h>
#include <Module.h>

LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "WirelessMPI";


//============================================================================
//----------------------------------------------------------------------------
CWirelessMPI::CWirelessMPI() : pModule_(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kWirelessModuleName, kWirelessModuleVersion);
	pModule_ = reinterpret_cast<CWirelessModule*>(pModule);
}

//----------------------------------------------------------------------------
CWirelessMPI::~CWirelessMPI()
{
	Module::Disconnect(pModule_);
}

//----------------------------------------------------------------------------
Boolean	CWirelessMPI::IsValid() const
{
	return (pModule_ != NULL) ? true : false;
}

//----------------------------------------------------------------------------
const CString* CWirelessMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CWirelessMPI::GetModuleVersion() const
{
	if(!pModule_)
		return kUndefinedVersion;
	return pModule_->GetModuleVersion();
}

//----------------------------------------------------------------------------
const CString* CWirelessMPI::GetModuleName() const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetModuleName();
}

//----------------------------------------------------------------------------
const CURI* CWirelessMPI::GetModuleOrigin() const
{
	if(!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
}


//============================================================================
//----------------------------------------------------------------------------
tErrType CWirelessMPI::RegisterEventListener(const IEventListener *pListener)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	if(!pListener)
		return kInvalidParamErr;
	return pModule_->eventmgr_.RegisterEventListener(pListener);
}

//----------------------------------------------------------------------------
tErrType CWirelessMPI::UnregisterEventListener(const IEventListener *pListener)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	if(!pListener)
		return kInvalidParamErr;
	return pModule_->eventmgr_.UnregisterEventListener(pListener);
}

//----------------------------------------------------------------------------
tErrType CWirelessMPI::SetWirelessPower(Boolean power)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SetWirelessPower(power);
}

//----------------------------------------------------------------------------
Boolean CWirelessMPI::GetWirelessPower()
{
	if(!pModule_)
		return false;
	return pModule_->GetWirelessPower();
}

//----------------------------------------------------------------------------
tErrType CWirelessMPI::JoinAdhocNetwork( CString ssid, Boolean encrypted, CString password )
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->JoinAdhocNetwork(ssid, encrypted, password);
}

//----------------------------------------------------------------------------
tErrType CWirelessMPI::LeaveAdhocNetwork()
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->LeaveAdhocNetwork();
}

//----------------------------------------------------------------------------
tErrType CWirelessMPI::GetPossiblePlayers(PlayerList& players)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetPossiblePlayers(players);
}

//----------------------------------------------------------------------------
tWirelessState CWirelessMPI::GetState()
{
	if(!pModule_)
		return kWirelessOff;
	return pModule_->GetState();
}

//----------------------------------------------------------------------------
tWirelessMode CWirelessMPI::GetMode()
{
	if(!pModule_)
		return kWirelessNone;
	return pModule_->GetMode();
}

tErrType CWirelessMPI::GetLocalWirelessAddress(in_addr& address)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetLocalWirelessAddress(address);
}

//============================================================================
//----------------------------------------------------------------------------

CRemotePlayerMessage::CRemotePlayerMessage( const tRemotePlayer& player, Boolean added ) :
	mPlayer(player), mAdded(added), IEventMessage(kRemotePlayerChanged)
{
}

CRemotePlayerMessage::~CRemotePlayerMessage()
{
}

U16 CRemotePlayerMessage::GetSizeInBytes() const
{
	return sizeof(CRemotePlayerMessage);
}

Boolean CRemotePlayerMessage::PlayerAdded() const
{
	return mAdded;
}

Boolean CRemotePlayerMessage::PlayerRemoved() const
{
	return !mAdded;
}

tRemotePlayer CRemotePlayerMessage::GetPlayer() const
{
	return mPlayer;
}

//============================================================================
//----------------------------------------------------------------------------

CWirelessStateMessage::CWirelessStateMessage( tWirelessState state, tWirelessMode mode ) :
	mState(state), mMode(mode), IEventMessage(kWirelessStateChanged)
{
}

U16 CWirelessStateMessage::GetSizeInBytes() const
{
	return sizeof(CWirelessStateMessage);
}

tWirelessState CWirelessStateMessage::GetState() const
{
	return mState;
}

tWirelessMode CWirelessStateMessage::GetMode() const
{
	return mMode;
}

LF_END_BRIO_NAMESPACE()
// EOF
