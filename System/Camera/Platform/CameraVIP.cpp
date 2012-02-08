#include <SystemTypes.h>

#include <CameraPriv.h>
#include <DisplayPriv.h>
#include <DisplayMPI.h>

#ifndef EMULATION
#include <sys/ioctl.h>
#include <linux/lf2000/lf2000vip.h>
#endif

// PNG wrapper function for saving file
bool PNG_save(const char* file, int width, int height, int pitch, char* data);

LF_BEGIN_BRIO_NAMESPACE()

#define CAMERA_LOCK 	kernel_.LockMutex(mutex_)
#define CAMERA_UNLOCK	kernel_.UnlockMutex(mutex_)

tCaptureMode VGA  = {kCaptureFormatYUV420, 640, 480, 1, 30};
tCaptureMode QVGA = {kCaptureFormatYUV420, 320, 240, 1, 30};
tCaptureMode SVGA = {kCaptureFormatRAWYUYV, 800, 600, 1, 10};
tCaptureMode QSVGA = {kCaptureFormatYUV420, 400, 300, 1, 30};
tCaptureMode WXGA  = {kCaptureFormatRAWYUYV, 1280, 800, 1, 10};
tCaptureMode SXGA  = {kCaptureFormatRAWYUYV, 1280, 960, 1, 10};
tCaptureMode HD16  = {kCaptureFormatRAWYUYV, 1600, 900, 1, 10};
tCaptureMode UXGA  = {kCaptureFormatRAWYUYV, 1600, 1200, 1, 10};

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

//----------------------------------------------------------------------------
tErrType CVIPCameraModule::EnumFormats(tCaptureModes& pModeList)
{
	// Filter supported mode list per high-res flag
	int xmax = 1600;
	int ymax = 1200;
	FILE* fp = fopen("/flags/high-res", "r");
	if (fp)
	{
		fscanf(fp, "%dx%d", &xmax, &ymax);
		fclose(fp);
	}

	// FIXME: mode list supposed to be returned via V4L
	if (camCtx_.modes->empty())
	{
		camCtx_.modes->push_back(new tCaptureMode(QSVGA));
		camCtx_.modes->push_back(new tCaptureMode(SVGA));
		camCtx_.modes->push_back(new tCaptureMode(QVGA));
		camCtx_.modes->push_back(new tCaptureMode(VGA));
		if (xmax >= WXGA.width && ymax >= WXGA.height)
			camCtx_.modes->push_back(new tCaptureMode(WXGA));
		if (xmax >= SXGA.width && ymax >= SXGA.height)
			camCtx_.modes->push_back(new tCaptureMode(SXGA));
		if (xmax >= HD16.width && ymax >= HD16.height)
			camCtx_.modes->push_back(new tCaptureMode(HD16));
		if (xmax >= UXGA.width && ymax >= UXGA.height)
			camCtx_.modes->push_back(new tCaptureMode(UXGA));
	}

	return CCameraModule::EnumFormats(pModeList);
}

//============================================================================
// Ctor & dtor
//============================================================================
CVIPCameraModule::CVIPCameraModule()
{
	camCtx_.mode = QVGA;
	valid = InitCameraInt(&camCtx_.mode);

	// FIXME: mode list supposed to be returned via V4L
	if (camCtx_.modes->empty())
	{
		EnumFormats(*camCtx_.modes);
	}

	overlaySurf.buffer	= NULL;
	overlaySurf.format	= kPixelFormatError;
	overlaySurf.height	= 0;
	overlaySurf.width	= 0;
	overlaySurf.pitch	= 0;
	overlayEnabled 		= false;
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

	memset(&v4l2, 0, sizeof(v4l2));
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
	default:
		return false;
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
Boolean CVIPCameraModule::EnableOverlay(int fd, int enable)
{
#ifndef EMULATION
	if(ioctl(fd, VIDIOC_OVERLAY, &enable) < 0)
			return false;
	overlayEnabled = enable;
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
	struct tCaptureMode qSVGA = {kCaptureFormatYUV420, 320, 240, 1, 30};
	tVidCapHndl hndl = kInvalidVidCapHndl;

	CAMERA_LOCK;

	if (IS_STREAMING_HANDLE(camCtx_.hndl))
	{
		if (!EnableOverlay(camCtx_.fd, 0))
			goto out;
	}

	// FIXME: re-init required always
	if (true || camCtx_.mode.width != qSVGA.width || camCtx_.mode.height != qSVGA.height)
	{
		/* Close camera fd */
		if (!CCameraModule::DeinitCameraInt())
			goto out;

		/* Re-open camera */
		if (!CCameraModule::InitCameraInt(&qSVGA))
			goto out;
	}

	if(path.length())
	{
		CAMERA_UNLOCK;

		/* Spawn capture thread w/out viewfinder */
		camCtx_.mode = qSVGA;
		hndl = CCameraModule::StartVideoCapture(path, NULL, pListener, maxLength, bAudio);

		CAMERA_LOCK;
	}

	if (pSurf)
	{
		/* Spawn hardware viewfinder */
		if(!SetOverlay(camCtx_.fd, pSurf))
			goto out;

		if(!EnableOverlay(camCtx_.fd, 1))
			goto out;

		overlaySurf = *pSurf;

		if (hndl == kInvalidVidCapHndl)
			hndl = kStreamingActive;

		CDisplayMPI display;
		display.Invalidate(0);
	}

out:
	CAMERA_UNLOCK;

	return hndl;
}

//----------------------------------------------------------------------------
Boolean CVIPCameraModule::StopVideoCapture(const tVidCapHndl hndl)
{
	Boolean ret = true;

	CAMERA_LOCK;

	if(hndl & kStreamingActive)
	{
		ret = EnableOverlay(camCtx_.fd, 0);
		overlaySurf.buffer	= NULL;
		overlaySurf.format	= kPixelFormatError;
		overlaySurf.height	= 0;
		overlaySurf.width	= 0;
		overlaySurf.pitch	= 0;
	}

	CAMERA_UNLOCK;

	if(IS_THREAD_HANDLE(hndl))
	{
		return CCameraModule::StopVideoCapture(hndl);
	}

	camCtx_.hndl = kInvalidVidCapHndl;

	return ret;
}

//----------------------------------------------------------------------------
Boolean CVIPCameraModule::PauseVideoCapture(const tVidCapHndl hndl, const Boolean display)
{
	CAMERA_LOCK;
	if (hndl & kStreamingActive)
		 EnableOverlay(camCtx_.fd, display ? 0 : 1);
	CAMERA_UNLOCK;
	return CCameraModule::PauseVideoCapture(hndl, display);
}

//----------------------------------------------------------------------------
Boolean CVIPCameraModule::ResumeVideoCapture(const tVidCapHndl hndl)
{
	CAMERA_LOCK;
	if (hndl & kStreamingActive)
		 EnableOverlay(camCtx_.fd, (overlaySurf.buffer != NULL) ? 1 : 0);
	CAMERA_UNLOCK;
	return CCameraModule::ResumeVideoCapture(hndl);
}

//----------------------------------------------------------------------------
Boolean	CVIPCameraModule::SnapFrame(const tVidCapHndl hndl, const CPath &path)
{
	Boolean				ret		= false;
	tFrameInfo			frame	= {kCaptureFormatYUV420, 640, 480, 0, NULL, 0};
	tVideoSurf			surf	= {frame.width, frame.height, 4096, NULL, kPixelFormatYUV420};

	// Grab video frame as RGB format for saving as PNG
	if (path.rfind(".png") != std::string::npos)
	{
		frame.size = frame.width * frame.height * 3;
		frame.data = kernel_.Malloc(frame.size);

		surf.buffer = (U8*)frame.data;
		surf.format = kPixelFormatRGB888;
		surf.pitch  = frame.width * 3;
		ret = CVIPCameraModule::GetFrame(hndl, &surf, kDisplayRgb);
		if (!ret)
			goto out;

		ret = PNG_save(path.c_str(), surf.width, surf.height, surf.pitch, (char*)surf.buffer);
		goto out;
	}

	// Grab video frame as YUV format for saving as JPEG
	frame.size = 4096 * frame.height;
	frame.data = kernel_.Malloc(frame.size);

	surf.buffer = (U8*)frame.data;
	surf.format = kPixelFormatYUV420;
	surf.pitch  = 4096;
	ret = CVIPCameraModule::GetFrame(hndl, &surf, kDisplayRgb);
	if(!ret)
		goto out;

	/* Compress frame */
	ret = CompressFrame(&frame, 4096);
	if(!ret)
		goto out;

	ret = SaveFrame(path, &frame);

out:
	if (frame.data)
		kernel_.Free(frame.data);

	return ret;
}

//----------------------------------------------------------------------------
Boolean	CVIPCameraModule::GetFrame(const tVidCapHndl hndl, U8 *pixels, tColorOrder color_order)
{
	tVideoSurf 	surf = {640, 480, 640*3, pixels, kPixelFormatRGB888};

	return CVIPCameraModule::GetFrame(hndl, &surf, color_order);
}

//----------------------------------------------------------------------------
Boolean	CVIPCameraModule::GetFrame(const tVidCapHndl hndl, tVideoSurf *pSurf, tColorOrder color_order)
{
	Boolean ret;
	tFrameInfo frame	= {kCaptureFormatYUV420, pSurf->width, pSurf->height, 0, NULL, 0};
	struct tCaptureMode	oldMode	= camCtx_.mode;
	struct tCaptureMode	newMode = {kCaptureFormatYUV420, pSurf->width, pSurf->height, 1, 10};
	Boolean				visible = (overlaySurf.buffer != NULL && overlayEnabled) ? 1 : 0;

	// Switch to YUYV packed format for high-res capture modes
	if (pSurf->width > VGA.width || pSurf->height > VGA.height)
	{
		newMode.pixelformat = frame.pixelformat = kCaptureFormatRAWYUYV;
	}

	CAMERA_LOCK;

	/* Stop viewfinder */
	if (hndl & kStreamingActive)
		EnableOverlay(camCtx_.fd, 0);

	/* Close camera fd */
	ret = CCameraModule::DeinitCameraInt();
	if (!ret)
		goto out;

	/* Re-open camera */
	ret = CCameraModule::InitCameraInt(&newMode);
	if (!ret)
		goto out;

	CAMERA_UNLOCK;

	ret = CCameraModule::GrabFrame(hndl, &frame);

	CAMERA_LOCK;

	if (ret)
	{
		int			i,j,k,m;
		int			r = (color_order == kDisplayRgb) ? 2 : 0;
		int			b = (color_order == kDisplayRgb) ? 0 : 2;
		int			pitch = frame.size / frame.height;
		int			width = MIN(frame.width, pSurf->width);
		int			height = MIN(frame.height, pSurf->height);
		U8* 		sy = (U8*)frame.data;
		U8*			su = sy + pitch/2;
		U8*			sv = su + pitch * frame.height/2;
		U8*			dy = pSurf->buffer;
		U8*			du = dy + pSurf->pitch/2;
		U8*			dv = du + pSurf->pitch * pSurf->height/2;
		if (frame.pixelformat == kCaptureFormatRAWYUYV)
		{
			pitch = frame.width * 2;
			if (pSurf->format == kPixelFormatRGB888)
			{
				// Convert YUYV to RGB format surface
				for (i = 0; i < height; i++)
				{
					for (j = k = m = 0; j < width; j+=2, k+=4, m+=6)
					{
						U8 y0 = sy[k+0];
						U8 v0 = sy[k+1];
						U8 y1 = sy[k+2];
						U8 u0 = sy[k+3];
						dy[m+0] = B(y0,u0,v0);
						dy[m+1] = G(y0,u0,v0);
						dy[m+2] = R(y0,u0,v0);
						dy[m+3] = B(y1,u0,v0);
						dy[m+4] = G(y1,u0,v0);
						dy[m+5] = R(y1,u0,v0);
					}
					sy += pitch;
					dy += pSurf->pitch;
				}
			}
			else if (pSurf->format == kPixelFormatARGB8888)
			{
				// Convert YUYV to ARGB format surface
				for (i = 0; i < height; i++)
				{
					for (j = k = m = 0; j < width; j+=2, k+=4, m+=8)
					{
						U8 y0 = sy[k+0];
						U8 u0 = sy[k+1];
						U8 y1 = sy[k+2];
						U8 v0 = sy[k+3];
						dy[m+0] = B(y0,u0,v0);
						dy[m+1] = G(y0,u0,v0);
						dy[m+2] = R(y0,u0,v0);
						dy[m+3] = 0xFF;
						dy[m+4] = B(y1,u0,v0);
						dy[m+5] = G(y1,u0,v0);
						dy[m+6] = R(y1,u0,v0);
						dy[m+7] = 0xFF;
					}
					sy += pitch;
					dy += pSurf->pitch;
				}
			}
		}
		else if (pSurf->format == kPixelFormatRGB888 || pSurf->pitch == 3 * pSurf->width)
		{
			// Convert YUV to RGB format surface
			for (i = 0; i < height; i++)
			{
				for (j = k = m = 0; k < width; j++, k+=2, m+=6)
				{
					U8 y0 = sy[k];
					U8 y1 = sy[k+1];
					U8 u0 = su[j];
					U8 v0 = sv[j];
					dy[m+b] = B(y0,u0,v0);
					dy[m+1] = G(y0,u0,v0);
					dy[m+r] = R(y0,u0,v0);
					dy[m+3+b] = B(y1,u0,v0);
					dy[m+4]   = G(y1,u0,v0);
					dy[m+3+r] = R(y1,u0,v0);
				}
				sy += pitch;
				dy += pSurf->pitch;
				if (i % 2)
				{
					su += pitch;
					sv += pitch;
				}
			}
		}
		else if (pSurf->format == kPixelFormatARGB8888)
		{
			// Convert YUV to ARGB format surface
			for (i = 0; i < height; i++)
			{
				for (j = k = m = 0; k < width; j++, k+=2, m+=8)
				{
					U8 y0 = sy[k];
					U8 y1 = sy[k+1];
					U8 u0 = su[j];
					U8 v0 = sv[j];
					dy[m+b] = B(y0,u0,v0);
					dy[m+1] = G(y0,u0,v0);
					dy[m+r] = R(y0,u0,v0);
					dy[m+3] = 0xFF;
					dy[m+4+b] = B(y1,u0,v0);
					dy[m+5]   = G(y1,u0,v0);
					dy[m+4+r] = R(y1,u0,v0);
					dy[m+7] = 0xFF;
				}
				sy += pitch;
				dy += pSurf->pitch;
				if (i % 2)
				{
					su += pitch;
					sv += pitch;
				}
			}
		}
		else if (pSurf->format == kPixelFormatYUV420)
		{
			for (i = 0; i < height; i++)
			{
				memcpy(dy, sy, width);
				sy += pitch;
				dy += pSurf->pitch;
				if (i % 2)
				{
					memcpy(du, su, width/2);
					memcpy(dv, sv, width/2);
					su += pitch;
					sv += pitch;
					du += pSurf->pitch;
					dv += pSurf->pitch;
				}
			}
		}
	}

	if (frame.data)
		kernel_.Free(frame.data);

	/* Close camera fd */
	ret = CCameraModule::DeinitCameraInt();
	if (!ret)
		goto out;

	/* Re-open camera */
	ret = CCameraModule::InitCameraInt(&oldMode);
	if (!ret)
		goto out;

	if (hndl & kStreamingActive)
	{
		SetOverlay(camCtx_.fd, &overlaySurf);
		EnableOverlay(camCtx_.fd, visible);
		camCtx_.hndl = hndl;
	}

out:
	CAMERA_UNLOCK;

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
