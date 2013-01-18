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
//		Implementation of the GStreamerPlayer class derived from VideoPlayer.
//
//==============================================================================
#include <SystemTypes.h>
#include <VideoTypes.h>
#include <VideoPlayer.h>
#include <VideoPriv.h>
#include <GStreamerPlayer.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Implementation
//==============================================================================

//----------------------------------------------------------------------------
CGStreamerPlayer::CGStreamerPlayer()
{
}

//----------------------------------------------------------------------------
CGStreamerPlayer::~CGStreamerPlayer()
{
}

//----------------------------------------------------------------------------
Boolean	CGStreamerPlayer::InitVideo(tVideoHndl hVideo)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean	CGStreamerPlayer::DeInitVideo(tVideoHndl hVideo)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CGStreamerPlayer::GetVideoFrame(tVideoHndl hVideo, void* pCtx)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CGStreamerPlayer::PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pCtx)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CGStreamerPlayer::GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CGStreamerPlayer::GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CGStreamerPlayer::SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CGStreamerPlayer::SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bExact, Boolean bUpdateVideoDisplay)
{
	return false;
}

S64 CGStreamerPlayer::GetVideoLength(tVideoHndl hVideo)
{
	return 0;
}
LF_END_BRIO_NAMESPACE()	

// EOF
