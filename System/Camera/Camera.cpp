//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		Camera.cpp
//
// Description:
//		Camera module implementation.
//
//==============================================================================

#include <SystemTypes.h>
#include <SystemErrors.h>
#include <CameraTypes.h>
#include <CameraPriv.h>
//#include <AudioMPI.h>
#include <KernelMPI.h>
#include <DisplayMPI.h>
#include <errno.h>
#include <string.h>

//============================================================================
// This Brio implementation is a layer above the V4L2 Linux kernel API:
// http://www.linuxtv.org/downloads/video4linux/API/V4L2_API/spec/index.html
//
// It also utilizes libjpeg for image processing:
// http://www.ijg.org/
//============================================================================
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <jpeglib.h>

#define USE_PROFILE			0

LF_BEGIN_BRIO_NAMESPACE()
//============================================================================
// Constants
//============================================================================
const CURI	kModuleURI	= "/LF/System/Camera";
const char*	gCamFile	= "/dev/video0";
const U32 NUM_BUFS		= 16;

//==============================================================================
// Defines
//==============================================================================
#define CAMERA_LOCK dbg_.Assert((kNoErr == kernel_.LockMutex(mutex_)),\
									  "Couldn't lock mutex.\n")

#define CAMERA_UNLOCK dbg_.Assert((kNoErr == kernel_.UnlockMutex(mutex_)),\
										"Couldn't unlock mutex.\n");

//============================================================================
// CCameraModule: Informational functions
//============================================================================
//----------------------------------------------------------------------------
tVersion CCameraModule::GetModuleVersion() const
{
	return kCameraModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CCameraModule::GetModuleName() const
{
	return &kCameraModuleName;
}

//----------------------------------------------------------------------------
const CURI* CCameraModule::GetModuleOrigin() const
{
	return &kModuleURI;
}

//============================================================================
// Local variables
//============================================================================
namespace
{
	CPath				gpath = "";

	// Camera MPI global vars
	tCameraContext*		gpCamCtx = NULL;
#if USE_PROFILE
	// Profile vars
#endif
}

//============================================================================
// C function support
//============================================================================

//----------------------------------------------------------------------------
inline void PROFILE_BEGIN(void)
{
#if USE_PROFILE
	CKernelMPI	kernel;
	kernel.GetHRTAsUsec(usecStart);
#endif
}

//----------------------------------------------------------------------------
inline void PROFILE_END(const char* msg)
{
	(void )msg;	/* Prevent unused variable warnings. */
#if USE_PROFILE
	CKernelMPI	kernel;
	CDebugMPI	dbg(kGroupCamera);
	kernel.GetHRTAsUsec(usecEnd);
	usecDiff = usecEnd - usecStart;
	dbg.DebugOut(kDbgLvlVerbose, "%s: lapse time = %u\n", msg, static_cast<unsigned int>(usecDiff));
#endif
}

//----------------------------------------------------------------------------

//============================================================================
// Ctor & dtor
//============================================================================
CCameraModule::CCameraModule() : dbg_(kGroupCamera)
{
	tErrType			err = kNoErr;
	const tMutexAttr	attr = {0};

	dbg_.SetDebugLevel(kCameraDebugLevel);

	camCtx_.file 			= gCamFile;
	camCtx_.numBufs 		= 0;
	camCtx_.bufs 			= NULL;
	camCtx_.fd 				= -1;
	camCtx_.hndl 			= kInvalidVidCapHndl;
	camCtx_.hCameraThread	= kNull;

	camCtx_.surf			= NULL;
	camCtx_.rect			= NULL;

	camCtx_.modes = new tCaptureModes;

	err = kernel_.InitMutex( mutex_, attr );
	dbg_.Assert((kNoErr == err), "CCameraModule::ctor: Couldn't init mutex.\n");

	valid = InitCameraInt();
}

//----------------------------------------------------------------------------
CCameraModule::~CCameraModule()
{
	valid = DeinitCameraInt();

	kernel_.DeInitMutex(mutex_);

	delete camCtx_.modes;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::IsValid() const
{
	return valid;
}

//============================================================================
// Camera-specific Implementation
//============================================================================

//----------------------------------------------------------------------------
tErrType CCameraModule::SetCameraResourcePath(const CPath &path)
{
	CAMERA_LOCK;

	gpath = path;
	if (gpath.length() > 1 && gpath.at(gpath.length()-1) != '/')
		gpath += '/';

	CAMERA_UNLOCK;

	return kNoErr;
}

//----------------------------------------------------------------------------
CPath* CCameraModule::GetCameraResourcePath()
{
	CPath *path;

	CAMERA_LOCK;

	path = &gpath;

	CAMERA_UNLOCK;

	return path;
}

//----------------------------------------------------------------------------
static Boolean DeinitCameraBufferInt(tCameraContext *pCamCtx)
{
	struct v4l2_requestbuffers rb;
	int i, ret;

	if(pCamCtx->numBufs == 0)
	{
		return true;
	}

	for(i = 0; i < pCamCtx->numBufs; i++)
	{
		memset(&pCamCtx->buf, 0, sizeof(struct v4l2_buffer));

		pCamCtx->buf.index	= i;
		pCamCtx->buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		pCamCtx->buf.memory	= V4L2_MEMORY_MMAP;

		if(ioctl(pCamCtx->fd, VIDIOC_QUERYBUF, &pCamCtx->buf) < 0)
		{
			return false;
		}

		if(munmap(pCamCtx->bufs[i], pCamCtx->buf.length) < 0)
		{
			return false;
        }
	}

	memset(&rb, 0, sizeof(struct v4l2_requestbuffers));

	rb.count  = 0;
	rb.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rb.memory = V4L2_MEMORY_MMAP;

	if(ioctl(pCamCtx->fd, VIDIOC_REQBUFS, &rb) < 0)
	{
		return false;
	}

	delete[] pCamCtx->bufs;
	pCamCtx->bufs = NULL;

	pCamCtx->numBufs = 0;

	return true;
}

//----------------------------------------------------------------------------
static Boolean DeinitCameraFormatInt(tCameraContext *pCamCtx)
{
	tCaptureModes::iterator	mode_it;

	for( mode_it = pCamCtx->modes->begin() ; mode_it < pCamCtx->modes->end(); mode_it++ )
	{
		delete *mode_it;
	}

	pCamCtx->modes->clear();

	return true;
}

//----------------------------------------------------------------------------
static Boolean DeinitCameraHWInt(tCameraContext *pCamCtx)
{
	close(pCamCtx->fd);
	pCamCtx->fd = -1;

	return true;
}

//----------------------------------------------------------------------------
static Boolean InitCameraHWInt(tCameraContext *pCamCtx)
{
	int cam;
	struct v4l2_capability *cap =  &pCamCtx->cap;

	if((cam = open(pCamCtx->file, O_RDWR)) == -1)
	{
		return false;
	}

	if(ioctl(cam, VIDIOC_QUERYCAP, cap) == -1)
	{
		close(cam);
		return false;
	}

	if(!(cap->capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		close(cam);
		return false;
	}

	if(!(cap->capabilities & V4L2_CAP_STREAMING))
	{
		close(cam);
		return false;
	}

	pCamCtx->fd = cam;

	return true;
}

//----------------------------------------------------------------------------
static Boolean	EnumFramerates(tCameraContext *pCamCtx, v4l2_frmsizeenum *frm )
{
	int ret, idx = 0;
	struct v4l2_frmivalenum *fps;
	Boolean bRet = false;

	tCaptureMode *mode;

	std::vector<v4l2_frmivalenum*>				fpss;
	std::vector<v4l2_frmivalenum*>::iterator	fps_it;

	fps = new v4l2_frmivalenum;
	memset(fps, 0, sizeof(v4l2_frmivalenum));

	fps->pixel_format	= frm->pixel_format;
	fps->height			= frm->discrete.height;
	fps->width			= frm->discrete.width;
	fps->index			= 0;

	while((ret = ioctl(pCamCtx->fd, VIDIOC_ENUM_FRAMEINTERVALS, fps)) == 0)
	{
		// TODO: support other types?
		if(fps->type != V4L2_FRMIVAL_TYPE_DISCRETE)
		{
			ret = -2;
			break;
		}
		fpss.push_back(fps);

		fps = new v4l2_frmivalenum;
		memset(fps, 0, sizeof(v4l2_frmivalenum));

		fps->pixel_format	= frm->pixel_format;
		fps->height			= frm->discrete.height;
		fps->width			= frm->discrete.width;
		fps->index			= ++idx;
    }
	delete fps;

	if(ret == -1 && errno == EINVAL)
	{
		for( fps_it = fpss.begin() ; fps_it < fpss.end(); fps_it++ )
		{
			mode 					= new tCaptureMode;
			mode->fps_numerator		= (*fps_it)->discrete.numerator;
			mode->fps_denominator	= (*fps_it)->discrete.denominator;
			mode->width				= (*fps_it)->width;
			mode->height			= (*fps_it)->height;

			switch((*fps_it)->pixel_format)
			{
			case V4L2_PIX_FMT_MJPEG:
				mode->pixelformat = kCaptureFormatMJPEG;
				break;
			default:
				mode->pixelformat = kCaptureFormatError;
				break;
			}

			pCamCtx->modes->push_back(mode);
		}
		bRet = true;
	}


	for( fps_it = fpss.begin() ; fps_it < fpss.end(); fps_it++ )
	{
		delete *fps_it;
	}
	fpss.clear();

	return bRet;
}

//----------------------------------------------------------------------------
static Boolean EnumFramesizes(tCameraContext *pCamCtx, v4l2_fmtdesc *fmt )
{
	int ret, idx = 0;
	struct v4l2_frmsizeenum *frm;
	Boolean bRet = true;

	std::vector<v4l2_frmsizeenum*>				frms;
	std::vector<v4l2_frmsizeenum*>::iterator	frm_it;

	frm = new v4l2_frmsizeenum;
	memset(frm, 0, sizeof(v4l2_frmsizeenum));

	frm->pixel_format	= fmt->pixelformat;
	frm->index			= 0;

	while((ret = ioctl(pCamCtx->fd, VIDIOC_ENUM_FRAMESIZES, frm)) == 0)
	{
		// TODO: support more camera types (do they exist?)
		if(frm->type != V4L2_FRMSIZE_TYPE_DISCRETE)
		{
			bRet = false;
			break;
		}
		frms.push_back(frm);

		frm = new v4l2_frmsizeenum;
		memset(frm, 0, sizeof(v4l2_frmsizeenum));

		frm->pixel_format	= fmt->pixelformat;
		frm->index			= ++idx;
    }
	delete frm;

	if(ret == -1 && errno == EINVAL)
	{
		for(frm_it = frms.begin() ; frm_it < frms.end() && bRet == true; frm_it++)
		{
			bRet = EnumFramerates(pCamCtx, *frm_it);
		}
	}

	for(frm_it = frms.begin() ; frm_it < frms.end(); frm_it++)
	{
		delete *frm_it;
	}
	frms.clear();

	return bRet;
}

//----------------------------------------------------------------------------
static Boolean InitCameraFormatInt(tCameraContext *pCamCtx)
{
	tCaptureMode *mode;
	int ret, idx = 0;
	Boolean bRet = true;
	v4l2_fmtdesc *fmt;

	std::vector<v4l2_fmtdesc*>				fmts;
	std::vector<v4l2_fmtdesc*>::iterator	fmt_it;

	fmt = new v4l2_fmtdesc;
	memset(fmt, 0, sizeof(v4l2_fmtdesc));

	fmt->type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt->index	= 0;

	while((ret = ioctl(pCamCtx->fd, VIDIOC_ENUM_FMT, fmt)) == 0)
	{
		fmts.push_back(fmt);

		fmt = new v4l2_fmtdesc;
		memset(fmt, 0, sizeof(v4l2_fmtdesc));

		fmt->type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt->index	= ++idx;
    }
	delete fmt;

	if(ret == -1 && errno == EINVAL)
	{
		for(fmt_it = fmts.begin() ; fmt_it < fmts.end() && bRet == true; fmt_it++)
		{
			bRet = EnumFramesizes(pCamCtx, *fmt_it);
		}
	}

	for(fmt_it = fmts.begin() ; fmt_it < fmts.end(); fmt_it++)
	{
		delete *fmt_it;
	}
	fmts.clear();

	return bRet;
}

//----------------------------------------------------------------------------
static Boolean InitCameraBufferInt(tCameraContext *pCamCtx)
{
	struct v4l2_requestbuffers rb;
	int i, ret, cam = pCamCtx->fd;

	memset(&rb, 0, sizeof(struct v4l2_requestbuffers));

	rb.count  = pCamCtx->numBufs;
	rb.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rb.memory = V4L2_MEMORY_MMAP;

	if(ioctl(cam, VIDIOC_REQBUFS, &rb) < 0)
	{
		return false;
	}

	pCamCtx->bufs = new void*[pCamCtx->numBufs];
	memset(pCamCtx->bufs, 0, pCamCtx->numBufs * sizeof(void*));

	for(i = 0; i < pCamCtx->numBufs; i++)
	{
		memset(&pCamCtx->buf, 0, sizeof(struct v4l2_buffer));

		pCamCtx->buf.index	= i;
		pCamCtx->buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		pCamCtx->buf.memory	= V4L2_MEMORY_MMAP;

		if(ioctl(cam, VIDIOC_QUERYBUF, &pCamCtx->buf) < 0)
		{
			delete[] pCamCtx->bufs;
			pCamCtx->bufs = NULL;
			return false;
		}

		pCamCtx->bufs[i] = mmap(0, pCamCtx->buf.length, PROT_READ, MAP_SHARED, cam, pCamCtx->buf.m.offset);
        if(pCamCtx->bufs[i] == MAP_FAILED)
        {
        	// TODO: munmap()
			delete[] pCamCtx->bufs;
			pCamCtx->bufs = NULL;
			return false;
        }
	}

	for(i = 0; i < pCamCtx->numBufs; i++)
	{
		memset(&pCamCtx->buf, 0, sizeof(struct v4l2_buffer));

		pCamCtx->buf.index  = i;
		pCamCtx->buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		pCamCtx->buf.memory = V4L2_MEMORY_MMAP;

		if(ioctl(pCamCtx->fd, VIDIOC_QBUF, &pCamCtx->buf) < 0)
		{
			delete[] pCamCtx->bufs;
			pCamCtx->bufs = NULL;
			return false;
        }
	}

	return true;
}

//----------------------------------------------------------------------------
static Boolean InitCameraStartInt(tCameraContext *pCamCtx)
{
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if(ioctl(pCamCtx->fd, VIDIOC_STREAMON, &type) < 0)
	{
		return false;
    }

	return true;
}

//----------------------------------------------------------------------------
Boolean CCameraModule::GetCameraModes(tCaptureModes &modes)
{
	CAMERA_LOCK;

	modes = *(camCtx_.modes);

	CAMERA_UNLOCK;

	return true;
}

//----------------------------------------------------------------------------
Boolean CCameraModule::SetCameraMode(const tCaptureMode* mode)
{
	struct v4l2_format fmt;
    struct v4l2_streamparm fps;


	/* format and resolution */
	memset(&fmt, 0, sizeof(struct v4l2_format));
	fmt.type				= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width		= mode->width;
	fmt.fmt.pix.height		= mode->height;
	fmt.fmt.pix.field		= V4L2_FIELD_ANY;

	switch(mode->pixelformat)
	{
	case kCaptureFormatMJPEG:
		fmt.fmt.pix.pixelformat	= V4L2_PIX_FMT_MJPEG;
		break;
	}

	CAMERA_LOCK;

    if(ioctl(camCtx_.fd, VIDIOC_S_FMT, &fmt) < 0)
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::SetCameraMode: mode selection failed for %s, %d\n", camCtx_.file, errno);
		CAMERA_UNLOCK;
		return false;
	}

    /* framerate */
    memset(&fps, 0, sizeof(struct v4l2_streamparm));
    fps.type									= V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fps.parm.capture.timeperframe.numerator		= mode->fps_numerator;
    fps.parm.capture.timeperframe.denominator	= mode->fps_denominator;

    if(ioctl(camCtx_.fd, VIDIOC_S_PARM, &fps) < 0)
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::SetCameraMode: fps selection failed for %s\n", camCtx_.file);
		CAMERA_UNLOCK;
		return false;
	}

    CAMERA_UNLOCK;

	return true;
}

//----------------------------------------------------------------------------
Boolean CCameraModule::SetBuffers(const U32 numBuffers)
{
	CAMERA_LOCK;

	if(camCtx_.hndl != kInvalidVidCapHndl)
	{
		CAMERA_UNLOCK;
		return false;
	}

	if(camCtx_.numBufs == numBuffers)
	{
		CAMERA_UNLOCK;
		return true;
	}

	if(!DeinitCameraBufferInt(&camCtx_))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::SetBuffers: buffer unmapping failed for %s\n", camCtx_.file);
		CAMERA_UNLOCK;
		return false;
	}

	camCtx_.numBufs = numBuffers;
	if(!InitCameraBufferInt(&camCtx_))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::SetBuffers: buffer mapping failed for %s\n", camCtx_.file);
		CAMERA_UNLOCK;
		return false;
	}

	CAMERA_UNLOCK;
	return true;
}

//----------------------------------------------------------------------------
tVidCapHndl	CCameraModule::StartVideoCapture()
{
	int i;

 	CAMERA_LOCK;

	if(camCtx_.hndl != kInvalidVidCapHndl)
	{
		CAMERA_UNLOCK;
		return kInvalidVidCapHndl;
	}

	if(!InitCameraStartInt(&camCtx_))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::StartVideoCapture: streaming failed for %s\n", camCtx_.file);
		CAMERA_UNLOCK;
		return kInvalidVidCapHndl;
	}

	// TODO: multiple handles?
	camCtx_.hndl = (kStreamingActive | kStreamingFrame | 1);

	CAMERA_UNLOCK;
	return camCtx_.hndl;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::PollFrame(const tVidCapHndl hndl)
{
	int i = 0;

	CAMERA_LOCK;

	if(!STREAMING_HANDLE(hndl) || !FRAME_HANDLE(hndl))
	{
		CAMERA_UNLOCK;
		return false;
	}

	for(i = 0; i < camCtx_.numBufs; i++)
	{
		memset(&camCtx_.buf, 0, sizeof(struct v4l2_buffer));
		camCtx_.buf.index  = i;
		camCtx_.buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		camCtx_.buf.memory = V4L2_MEMORY_MMAP;
		if(ioctl(camCtx_.fd, VIDIOC_QUERYBUF, &camCtx_.buf) < 0)
		{
			continue;
		}

		if (camCtx_.buf.flags & V4L2_BUF_FLAG_DONE) {
			CAMERA_UNLOCK;
			return true;
		}
	}

	CAMERA_UNLOCK;
	return false;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::GetFrame(const tVidCapHndl hndl, tFrameInfo *frame)
{
	if(!STREAMING_HANDLE(hndl) || !FRAME_HANDLE(hndl))
	{
		return false;
	}

	CAMERA_LOCK;

	memset(&camCtx_.buf, 0, sizeof(struct v4l2_buffer));
	camCtx_.buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	camCtx_.buf.memory = V4L2_MEMORY_MMAP;
	if( ioctl(camCtx_.fd, VIDIOC_DQBUF, &camCtx_.buf) < 0)
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::GetFrame: failed to capture frame from %s\n", camCtx_.file);
		CAMERA_UNLOCK;
		return false;
    }

	frame->index	= camCtx_.buf.index;
	frame->data		= camCtx_.bufs[camCtx_.buf.index];
	frame->size		= camCtx_.buf.bytesused;

	CAMERA_UNLOCK;
	return true;
}

//----------------------------------------------------------------------------
Boolean CCameraModule::RenderFrame(tFrameInfo *frame, tBitmapInfo *bitmap)
{
	struct jpeg_decompress_struct	cinfo;
	struct jpeg_error_mgr			err;
	JSAMPARRAY						buffer = NULL;
	int 							row = 0;
	int								row_stride;

	/*
	 * libjpeg expects an array of pointers to bitmap output rows.
	 * The user supplies one contiguous array for bitmap output, so construct
	 * a JSAMPARRAY pointing to each row.
	 */
	buffer = static_cast<U8**>(kernel_.Malloc(bitmap->height * sizeof(U8*)));

	row_stride = bitmap->width;
	if(bitmap->format == kBitmapFormatYCbCr888 || bitmap->format == kBitmapFormatRGB888)
	{
		row_stride *= 3;
	}

	for(int i = 0; i < bitmap->height; i++)
	{
		buffer[i] = &bitmap->data[i*row_stride];
	}

	cinfo.err = jpeg_std_error(&err);

	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, (U8*)frame->data, frame->size);

	(void) jpeg_read_header(&cinfo, TRUE);

	switch(bitmap->format)
	{
	case kBitmapFormatYCbCr888:
		cinfo.out_color_space = JCS_YCbCr;
		break;
	case kBitmapFormatRGB888:
		cinfo.out_color_space = JCS_RGB;
		break;
	case kBitmapFormatGrayscale8:
		break;
	case kBitmapFormatError:
		/* fall through */
	default:
		;
	}

	/*
	 * libjpeg natively supports 1/1, 1/2, 1/4, and 1/8 scaling.
	 */
	cinfo.scale_num = 1;
	cinfo.scale_denom = frame->height / bitmap->height;

	(void) jpeg_start_decompress(&cinfo);

	row_stride = cinfo.output_width * cinfo.output_components;

	while (cinfo.output_scanline < cinfo.output_height) {
	    /* jpeg_read_scanlines expects an array of pointers to scanlines.
	     * Here the array is only one element long, but you could ask for
	     * more than one scanline at a time if that's more convenient.
	     */
	    row += jpeg_read_scanlines(&cinfo, &buffer[row], cinfo.output_height);
	}

	(void) jpeg_finish_decompress(&cinfo);

	jpeg_destroy_decompress(&cinfo);

	kernel_.Free(buffer);

	return true;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::PutFrame(const tVidCapHndl hndl, const tFrameInfo *frame)
{
	if(!STREAMING_HANDLE(hndl) || !FRAME_HANDLE(hndl))
	{
		return false;
	}

	CAMERA_LOCK;

	memset(&camCtx_.buf, 0, sizeof(struct v4l2_buffer));
	camCtx_.buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	camCtx_.buf.memory = V4L2_MEMORY_MMAP;
	camCtx_.buf.index = frame->index;
	if(ioctl(camCtx_.fd, VIDIOC_QBUF, &camCtx_.buf) < 0)
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::PutFrame: failed to return frame %lu to %s\n", frame->index, camCtx_.file);
		CAMERA_UNLOCK;
		return false;
    }

	CAMERA_UNLOCK;
	return true;
}

//----------------------------------------------------------------------------
tVidCapHndl CCameraModule::StartVideoCapture(const CPath& path, Boolean audio, tVideoSurf* pSurf, tRect *rect)
{
	CDisplayMPI dispmgr;
	U32 row;
	int pix = 0xFF;
	U8* buf = pSurf->buffer;

	CAMERA_LOCK;

	if(camCtx_.hndl != kInvalidVidCapHndl)
	{
		CAMERA_UNLOCK;
		return kInvalidVidCapHndl;
	}

	dbg_.Assert((audio == false), "Audio recording not implemented!.\n");

	camCtx_.surf	= pSurf;
	camCtx_.rect	= rect;

	if(camCtx_.fd == -1)
	{
		InitCameraInt();
		DeinitCameraInt();
	}

	InitCameraTask(&camCtx_);

//	for(row = 0; row < pSurf->height; row++)
//	{
//		memset(buf, /*rand()*/pix--, pSurf->pitch);
//		buf += pSurf->pitch;
//		if(pix < 0)
//		{
//			pix = 0xFF;
//		}
//	}

//	dispmgr.Invalidate(0, rect);

#if 0
	// save-to-file requested
	if(path != NULL /* && path exists */)
	{

	}
#endif
	// render-to-display requested
	if(pSurf)
	{

	}


#if 0
	int	layer, r, dw, dh;
	union mlc_cmd c;

	// Open video layer device
	layer = open("/dev/layer2", O_RDWR|O_SYNC);
	if (layer < 0)
		return;

	// Get position info when video context was created
	r = ioctl(layer, MLC_IOCGPOSITION, &c);
#endif

	camCtx_.hndl = (kStreamingActive | kStreamingThread | 1);
	CAMERA_UNLOCK;
	return camCtx_.hndl;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::StopVideoCapture(const tVidCapHndl hndl)
{
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if(!STREAMING_HANDLE(hndl))
	{
		return false;
	}

	CAMERA_LOCK;

	if(ioctl(camCtx_.fd, VIDIOC_STREAMOFF, &type) < 0)
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::StopVideoCapture: failed to halt streaming from %s\n", camCtx_.file);
		CAMERA_UNLOCK;
		return false;
    }

	camCtx_.hndl 	= kInvalidVidCapHndl;

	CAMERA_UNLOCK;
	return true;
}

Boolean	CCameraModule::InitCameraInt()
{
	if(!InitCameraHWInt(&camCtx_))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::InitCameraInt: hardware initialization failed for %s\n", camCtx_.file);
		return false;
	}

	if(!InitCameraFormatInt(&camCtx_))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::InitCameraInt: format probing failed for %s\n", camCtx_.file);
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::DeinitCameraInt()
{
	if(!DeinitCameraBufferInt(&camCtx_))
	{
		return false;
	}

	if(!DeinitCameraFormatInt(&camCtx_))
	{
		return false;
	}

	if(!DeinitCameraHWInt(&camCtx_))
	{
		return false;
	}

	return true;
}
//----------------------------------------------------------------------------

LF_END_BRIO_NAMESPACE()


//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()

static CCameraModule*	sinst = NULL;

extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion version)
	{
		(void )version;	/* Prevent unused variable warnings. */
		if( sinst == NULL )
			sinst = new CCameraModule;
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


// EOF
