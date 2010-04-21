//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		VideoPlayer.cpp
//
// Description:
//		Defines the base VideoPlayer class. 
//
//==============================================================================
#include <SystemTypes.h>
#include <VideoTypes.h>
#include <VideoPlayer.h>
#include <VideoPriv.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Implementation
//==============================================================================

//----------------------------------------------------------------------------
CVideoPlayer::CVideoPlayer()
	: dbg_(kGroupVideo)
{
	dbg_.SetDebugLevel(kVideoDebugLevel);
}

//----------------------------------------------------------------------------
CVideoPlayer::~CVideoPlayer()
{
}

//----------------------------------------------------------------------------
Boolean	CVideoPlayer::InitVideo(tVideoHndl hVideo)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean	CVideoPlayer::DeInitVideo(tVideoHndl hVideo)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CVideoPlayer::GetVideoFrame(tVideoHndl hVideo, void* pCtx)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CVideoPlayer::PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pCtx)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CVideoPlayer::GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CVideoPlayer::GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CVideoPlayer::SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CVideoPlayer::SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bExact)
{
	return false;
}

LF_END_BRIO_NAMESPACE()	

// EOF
