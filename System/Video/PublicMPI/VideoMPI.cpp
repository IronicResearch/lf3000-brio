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
// CVideoEventMessage
//============================================================================
//------------------------------------------------------------------------------
CVideoEventMessage::CVideoEventMessage( const tVideoMsgData& data ) 
	: IEventMessage(kVideoCompletedEvent), data_(data)
{
}

//------------------------------------------------------------------------------
U16	CVideoEventMessage::GetSizeInBytes() const
{
	return sizeof(CVideoEventMessage);
}

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
tErrType CVideoMPI::SetVideoResourcePath(const CPath& path)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SetVideoResourcePath(path);
}

//----------------------------------------------------------------------------
CPath* CVideoMPI::GetVideoResourcePath() const
{
	if (!pModule_)
		return kNull;
	return pModule_->GetVideoResourcePath();
}

//----------------------------------------------------------------------------
tVideoHndl CVideoMPI::StartVideo(const CPath& path)
{
	if (!pModule_)
		return kInvalidVideoHndl;
	return pModule_->StartVideo(path);
}

//----------------------------------------------------------------------------
tVideoHndl CVideoMPI::StartVideo(const CPath& path, tVideoSurf* pSurf, Boolean bLoop, IEventListener* pListener)
{
	if (!pModule_)
		return kInvalidVideoHndl;
	return pModule_->StartVideo(path, pSurf, bLoop, pListener);
}

//----------------------------------------------------------------------------
tVideoHndl CVideoMPI::StartVideo(const CPath& path, const CPath& pathAudio, tVideoSurf* pSurf, Boolean bLoop, IEventListener* pListener)
{
	if (!pModule_)
		return kInvalidVideoHndl;
	return pModule_->StartVideo(path, pathAudio, pSurf, bLoop, pListener);
}

#if 0	// deprecated
//----------------------------------------------------------------------------
tVideoHndl CVideoMPI::StartVideo(tRsrcHndl hRsrc)
{
	if (!pModule_)
		return false;
	return pModule_->StartVideo(hRsrc);
}

//----------------------------------------------------------------------------
tVideoHndl CVideoMPI::StartVideo(tRsrcHndl hRsrc, tRsrcHndl hAudio, tVideoSurf* pSurf, Boolean bLoop, IEventListener* pListener)
{
	if (!pModule_)
		return false;
	return pModule_->StartVideo(hRsrc, hAudio, pSurf, bLoop, pListener);
}
#endif

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
Boolean CVideoMPI::SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pTime)
{
	if (!pModule_)
		return false;
	return pModule_->SeekVideoFrame(hVideo, pTime, true, false);
}
//----------------------------------------------------------------------------
Boolean CVideoMPI::SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pTime, Boolean bUpdateVideoFrame)
{
	if (!pModule_)
		return false;
	return pModule_->SeekVideoFrame(hVideo, pTime, true, bUpdateVideoFrame);
}
//----------------------------------------------------------------------------
Boolean CVideoMPI::SeekVideoKeyFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bUpdateVideoFrame)
{
	if (!pModule_)
		return false;
	return pModule_->SeekVideoFrame(hVideo, pCtx, false, bUpdateVideoFrame);
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

//----------------------------------------------------------------------------
Boolean CVideoMPI::PauseVideo(tVideoHndl hVideo)
{
	if (!pModule_)
		return false;
	return pModule_->PauseVideo(hVideo);
}

//----------------------------------------------------------------------------
Boolean CVideoMPI::ResumeVideo(tVideoHndl hVideo)
{
	if (!pModule_)
		return false;
	return pModule_->ResumeVideo(hVideo);
}

//----------------------------------------------------------------------------
Boolean CVideoMPI::IsVideoPaused(tVideoHndl hVideo)
{
	if (!pModule_)
		return false;
	return pModule_->IsVideoPaused(hVideo);
}

//----------------------------------------------------------------------------
Boolean CVideoMPI::IsVideoPlaying(tVideoHndl hVideo)
{
	if (!pModule_)
		return false;
	return pModule_->IsVideoPlaying(hVideo);
}

//----------------------------------------------------------------------------
Boolean CVideoMPI::IsVideoLooped(tVideoHndl hVideo)
{
	if (!pModule_)
		return false;
	return pModule_->IsVideoLooped(hVideo);
}

//----------------------------------------------------------------------------
tVideoHndl 	CVideoMPI::GetCurrentVideoHandle(void)
{
	if (!pModule_)
		return kInvalidVideoHndl;
	return pModule_->GetCurrentVideoHandle();
}

LF_END_BRIO_NAMESPACE()
// EOF
