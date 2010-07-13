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
#include <KernelMPI.h>
#include <DisplayMPI.h>
#include <Utility.h>
#include <USBDeviceMPI.h>
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
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/statvfs.h>

/* For libjpeg error handling.  TODO: get rid of this somehow? */
#include <setjmp.h>

#ifndef EMULATION
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/lf1000/mlc_ioctl.h>
#endif

// PNG wrapper function for saving file
bool PNG_save(const char* file, int width, int height, int pitch, char* data);

#define USE_PROFILE			0

LF_BEGIN_BRIO_NAMESPACE()
//============================================================================
// Constants
//============================================================================
const CURI	kModuleURI	= "/LF/System/Camera";
const char*	gCamFile	= "/dev/video0";
const char*	gIDCTFile	= "/dev/idct";
const U32	NUM_BUFS	= 3;
const U32	VID_BITRATE	= 275*1024;			/* ~240 KB/s video, 31.25 KB/s audio */

//==============================================================================
// Defines
//==============================================================================
#define USB_DEV_ROOT				"/sys/class/usb_device/"

#define CAMERA_LOCK dbg_.Assert((kNoErr == kernel_.LockMutex(mutex_)),\
									  "Couldn't lock mutex.\n")

#define CAMERA_UNLOCK dbg_.Assert((kNoErr == kernel_.UnlockMutex(mutex_)),\
										"Couldn't unlock mutex.\n");

#define DATA_LOCK dbg_.Assert((kNoErr == kernel_.LockMutex(dlock)),\
									  "Couldn't lock mutex.\n")

#define DATA_UNLOCK dbg_.Assert((kNoErr == kernel_.UnlockMutex(dlock)),\
										"Couldn't unlock mutex.\n");

#define THREAD_LOCK dbg_.Assert((kNoErr == kernel_.LockMutex(camCtx_.mThread)),\
									  "Couldn't lock mutex.\n")

#define THREAD_UNLOCK dbg_.Assert((kNoErr == kernel_.UnlockMutex(camCtx_.mThread)),\
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
	CPath				vpath = "";
	CPath				spath = "";
	tMutex				dlock;

	S8					nLUT[1025];	/* To convert 9-bit HW IDCT output to 8-bit color values.
									 * Should only be [512], but the IDCT sometimes erroneously
									 * produces 256 (0x0100) or 768 (0x0300)
									 */
	S8					*NormLUT;	/* Offset table indicies by 256 */
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
// Set video scaler (embedded only)
static void SetScaler(int width, int height, bool centered)
{
#ifndef EMULATION
	int	layer, r, dw, dh;
	union mlc_cmd c;

	// Open video layer device
	layer = open("/dev/layer2", O_RDWR|O_SYNC);
	if (layer < 0)
		return;

	// Get position info when video context was created
	r = ioctl(layer, MLC_IOCGPOSITION, &c);
	dw = (r == 0) ? c.position.right - c.position.left + 0 : 320;
	dh = (r == 0) ? c.position.bottom - c.position.top + 0 : 240;
	
	// Reposition with fullscreen centering instead of scaling
	if (centered) 
	{
		dw = width;
		dh = height;		
		c.position.left = (320 - width) / 2;
		c.position.right = c.position.left + width;
		c.position.top = (240 - height) / 2;
		c.position.bottom = c.position.top + height;
		ioctl(layer, MLC_IOCSPOSITION, &c);
	}
	// Special case handling for 320x176 scaling (non-letterbox)
	else if (176 == height && 320 == width) 
	{
		dw = 320;
		dh = 240;
		c.position.left = c.position.top = 0;
		c.position.right = c.position.left + dw;
		c.position.bottom = c.position.top + dh;
		ioctl(layer, MLC_IOCSPOSITION, &c);
	}

	// Set video scaler for video source and screen destination
	c.overlaysize.srcwidth = width;
	c.overlaysize.srcheight = height;
	c.overlaysize.dstwidth = dw; 
	c.overlaysize.dstheight = dh; 
	ioctl(layer, MLC_IOCSOVERLAYSIZE, &c);
	ioctl(layer, MLC_IOCTDIRTY, 0);

	close(layer);
#endif
}

//----------------------------------------------------------------------------

//============================================================================
// Local event listener
//============================================================================
const tEventType LocalCameraEvents[] = {kAllUSBDeviceEvents};

Boolean EnumCameraCallback(const CPath& path, void* pctx)
{
	CCameraModule* pObj	= (CCameraModule*)pctx;
	U32 id				= FindDevice(path);

	if (id == USB_CAM_ID) {
		pObj->sysfs = path;
		return false; // stop
	}
	return true; // continue
}

CCameraModule::CameraListener::CameraListener(CCameraModule* mod):
			IEventListener(LocalCameraEvents, ArrayCount(LocalCameraEvents)),
			pMod(mod), running(false)
			{}

CCameraModule::CameraListener::~CameraListener()
{
	/*
	 * There is a potential race here if a CameraModule object is deleted just
	 * as the Notify() method is invoked by the either the EventDispatch thread or
	 * ButtonPowerUSB thread.  The proper fix would be to delay destruction until
	 * these threads have join()ed, but that is not possible, because:
	 *  a.) those threads are join()ed only when there are no outstanding clients
	 *  b.) this listener object itself counts as an outstanding client
	 *
	 *  This hack seems to make the race unlikely.
	 */
	while(running)
		pMod->kernel_.TaskSleep(1);
}

tEventStatus CCameraModule::CameraListener::Notify(const IEventMessage& msg)
{
	running = true;

	tEventType event_type = msg.GetEventType();
	if(event_type == kUSBDevicePriorityStateChange)
	{
		const CUSBDeviceMessage& usbmsg = dynamic_cast<const CUSBDeviceMessage&>(msg);
		tUSBDeviceData usbData = usbmsg.GetUSBDeviceState();

		/* a device was inserted or removed */
		if(usbData.USBDeviceState & kUSBDeviceHotPlug)
		{
			/* enumerate sysfs to see if camera entry exists */
			pMod->sysfs.clear();
			EnumFolder(USB_DEV_ROOT, EnumCameraCallback, kFoldersOnly, pMod);

			if(!pMod->sysfs.empty())
			{
				/* camera present or added */
				pMod->kernel_.LockMutex(pMod->mutex_);
				pMod->valid = pMod->InitCameraInt();
				pMod->kernel_.UnlockMutex(pMod->mutex_);
			}
			else
			{
				if(pMod->valid)
				{
					/* camera removed */

					/* set this to inform capture threads */
					pMod->valid = false;
					pMod->StopVideoCapture(pMod->camCtx_.hndl);
					pMod->StopAudioCapture(pMod->micCtx_.hndl);

					/* must uninitialize camera HW to allow 'modprobe -r' */
					pMod->kernel_.LockMutex(pMod->mutex_);
					pMod->DeinitCameraInt();
					pMod->kernel_.UnlockMutex(pMod->mutex_);
				}
				else
				{
					/* camera absent upon initialization*/
				}

			}
		}
	}
	running = false;
	return kEventStatusOK;
}

//============================================================================
// Ctor & dtor
//============================================================================
CCameraModule::CCameraModule() : dbg_(kGroupCamera)
{
	tErrType			err = kNoErr;
	const tMutexAttr	attr = {0};
	tUSBDeviceData		usb_data;

	dbg_.SetDebugLevel(kCameraDebugLevel);

	sysfs.clear();
	camCtx_.file 			= gCamFile;
	camCtx_.numBufs 		= 0;
	camCtx_.bufs 			= NULL;
	camCtx_.fd 				= -1;
	camCtx_.hndl 			= kInvalidVidCapHndl;
	camCtx_.hCameraThread	= kNull;

	camCtx_.surf			= NULL;

	camCtx_.modes			= new tCaptureModes;
	camCtx_.controls		= new tCameraControls;

	camCtx_.module			= this;

	micCtx_.hndl			= kInvalidAudCapHndl;
	micCtx_.hMicThread		= kNull;
	micCtx_.poll_buf		= NULL;
	micCtx_.pcm_handle		= NULL;
	micCtx_.fd[0]			= -1;
	micCtx_.fd[1]			= -1;

	idctCtx_.file			= gIDCTFile;
	idctCtx_.fd				= -1;
	idctCtx_.reg			= MAP_FAILED;

	err = kernel_.InitMutex( micCtx_.dlock, attr );
	dbg_.Assert((kNoErr == err), "CCameraModule::ctor: Couldn't init mic mutex.\n");

	err = kernel_.InitMutex( dlock, attr );
	dbg_.Assert((kNoErr == err), "CCameraModule::ctor: Couldn't init mutex.\n");

	err = kernel_.InitMutex( mutex_, attr );
	dbg_.Assert((kNoErr == err), "CCameraModule::ctor: Couldn't init mutex.\n");

	err = kernel_.InitMutex( camCtx_.mThread, attr );
	dbg_.Assert((kNoErr == err), "CCameraModule::ctor: Couldn't init mutex.\n");

	/* hotplug subsystem calls this handler upon device insertion/removal */
	listener_ = new CameraListener(this);
	event_.RegisterEventListener(listener_);

	/* spoof a hotplug event to force enumerate sysfs and see if camera is present */
	usb_data = GetCurrentUSBDeviceState();
	usb_data.USBDeviceState &= kUSBDeviceConnected;
	usb_data.USBDeviceState |= kUSBDeviceHotPlug;

	CUSBDeviceMessage usb_msg(usb_data, kUSBDevicePriorityStateChange);
	/* Event is synchronous and handled in-thread */
	valid = false;
	event_.PostEvent(usb_msg, 0, listener_);

	/* local listener handles hardware initialization as appropriate */
	InitLut();
}

//----------------------------------------------------------------------------
CCameraModule::~CCameraModule()
{
	event_.UnregisterEventListener(listener_);
	delete listener_;

	StopVideoCapture(camCtx_.hndl);
	StopAudioCapture(micCtx_.hndl);

	valid = false;
	CAMERA_LOCK;
	DeinitCameraInt();
	CAMERA_UNLOCK;

	kernel_.DeInitMutex(camCtx_.mThread);
	kernel_.DeInitMutex(mutex_);
	kernel_.DeInitMutex(dlock);

	kernel_.DeInitMutex(micCtx_.dlock);

	delete camCtx_.controls;
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

Boolean		CCameraModule::IsCameraPresent()
{
	return valid;
}

//----------------------------------------------------------------------------
tErrType CCameraModule::SetCameraVideoPath(const CPath &path)
{
	DATA_LOCK;

	vpath = path;
	if (vpath.length() > 1 && vpath.at(vpath.length()-1) != '/')
		vpath += '/';

	DATA_UNLOCK;

	return kNoErr;
}

//----------------------------------------------------------------------------
CPath* CCameraModule::GetCameraVideoPath()
{
	CPath *path;

	DATA_LOCK;

	path = &vpath;

	DATA_UNLOCK;

	return path;
}

//----------------------------------------------------------------------------
tErrType CCameraModule::SetCameraStillPath(const CPath &path)
{
	DATA_LOCK;

	spath = path;
	if (spath.length() > 1 && spath.at(spath.length()-1) != '/')
		spath += '/';

	DATA_UNLOCK;

	return kNoErr;
}

//----------------------------------------------------------------------------
CPath* CCameraModule::GetCameraStillPath()
{
	CPath *path;

	DATA_LOCK;

	path = &spath;

	DATA_UNLOCK;

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
static Boolean DeinitCameraControlsInt(tCameraContext *pCamCtx)
{
	tCameraControls::iterator	ctrl_it;

	for( ctrl_it = pCamCtx->controls->begin() ; ctrl_it < pCamCtx->controls->end(); ctrl_it++ )
	{
		delete *ctrl_it;
	}

	pCamCtx->controls->clear();

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
	if(pCamCtx->fd != -1)
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
static Boolean InitCameraControlsInt(tCameraContext *pCamCtx)
{
	tControlInfo	*control = NULL;
	int				ret, idx = 0;
	Boolean			bRet = true;
	v4l2_queryctrl	query;
	v4l2_control	ctrl;

	memset(&query, 0, sizeof (v4l2_queryctrl));
	memset(&ctrl, 0, sizeof(v4l2_control));

	for(query.id = V4L2_CID_BASE; query.id < V4L2_CID_LASTP1; query.id++)
	{
		ret = ioctl(pCamCtx->fd, VIDIOC_QUERYCTRL, &query);
		if((ret != 0) || (query.flags & V4L2_CTRL_FLAG_DISABLED))
		{
			continue;
		}

		ctrl.id = query.id;

		ioctl(pCamCtx->fd, VIDIOC_G_CTRL, &ctrl);

		control				= new tControlInfo;
		control->max		= query.maximum;
		control->min		= query.minimum;
		control->preset		= query.default_value;
		control->current	= ctrl.value;

		switch(query.id)
		{
		case V4L2_CID_BRIGHTNESS:
			control->type = kControlTypeBrightness;
			break;
		case V4L2_CID_CONTRAST:
			control->type = kControlTypeContrast;
			break;
		case V4L2_CID_SATURATION:
			control->type = kControlTypeSaturation;
			break;
		case V4L2_CID_HUE:
			control->type = kControlTypeHue;
			break;
		case V4L2_CID_GAMMA:
			control->type = kControlTypeGamma;
			break;
/*
 * These three controls were added some time between 2.6.24 and 2.6.31.
 * The emulator builds against 2.6.24.
 */
#if defined V4L2_CID_POWER_LINE_FREQUENCY
		case V4L2_CID_POWER_LINE_FREQUENCY:
			control->type = kControlPowerLineFreq;
			break;
		case V4L2_CID_SHARPNESS:
			control->type = kControlTypeSharpness;
			break;
		case V4L2_CID_BACKLIGHT_COMPENSATION:
			control->type = kControlTypeBacklightComp;
			break;
#endif /* V4L2_CID_POWER_LINE_FREQUENCY */
		default:
			control->type = kControlTypeError;
		}

		pCamCtx->controls->push_back(control);
	}

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

		pCamCtx->bufs[i] = mmap(0, pCamCtx->buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, cam, pCamCtx->buf.m.offset);
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
	pCamCtx->bVPaused = pCamCtx->bPaused = false;

	return true;
}

//----------------------------------------------------------------------------
static Boolean DrawFrame(tVideoSurf *surf, tBitmapInfo *image)
{

	U8*	ps = image->data;
	U8*	pd = surf->buffer;

	if(surf->format == kPixelFormatYUV420)
	{
		U8*		du = pd + surf->pitch/2; // U,V in double-width buffer
		U8*		dv = pd + surf->pitch/2 + surf->pitch * surf->height/2;
		int		i,j,m,n;
#if 0
		/*
		 * This algorithm naively copies each packed pixel component (Y, Cb, Cr) from the
		 * decompressed JPEG to the appropriate planar buffers of the video layer.
		 * It is valuable for demonstrating how the image is painted, but it is not
		 * very efficient.  Therefore, it is disabled in favor of the unrolled version below.
		 */
		U8		y,cb,cr;

		for (i = 0; i < surf->height; i++)
		{

			for (j = m = n = 0; n < surf->width; m+=3, n++)
			{
				y		= ps[m+0];
				cb		= ps[m+1];
				cr		= ps[m+2];

				pd[n]	= y;
				if(!(i % 2) && !(n % 2))
				{
					du[j]	= cb;
					dv[j++]	= cr;
				}
			}
			ps += m;
			pd += surf->pitch;
			if (i % 2)
			{
				du += surf->pitch;
				dv += surf->pitch;
			}
		}
#else
		/*
		 * This algorithm copies each packed pixel component (Y, Cb, Cr) from the
		 * decompressed JPEG to the appropriate planar buffers of the video layer.
		 * The LF1000's ARM926EJ has a 16 kB data cache with 32 byte (8 word) Cache lines.
		 * The six words read below (w1...w6) constitute 24 bytes, but since they are
		 * read from contiguous memory, w2...w6 are very cheap to access.  Similarly,
		 * writing 1 word (32 bits) at a time is more efficient than writing a byte at a time.
		 * This unrolling yields a ~50% speedup from the algorithm above.
		 */

		U32		*ps32, *pd32, *pu32, *pv32;

		for (i = 0; i < surf->height; i++)
		{
			ps32 = reinterpret_cast<U32*>(ps);
			pd32 = reinterpret_cast<U32*>(pd);
			pu32 = reinterpret_cast<U32*>(du);
			pv32 = reinterpret_cast<U32*>(dv);

			for (j = m  = 0; j < surf->width; j+=8, m+=24)
			{
				register U32 w1, w2, w3, w4, w5, w6;

				w1 = *ps32++;
				w2 = *ps32++;
				w3 = *ps32++;
				w4 = *ps32++;
				w5 = *ps32++;
				w6 = *ps32++;

				// w1 =	Y2	Cr1	Cb1	Y1		//Y1	Cb1	Cr1	Y2
				// w2 = Cb3	Y3	Cr2	Cb2		//Cb2	Cr2	Y3	Cb3
				// w3 =	Cr4	Cb4	Y4	Cr3		//Cr3	Y4	Cb4	Cr4
				// w4 = Y6	Cr5	Cb5	Y5		//Y5	Cb5	Cr5	Y6
				// w5 = Cb7	Y7	Cr6	Cb6		//Cb6	Cr6	Y7	Cb7
				// w6 = Cr8	Cb8	Y8	Cr7		//Cr7	Y8	Cb8	Cr8

				// Y = 	Y4	Y3	Y2	Y1		// Y1	Y2	Y3	Y4
                *pd32++ = (w1 & 0x000000FF)
                            | ((w1 >> 16)  & 0x0000FF00)
                            | ((w2) & 0x00FF0000)
                            | ((w3 << 16)  & 0xFF000000);

                // Y = Y8	Y7	Y6	Y5		// Y5	Y6	Y7	Y8
                *pd32++ = (w4 & 0x000000FF)
                            | ((w4 >> 16)  & 0x0000FF00)
                            | ((w5) & 0x00FF0000)
                            | ((w6 << 16)  & 0xFF000000);

                // U = 	Cb7	Cb5	Cb3	Cb1		// Cb1	Cb3	Cb5	Cb7
                // V =	Cr7	Cr5	Cr3	Cr1		// Cr1	Cr3	Cr5	Cr7
                if (!(i % 2)) {
                   *pu32++ = ((w1 >> 8)  & 0x000000FF)
                               |  ((w2 >> 16)  & 0x0000FF00)
                               | ((w4 << 8)   & 0x00FF0000)
                               | ((w5) & 0xFF000000);
                    *pv32++ = (  (w1 >> 16) & 0x000000FF)
                               | ((w3 << 8) & 0x0000FF00)
                               | ((w4)  & 0x00FF0000)
                               |  ((w6 << 24)       & 0xFF000000);
                }

			}
			ps += m;
			pd += surf->pitch;
			if (i % 2)
			{
				du += surf->pitch;
				dv += surf->pitch;
			}
		}
#endif
	}
	else if(surf->format == kPixelFormatRGB888)
	{
		int			i,j,n;
#if 1
		for (i = 0; i < surf->height; i++)
		{
			for (j = n = 0; j < surf->width; j+=2, n+=6)
			{
				// TODO: why is the source BGR?
				pd[n+0]	= ps[n+2];
				pd[n+1]	= ps[n+1];
				pd[n+2]	= ps[n+0];

				pd[n+3] = ps[n+5];
				pd[n+4] = ps[n+4];
				pd[n+5] = ps[n+3];
			}
			ps += n;
			pd += n;
		}
#else
		memcpy(pd, ps, image->size);
#endif
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
	struct v4l2_format		fmt;
	struct v4l2_streamparm	fps;

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

    if(ioctl(camCtx_.fd, VIDIOC_S_FMT, &fmt) < 0)
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::SetCameraMode: mode selection failed for %s, %d\n", camCtx_.file, errno);
		return false;
	}

    camCtx_.mode	= *mode;
    camCtx_.fmt		= fmt;

    /* framerate */
    memset(&fps, 0, sizeof(struct v4l2_streamparm));
    fps.type									= V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fps.parm.capture.timeperframe.numerator		= mode->fps_numerator;
    fps.parm.capture.timeperframe.denominator	= mode->fps_denominator;

	if(ioctl(camCtx_.fd, VIDIOC_S_PARM, &fps) < 0)
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::SetCameraMode: fps selection failed for %s\n", camCtx_.file);
		return false;
	}

	// NOTE: "fps" vars are misnamed since Video4Linux expects time per frame
    camCtx_.fps = fps.parm.capture.timeperframe.denominator / fps.parm.capture.timeperframe.numerator;

	return true;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::GetCameraControls(tCameraControls &controls)
{
	CAMERA_LOCK;

	controls = *(camCtx_.controls);

	CAMERA_UNLOCK;

	return true;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::SetCameraControl(const tControlInfo* control, const S32 value)
{
	v4l2_control	ctrl;
	Boolean			bRet = true;
	tCameraControls::iterator	it;

	CAMERA_LOCK;

	switch(control->type)
	{
	case kControlTypeBrightness:
		ctrl.id = V4L2_CID_BRIGHTNESS;
		break;
	case kControlTypeContrast:
		ctrl.id = V4L2_CID_CONTRAST;
		break;
	case kControlTypeSaturation:
		ctrl.id = V4L2_CID_SATURATION;
		break;
	case kControlTypeHue:
		ctrl.id = V4L2_CID_HUE;
		break;
	case kControlTypeGamma:
		ctrl.id = V4L2_CID_GAMMA;
		break;

	/*
	 * These three controls were added some time between 2.6.24 and 2.6.31.
	 * The emulator builds against 2.6.24.
	 */
#if defined V4L2_CID_POWER_LINE_FREQUENCY
	case kControlPowerLineFreq:
		ctrl.id = V4L2_CID_POWER_LINE_FREQUENCY;
		break;
	case kControlTypeSharpness:
		ctrl.id = V4L2_CID_SHARPNESS;
		break;
	case kControlTypeBacklightComp:
		ctrl.id = V4L2_CID_BACKLIGHT_COMPENSATION;
		break;
#endif /* V4L2_CID_POWER_LINE_FREQUENCY */
	default:
		bRet = false;
		;
	}

	ctrl.value = value;

	if(0 != ioctl(camCtx_.fd, VIDIOC_S_CTRL, &ctrl))
	{
		bRet = false;
	}

	/* save updated value */
	for(it = camCtx_.controls->begin(); it < camCtx_.controls->end(); it++)
	{
		if( control->type == (*it)->type )
		{
			(*it)->current = value;
			break;
		}
	}

	CAMERA_UNLOCK;

	return bRet;
}

//----------------------------------------------------------------------------
Boolean CCameraModule::SetBuffers(const U32 numBuffers)
{
	Boolean	bRet = false;
	U32		oldBufs;

	CAMERA_LOCK;

	if(camCtx_.hndl != kInvalidVidCapHndl)
	{
		goto out;
	}

	if(camCtx_.numBufs == numBuffers)
	{
		goto out;
	}

	if(!DeinitCameraBufferInt(&camCtx_))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::SetBuffers: buffer unmapping failed for %s\n", camCtx_.file);
		goto out;
	}

	oldBufs = camCtx_.numBufs;
	camCtx_.numBufs = numBuffers;
	if(!InitCameraBufferInt(&camCtx_))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::SetBuffers: buffer mapping failed for %s\n", camCtx_.file);
		camCtx_.numBufs = oldBufs;
		goto out;
	}

	bRet = true;

out:
	CAMERA_UNLOCK;
	return bRet;
}

//----------------------------------------------------------------------------
tVidCapHndl	CCameraModule::StartVideoCapture()
{
	tVidCapHndl	hndl = kInvalidVidCapHndl;

 	CAMERA_LOCK;

	if(camCtx_.hndl != kInvalidVidCapHndl)
	{
		goto out;
	}

	if(!InitCameraStartInt(&camCtx_))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::StartVideoCapture: streaming failed for %s\n", camCtx_.file);
		goto out;
	}

	// TODO: multiple handles?
	hndl = camCtx_.hndl = STREAMING_HANDLE(FRAME_HANDLE(1));

out:
	CAMERA_UNLOCK;
	return hndl;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::PollFrame(const tVidCapHndl hndl)
{
	int i = 0;

	if(!IS_STREAMING_HANDLE(hndl))
	{
		return false;
	}

	CAMERA_LOCK;

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

static Boolean GetFrameInt(tCameraContext *pCtx, int index)
{
	memset(&pCtx->buf, 0, sizeof(struct v4l2_buffer));
	pCtx->buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	pCtx->buf.memory = V4L2_MEMORY_MMAP;
	pCtx->buf.index  = index;
	if( ioctl(pCtx->fd, VIDIOC_DQBUF, &pCtx->buf) < 0)
	{
		return false;
    }

	return true;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::GetFrame(const tVidCapHndl hndl, tFrameInfo *frame)
{
	if(!IS_STREAMING_HANDLE(hndl))
	{
		return false;
	}

	CAMERA_LOCK;

	if( !GetFrameInt(&camCtx_, camCtx_.buf.index))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::GetFrame: failed to capture frame from %s\n", camCtx_.file);
		CAMERA_UNLOCK;
		return false;
    }

	frame->index	= camCtx_.buf.index;
	frame->data		= camCtx_.bufs[camCtx_.buf.index];
	frame->size		= camCtx_.buf.bytesused;
	frame->width	= camCtx_.fmt.fmt.pix.width;
	frame->height	= camCtx_.fmt.fmt.pix.height;

	CAMERA_UNLOCK;
	return true;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::RenderFrame(const CPath &path, tVideoSurf *pSurf)
{
	Boolean ret = false;
	tFrameInfo frame;

	ret = OpenFrame(path, &frame);
	if(!ret)
	{
		goto out;
	}

	ret = RenderFrame(&frame, pSurf, NULL, JPEG_SLOW);

out:
	if (frame.data)
		kernel_.Free(frame.data);

	return ret;
}



typedef struct my_error_mgr
{
	struct jpeg_error_mgr pub;	/* "public" fields */

	jmp_buf setjmp_buffer;	/* for return to caller */
} *my_error_ptr;

static void silence_warning(j_common_ptr cinfo, int msg_level)
{

}

static void my_error_exit (j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message) (cinfo);

	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

__attribute__((always_inline)) static inline void Dequantize(JCOEFPTR ptr, JQUANT_TBL * quantptr, S16 *out)
{
	int i,j;

	/* De-quantize coefficients.  Too bad we don't have NEON. */
	for(i = j = 0; i < DCTSIZE; i++, j+= 8)
	{
		out[j] = ptr[j] * quantptr->quantval[j];
		out[j+1] = ptr[j+1] * quantptr->quantval[j+1];
		out[j+2] = ptr[j+2] * quantptr->quantval[j+2];
		out[j+3] = ptr[j+3] * quantptr->quantval[j+3];
		out[j+4] = ptr[j+4] * quantptr->quantval[j+4];
		out[j+5] = ptr[j+5] * quantptr->quantval[j+5];
		out[j+6] = ptr[j+6] * quantptr->quantval[j+6];
		out[j+7] = ptr[j+7] * quantptr->quantval[j+7];
	}
}

__attribute__((always_inline)) static inline void NormalizeAndPaint(U8 *buf, JCOEFPTR ptr, int ci, int stride)
{
	int i, j;
	S8 tmp[DCTSIZE2];

	/* Scale back from 9 bits to 8 bits, and level-shift samples */
	/* Don't bother computing U & V samples that aren't used */
	if(!ci)
	{
		for(i = j = 0; i < DCTSIZE; i++, j+= 8)
		{
			/* this lookup table implements range-clamping and level-shift:
			 * 	tmp[j] = CLAMP(ptr[j] + 128, 0, UCHAR_MAX);
			 */
			tmp[j+0] = NormLUT[ptr[j+0]];
			tmp[j+1] = NormLUT[ptr[j+1]];
			tmp[j+2] = NormLUT[ptr[j+2]];
			tmp[j+3] = NormLUT[ptr[j+3]];
			tmp[j+4] = NormLUT[ptr[j+4]];
			tmp[j+5] = NormLUT[ptr[j+5]];
			tmp[j+6] = NormLUT[ptr[j+6]];
			tmp[j+7] = NormLUT[ptr[j+7]];
		}
	}
	else
	{
		for(i = j = 0; i < DCTSIZE; i+=2, j+=16)
		{
			tmp[j+0] = NormLUT[ptr[j+0]];
			tmp[j+1] = NormLUT[ptr[j+1]];
			tmp[j+2] = NormLUT[ptr[j+2]];
			tmp[j+3] = NormLUT[ptr[j+3]];
			tmp[j+4] = NormLUT[ptr[j+4]];
			tmp[j+5] = NormLUT[ptr[j+5]];
			tmp[j+6] = NormLUT[ptr[j+6]];
			tmp[j+7] = NormLUT[ptr[j+7]];
		}
	}

	/*
	 * Paint planar YUV420 components.  This requires skipping
	 * every other row of U & V data since the JPEG is packed
	 * YUV 4:2:2 (YUYV-encoded).
	 */
	if(!ci)
	{
		for(i = j = 0; i < DCTSIZE; i++)
		{
			buf[0] = tmp[j];
			buf[1] = tmp[j+1];
			buf[2] = tmp[j+2];
			buf[3] = tmp[j+3];
			buf[4] = tmp[j+4];
			buf[5] = tmp[j+5];
			buf[6] = tmp[j+6];
			buf[7] = tmp[j+7];

			buf += stride;
			j += 8;
		}
	}
	else
	{
		for(i = j = 0; i < DCTSIZE; i+=2)
		{
			buf[0] = tmp[j];
			buf[1] = tmp[j+1];
			buf[2] = tmp[j+2];
			buf[3] = tmp[j+3];
			buf[4] = tmp[j+4];
			buf[5] = tmp[j+5];
			buf[6] = tmp[j+6];
			buf[7] = tmp[j+7];

			buf += stride;
			j += 16;
		}
	}
}

/*
* JPEG coefficient access code based on:
* http://svn.assembla.com/svn/FIUBA7506/Codigo/Imagen/JPG.cpp
*/
__attribute__((always_inline)) inline void CCameraModule::DecompressAndPaint(struct jpeg_decompress_struct *cinfo, tVideoSurf *surf)

{
	jvirt_barray_ptr* src_coef_arrays = jpeg_read_coefficients(cinfo);

	JDIMENSION MCU_cols, comp_width, blk_x, blk_y;
	int ci, k, offset_y, i, j;
	JBLOCKARRAY buffer;
	JCOEFPTR ptr, ptr1;
	JQUANT_TBL * quantptr;
	jpeg_component_info *compptr;

	U8 *buf, *fb, *yuv[3];
	S16 tmp[2][DCTSIZE2];	/* tmp[0] is for IDCT input, tmp[1] output */

	MCU_cols = cinfo->image_width / (cinfo->max_h_samp_factor * DCTSIZE);


	yuv[0] = surf->buffer;											// Y
	yuv[1] = yuv[0] + surf->pitch/2;								// U
	yuv[2] = yuv[0] + surf->pitch/2 + surf->pitch * surf->height/2;	// V

	for(ci = 0; ci < cinfo->num_components; ci++)
	{
    	buf = fb = yuv[ci];

    	compptr = cinfo->comp_info + ci;
    	quantptr = compptr->quant_table;

    	comp_width = MCU_cols * compptr->h_samp_factor;
    	for(blk_y = 0; blk_y < compptr->height_in_blocks; blk_y += compptr->v_samp_factor)
    	{
    		buffer = (*cinfo->mem->access_virt_barray)
						((j_common_ptr) cinfo, src_coef_arrays[ci],
						blk_y,
						(JDIMENSION) compptr->v_samp_factor,
						FALSE);

    		for (offset_y = 0; offset_y < compptr->v_samp_factor; offset_y++)
    		{
    			for (blk_x = 0; blk_x < comp_width; blk_x++)
    			{
    				ptr = buffer[offset_y][blk_x];

    				/*
    				 * De-quantize coefficients of first block in row.  The
    				 * other blocks are done in parallel with IDCT.
    				 */
    				if(!blk_x)
    				{
    					Dequantize(ptr, quantptr, tmp[0]);
    				}

    				/* initiate IDCT */
    				StartIDCT(tmp[0]);

    				/*
    				 * IDCT is busy.  To extract some parallelism:
    				 * 1.) De-quantize next block's coefficients
    				 * 2.) Normalize and paint previous block
    				 */
    				if(blk_x != comp_width-1)
    				{
    					/* next block in the row */
    					ptr1 = buffer[offset_y][blk_x+1];
    					Dequantize(ptr1, quantptr, tmp[0]);
    				}
    				if(blk_x)
    				{
    					/* previous block in the row */

    					NormalizeAndPaint(buf, tmp[1], ci, surf->pitch);

    					/* next block in framebuffer row (next block column) */
    					buf += 8;
    				}

    				/* fetch IDCT result */
    				RetrieveIDCT(tmp[1]);

    				/* last block has to paint itself */
    				if(blk_x == comp_width-1)
    				{
    					NormalizeAndPaint(buf, tmp[1], ci, surf->pitch);

    					/* next block in framebuffer row (next block column) */
    					buf += 8;
    				}
    			}
    		}
    		if(!ci)
    		{
    			/* next block in framebuffer column (next block row) */
    			fb += 8*surf->pitch;
    		}
    		else
    		{
    			fb += 4*surf->pitch;
    		}
    		buf = fb;
    	}
    }

}

//----------------------------------------------------------------------------
Boolean CCameraModule::RenderFrame(tFrameInfo *frame, tVideoSurf *surf, tBitmapInfo *bitmap, const JPEG_METHOD method)
{
	struct jpeg_decompress_struct	cinfo;
	struct my_error_mgr				jerr;
	int 							row = 0;
	int								row_stride;
	Boolean							bRet = true, bAlloc = false;

	// TODO: don't allocate this on every render
	if(bitmap == NULL)
	{
		dbg_.Assert((surf != NULL), "CameraModule::RenderFrame: no render target!\n" );
		bitmap			= new tBitmapInfo;
		bitmap->width	= surf->width;
		bitmap->height	= surf->height;
		switch(surf->format)
		{
		case kPixelFormatYUV420:
			bitmap->format	= kBitmapFormatYCbCr888;
			bitmap->depth	= 3;
			break;
		case kPixelFormatRGB888:
			bitmap->format	= kBitmapFormatRGB888;
			bitmap->depth	= 3;
			break;
		}

		bitmap->size	= bitmap->width * bitmap->height * bitmap->depth;

		bitmap->data	= static_cast<U8*>( kernel_.Malloc(bitmap->size) );

		/*
		 * libjpeg expects an array of pointers to bitmap output rows.
		 * The user supplies one contiguous array for bitmap output, so construct
		 * a JSAMPARRAY pointing to each row.
		 */
		bitmap->buffer = static_cast<U8**>(kernel_.Malloc(bitmap->height * sizeof(U8*)));

		row_stride = bitmap->width;
		if(bitmap->format == kBitmapFormatYCbCr888 || bitmap->format == kBitmapFormatRGB888)
		{
			row_stride *= 3;
		}

		for(int i = 0; i < bitmap->height; i++)
		{
			bitmap->buffer[i] = &bitmap->data[i*row_stride];
		}

		bAlloc = true;
	}

	/* We set up the normal JPEG error routines, then override error_exit
	 * and emit_message. */
	cinfo.err = jpeg_std_error(&jerr.pub);

	/*
	 * The PixArt camera produces JPEGs which libjpeg complains about:
	 *   "Corrupt JPEG data: 1 extraneous bytes before marker 0xd9"
	 * These appear visually fine, but the warnings on stderr are distracting,
	 * so stifle them.
	 */
	jerr.pub.emit_message	= silence_warning;
	/*
	 * v4l2 sometimes produces totally invalid JPEGs which cause libjpeg to exit.
	 * TODO: root cause why so this stop-gap isn't needed.
	 */
	jerr.pub.error_exit		= my_error_exit;

	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer))
	{
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */
		jpeg_destroy_decompress(&cinfo);
		bRet = false;
		goto out;
	}

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
		cinfo.out_color_space = JCS_GRAYSCALE;
		break;
	case kBitmapFormatError:
		/* fall through */
	default:
		bRet = false;
		;
	}
#ifndef EMULATION
	/*
	 * Decode JPEG using jpeg_read_coefficients() and LF1000 IDCT, then paint
	 * video layer directly.  This has about 5 fps better performance than the
	 * jpeg_read_scanlines() version below.
	 *
	 * TODO: Still outstanding:
	 * -better integration with scaler (internal tVideoSurf to match camera size?)
	 *
	 * -Implement QVGA->QQVGA rendering that doesn't look terrible
	 *
	 * -IDCT independent?: HW scaler produces a few rows of green pixels at
	 *  bottom edge of screen (lack U & V?) when scaling up
	 *
	 * -use hardware color space converter to support RGB textures
	 *
	 * -determine why colors look somewhat muted
	 *
	 * -verify IDCT resolution
	 */
	if(method == JPEG_HW2)
	{
		DecompressAndPaint(&cinfo, surf);
		/* JPEG has been rendered, prevent re-draw below */
		surf = NULL;
	}
	else
#endif
	{
	/*
	 * libjpeg natively supports M/8, where M is 1..16.
	 */

	//scale up
	if( frame->height > bitmap->height)
	{
		cinfo.scale_num		= (8 * bitmap->height) / frame->height;
	}
	else
	{
		cinfo.scale_num		= (8 * (bitmap->height / frame->height));
	}

	cinfo.dct_method	= (J_DCT_METHOD)method;

	(void) jpeg_start_decompress(&cinfo);

	row_stride = cinfo.output_width * cinfo.output_components;

	while (cinfo.output_scanline < cinfo.output_height) {
		row += jpeg_read_scanlines(&cinfo, &bitmap->buffer[row], cinfo.output_height);
	}

	}
	(void) jpeg_finish_decompress(&cinfo);

	jpeg_destroy_decompress(&cinfo);

	// draw to screen
	if(surf != NULL)
	{
		if(surf->format == kPixelFormatYUV420)
			SetScaler(surf->width, surf->height, false);
		bRet = DrawFrame(surf, bitmap);
	}

out:
	if(bAlloc)
	{
		kernel_.Free(bitmap->data);
		kernel_.Free(bitmap->buffer);
		delete bitmap;
	}

	return bRet;
}

static Boolean ReturnFrameInt(tCameraContext *pCtx, const U32 index)
{
	memset(&pCtx->buf, 0, sizeof(struct v4l2_buffer));
	pCtx->buf.type  	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	pCtx->buf.memory	= V4L2_MEMORY_MMAP;
	pCtx->buf.index		= index;

	if(ioctl(pCtx->fd, VIDIOC_QBUF, &pCtx->buf) < 0)
	{
		return false;
    }

	return true;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::ReturnFrame(const tVidCapHndl hndl, const tFrameInfo *frame)
{
	if(!IS_STREAMING_HANDLE(hndl))
	{
		return false;
	}

	CAMERA_LOCK;

	if(!ReturnFrameInt(&camCtx_, frame->index))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::PutFrame: failed to return frame %lu to %s\n", frame->index, camCtx_.file);
		CAMERA_UNLOCK;
		return false;
    }

	CAMERA_UNLOCK;
	return true;
}

//----------------------------------------------------------------------------
tVidCapHndl CCameraModule::StartVideoCapture(const CPath& path, tVideoSurf* pSurf,
		IEventListener * pListener, const U32 maxLength, Boolean bAudio)
{
	CPath fpath = path;
	struct tCaptureMode QVGA = {kCaptureFormatMJPEG, 320, 240, 1, 15};  // "fps" fields misnamed
	tVidCapHndl hndl = kInvalidVidCapHndl;
	struct statvfs buf;
	U64 length;
	int err;

	CAMERA_LOCK;

	if(camCtx_.hndl != kInvalidVidCapHndl || !valid)
	{
		CAMERA_UNLOCK;
		return hndl;
	}

	if(fpath.length() > 0)
	{
		if(fpath.at(0) != '/')
		{
			DATA_LOCK;
			fpath	= vpath + path;
			DATA_UNLOCK;
		}

		/* statvfs path must exist, so use the parent directory */
		err = statvfs(fpath.substr(0, fpath.rfind('/')).c_str(), &buf);

		length =  buf.f_bsize * buf.f_bavail;

		if((err < 0) || (length < MIN_FREE))
		{
			dbg_.DebugOut(kDbgLvlCritical, "CameraModule::StartVideoCapture: not enough disk space, %lld required, %lld available\n", MIN_FREE, length);
			CAMERA_UNLOCK;
			return hndl;
		}

		length /= VID_BITRATE;	/* How many seconds can we afford? */

		camCtx_.maxLength = ((maxLength == 0) ? length : MIN(length, maxLength));
	}
	else
	{
		camCtx_.maxLength = maxLength;
	}

	camCtx_.path	= fpath;
	camCtx_.bAudio	= bAudio && (fpath.length() > 0);

	camCtx_.reqLength = maxLength;
	camCtx_.pListener = pListener;

	camCtx_.surf	= pSurf;

	if(!DeinitCameraBufferInt(&camCtx_))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::StartVideoCapture: buffer unmapping failed for %s\n", camCtx_.file);
		CAMERA_UNLOCK;
		return hndl;
	}

	if(!SetCameraMode(&QVGA))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::StartVideoCapture: mode setting failed for %s\n", camCtx_.file);
		CAMERA_UNLOCK;
		return hndl;
	}

	camCtx_.numBufs = NUM_BUFS;
	if(!InitCameraBufferInt(&camCtx_))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::StartVideoCapture: buffer mapping failed for %s\n", camCtx_.file);
		CAMERA_UNLOCK;
		return hndl;
	}

	if(!InitCameraStartInt(&camCtx_))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::StartVideoCapture: streaming failed for %s\n", camCtx_.file);
		CAMERA_UNLOCK;
		return hndl;
	}

	/* for HW IDCT viewfinder */
	/* TODO: libjpeg QVGA->QQVGA rendering + HW scaler is faster than
	 * HW IDCT QVGA->QVGA rendering
	 */
	if(pSurf && pSurf->format == kPixelFormatYUV420 && pSurf->width == QVGA.width)
	{
		//SetScaler(QVGA.width, QVGA.height, false);
		SetScaler(pSurf->width, pSurf->height, false);
	}

	//hndl must be set before thread starts.  It is used in thread initialization.
	camCtx_.hndl = STREAMING_HANDLE(THREAD_HANDLE(1));

	if(kNoErr == InitCameraTask(&camCtx_))
	{
		hndl = camCtx_.hndl;
	}
	else
	{
		camCtx_.hndl = hndl;
	}

	CAMERA_UNLOCK;
	return hndl;
}

//----------------------------------------------------------------------------
static Boolean StopVideoCaptureInt(int fd)
{
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0)
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::StopVideoCapture(const tVidCapHndl hndl)
{
	THREAD_LOCK;
	CAMERA_LOCK;

	if(!IS_STREAMING_HANDLE(hndl))
	{
		CAMERA_UNLOCK;
		THREAD_UNLOCK;
		return false;
	}

	if(IS_THREAD_HANDLE(hndl))
	{
		/* The camera thread must be halted before issuing VIDIOC_STREAMOFF */
		DeInitCameraTask(&camCtx_);
	}

	if(!StopVideoCaptureInt(camCtx_.fd))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::StopVideoCapture: failed to halt streaming from %s\n", camCtx_.file);
		CAMERA_UNLOCK;
		THREAD_UNLOCK;
		return false;
    }

	if(camCtx_.path.length() && camCtx_.bAudio)
	{
		if(!StopAudio())
		{
			dbg_.DebugOut(kDbgLvlCritical, "CameraModule::StopVideoCapture: failed to halt audio streaming\n");
			CAMERA_UNLOCK;
			THREAD_UNLOCK;
			return false;
		}
	}

	camCtx_.hndl 	= kInvalidVidCapHndl;

	CAMERA_UNLOCK;
	THREAD_UNLOCK;

	return true;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::SnapFrameRGB(const tVidCapHndl hndl, const CPath &path)
{
	// Grab frame to local RGB buffer instead and save as PNG
	U8* rgbbuf = new U8[640 * 480 * 3];

	Boolean ret = GetFrame(hndl, rgbbuf, kOpenGlRgb);

	if (ret)
	{
		CPath filepath = (path.at(0) == '/') ? path : spath + path;
		PNG_save(filepath.c_str(), 640, 480, 640*3, (char*)rgbbuf);
	}
	
	delete[] rgbbuf;
	return ret;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::SnapFrame(const tVidCapHndl hndl, const CPath &path)
{
	Boolean ret;
	tFrameInfo frame	= {kCaptureFormatMJPEG, 640, 480, 0, NULL, 0};

	if (path.rfind(".png") != std::string::npos) 
		return SnapFrameRGB(hndl, path);
	
	ret = GrabFrame(hndl, &frame);
	if(!ret)
	{
		goto out;
	}

	ret = SaveFrame(path, &frame);

out:
	kernel_.Free(frame.data);

	return ret;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::GetFrame(const tVidCapHndl hndl, U8 *pixels, tColorOrder color_order)
{
	int i, row_stride;
	Boolean ret;
	tFrameInfo frame	= {kCaptureFormatMJPEG, 640, 480, 0, NULL, 0};
	tBitmapInfo bmp 	= {kBitmapFormatRGB888, 640, 480, 3, pixels, 921600, NULL};

	ret = GrabFrame(hndl, &frame);
	if(ret)
	{
		bmp.buffer = static_cast<U8**>(kernel_.Malloc(bmp.height * sizeof(U8*)));

		row_stride = bmp.width * bmp.depth;
		for(i = 0; i < bmp.height; i++)
		{
			bmp.buffer[i] = &bmp.data[i*row_stride];
		}

		ret = RenderFrame(&frame, NULL, &bmp, JPEG_SLOW);

		if(color_order == kDisplayRgb)
		{
			for(i = 0; i < bmp.height; ++i)
			{
				int i_stride = i * row_stride;
				for(int j = 0; j < bmp.width * 3; j += 3)
				{
					U8 temp = pixels[i_stride + j];
					pixels[i_stride + j] = pixels[i_stride + j + 2];
					pixels[i_stride + j + 2] = temp;
				}
			}
		}

		kernel_.Free(frame.data);	/* alloced by GrabFrame */
		kernel_.Free(bmp.buffer);
	}

	return ret;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::GrabFrame(const tVidCapHndl hndl, tFrameInfo *frame)
{
	Boolean			bRet	= false;
	tCaptureMode	oldmode, newmode;
	U32				oldbufs	= camCtx_.numBufs;
	tFrameInfo		frm;
	Boolean			bPause, vPause;

	if(!IS_STREAMING_HANDLE(hndl) || !IS_THREAD_HANDLE(hndl))
	{
		return false;
	}

	oldmode = newmode = camCtx_.mode;

	newmode.width	= frame->width;
	newmode.height	= frame->height;
	newmode.fps_numerator	= 1;
	newmode.fps_denominator	= 5;

	// don't let the viewfinder run while we muck with the camera settings
	THREAD_LOCK;
	CAMERA_LOCK;

	bPause	= camCtx_.bPaused;
	vPause	= camCtx_.bVPaused;

	/*
	 * Changing resolutions requires that the camera be off.  This implies unmapping all of the buffers
	 * and restarting everything from scratch.
	 *
	 * TODO: optimize same-resolution case
	 */

	if(!DeinitCameraBufferInt(&camCtx_))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::GrabFrame: failed to free buffers for %s\n", camCtx_.file);
		goto bail_out;

	}

	if(!StopVideoCaptureInt(camCtx_.fd))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::GrabFrame: failed to halt streaming from %s\n", camCtx_.file);
		goto bail_out;
    }

	if(!SetCameraMode(&newmode))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::GrabFrame: failed to set resolution %s\n", camCtx_.file);
		goto bail_out;
	}

	// just one buffer needed for the snapshot
	camCtx_.numBufs = 1;

	if(!InitCameraBufferInt(&camCtx_))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::GrabFrame: failed to acquire buffers for %s\n", camCtx_.file);
		goto bail_out;
	}

	if(!InitCameraStartInt(&camCtx_))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::GrabFrame: streaming failed for %s\n", camCtx_.file);
		goto bail_out;
	}

	// acquire the snapshot
	if( !GetFrameInt(&camCtx_, 0))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::GetFrame: failed to capture frame from %s\n", camCtx_.file);
		goto bail_out;
    }

	//RenderFrame (scaled)

	//copy data
	frame->index	= camCtx_.buf.index;
	frame->size		= camCtx_.buf.bytesused;
	frame->width	= camCtx_.fmt.fmt.pix.width;
	frame->height	= camCtx_.fmt.fmt.pix.height;
	frame->data		= kernel_.Malloc(frame->size);
	memcpy(frame->data, camCtx_.bufs[camCtx_.buf.index], frame->size);

	if(!ReturnFrameInt(&camCtx_, frame->index))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::GetFrame: failed to requeue frame to %s\n", camCtx_.file);
		goto frame_out;
	}

	if(!DeinitCameraBufferInt(&camCtx_))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::GrabFrame: failed to free buffers for %s\n", camCtx_.file);
		goto frame_out;
	}

	if(!StopVideoCaptureInt(camCtx_.fd))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::GrabFrame: failed to halt streaming from %s\n", camCtx_.file);
		goto frame_out;
    }

	if(!SetCameraMode(&oldmode))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::GrabFrame: failed to set resolution %s\n", camCtx_.file);
		goto frame_out;
	}

	camCtx_.numBufs = oldbufs;

	if(!InitCameraBufferInt(&camCtx_))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::GrabFrame: failed to restore buffers for %s\n", camCtx_.file);
		goto frame_out;
	}

	if(!InitCameraStartInt(&camCtx_))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::GrabFrame: streaming failed for %s\n", camCtx_.file);
		goto frame_out;
	}

	camCtx_.bPaused = bPause;
	camCtx_.bVPaused = vPause;

	CAMERA_UNLOCK;
	THREAD_UNLOCK;

	return true;

frame_out:
	kernel_.Free(frame->data);
	frame->data	= NULL;
	frame->size	= 0;

bail_out:

	bPause	= camCtx_.bPaused;
	vPause	= camCtx_.bVPaused;

	CAMERA_UNLOCK;
	THREAD_UNLOCK;
	return false;

}

//----------------------------------------------------------------------------
Boolean	CCameraModule::SaveFrame(const CPath &path, const tFrameInfo *frame)
{
	Boolean bRet = false;

	// TODO: protect spath with DATA_LOCK
	CPath		filepath = (path.at(0) == '/') ? path : spath + path;
	const char*	filename = filepath.c_str();

	FILE *f = fopen(filename, "wb");
	if(f != NULL)
	{
		if(frame->size == fwrite(frame->data, sizeof(U8), frame->size, f))
		{
			bRet = true;
		}
		fclose(f);
	}

	return bRet;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::OpenFrame(const CPath &path, tFrameInfo *frame)
{
	Boolean bRet = false;
	struct stat buf;

	struct jpeg_decompress_struct	cinfo;
	struct jpeg_error_mgr			err;

	// TODO: protect spath with DATA_LOCK
	CPath		filepath = (path.at(0) == '/') ? path : spath + path;
	const char*	filename = filepath.c_str();

	frame->size = 0;
	frame->data = NULL;
	
	if(0 != stat(filename, &buf))
	{
		return bRet;
	}

	cinfo.err = jpeg_std_error(&err);

	jpeg_create_decompress(&cinfo);

	FILE *f = fopen(filename, "rb");
	if(f != NULL)
	{
		size_t 	done = 0, left = 0;
		U8*		dest;

		left = frame->size = buf.st_size;

		frame->data = kernel_.Malloc(frame->size);

		jpeg_stdio_src(&cinfo, f);

		(void) jpeg_read_header(&cinfo, TRUE);

		fseek(f, 0, SEEK_SET);

		frame->width		= cinfo.image_width;
		frame->height		= cinfo.image_height;
		frame->pixelformat	= kCaptureFormatMJPEG;

		dest = (reinterpret_cast<U8*>(frame->data));

		do
		{
			dest += done;
			done = fread(dest, sizeof(U8), left, f);
			left -= done;
		} while( done );

		if((left == 0) && !ferror(f))
		{
			bRet = true;
		}
		else
		{
			frame->size = 0;
			kernel_.Free(frame->data);
			frame->data = NULL;
		}
		fclose(f);
	}

	jpeg_destroy_decompress(&cinfo);

	return bRet;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::PauseVideoCapture(const tVidCapHndl hndl, const Boolean display)
{
	int 	type 	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	Boolean	bRet	= true;

	if(!IS_STREAMING_HANDLE(hndl) || !IS_THREAD_HANDLE(hndl))
	{
		return false;
	}

	dbg_.Assert((kNoErr == kernel_.LockMutex(camCtx_.mThread)),\
												  "Couldn't lock mutex.\n");
	if(camCtx_.bAudio)
	{
		StopAudio();
	}

	camCtx_.bPaused = true;
	camCtx_.bVPaused = display;

	dbg_.Assert((kNoErr == kernel_.UnlockMutex(camCtx_.mThread)),\
												  "Couldn't unlock mutex.\n");

	return bRet;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::ResumeVideoCapture(const tVidCapHndl hndl)
{
	int 	type 	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	Boolean	bRet	= true;

	if(!IS_STREAMING_HANDLE(hndl) || !IS_THREAD_HANDLE(hndl))
	{
		return false;
	}

	dbg_.Assert((kNoErr == kernel_.LockMutex(camCtx_.mThread)),\
												  "Couldn't lock mutex.\n");

	if(camCtx_.bAudio)
	{
		StartAudio(false);
	}

	camCtx_.bPaused = false;
	camCtx_.bVPaused = false;

	dbg_.Assert((kNoErr == kernel_.UnlockMutex(camCtx_.mThread)),\
												  "Couldn't unlock mutex.\n");

	return bRet;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::IsVideoCapturePaused(const tVidCapHndl hndl)
{
	int 	type 	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	Boolean	bRet	= true;

	if(!IS_STREAMING_HANDLE(hndl) || !IS_THREAD_HANDLE(hndl))
	{
		return false;
	}

	return camCtx_.bPaused;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::InitCameraInt()
{
	struct tCaptureMode QVGA = {kCaptureFormatMJPEG, 320, 240, 1, 15};

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

	if(!SetCameraMode(&QVGA))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::InitCameraInt: failed to set resolution %s\n", camCtx_.file);
		return false;
	}

	if(!InitCameraControlsInt(&camCtx_))
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::InitCameraInt: controls probing failed for %s\n", camCtx_.file);
		return false;
	}

	if(kNoErr != InitMicInt())
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::InitCameraInt: microphone init failed\n");
		return false;
	}

	if(kNoErr != InitIDCTInt())
	{
		dbg_.DebugOut(kDbgLvlCritical, "CameraModule::InitCameraInt: idct init failed\n");
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
void CCameraModule::InitLut(void)
{
	int i;

	NormLUT = &nLUT[256];	/* we want to use indicies -256 to 768 */

	/* IDCT 9-bit output: -256 ... 255 becomes 8-bit color 0 ... 255 */
	/* Dividing the whole range by 2 and shifting up by 128 results in
	 * washed-out color.  Instead, use a simple clamp.
	 */
	for(i = -256; i < -128; i++)
	{
		NormLUT[i]	= CHAR_MIN;
	}
	for(i = -128; i < 128; i++)
	{
		NormLUT[i]	= i + 128;
	}
	for(i = 128; i < 255; i++)
	{
		NormLUT[i]	= UCHAR_MAX;
	}

	/* Undocumented IDCT errata: +256 (0x0100) and +768 (0x0300) occasionally
	 * produced in 9-bit mode.  These values are out of range, and presumably
	 * have the value -256 (0x100) which failed to be sign-extended.
	 */
	NormLUT[256] = CHAR_MIN;
	NormLUT[768] = CHAR_MIN;
}

//----------------------------------------------------------------------------
Boolean	CCameraModule::DeinitCameraInt()
{
	if(kNoErr != DeinitIDCTInt())
	{
		return false;
	}

	if(kNoErr != DeinitMicInt())
	{
		return false;
	}

	if(!DeinitCameraBufferInt(&camCtx_))
	{
		return false;
	}

	if(!DeinitCameraControlsInt(&camCtx_))
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
