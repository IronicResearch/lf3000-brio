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
#include <Utility.h>
LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "ButtonMPI";


//============================================================================
// CButtonMessage
//============================================================================
//------------------------------------------------------------------------------
CButtonMessage::CButtonMessage( const tButtonData2& data ) 
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
	tButtonData data = {mData.buttonState, mData.buttonTransition};
	return data;
}
tButtonData2 CButtonMessage::GetButtonState2() const
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
	pModule_ = new CButtonModule();
}

//----------------------------------------------------------------------------
CButtonMPI::~CButtonMPI()
{
	delete pModule_;
}

//----------------------------------------------------------------------------
Boolean	CButtonMPI::IsValid() const
{
	return true;
}

//----------------------------------------------------------------------------
const CString* CButtonMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CButtonMPI::GetModuleVersion() const
{
	return kButtonModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CButtonMPI::GetModuleName() const
{
	return &kButtonModuleName;
}

//----------------------------------------------------------------------------
const CURI* CButtonMPI::GetModuleOrigin() const
{
	return &kModuleURI;
}


//============================================================================
//----------------------------------------------------------------------------
tErrType CButtonMPI::RegisterEventListener(const IEventListener *pListener)
{
	return pModule_->eventmgr_.RegisterEventListener(pListener);
}

//----------------------------------------------------------------------------
tErrType CButtonMPI::UnregisterEventListener(const IEventListener *pListener)
{
	return pModule_->eventmgr_.UnregisterEventListener(pListener);
}

//----------------------------------------------------------------------------
tButtonData CButtonMPI::GetButtonState() const
{
	return LeapFrog::Brio::GetButtonState();
}

//----------------------------------------------------------------------------
tButtonData2 CButtonMPI::GetButtonState2() const
{
	return LeapFrog::Brio::GetButtonState2();
}

//----------------------------------------------------------------------------
U32	CButtonMPI::GetTouchRate() const
{
#ifndef EMULATION
	U32 	rate = 0;
	FILE*	fd = fopen("/sys/devices/platform/lf1000-touchscreen/sample_rate_in_hz", "r");
	if (fd != NULL) {
		fscanf(fd, "%u\n", (unsigned int*)&rate);
		fclose(fd);
	}
	return rate;
#else
	return 0;	// not implemented
#endif
}

//----------------------------------------------------------------------------
tErrType CButtonMPI::SetTouchRate(U32 rate)
{
#if 0 // disabled per TTP 2419
	FILE*	fd = fopen("/sys/devices/platform/lf1000-touchscreen/sample_rate_in_hz", "w");
	if (fd != NULL) {
		fprintf(fd, "%u\n", (unsigned int)rate);
		fclose(fd);
		return kNoErr;
	}
	return kNoImplErr;
#else
	return kNoImplErr;	// not implemented
#endif
}

//----------------------------------------------------------------------------
tTouchMode CButtonMPI::GetTouchMode() const
{
#ifndef EMULATION
	U32 	debounce = 0;
	FILE*	fd = fopen("/sys/devices/platform/lf1000-touchscreen/debounce_in_samples", "r");
	if (fd != NULL) {
		fscanf(fd, "%u\n", (unsigned int*)&debounce);
		fclose(fd);
	}
	switch (debounce) {
	case 4:
		return kTouchModeDrawing;
	case 1:
	default:
		return kTouchModeDefault;
	}
#else
	return 0;	// not implemented
#endif
}

//----------------------------------------------------------------------------
tErrType CButtonMPI::SetTouchMode(tTouchMode mode)
{
#ifndef EMULATION
	U32 	debounce = 0;
	switch (mode) {
	case kTouchModeDrawing:
		debounce = 4;
		break;
	case kTouchModeDefault:
	default:
		debounce = 1;
		break;
	}
	FILE*	fd = fopen("/sys/devices/platform/lf1000-touchscreen/debounce_in_samples", "w");
	if (fd != NULL) {
		fprintf(fd, "%u\n", (unsigned int)debounce);
		fclose(fd);
		return kNoErr;
	}
	return kNoImplErr;
#else
	return kNoImplErr;	// not implemented
#endif
}

LF_END_BRIO_NAMESPACE()
// EOF
