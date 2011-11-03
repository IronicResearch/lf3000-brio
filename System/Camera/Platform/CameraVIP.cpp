#include <SystemTypes.h>

#include <CameraPriv.h>

#ifndef EMULATION
#include <sys/ioctl.h>
#include <linux/lf2000/lf2000vip.h>
#endif

LF_BEGIN_BRIO_NAMESPACE()

//============================================================================
// CCameraModule: Informational functions
//============================================================================
//----------------------------------------------------------------------------
tVersion CVIPCameraModule::GetModuleVersion() const
{
	return kVIPCameraModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CVIPCameraModule::GetModuleName() const
{
	return &kVIPCameraModuleName;
}

//============================================================================
// Ctor & dtor
//============================================================================
CVIPCameraModule::CVIPCameraModule()
{
	tCaptureMode SVGA = {kCaptureFormatYUV420, 1024, 768, 1, 15};

	camCtx_.mode = SVGA;
	valid = InitCameraInt(&camCtx_.mode);
}

//----------------------------------------------------------------------------
CVIPCameraModule::~CVIPCameraModule()
{
}

//----------------------------------------------------------------------------
static Boolean SetOverlay(int fd, tVideoSurf* pSurf)
{
#ifndef EMULATION
	struct v4l2_framebuffer v4l2;

	switch(pSurf->format)
	{
	case kPixelFormatYUV420:
		v4l2.fmt.pixelformat = V4L2_PIX_FMT_YUV420;
		v4l2.fmt.priv = PIPE_TO_FMT_PRIV(VIP_OUTPUT_PIPE_DECIMATOR);
		break;
	case kPixelFormatYUYV422:
		v4l2.fmt.pixelformat = V4L2_PIX_FMT_YUYV;
		v4l2.fmt.priv = PIPE_TO_FMT_PRIV(VIP_OUTPUT_PIPE_CLIPPER);
		break;
	}

	v4l2.base = pSurf->buffer;

	v4l2.fmt.bytesperline =  pSurf->pitch;
	v4l2.fmt.width = pSurf->width;
	v4l2.fmt.height = pSurf->height;

	v4l2.fmt.priv |= DOUBLE_BUF_TO_FMT_PRIV(VIP_OVERLAY_DOUBLE_BUF);
	// FIXME: video layer device minor number
	v4l2.fmt.priv |= FB_TO_FMT_PRIV(2);

	if(ioctl(fd, VIDIOC_S_FBUF, &v4l2) < 0)
	{
		return false;
	}
#endif
	return true;
}

//----------------------------------------------------------------------------
static Boolean EnableOverlay(int fd, int enable)
{
#ifndef EMULATION
	if(ioctl(fd, VIDIOC_OVERLAY, &enable) < 0)
			return false;
#endif
	return true;
}

//----------------------------------------------------------------------------
tVidCapHndl CVIPCameraModule::StartVideoCapture(const CPath& path, tVideoSurf* pSurf,
		IEventListener * pListener, const U32 maxLength, Boolean bAudio)
{
	struct tCaptureMode mode = {kCaptureFormatYUV420, 400, 300, 1, 15};
	tVidCapHndl hndl = kInvalidVidCapHndl;

	if(pSurf)
	{
		/* Spawn hardware viewfinder */

		if(!SetOverlay(camCtx_.fd, pSurf))
			return hndl;

		if(!EnableOverlay(camCtx_.fd, 1))
			return hndl;

		hndl = kStreamingActive;
	}

	if(path.length())
	{
		/* Spawn capture thread w/out viewfinder */
		camCtx_.mode = mode;
		hndl = CCameraModule::StartVideoCapture(path, NULL, pListener, maxLength, bAudio);
	}

	return hndl;
}

//----------------------------------------------------------------------------
Boolean CVIPCameraModule::StopVideoCapture(const tVidCapHndl hndl)
{
	Boolean ret = true;

	if(hndl & kStreamingActive)
	{
		ret = EnableOverlay(camCtx_.fd, 0);
	}

	if(IS_THREAD_HANDLE(hndl))
	{
		return CCameraModule::StopVideoCapture(hndl);
	}

	return ret;
}

LF_END_BRIO_NAMESPACE()

//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()

static CVIPCameraModule*	sinst = NULL;

extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion version)
	{
		(void )version;	/* Prevent unused variable warnings. */
		if( sinst == NULL )
			sinst = new CVIPCameraModule;
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
