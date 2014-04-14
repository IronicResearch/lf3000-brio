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
#include <KeypadTypes.h>
#include <Utility.h>
LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "ButtonMPI";

static tButtonData2 	gCachedButtonData 	= {0, 0, {0, 0}};
static tKeypadData	 	gCachedKeypadData 	= {0, 0, {0, 0}};
static tTouchData		gCachedTouchData	= {0, 0, 0, {0, 0}};
static tTouchMode		gCachedTouchMode	= kTouchModeDefault;
static bool				gIsPressureMode		= false;
static tDpadOrientation gDpadOrientation 	= (GetPlatformFamily() == "LEX") ? kDpadLandscape : kDpadPortrait;
static tDpadOrientation gNativeOrientation 	= (GetPlatformFamily() == "LEX") ? kDpadLandscape : kDpadPortrait;
static tMutex 			gButtonDataMutex	= PTHREAD_MUTEX_INITIALIZER;

const CString	SYSFS_TOUCHSCREEN_LF1000	= "/sys/devices/platform/lf1000-touchscreen/";
const CString	SYSFS_TOUCHSCREEN_LF2000	= "/sys/devices/platform/lf2000-touchscreen/";
static CString	SYSFS_TOUCHSCREEN_ROOT		= "";

inline const char* SYSFS_TOUCHSCREEN_PATH(const char* path)
{
	if(SYSFS_TOUCHSCREEN_ROOT.empty())
	{
		CString platform_name = GetPlatformName();
		if(platform_name == "Emerald" || platform_name == "Madrid")
			SYSFS_TOUCHSCREEN_ROOT = SYSFS_TOUCHSCREEN_LF1000;
		else if(platform_name == "LUCY" || platform_name == "VALENCIA" || platform_name == "RIO")
			SYSFS_TOUCHSCREEN_ROOT = SYSFS_TOUCHSCREEN_LF2000;
		else {
			SYSFS_TOUCHSCREEN_ROOT = "/sys/module/";
			FILE *ts_driver = fopen("/tmp/ts_driver", "r");
			if(ts_driver) {
				char module_name[80];
				fscanf(ts_driver, "%s", module_name);
				fclose(ts_driver);
				SYSFS_TOUCHSCREEN_ROOT += module_name;
				SYSFS_TOUCHSCREEN_ROOT += "/parameters/";
			} else {
				SYSFS_TOUCHSCREEN_ROOT = "/error_no_ts";
			}
		}
	}
	return CString(SYSFS_TOUCHSCREEN_ROOT + path).c_str();
}

//============================================================================
// C function support
//============================================================================

//------------------------------------------------------------------------------
tDpadOrientation GetDpadOrientationState()
{
	return gDpadOrientation;
}

//------------------------------------------------------------------------------
void RotateDpad(int rotation, tButtonData2& gButtonData)
{
	U32 button_mask = kButtonUp | kButtonDown | kButtonRight | kButtonLeft;
	tButtonData2 old_dpad = gButtonData;
	gButtonData.buttonState &= ~button_mask;
	gButtonData.buttonTransition &= ~button_mask;

	switch(rotation)
	{
	case 0:
		gButtonData.buttonState = old_dpad.buttonState;
		break;
	case 1:
		if(old_dpad.buttonState & kButtonUp)
			gButtonData.buttonState |= kButtonLeft;
		if(old_dpad.buttonTransition & kButtonUp)
			gButtonData.buttonTransition |= kButtonLeft;
		if(old_dpad.buttonState & kButtonDown)
			gButtonData.buttonState |= kButtonRight;
		if(old_dpad.buttonTransition & kButtonDown)
			gButtonData.buttonTransition |= kButtonRight;
		if(old_dpad.buttonState & kButtonRight)
			gButtonData.buttonState |= kButtonUp;
		if(old_dpad.buttonTransition & kButtonRight)
			gButtonData.buttonTransition |= kButtonUp;
		if(old_dpad.buttonState & kButtonLeft)
			gButtonData.buttonState |= kButtonDown;
		if(old_dpad.buttonTransition & kButtonLeft)
			gButtonData.buttonTransition |= kButtonDown;
		break;
	case 2:
		if(old_dpad.buttonState & kButtonUp)
			gButtonData.buttonState |= kButtonDown;
		if(old_dpad.buttonTransition & kButtonUp)
			gButtonData.buttonTransition |= kButtonDown;
		if(old_dpad.buttonState & kButtonDown)
			gButtonData.buttonState |= kButtonUp;
		if(old_dpad.buttonTransition & kButtonDown)
			gButtonData.buttonTransition |= kButtonUp;
		if(old_dpad.buttonState & kButtonRight)
			gButtonData.buttonState |= kButtonLeft;
		if(old_dpad.buttonTransition & kButtonRight)
			gButtonData.buttonTransition |= kButtonLeft;
		if(old_dpad.buttonState & kButtonLeft)
			gButtonData.buttonState |= kButtonRight;
		if(old_dpad.buttonTransition & kButtonLeft)
			gButtonData.buttonTransition |= kButtonRight;
		break;
	case 3:
		if(old_dpad.buttonState & kButtonUp)
			gButtonData.buttonState |= kButtonRight;
		if(old_dpad.buttonTransition & kButtonUp)
			gButtonData.buttonTransition |= kButtonRight;
		if(old_dpad.buttonState & kButtonDown)
			gButtonData.buttonState |= kButtonLeft;
		if(old_dpad.buttonTransition & kButtonDown)
			gButtonData.buttonTransition |= kButtonLeft;
		if(old_dpad.buttonState & kButtonRight)
			gButtonData.buttonState |= kButtonDown;
		if(old_dpad.buttonTransition & kButtonRight)
			gButtonData.buttonTransition |= kButtonDown;
		if(old_dpad.buttonState & kButtonLeft)
			gButtonData.buttonState |= kButtonUp;
		if(old_dpad.buttonTransition & kButtonLeft)
			gButtonData.buttonTransition |= kButtonUp;
		break;
	}
}

//------------------------------------------------------------------------------
tErrType SetDpadOrientationState(tDpadOrientation dpad_orientation)
{
	if(gDpadOrientation == dpad_orientation)
		return kNoErr;

	CKernelMPI kernel_mpi;
	kernel_mpi.LockMutex(gButtonDataMutex);
	int rotations = 0;
	switch(gDpadOrientation)
	{
	case kDpadLandscape:
		switch(dpad_orientation)
		{
		case kDpadPortrait:
			rotations = 1;
			break;
		case kDpadLandscapeUpsideDown:
			rotations = 2;
			break;
		case kDpadPortraitUpsideDown:
			rotations = 3;
			break;
		}
		break;
	case kDpadPortrait:
		switch(dpad_orientation)
		{
		case kDpadLandscape:
			rotations = 3;
			break;
		case kDpadLandscapeUpsideDown:
			rotations = 1;
			break;
		case kDpadPortraitUpsideDown:
			rotations = 2;
			break;
		}
		break;
	case kDpadLandscapeUpsideDown:
		switch(dpad_orientation)
		{
		case kDpadLandscape:
			rotations = 2;
			break;
		case kDpadPortrait:
			rotations = 3;
			break;
		case kDpadPortraitUpsideDown:
			rotations = 1;
			break;
		}
		break;
	case kDpadPortraitUpsideDown:
		switch(dpad_orientation)
		{
		case kDpadLandscape:
			rotations = 1;
			break;
		case kDpadPortrait:
			rotations = 2;
			break;
		case kDpadLandscapeUpsideDown:
			rotations = 3;
			break;
		}
		break;
	}
	RotateDpad(rotations, gCachedButtonData);
	gDpadOrientation = dpad_orientation;
	if (gCachedButtonData.buttonTransition != 0) {
		U64 usec = kernel_mpi.GetElapsedTimeAsUSecs();
		U64 sec = usec / 1000000;
		usec %= 1000000;
		gCachedButtonData.time.seconds      = sec;
		gCachedButtonData.time.microSeconds = usec;
		CButtonMessage button_msg(gCachedButtonData);
		CEventMPI event_mpi;
		event_mpi.PostEvent(button_msg, 128, 0); // async
	}
	kernel_mpi.UnlockMutex(gButtonDataMutex);
	return kNoErr;
}

//------------------------------------------------------------------------------
tButtonData2 SwizzleDpad(tButtonData2 data)
{
	tButtonData2 swizzle = data;
	U32 mask = ~(kButtonUp | kButtonDown | kButtonRight | kButtonLeft);

	// Native Dpad orientation is either Portrait (LPAD) or Landscape (LEX)
	if (gNativeOrientation == kDpadPortrait) {
		switch (gDpadOrientation)
		{
		case kDpadLandscape:
			RotateDpad(3, swizzle);
			break;
		case kDpadPortrait:
			RotateDpad(0, swizzle);
			break;
		case kDpadLandscapeUpsideDown:
			RotateDpad(1, swizzle);
			break;
		case kDpadPortraitUpsideDown:
			RotateDpad(2, swizzle);
			break;
		}
	}
	else {
		switch (gDpadOrientation)
		{
		case kDpadLandscape:
			RotateDpad(0, swizzle);
			break;
		case kDpadPortrait:
			RotateDpad(1, swizzle);
			break;
		case kDpadLandscapeUpsideDown:
			RotateDpad(2, swizzle);
			break;
		case kDpadPortraitUpsideDown:
			RotateDpad(3, swizzle);
			break;
		}
	}

	// Account for unswizzled states
	if (swizzle.buttonTransition == 0)
		swizzle.buttonTransition |= (data.buttonTransition);
	swizzle.buttonTransition |= (data.buttonTransition & mask);
	return swizzle;
}

//============================================================================
// CButtonMessage
//============================================================================
//------------------------------------------------------------------------------
CButtonMessage::CButtonMessage( const tButtonData2& data ) 
	: IEventMessage(kButtonStateChanged), mData(data)
{
	gCachedButtonData = mData;
}

//------------------------------------------------------------------------------
CButtonMessage::CButtonMessage( const tButtonData2& data, bool transform )
	: IEventMessage(kButtonStateChanged), mData(data)
{
	if (transform
			&& gDpadOrientation != gNativeOrientation
			&& data.buttonTransition & (kButtonUp | kButtonDown | kButtonRight | kButtonLeft))
	{
		CKernelMPI kernel;
		kernel.LockMutex(gButtonDataMutex);
		gCachedButtonData = mData = SwizzleDpad(data);
		kernel.UnlockMutex(gButtonDataMutex);
	}
	else
		gCachedButtonData = mData;
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

//------------------------------------------------------------------------------
tButtonData2 CButtonMessage::GetButtonState2() const
{
	return mData;
}

//============================================================================
// CKeypadMessage
//============================================================================
//------------------------------------------------------------------------------
CKeypadMessage::CKeypadMessage( const tKeypadData& data )
	: IEventMessage(kKeypadStateChanged), mData(data)
{
	gCachedKeypadData = data;
}

//------------------------------------------------------------------------------
U16	CKeypadMessage::GetSizeInBytes() const
{
	return sizeof(CKeypadMessage);
}

//------------------------------------------------------------------------------
tKeypadData CKeypadMessage::GetKeypadState() const
{
	return mData;
}

//============================================================================
// CTouchMessage
//============================================================================
//------------------------------------------------------------------------------
CTouchMessage::CTouchMessage( const tTouchData& data ) 
	: IEventMessage(kTouchStateChanged)
{
	mData.td = data;
	if (!gIsPressureMode)
		mData.td.touchState = (data.touchState) ? 1 : 0;
	gCachedTouchData = mData.td;
}

//------------------------------------------------------------------------------
CTouchMessage::CTouchMessage( const tMultiTouchData& data )
	: IEventMessage(kTouchEventMultiTouch)
{
	mData.mtd = data;
	if (!gIsPressureMode)
		mData.mtd.td.touchState = (data.td.touchState) ? 1 : 0;
	gCachedTouchData = mData.td;
}

//------------------------------------------------------------------------------
U16	CTouchMessage::GetSizeInBytes() const
{
	return sizeof(CTouchMessage);
}

//------------------------------------------------------------------------------
tTouchData CTouchMessage::GetTouchState() const
{
	return mData.td;
}

//------------------------------------------------------------------------------
tMultiTouchData CTouchMessage::GetMultiTouchState() const
{
	return mData.mtd;
}

//------------------------------------------------------------------------------
tTouchMode CTouchMessage::GetTouchMode() const
{
	return gCachedTouchMode;
}

//============================================================================
// CButtonMPI
//============================================================================
//----------------------------------------------------------------------------
CButtonMPI::CButtonMPI() : pModule_(NULL)
{
#ifdef LF1000
	SYSFS_TOUCHSCREEN_ROOT = (HasPlatformCapability(kCapsLF1000)) ? SYSFS_TOUCHSCREEN_LF1000 : SYSFS_TOUCHSCREEN_LF2000;
#endif
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
	FILE*	fd = fopen(SYSFS_TOUCHSCREEN_PATH("sample_rate_in_hz"), "r");
	if (fd != NULL) {
		fscanf(fd, "%u\n", (unsigned int*)&rate);
		fclose(fd);
	}
	else {
		fd = fopen(SYSFS_TOUCHSCREEN_PATH("scanning_frequency"), "r");
		if (fd != NULL) {
			unsigned int x, y, z;
			fscanf(fd, "Idle: %u Finger: %u Stylus: %u\n", &x, &y, &z);
			fclose(fd);
			rate = z;
		}
		else {
			fd = fopen(SYSFS_TOUCHSCREEN_PATH("report_rate"), "r");
			if(fd)
			{
				fscanf(fd, "%u\n", (unsigned int*)&rate);
				fclose(fd);
			}
		}
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
	FILE*	fd = fopen(SYSFS_TOUCHSCREEN_PATH("sample_rate_in_hz"), "w");
	if (fd != NULL) {
		fprintf(fd, "%u\n", (unsigned int)rate);
		fclose(fd);
		return kNoErr;
	}
	fd = fopen(SYSFS_TOUCHSCREEN_PATH("scanning_frequency"), "w");
	if (fd != NULL) {
		if (rate > kTouchTableRio[kTouchParamSampleRate])
			rate = kTouchTableRio[kTouchParamSampleRate];
		fprintf(fd, "%u %u %u\n", 25, (unsigned int)rate, (unsigned int)rate);
		fclose(fd);
		return kNoErr;
	}
	fd = fopen(SYSFS_TOUCHSCREEN_PATH("report_rate"), "w");
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
	if (HasPlatformCapability(kCapsMultiTouch))
		return gCachedTouchMode;
	// Emerald Legacy support
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
	gCachedTouchMode = mode;
	switch (mode) {
	case kTouchModeDrawing:
		if (HasPlatformCapability(kCapsMultiTouch))
			return SetTouchParam(kTouchParamSampleRate, kTouchTableRio[kTouchParamSampleRate]);
		r = SetTouchParam(kTouchParamSampleRate, 	kTouchTableDrawing[kTouchParamSampleRate]);
		r |= SetTouchParam(kTouchParamDebounceDown, kTouchTableDrawing[kTouchParamDebounceDown]);
		r |= SetTouchParam(kTouchParamDebounceUp, 	kTouchTableDrawing[kTouchParamDebounceUp]);
		break;
	case kTouchModePressure:
		gIsPressureMode = true;
	case kTouchModeDefault:
	default:
		if (HasPlatformCapability(kCapsMultiTouch))
			return SetTouchParam(kTouchParamSampleRate, kTouchTableRio[kTouchParamSampleRate]);
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
		if (HasPlatformCapability(kCapsMultiTouch))
			return GetTouchRate();
		sysfspath = SYSFS_TOUCHSCREEN_PATH("sample_rate_in_hz");
		break;
	case kTouchParamDebounceDown:
		if (HasPlatformCapability(kCapsMultiTouch))
			return 0;
		sysfspath = SYSFS_TOUCHSCREEN_PATH("debounce_in_samples_down");
		break;
	case kTouchParamDebounceUp:
		if (HasPlatformCapability(kCapsMultiTouch))
			return 0;
		sysfspath = SYSFS_TOUCHSCREEN_PATH("debounce_in_samples_up");
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
		if (HasPlatformCapability(kCapsMultiTouch))
			return SetTouchRate(value);
		sysfspath = SYSFS_TOUCHSCREEN_PATH("sample_rate_in_hz");
		break;
	case kTouchParamDebounceDown:
		if (HasPlatformCapability(kCapsMultiTouch))
			return 0;
		sysfspath = SYSFS_TOUCHSCREEN_PATH("debounce_in_samples_down");
		break;
	case kTouchParamDebounceUp:
		if (HasPlatformCapability(kCapsMultiTouch))
			return 0;
		sysfspath = SYSFS_TOUCHSCREEN_PATH("debounce_in_samples_up");
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
	return SetDpadOrientationState(dpad_orientation);
}
//----------------------------------------------------------------------------

LF_END_BRIO_NAMESPACE()
// EOF
