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

static tButtonData2 	gCachedButtonData 	= {0, 0, {0, 0}};
static tTouchData		gCachedTouchData	= {0, 0, 0, {0, 0}};
static bool				gIsPressureMode		= false;

//============================================================================
// CButtonMessage
//============================================================================
//------------------------------------------------------------------------------
CButtonMessage::CButtonMessage( const tButtonData2& data ) 
	: IEventMessage(kButtonStateChanged), mData(data)
{
	gCachedButtonData = data;
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
	if (!gIsPressureMode)
		mData.touchState = (data.touchState) ? 1 : 0;
	gCachedTouchData = mData;
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
	tButtonData data = {gCachedButtonData.buttonState, gCachedButtonData.buttonTransition};
	return data;
}

//----------------------------------------------------------------------------
tButtonData2 CButtonMPI::GetButtonState2() const
{
	return gCachedButtonData;
}

//----------------------------------------------------------------------------
tTouchData CButtonMPI::GetTouchState() const
{
	return gCachedTouchData;
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
#if 1 // Re-Enabled for Madrid.  This is to fix TTPro 527.  // disabled per TTP 2419
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
	U32 a = GetTouchParam(kTouchParamSampleRate);
	U32 b = GetTouchParam(kTouchParamDebounceDown);
	U32 c = GetTouchParam(kTouchParamDebounceUp);
	if (gIsPressureMode)
		return kTouchModePressure;
	if (a == kTouchTableDrawing[kTouchParamSampleRate]
		&& b == kTouchTableDrawing[kTouchParamDebounceDown]
		&& c == kTouchTableDrawing[kTouchParamDebounceUp])
			return kTouchModeDrawing;
	if (a == kTouchTableDefault[kTouchParamSampleRate]
		&& b == kTouchTableDefault[kTouchParamDebounceDown]
		&& c == kTouchTableDefault[kTouchParamDebounceUp])
			return kTouchModeDefault;
	return kTouchModeCustom;
}

//----------------------------------------------------------------------------
tErrType CButtonMPI::SetTouchMode(tTouchMode mode)
{
	tErrType r = kNoErr; // 0
	gIsPressureMode = false;
	switch (mode) {
	case kTouchModeDrawing:
		r = SetTouchParam(kTouchParamSampleRate, 	kTouchTableDrawing[kTouchParamSampleRate]);
		r |= SetTouchParam(kTouchParamDebounceDown, kTouchTableDrawing[kTouchParamDebounceDown]);
		r |= SetTouchParam(kTouchParamDebounceUp, 	kTouchTableDrawing[kTouchParamDebounceUp]);
		break;
	case kTouchModePressure:
		gIsPressureMode = true;
	case kTouchModeDefault:
	default:
		r = SetTouchParam(kTouchParamSampleRate, 	kTouchTableDefault[kTouchParamSampleRate]);
		r |= SetTouchParam(kTouchParamDebounceDown, kTouchTableDefault[kTouchParamDebounceDown]);
		r |= SetTouchParam(kTouchParamDebounceUp, 	kTouchTableDefault[kTouchParamDebounceUp]);
		break;
	}
	return r;
}

//----------------------------------------------------------------------------
U32	CButtonMPI::GetTouchParam(tTouchParam param) const
{
	U32 value = 0;
	CPath sysfspath;
	switch (param) {
	case kTouchParamSampleRate:
		sysfspath = "/sys/devices/platform/lf1000-touchscreen/sample_rate_in_hz";
		break;
	case kTouchParamDebounceDown:
		sysfspath = "/sys/devices/platform/lf1000-touchscreen/debounce_in_samples_down";
		break;
	case kTouchParamDebounceUp:
		sysfspath = "/sys/devices/platform/lf1000-touchscreen/debounce_in_samples_up";
		break;
	default:
		return 0;
	}
	FILE* fd = fopen(sysfspath.c_str(), "r");
	if (fd != NULL) {
		fscanf(fd, "%u\n", (unsigned int*)&value);
		fclose(fd);
	}
	return value;
}

//----------------------------------------------------------------------------
tErrType CButtonMPI::SetTouchParam(tTouchParam param, U32 value)
{
	CPath sysfspath;
	switch (param) {
	case kTouchParamSampleRate:
		sysfspath = "/sys/devices/platform/lf1000-touchscreen/sample_rate_in_hz";
		break;
	case kTouchParamDebounceDown:
		sysfspath = "/sys/devices/platform/lf1000-touchscreen/debounce_in_samples_down";
		break;
	case kTouchParamDebounceUp:
		sysfspath = "/sys/devices/platform/lf1000-touchscreen/debounce_in_samples_up";
		break;
	default:
		return kNoImplErr;
	}
	FILE* fd = fopen(sysfspath.c_str(), "w");
	if (fd != NULL) {
		fprintf(fd, "%u\n", (unsigned int)value);
		fclose(fd);
		return kNoErr;
	}
	return kNoImplErr;
}
//----------------------------------------------------------------------------
tDpadOrientation CButtonMPI::GetDpadOrientation()
{
	return GetDpadOrientationState();
}
//----------------------------------------------------------------------------
tErrType CButtonMPI::SetDpadOrientation(tDpadOrientation dpad_orientation)
{
	SetDpadOrientationState(dpad_orientation);
	return kNoErr;
}
LF_END_BRIO_NAMESPACE()
// EOF
