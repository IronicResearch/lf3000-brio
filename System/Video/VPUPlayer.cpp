//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		VPUPlayer.cpp
//
// Description:
//		Implementation of the VPUPlayer class derived from AVIPlayer.
//
//==============================================================================
#include <SystemTypes.h>
#include <VideoTypes.h>
#include <VideoPlayer.h>
#include <VideoPriv.h>
#include <VPUPlayer.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Implementation
//==============================================================================

//----------------------------------------------------------------------------
CVPUPlayer::CVPUPlayer()
	: CAVIPlayer()
{
}

//----------------------------------------------------------------------------
CVPUPlayer::~CVPUPlayer()
{
}

//----------------------------------------------------------------------------
Boolean	CVPUPlayer::InitVideo(tVideoHndl hVideo)
{
	return CAVIPlayer::InitVideo(hVideo);
}

//----------------------------------------------------------------------------
Boolean	CVPUPlayer::DeInitVideo(tVideoHndl hVideo)
{
	return CAVIPlayer::DeInitVideo(hVideo);
}

//----------------------------------------------------------------------------
Boolean CVPUPlayer::GetVideoFrame(tVideoHndl hVideo, void* pCtx)
{
	return CAVIPlayer::GetVideoFrame(hVideo, pCtx);
}

//----------------------------------------------------------------------------
Boolean CVPUPlayer::PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pCtx)
{
	return CAVIPlayer::PutVideoFrame(hVideo, pCtx);
}

//----------------------------------------------------------------------------
Boolean CVPUPlayer::GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo)
{
	return CAVIPlayer::GetVideoInfo(hVideo, pInfo);
}

//----------------------------------------------------------------------------
Boolean CVPUPlayer::GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime)
{
	return CAVIPlayer::GetVideoTime(hVideo, pTime);
}

//----------------------------------------------------------------------------
Boolean CVPUPlayer::SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop)
{
	return CAVIPlayer::SyncVideoFrame(hVideo, pCtx, bDrop);
}

//----------------------------------------------------------------------------
Boolean CVPUPlayer::SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bExact, Boolean bUpdateVideoDisplay)
{
	return CAVIPlayer::SeekVideoFrame(hVideo, pCtx, bExact, bUpdateVideoDisplay);
}

S64 CVPUPlayer::GetVideoLength(tVideoHndl hVideo)
{
	return CAVIPlayer::GetVideoLength(hVideo);
}

LF_END_BRIO_NAMESPACE()	

// EOF
