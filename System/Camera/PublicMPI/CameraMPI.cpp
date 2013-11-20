//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		CameraMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio
//		Camera Manager module.
//
//==============================================================================

#include <CameraTypes.h>
#include <VideoTypes.h>
#include <CameraMPI.h>
#include <CameraPriv.h>
#include <Module.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <sys/stat.h>
LF_BEGIN_BRIO_NAMESPACE()

namespace
{
	//------------------------------------------------------------------------
	inline bool HaveLF2000VIPDriver(void)
	{
		struct stat st;

		return (stat("/sys/devices/platform/vip.0/driver", &st) == 0)
			|| (stat("/sys/devices/platform/nxp-v4l2.0/driver", &st) == 0);
	}
}

const CString	kMPIName = "CameraMPI";

//============================================================================
// CCameraEventMessage
//============================================================================
//------------------------------------------------------------------------------
CCameraEventMessage::CCameraEventMessage( const tCaptureTimeoutMsg& data )
	: IEventMessage(kCaptureTimeOutEvent)
{
	this->data.timeOut = data;
}

CCameraEventMessage::CCameraEventMessage( const tCaptureQuotaHitMsg& data )
	: IEventMessage(kCaptureQuotaHitEvent)
{
	this->data.quotaHit = data;
}

CCameraEventMessage::CCameraEventMessage( const tCaptureStoppedMsg& data )
	: IEventMessage(kCaptureStoppedEvent)
{
	this->data.stopped = data;
}

CCameraEventMessage::CCameraEventMessage( const tCameraRemovedMsg& data )
	: IEventMessage(kCameraRemovedEvent)
{
	this->data.removed = data;
}

CCameraEventMessage::CCameraEventMessage( const tAudioTriggeredMsg& data )
	: IEventMessage(kAudioTriggeredEvent)
{
	this->data.triggered = data;
}

CCameraEventMessage::CCameraEventMessage( const tCaptureFrameMsg& data )
	: IEventMessage(kCaptureFrameEvent)
{
	this->data.framed = data;
}

//------------------------------------------------------------------------------
U16	CCameraEventMessage::GetSizeInBytes() const
{
	return sizeof(CCameraEventMessage);
}   // ---- end GetSizeInBytes() ----

//============================================================================
// CCameraMPI
//============================================================================
//----------------------------------------------------------------------------
CCameraMPI::CCameraMPI() : pModule_(NULL)
{
	ICoreModule*	pModule;

	if(HaveLF2000VIPDriver())
		Module::Connect(pModule, kVIPCameraModuleName, kVIPCameraModuleVersion);
	else {
#ifdef EMULATION
		Module::Connect(pModule, kEmulCameraModuleName, kUSBCameraModuleVersion);
#else
		Module::Connect(pModule, kUSBCameraModuleName, kUSBCameraModuleVersion);
#endif
	}

	pModule_ = reinterpret_cast<CCameraModule*>(pModule);
}

//----------------------------------------------------------------------------
CCameraMPI::~CCameraMPI()
{
	Module::Disconnect(pModule_);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::IsValid() const
{
	if(!pModule_)
		return false;
	return pModule_->IsValid();
}

//----------------------------------------------------------------------------
const CString* CCameraMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CCameraMPI::GetModuleVersion() const
{
	if (!pModule_)
		return kUndefinedVersion;
	return pModule_->GetModuleVersion();
}

//----------------------------------------------------------------------------
const CString* CCameraMPI::GetModuleName() const
{
	if (!pModule_)
		return &kNullString;
	return pModule_->GetModuleName();
}

//----------------------------------------------------------------------------
const CURI* CCameraMPI::GetModuleOrigin() const
{
	if (!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
}

//============================================================================

Boolean CCameraMPI::IsCameraPresent()
{
	if(!pModule_)
		return false;
	return pModule_->IsCameraPresent();
}

//----------------------------------------------------------------------------
tErrType CCameraMPI::SetCameraVideoPath(const CPath& path)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SetCameraVideoPath(path);
}

//----------------------------------------------------------------------------
CPath* CCameraMPI::GetCameraVideoPath()
{
	if (!pModule_)
		return kNull;
	return pModule_->GetCameraVideoPath();
}

//----------------------------------------------------------------------------
tErrType CCameraMPI::SetCameraStillPath(const CPath& path)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SetCameraStillPath(path);
}

//----------------------------------------------------------------------------
CPath* CCameraMPI::GetCameraStillPath()
{
	if (!pModule_)
		return kNull;
	return pModule_->GetCameraStillPath();
}

//----------------------------------------------------------------------------
tErrType CCameraMPI::SetCameraAudioPath(const CPath& path)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SetCameraAudioPath(path);
}

//----------------------------------------------------------------------------
CPath* CCameraMPI::GetCameraAudioPath()
{
	if (!pModule_)
		return kNull;
	return pModule_->GetCameraAudioPath();
}

//----------------------------------------------------------------------------
Boolean CCameraMPI::GetCameraControls(tCameraControls &controls)
{
	if (!pModule_)
		return false;
	return pModule_->GetCameraControls(controls);
}

//----------------------------------------------------------------------------
Boolean CCameraMPI::SetCameraControl(const tControlInfo* control, const S32 value)
{
	if (!pModule_)
		return false;
	return pModule_->SetCameraControl(control, value);
}

//----------------------------------------------------------------------------
tVidCapHndl	CCameraMPI::StartVideoCapture(tVideoSurf* pSurf, IEventListener * pListener,
		const CPath& path, const U32 maxLength, const Boolean audio)
{
	if (!pModule_)
		return kInvalidVidCapHndl;

	return pModule_->StartVideoCapture(path, pSurf, pListener, maxLength, audio);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::SnapFrame(const tVidCapHndl hndl, const CPath &path)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->SnapFrame(hndl, path);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::GetFrame(const tVidCapHndl hndl, U8 *pixels, tColorOrder color_order)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->GetFrame(hndl, pixels, color_order);
}
//----------------------------------------------------------------------------
Boolean	CCameraMPI::GetFrame(const tVidCapHndl hndl, tVideoSurf *pSurf, tColorOrder color_order)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->GetFrame(hndl, pSurf, color_order);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::RenderFrame(const CPath &path, tVideoSurf *pSurf)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->RenderFrame(path, pSurf);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::PauseVideoCapture(const tVidCapHndl hndl, const Boolean display)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->PauseVideoCapture(hndl, display);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::ResumeVideoCapture(const tVidCapHndl hndl)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->ResumeVideoCapture(hndl);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::IsVideoCapturePaused(const tVidCapHndl hndl)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->IsVideoCapturePaused(hndl);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::StopVideoCapture(const tVidCapHndl hndl)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->StopVideoCapture(hndl);
}

//----------------------------------------------------------------------------
tAudCapHndl	CCameraMPI::StartAudioCapture(const CPath& path, IEventListener * pListener, const U32 maxLength, const Boolean paused)
{
	if (!pModule_)
		return kInvalidAudCapHndl;
	return pModule_->StartAudioCapture(path, pListener, maxLength, paused);
}

//----------------------------------------------------------------------------
Boolean CCameraMPI::PauseAudioCapture(const tAudCapHndl hndl)
{
	if (!pModule_)
		return false;
	return pModule_->PauseAudioCapture(hndl);
}

//----------------------------------------------------------------------------
Boolean CCameraMPI::ResumeAudioCapture(const tAudCapHndl hndl)
{
	if (!pModule_)
		return false;
	return pModule_->ResumeAudioCapture(hndl);
}

//----------------------------------------------------------------------------
Boolean CCameraMPI::IsAudioCapturePaused(const tAudCapHndl hndl)
{
	if (!pModule_)
		return false;
	return pModule_->IsAudioCapturePaused(hndl);
}

//----------------------------------------------------------------------------
Boolean CCameraMPI::StopAudioCapture(const tAudCapHndl hndl)
{
	if (!pModule_)
		return false;
	return pModule_->StopAudioCapture(hndl);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::SetMicrophoneParam(enum tMicrophoneParam param, S32 value)
{
	if (!pModule_)
		return false;
	return pModule_->SetMicrophoneParam(param, value);
}

//----------------------------------------------------------------------------
S32	CCameraMPI::GetMicrophoneParam(enum tMicrophoneParam param)
{
	if (!pModule_)
		return 0;
	return pModule_->GetMicrophoneParam(param);
}

//----------------------------------------------------------------------------
tErrType CCameraMPI::EnumFormats(tCaptureModes& pModeList)
{
	if (!pModule_)
		return 0;
	return pModule_->EnumFormats(pModeList);
}

//----------------------------------------------------------------------------
tErrType CCameraMPI::SetCurrentFormat(tCaptureMode* pMode)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SetCurrentFormat(pMode);
}

//----------------------------------------------------------------------------
tCaptureMode* CCameraMPI::GetCurrentFormat()
{
	if (!pModule_)
		return kNull;
	return pModule_->GetCurrentFormat();
}

//----------------------------------------------------------------------------
tErrType CCameraMPI::SetCurrentCamera(tCameraDevice device)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SetCurrentCamera(device);
}

//----------------------------------------------------------------------------
tCameraDevice CCameraMPI::GetCurrentCamera()
{
	if (!pModule_)
		return kCameraNone;
	return pModule_->GetCurrentCamera();
}

tVideoSurf* CCameraMPI::GetCaptureVideoSurface(const tVidCapHndl hndl)
{
	if(!pModule_)
		return NULL;

	return pModule_->GetCaptureVideoSurface(hndl);
}

Boolean CCameraMPI::LockCaptureVideoSurface(const tVidCapHndl hndl)
{
	if(!pModule_)
		return false;
	return pModule_->LockCaptureVideoSurface(hndl);
}

Boolean CCameraMPI::UnLockCaptureVideoSurface(const tVidCapHndl hndl)
{
	if(!pModule_)
		return false;
	return pModule_->UnLockCaptureVideoSurface(hndl);
}

//----------------------------------------------------------------------------
LF_END_BRIO_NAMESPACE()
// EOF
