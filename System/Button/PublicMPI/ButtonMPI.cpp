//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		ButtonMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio 
//		Button Manager module.
//
//============================================================================

#include <ButtonMPI.h>
#include <ButtonPriv.h>
#include <EventMPI.h>
#include <Module.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <TouchTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "ButtonMPI";


//============================================================================
// CButtonMessage
//============================================================================
//------------------------------------------------------------------------------
CButtonMessage::CButtonMessage( const tButtonData& data ) 
	: IEventMessage(kButtonStateChanged), mData(data)
{
}

//------------------------------------------------------------------------------
U16	CButtonMessage::GetSizeInBytes() const
{
	return sizeof(CButtonMessage);
}

//------------------------------------------------------------------------------
tButtonData CButtonMessage::GetButtonState() const
{
	return mData;
}

//============================================================================
// CTouchMessage
//============================================================================
//------------------------------------------------------------------------------
CTouchMessage::CTouchMessage( const tTouchData& data ) 
	: IEventMessage(kTouchStateChanged), mData(data)
{
}

//------------------------------------------------------------------------------
U16	CTouchMessage::GetSizeInBytes() const
{
	return sizeof(CTouchMessage);
}

//------------------------------------------------------------------------------
tTouchData CTouchMessage::GetTouchState() const
{
	return mData;
}
//============================================================================
// CButtonMPI
//============================================================================
//----------------------------------------------------------------------------
CButtonMPI::CButtonMPI() : pModule_(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kButtonModuleName, kButtonModuleVersion);
	pModule_ = reinterpret_cast<CButtonModule*>(pModule);
}

//----------------------------------------------------------------------------
CButtonMPI::~CButtonMPI()
{
	Module::Disconnect(pModule_);
}

//----------------------------------------------------------------------------
Boolean	CButtonMPI::IsValid() const
{
	return (pModule_ != NULL) ? true : false;
}

//----------------------------------------------------------------------------
const CString* CButtonMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CButtonMPI::GetModuleVersion() const
{
	if(!pModule_)
		return kUndefinedVersion;
	return pModule_->GetModuleVersion();
}

//----------------------------------------------------------------------------
const CString* CButtonMPI::GetModuleName() const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetModuleName();
}

//----------------------------------------------------------------------------
const CURI* CButtonMPI::GetModuleOrigin() const
{
	if(!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
}


//============================================================================
//----------------------------------------------------------------------------
tErrType CButtonMPI::RegisterEventListener(const IEventListener *pListener)
{
	CEventMPI eventmgr;
	return eventmgr.RegisterEventListener(pListener);
}

//----------------------------------------------------------------------------
tErrType CButtonMPI::UnregisterEventListener(const IEventListener *pListener)
{
	CEventMPI eventmgr;
	return eventmgr.UnregisterEventListener(pListener);
}

//----------------------------------------------------------------------------
tButtonData CButtonMPI::GetButtonState() const
{
	if(!pModule_)
	{
		tButtonData	data = { 0, 0 };
		return data;
	}
	return pModule_->GetButtonState();
}


LF_END_BRIO_NAMESPACE()
// EOF
