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
	tCaptureMode SVGA = {kCaptureFormatYUV420, 800, 600, 1, 30};
	tCaptureMode QSVGA = {kCaptureFormatYUV420, 400, 300, 1, 30};

	camCtx_.mode = SVGA;
	valid = InitCameraInt(&camCtx_.mode);

	// FIXME: mode list supposed to be returned via V4L
	if (camCtx_.modes->empty())
	{
		camCtx_.modes->push_back(new tCaptureMode(QSVGA));
		camCtx_.modes->push_back(new tCaptureMode(SVGA));
	}
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

	// FIXME: re-enable double buffering when it's safe to pan in IRQ context
	//v4l2.fmt.priv |= DOUBLE_BUF_TO_FMT_PRIV(VIP_OVERLAY_DOUBLE_BUF);
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
Boolean CVIPCameraModule::CompressFrame(tFrameInfo *frame, int stride)
{
	Boolean			ret = false;
	int				out_size = frame->width * frame->height * 3;
	uint8_t			*frame_outbuf	= NULL;

	AVCodec			*pCodec	= NULL;
	AVCodecContext	*pCtx	= NULL;
	AVFrame			*pFrame	= NULL;

	avcodec_register_all();

	pCodec = avcodec_find_encoder(CODEC_ID_MJPEG);
	if(pCodec == NULL)
		goto out;

	pCtx = avcodec_alloc_context();
	if(pCtx == NULL)
		goto out;

	pCtx->pix_fmt			= PIX_FMT_YUVJ420P;
	pCtx->width				= frame->width;
	pCtx->height			= frame->height;
	pCtx->codec_id			= CODEC_ID_MJPEG;
	pCtx->codec_type 		= CODEC_TYPE_VIDEO;
	pCtx->bit_rate 			= 400000;
	pCtx->time_base.den 	= 1;
	pCtx->time_base.num 	= 1;
	pCtx->gop_size 			= 1;

	if(avcodec_open(pCtx, pCodec) < 0)
		goto out_context;

	pFrame = avcodec_alloc_frame();
	if(pFrame == NULL)
		goto out_frame;

	frame_outbuf = (uint8_t*)av_malloc(out_size);
	if(frame_outbuf == NULL)
		goto out_outbuf;

	pFrame->data[0] = (uint8_t *)frame->data;
	pFrame->data[1] = pFrame->data[0] + stride/2;
	pFrame->data[2] = pFrame->data[1] + stride * pCtx->height/2;

	pFrame->linesize[0] = stride;
	pFrame->linesize[1] = stride;
	pFrame->linesize[2] = stride;

	out_size = avcodec_encode_video(pCtx, frame_outbuf, out_size, pFrame);
	if(out_size <= 0)
		goto out_copy;
		
	memcpy(frame->data, frame_outbuf, out_size);
	frame->size	= out_size;

	ret = true;

out_copy:
	av_free(frame_outbuf);

out_outbuf:
	av_free(pFrame);

out_frame:
	avcodec_close(pCtx);

out_context:
	av_free(pCtx);

out:
	return ret;
}

//----------------------------------------------------------------------------
tVidCapHndl CVIPCameraModule::StartVideoCapture(const CPath& path, tVideoSurf* pSurf,
		IEventListener * pListener, const U32 maxLength, Boolean bAudio)
{
	struct tCaptureMode qSVGA = {kCaptureFormatYUV420, 400, 300, 1, 30};
	tVidCapHndl hndl = kInvalidVidCapHndl;

	if(pSurf)
	{
		if(!(SetCameraMode(&qSVGA)))
				return hndl;

		/* Spawn hardware viewfinder */
		if(!SetOverlay(camCtx_.fd, pSurf))
			return hndl;

		if(!EnableOverlay(camCtx_.fd, 1))
			return hndl;

		overlaySurf = *pSurf;

		if (pSurf->format == kPixelFormatYUV420 || pSurf->format == kPixelFormatYUYV422)
			CCameraModule::SetScaler(pSurf->width, pSurf->height, false);

		hndl = kStreamingActive;
	}

	if(path.length())
	{
		/* Spawn capture thread w/out viewfinder */
		camCtx_.mode = qSVGA;
		hndl = CCameraModule::StartVideoCapture(path, NULL, pListener, maxLength, bAudio);
	}
	else if (hndl == kInvalidVidCapHndl)
	{
		// Start video stream without viewfinder or recorder thread
		camCtx_.mode = qSVGA;
//		hndl = CCameraModule::StartVideoCapture();
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
		overlaySurf.buffer	= NULL;
		overlaySurf.format	= kPixelFormatError;
		overlaySurf.height	= 0;
		overlaySurf.width	= 0;
		overlaySurf.pitch	= 0;
	}

	if(IS_THREAD_HANDLE(hndl))
	{
		return CCameraModule::StopVideoCapture(hndl);
	}

	return ret;
}

//----------------------------------------------------------------------------
Boolean	CVIPCameraModule::SnapFrame(const tVidCapHndl hndl, const CPath &path)
{
	Boolean				ret;
	tVidCapHndl			origHndl, tmpHndl;
	tFrameInfo			frame	= {kCaptureFormatYUV420, 800, 600, 0, NULL, 0};
	struct tCaptureMode	UXGA	= {kCaptureFormatYUV420, 800, 600, 1, 10};
	struct tCaptureMode	oldMode	= camCtx_.mode;
	tVideoSurf			oldSurf;

#if 0
	if (path.rfind(".png") != std::string::npos)
		return SnapFrameRGB(hndl, path);
#endif

	/*
	 * FIXME:
	 * The videobuf helper used in the VIP driver refuses to be reconfigured
	 * once buffers have been queued (i.e., STREAMOFF, S_FMT, STREAMON).
	 * Therefore, we must "start over" when taking a snapshot. 
	 */
	origHndl = hndl;

	/* Stop viewfinder */
	if(hndl & kStreamingActive)
		EnableOverlay(camCtx_.fd, 0);

	/* Close camera fd */
	ret = CCameraModule::DeinitCameraInt();
	if(!ret)
		return ret;

	/* Re-open camera */
	ret = CCameraModule::InitCameraInt(&UXGA);
	if(!ret)
		return ret;

	ret = GrabFrame(hndl, &frame);

	/* Restore old mode */
	ret = CCameraModule::DeinitCameraInt();
	if(!ret)
		goto out;

	ret = CCameraModule::InitCameraInt(&oldMode);
	if(!ret)
		goto out;

	if(hndl & kStreamingActive)
	{
		SetOverlay(camCtx_.fd, &overlaySurf);
		EnableOverlay(camCtx_.fd, 1);
	}

	if(!ret)
		goto out;

	/* Compress frame */
	ret = CompressFrame(&frame, 4096);
	if(!ret)
		goto out;

	ret = SaveFrame(path, &frame);

out:
	kernel_.Free(frame.data);

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
