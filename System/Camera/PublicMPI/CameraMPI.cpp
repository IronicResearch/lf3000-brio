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
LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "CameraMPI";

//============================================================================
// CCameraEventMessage
//============================================================================
//------------------------------------------------------------------------------


//============================================================================
// CCameraMPI
//============================================================================
//----------------------------------------------------------------------------
CCameraMPI::CCameraMPI() : pModule_(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kCameraModuleName, kCameraModuleVersion);
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
	return (pModule_ != NULL) ? true : false;
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
Boolean CCameraMPI::GetCameraModes(tCaptureModes &modes)
{
	if (!pModule_)
		return false;
	return pModule_->GetCameraModes(modes);
}

//----------------------------------------------------------------------------
Boolean CCameraMPI::SetCameraMode(const tCaptureMode* mode)
{
	if (!pModule_)
		return false;
	return pModule_->SetCameraMode(mode);
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
Boolean CCameraMPI::SetBuffers(const U32 numBuffers)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->SetBuffers(numBuffers);
}

//----------------------------------------------------------------------------
tVidCapHndl CCameraMPI::StartVideoCapture()
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->StartVideoCapture();
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::PollFrame(const tVidCapHndl hndl)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->PollFrame(hndl);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::GetFrame(const tVidCapHndl hndl, tFrameInfo *frame)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->GetFrame(hndl, frame);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::RenderFrame(tFrameInfo *frame, tVideoSurf *pSurf, tBitmapInfo *image)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->RenderFrame(frame, pSurf, image);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::ReturnFrame(const tVidCapHndl hndl, const tFrameInfo *frame)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->ReturnFrame(hndl, frame);
}

//----------------------------------------------------------------------------
tVidCapHndl CCameraMPI::StartVideoCapture(const CPath& path, const Boolean audio, tVideoSurf* pSurf, tRect* rect)
{
	if (!pModule_)
		return kInvalidVidCapHndl;

	return pModule_->StartVideoCapture(path, audio, pSurf, rect);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::GrabFrame(const tVidCapHndl hndl, tFrameInfo *frame)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->GrabFrame(hndl, frame);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::SaveFrame(const CPath &path, const tFrameInfo *frame)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->SaveFrame(path, frame);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::OpenFrame(const CPath &path, tFrameInfo *frame)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->OpenFrame(path, frame);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::PauseVideoCapture(const tVidCapHndl hndl)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->PauseVideoCapture(hndl);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::ResumeVideoCapture(const tVidCapHndl hndl)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->ResumeVideoCapture(hndl);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::IsCapturePaused(const tVidCapHndl hndl)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->IsCapturePaused(hndl);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::StopVideoCapture(const tVidCapHndl hndl)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->StopVideoCapture(hndl);
}

LF_END_BRIO_NAMESPACE()
// EOF
