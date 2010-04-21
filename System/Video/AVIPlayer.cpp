//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		AVIPlayer.cpp
//
// Description:
//		Implementation of the AVIPlayer class derived from VideoPlayer. 
//
//==============================================================================
#include <SystemTypes.h>
#include <VideoTypes.h>
#include <VideoPlayer.h>
#include <VideoPriv.h>
#include <AVIPlayer.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Constants
//==============================================================================

//==============================================================================
// Typedefs
//==============================================================================

//==============================================================================
// Local Variables
//==============================================================================

//==============================================================================
// Local Functions
//==============================================================================

//==============================================================================
// Implementation
//==============================================================================

//----------------------------------------------------------------------------
CAVIPlayer::CAVIPlayer()
{
}

//----------------------------------------------------------------------------
CAVIPlayer::~CAVIPlayer()
{
}

//----------------------------------------------------------------------------
Boolean	CAVIPlayer::InitVideo(tVideoHndl hVideo)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean	CAVIPlayer::DeInitVideo(tVideoHndl hVideo)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CAVIPlayer::GetVideoFrame(tVideoHndl hVideo, void* pCtx)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CAVIPlayer::PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pCtx)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CAVIPlayer::GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CAVIPlayer::GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CAVIPlayer::SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean CAVIPlayer::SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bExact)
{
	return false;
}

LF_END_BRIO_NAMESPACE()	

// EOF
