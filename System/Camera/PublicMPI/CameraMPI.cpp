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
tVidCapHndl CCameraMPI::StartVideoCapture(const CPath& path, tVideoSurf* pSurf)
{
	if (!pModule_)
		return kInvalidVidCapHndl;

	return pModule_->StartVideoCapture(path, pSurf);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::SnapFrame(const tVidCapHndl hndl, const CPath &path)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->SnapFrame(hndl, path);
}

//----------------------------------------------------------------------------
Boolean	CCameraMPI::RenderFrame(const CPath &path, tVideoSurf *pSurf)
{
	if (!pModule_)
		return kInvalidVidCapHndl;
	return pModule_->RenderFrame(path, pSurf);
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

LF_END_BRIO_NAMESPACE()
// EOF
