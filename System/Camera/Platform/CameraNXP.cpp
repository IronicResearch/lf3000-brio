//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		CameraNXP.cpp
//
// Description:
//		Camera module implementation for LF3000 NXP V4L2 drivers.
//
//==============================================================================
#include <SystemTypes.h>
#include <Utility.h>

#include <CameraPriv.h>
#include <linux/v4l2-mediabus.h>
#include <lf3000/nxp-v4l2-media.h>

LF_BEGIN_BRIO_NAMESPACE()
//==============================================================================
// Defines
//==============================================================================

//============================================================================
// CCameraModule: Informational functions
//============================================================================
//----------------------------------------------------------------------------
tVersion CNXPCameraModule::GetModuleVersion() const
{
	return kNXPCameraModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CNXPCameraModule::GetModuleName() const
{
	return &kNXPCameraModuleName;
}

//============================================================================
// Ctor & dtor
//============================================================================
CNXPCameraModule::CNXPCameraModule()
{
	tCaptureMode QVGA = {kCaptureFormatYUV420, 400, 300, 1, 30};
    struct V4l2UsageScheme us;

    memset(&us, 0, sizeof(us));
    us.useClipper1 = true;
    us.useMlc0Video = true;

	// Init NXP V4L2 module
	nxphndl_ = v4l2_init(&us);
	dbg_.Assert(nxphndl_ != NULL, "%s: v4l2_init failed\n", __func__);

	v4l2_link(nxphndl_, nxp_v4l2_sensor1, nxp_v4l2_clipper1);
	v4l2_link(nxphndl_, nxp_v4l2_clipper1, nxp_v4l2_mlc0_video);

	v4l2_set_format(nxphndl_, nxp_v4l2_sensor1, QVGA.width, QVGA.height, PIXCODE_YUV420_PLANAR);
	v4l2_set_format(nxphndl_, nxp_v4l2_clipper1, QVGA.width, QVGA.height, PIXFORMAT_YUV420_PLANAR);
	v4l2_set_format(nxphndl_, nxp_v4l2_mlc0_video, QVGA.width, QVGA.height, PIXFORMAT_YUV420_PLANAR);
	v4l2_set_crop(nxphndl_, nxp_v4l2_clipper1, 0, 0, QVGA.width, QVGA.height);
	v4l2_set_crop(nxphndl_, nxp_v4l2_mlc0_video, 0, 0, QVGA.width, QVGA.height);

	// Enumerate camera modes to match preferred default
	camCtx_.mode = QVGA;
	camCtx_.file = "/dev/video2";
//	valid = InitCameraInt(&camCtx_.mode, false);
	camCtx_.modes->push_back(new tCaptureMode(QVGA));
	valid = true;
}

//----------------------------------------------------------------------------
CNXPCameraModule::~CNXPCameraModule()
{
//	DeinitCameraInt();

	v4l2_unlink(nxphndl_, nxp_v4l2_clipper1, nxp_v4l2_mlc0_video);
	v4l2_unlink(nxphndl_, nxp_v4l2_sensor1, nxp_v4l2_clipper1);

	// Exit NXP V4L2 module
	v4l2_exit(nxphndl_);
}

//============================================================================
// NXP specific implementation
//============================================================================
//----------------------------------------------------------------------------
tErrType CNXPCameraModule::EnumFormats(tCaptureModes& pModeList)
{
	pModeList = *camCtx_.modes;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CNXPCameraModule::SetCurrentFormat(tCaptureMode* pMode)
{
	camCtx_.mode = *pMode;
	v4l2_set_format(nxphndl_, nxp_v4l2_clipper1, camCtx_.mode.width, camCtx_.mode.height, PIXFORMAT_YUV420_PLANAR);
	v4l2_set_crop(nxphndl_, nxp_v4l2_clipper1, 0, 0, camCtx_.mode.width, camCtx_.mode.height);
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CNXPCameraModule::SetCurrentCamera(tCameraDevice device)
{
	return kNoImplErr;
}

//----------------------------------------------------------------------------
tVidCapHndl CNXPCameraModule::StartVideoCapture(const CPath& path, tVideoSurf* pSurf,
		IEventListener * pListener, const U32 maxLength, Boolean bAudio)
{
	v4l2_streamon(nxphndl_, nxp_v4l2_clipper1);

	if (pSurf) {
		v4l2_set_format(nxphndl_, nxp_v4l2_mlc0_video, pSurf->width, pSurf->height, PIXFORMAT_YUV420_PLANAR);
		v4l2_set_crop(nxphndl_, nxp_v4l2_mlc0_video, 0, 0, pSurf->width, pSurf->height);
		v4l2_streamon(nxphndl_, nxp_v4l2_mlc0_video);
	}

	return CCameraModule::StartVideoCapture(path, pSurf, pListener, maxLength, bAudio);
}

//----------------------------------------------------------------------------
Boolean CNXPCameraModule::StopVideoCapture(const tVidCapHndl hndl)
{
	v4l2_streamoff(nxphndl_, nxp_v4l2_mlc0_video);
	v4l2_streamoff(nxphndl_, nxp_v4l2_clipper1);

	return CCameraModule::StopVideoCapture(hndl);
}

//----------------------------------------------------------------------------
Boolean CNXPCameraModule::PauseVideoCapture(const tVidCapHndl hndl, const Boolean display)
{
	return CCameraModule::PauseVideoCapture(hndl, display);
}

//----------------------------------------------------------------------------
Boolean CNXPCameraModule::ResumeVideoCapture(const tVidCapHndl hndl)
{
	return CCameraModule::ResumeVideoCapture(hndl);
}

//----------------------------------------------------------------------------
Boolean	CNXPCameraModule::SnapFrame(const tVidCapHndl hndl, const CPath &path)
{
	return false;
}

//----------------------------------------------------------------------------
Boolean	CNXPCameraModule::GetFrame(const tVidCapHndl hndl, U8 *pixels, tColorOrder color_order)
{
	tVideoSurf 	surf = {640, 480, 640*3, pixels, kPixelFormatRGB888};

	return CNXPCameraModule::GetFrame(hndl, &surf, color_order);
}

//----------------------------------------------------------------------------
Boolean	CNXPCameraModule::GetFrame(const tVidCapHndl hndl, tVideoSurf *pSurf, tColorOrder color_order)
{
	return false;
}

LF_END_BRIO_NAMESPACE()

//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()

static CNXPCameraModule*	sinst = NULL;

extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion version)
	{
		(void )version;	/* Prevent unused variable warnings. */
		if( sinst == NULL )
			sinst = new CNXPCameraModule;
		return sinst;
	}

	//------------------------------------------------------------------------
	void DestroyInstance(ICoreModule* ptr)
	{
		(void )ptr;	/* Prevent unused variable warnings. */
	//		assert(ptr == sinst);
		delete sinst;
		sinst = NULL;
	}
}
#endif	// LF_MONOLITHIC_DEBUG
