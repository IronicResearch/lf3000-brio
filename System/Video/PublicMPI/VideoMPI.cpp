//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		VideoMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio 
//		Video Manager module.
//
//==============================================================================

#include <VideoTypes.h>
#include <VideoMPI.h>
#include <VideoPriv.h>
#include <Module.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "VideoMPI";


//============================================================================
// CVideoMPI
//============================================================================
//----------------------------------------------------------------------------
CVideoMPI::CVideoMPI() : pModule_(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kVideoModuleName, kVideoModuleVersion);
	pModule_ = reinterpret_cast<CVideoModule*>(pModule);
}

//----------------------------------------------------------------------------
CVideoMPI::~CVideoMPI()
{
	Module::Disconnect(pModule_);
}

//----------------------------------------------------------------------------
Boolean	CVideoMPI::IsValid() const
{
	return (pModule_ != NULL) ? true : false;
}

//----------------------------------------------------------------------------
const CString* CVideoMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CVideoMPI::GetModuleVersion() const
{
	if (!pModule_)
		return kUndefinedVersion;
	return pModule_->GetModuleVersion();
}

//----------------------------------------------------------------------------
const CString* CVideoMPI::GetModuleName() const
{
	if (!pModule_)
		return &kNullString;
	return pModule_->GetModuleName();
}

//----------------------------------------------------------------------------
const CURI* CVideoMPI::GetModuleOrigin() const
{
	if (!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
}

//============================================================================

//----------------------------------------------------------------------------
tVideoHndl CVideoMPI::StartVideo(tRsrcHndl hRsrc)
{
	if (!pModule_)
		return false;
	return pModule_->StartVideo(hRsrc);
}

//----------------------------------------------------------------------------
Boolean CVideoMPI::StopVideo(tVideoHndl hVideo)
{
	if (!pModule_)
		return false;
	return pModule_->StopVideo(hVideo);
}

//----------------------------------------------------------------------------
Boolean CVideoMPI::GetVideoFrame(tVideoHndl hVideo, void* pCtx)
{
	if (!pModule_)
		return false;
	return pModule_->GetVideoFrame(hVideo, pCtx);
}

//----------------------------------------------------------------------------
Boolean CVideoMPI::SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop)
{
	if (!pModule_)
		return false;
	return pModule_->SyncVideoFrame(hVideo, pCtx, bDrop);
}

//----------------------------------------------------------------------------
Boolean CVideoMPI::SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx)
{
	if (!pModule_)
		return false;
	return pModule_->SeekVideoFrame(hVideo, pCtx);
}

//----------------------------------------------------------------------------
Boolean CVideoMPI::PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pCtx)
{
	if (!pModule_)
		return false;
	return pModule_->PutVideoFrame(hVideo, pCtx);
}

//----------------------------------------------------------------------------
Boolean CVideoMPI::GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo)
{
	if (!pModule_)
		return false;
	return pModule_->GetVideoInfo(hVideo, pInfo);
}

//----------------------------------------------------------------------------
Boolean CVideoMPI::GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime)
{
	if (!pModule_)
		return false;
	return pModule_->GetVideoTime(hVideo, pTime);
}

LF_END_BRIO_NAMESPACE()
// EOF
