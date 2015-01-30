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

#include <nx_alloc_mem.h>
#include <nx_video_api.h>
#include <nx_fourcc.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Implementation
//==============================================================================

//----------------------------------------------------------------------------
CVPUPlayer::CVPUPlayer()
	: CAVIPlayer()
{
	dbg_.DebugOut(kDbgLvlCritical, "%s\n", __FUNCTION__);
}

//----------------------------------------------------------------------------
CVPUPlayer::~CVPUPlayer()
{
	dbg_.DebugOut(kDbgLvlCritical, "%s\n", __FUNCTION__);
}

//----------------------------------------------------------------------------
Boolean	CVPUPlayer::InitVideo(tVideoHndl hVideo)
{
//	hDec = NX_VidDecOpen( NX_VPX_THEORA, MAKEFOURCC('T', 'H', 'E', 'O'), 0, NULL );
	hDec = NX_VidDecOpen( NX_AVC_DEC, MAKEFOURCC('H', '2', '6', '4'), 0, NULL );
	dbg_.DebugOut(kDbgLvlCritical, "%s: NX_VidDecOpen returned %p\n", __FUNCTION__, hDec);
	if (hDec == NULL)
		return false;

	return CAVIPlayer::InitVideo(hVideo);
}

//----------------------------------------------------------------------------
Boolean	CVPUPlayer::DeInitVideo(tVideoHndl hVideo)
{
	NX_VidDecClose(hDec);

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

//----------------------------------------------------------------------------
S64 CVPUPlayer::GetVideoLength(tVideoHndl hVideo)
{
	return CAVIPlayer::GetVideoLength(hVideo);
}

//----------------------------------------------------------------------------
bool CVPUPlayer::GetNextFrame(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, int iVideoStream, AVFrame *pFrame)
{
	// TODO: The *main* method which needs to be overridden from LibAV calls to VPU calls
	return CAVIPlayer::GetNextFrame(pFormatCtx, pCodecCtx, iVideoStream, pFrame);
}

//----------------------------------------------------------------------------

LF_END_BRIO_NAMESPACE()	

// EOF
