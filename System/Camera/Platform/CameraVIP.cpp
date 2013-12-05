#include <SystemTypes.h>

#include <CameraPriv.h>
#include <DisplayPriv.h>
#include <DisplayMPI.h>

#if !defined(EMULATION) //&& defined(LF2000)
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/lf2000/lf2000vip.h>
#include <linux/media.h>
#include <vmem.h>
#endif

// PNG wrapper function for saving file
bool PNG_save(const char* file, int width, int height, int pitch, char* data);

LF_BEGIN_BRIO_NAMESPACE()

#define CAMERA_LOCK 	kernel_.LockMutex(mutex_)
#define CAMERA_UNLOCK	kernel_.UnlockMutex(mutex_)

tCaptureMode VGA  = {kCaptureFormatYUV420, 640, 480, 1, 15};
tCaptureMode QVGA = {kCaptureFormatYUV420, 320, 240, 1, 15};
tCaptureMode SVGA = {kCaptureFormatYUV420, 800, 600, 1, 15};
tCaptureMode QSVGA = {kCaptureFormatYUV420, 400, 300, 1, 15};
tCaptureMode QSVGA30 = {kCaptureFormatYUV420, 400, 300, 1, 30};
tCaptureMode WXGA  = {kCaptureFormatYUV420, 1280, 800, 1, 8};
tCaptureMode SXGA  = {kCaptureFormatYUV420, 1280, 960, 1, 8};
tCaptureMode HD16  = {kCaptureFormatYUV420, 1600, 900, 1, 8};
tCaptureMode UXGA  = {kCaptureFormatYUV420, 1600, 1200, 1, 8};

namespace
{
}

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
	int xmax = UXGA.width;
	int ymax = UXGA.height;
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
		camCtx_.modes->push_back(new tCaptureMode(QSVGA30));
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

//----------------------------------------------------------------------------
void CVIPCameraModule::AllocVMem(tCameraContext& camCtx_)
{
#if !defined(EMULATION)
	int r;

	dbg_.DebugOut(kDbgLvlImportant, "%s: pid=%d, fdvmem=%d\n", __FUNCTION__, getpid(), fdvmem);
	if (fdvmem > 0 || fd > 0)
		return;

	// Map memory for use by VIP driver
	fdvmem = open("/dev/vmem", O_RDWR);
	if (fdvmem > 0)
	{
		// Request vmem driver 8Meg memory block for video capture use
		memset(&vm, 0, sizeof(vm));
		vm.Flags = VMEM_BLOCK_BUFFER;
		vm.MemWidth  = 4096;
		vm.MemHeight = 2048;
		vm.HorAlign  = 64;
		vm.VerAlign  = 32;
		do {
			r = ioctl(fdvmem, IOCTL_VMEM_ALLOC, &vm);
		} while (r != 0 && (vm.MemHeight -= 256) > UXGA.height);
	}
	if (fdvmem > 0 && r == 0)
	{
		dbg_.Assert((r == 0), "CCameraModule::ctor: IOCTL_VMEM_ALLOC failed\n");
		fd = open("/dev/mem", O_RDWR | O_SYNC);
		dbg_.Assert((fd > 0), "CCameraModule::ctor: open /dev/mem failed\n");
		fi.smem_len = vm.MemWidth * vm.MemHeight;
		vi.reserved[0] = (unsigned int)mmap((void*)0, fi.smem_len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_POPULATE, fd, vm.Address);
		dbg_.Assert((vi.reserved[0] != (unsigned int)MAP_FAILED), "CCameraModule::ctor: mmap /dev/mem failed\n");
		dbg_.DebugOut(kDbgLvlImportant, "%s: mmap %08x: %08x, len %08x\n", __FUNCTION__, vm.Address, vi.reserved[0], fi.smem_len);
	}
	else
	{
		// Fallback to YUV video framebuffer memory for video capture
		fd = open("/dev/fb2", O_RDWR | O_SYNC);
		dbg_.Assert((fd > 0), "CCameraModule::ctor: open /dev/fb2 failed\n");
		r = ioctl(fd, FBIOGET_FSCREENINFO, &fi);
		dbg_.Assert((r == 0), "CCameraModule::ctor: FBIOGET_FSCREENINFO failed\n");
		r = ioctl(fd, FBIOGET_VSCREENINFO, &vi);
		dbg_.Assert((r == 0), "CCameraModule::ctor: FBIOGET_VSCREENINFO failed\n");
		vi.reserved[0] = (unsigned int)mmap((void*)fi.smem_start, fi.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		dbg_.Assert((vi.reserved[0] != (unsigned int)MAP_FAILED), "CCameraModule::ctor: mmap /dev/fb2 failed\n");
		dbg_.DebugOut(kDbgLvlImportant, "%s: mmap %08x: %08x, len %08x\n", __FUNCTION__, (unsigned int)fi.smem_start, vi.reserved[0], fi.smem_len);
	}
	camCtx_.fi = &fi;
	camCtx_.vi = &vi;
#endif
}

//----------------------------------------------------------------------------
bool EnumMediaNodes(tCameraContext& ctx)
{
	int fd = open("/dev/media0", O_RDWR);
	if (fd == -1)
		return false;

	struct media_entity_desc md;
	struct media_link_desc   ml;
	struct media_links_enum  me;
	memset(&md, 0, sizeof(md));
	memset(&ml, 0, sizeof(ml));
	memset(&me, 0, sizeof(me));

	for (int i = 0, r = 0; r == 0; i++)
	{
		md.id = i | MEDIA_ENT_ID_FLAG_NEXT;

		r = ioctl(fd, MEDIA_IOC_ENUM_ENTITIES, &md);
		if (r != 0)
			continue;

		ctx.dbg->DebugOut(kDbgLvlImportant, "%s: id=%d, name=%s, pads=%d, links=%d, type=%08x\n", __FUNCTION__, md.id, md.name, md.pads, md.links, md.type);

		if (!(md.type & MEDIA_ENT_T_V4L2_SUBDEV))
			continue;

		me.entity = md.id;
		me.pads   = new struct media_pad_desc[md.pads];
		me.links  = new struct media_link_desc[md.links];

		r = ioctl(fd, MEDIA_IOC_ENUM_LINKS, &me);

		if (strstr(md.name, "VIN CLIPPER1")) {
			ml.sink.entity = md.id;
			for (int j = 0; j < md.pads; j++) {
				if (me.pads[j].flags & MEDIA_PAD_FL_SINK) {
					ml.sink = me.pads[j];
					break;
				}
			}
		}
		if (strstr(md.name, "HI253 0")) {
			ml.source.entity = md.id;
			for (int j = 0; j < md.pads; j++) {
				if (me.pads[j].flags & MEDIA_PAD_FL_SOURCE) {
					ml.source = me.pads[j];
					break;
				}
			}
		}

		delete[] me.links;
		delete[] me.pads;
	}

	if (ml.sink.entity && ml.source.entity) {
		ml.sink.flags = MEDIA_PAD_FL_SINK;
		ml.source.flags = MEDIA_PAD_FL_SOURCE;
		ml.flags = MEDIA_LNK_FL_ENABLED;

		ioctl(fd, MEDIA_IOC_SETUP_LINK, &ml);

		ctx.dbg->DebugOut(kDbgLvlImportant, "%s: link source %d (pad %d) to sink %d (pad %d)\n", __FUNCTION__, ml.source.entity, ml.source.index, ml.sink.entity, ml.sink.index);
	}

//	close(fd);

	ctx.file = "/dev/video2"; // CLIPPER1

	return true;
}

//============================================================================
// Ctor & dtor
//============================================================================
CVIPCameraModule::CVIPCameraModule()
{
	tCameraControls::iterator	it;

	dbg_.DebugOut(kDbgLvlImportant, "%s: pid=%d\n", __FUNCTION__, getpid());

	// Enumerate media layer nodes, if any
	EnumMediaNodes(camCtx_);

	camCtx_.mode = QVGA;
	valid = InitCameraInt(&camCtx_.mode, false);

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

	// Cache copy of V4L controls to track across CameraMPI mode resets
	controlsCached 		= new tCameraControls;
	for ( it = camCtx_.controls->begin() ; it < camCtx_.controls->end(); it++ )
	{
		tControlInfo* ctrl = *it;
		controlsCached->push_back(new tControlInfo(*ctrl));
	}

	// Init fb vars
	fd = fdvmem = -1;
	memset(&fi, 0, sizeof(fi));
	memset(&vi, 0, sizeof(vi));
	memset(&vm, 0, sizeof(vm));
	
	// Release camera device for this process instance
	DeinitCameraInt(true);
}

//----------------------------------------------------------------------------
CVIPCameraModule::~CVIPCameraModule()
{
	tCameraControls::iterator	it;

	// Stop all streaming (before base destructor called)
	StopVideoCapture(camCtx_.hndl);

	// Reset VIP flip/rotate controls when CameraMPI instance done
	for ( it = controlsCached->begin() ; it < controlsCached->end(); it++ )
	{
		tControlInfo* ctrl = *it;
		switch (ctrl->type) {
		case kControlTypeHorizontalFlip:
		case kControlTypeVerticalFlip:
		case kControlTypeRotate:
			SetCameraControl(ctrl, 0);
			break;
		}
		delete ctrl;
	}

	delete controlsCached;
}

//----------------------------------------------------------------------------
void CVIPCameraModule::FreeVMem()
{
#if !defined(EMULATION)
	dbg_.DebugOut(kDbgLvlImportant, "%s: pid=%d, fdvmem=%d\n", __FUNCTION__, getpid(), fdvmem);
	if (fdvmem == -1 && fd == -1)
		return;

	// Release memory used by VIP driver
	munmap((void*)vi.reserved[0], fi.smem_len);
	close(fd);
	if (fdvmem > 0) {
		ioctl(fdvmem, IOCTL_VMEM_FREE, &vm);
		close(fdvmem);
	}
	fd = fdvmem = -1;
#endif
}

//----------------------------------------------------------------------------
static Boolean SetOverlay(int fd, tVideoSurf* pSurf)
{
#if !defined(EMULATION) //&& defined(LF2000)
	struct v4l2_framebuffer v4l2;

	memset(&v4l2, 0, sizeof(v4l2));
	switch(pSurf->format)
	{
	case kPixelFormatYUV420:
		v4l2.fmt.pixelformat = V4L2_PIX_FMT_YUV420;
		v4l2.fmt.priv = PIPE_TO_FMT_PRIV(VIP_OUTPUT_PIPE_DECIMATOR) | DOUBLE_BUF_TO_FMT_PRIV(VIP_OVERLAY_DOUBLE_BUF);
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
#if !defined(EMULATION) //&& defined(LF2000)
	if(ioctl(fd, VIDIOC_OVERLAY, &enable) < 0)
			return false;
	overlayEnabled = enable;
#endif
	return true;
}

//----------------------------------------------------------------------------
tVidCapHndl CVIPCameraModule::StartVideoCapture(const CPath& path, tVideoSurf* pSurf,
		IEventListener * pListener, const U32 maxLength, Boolean bAudio)
{
	tVidCapHndl hndl = kInvalidVidCapHndl;

	CAMERA_LOCK;

	if (IS_STREAMING_HANDLE(camCtx_.hndl))
	{
		if (!EnableOverlay(camCtx_.fd, 0))
			goto out;
	}

	// Re-init V4L device, as needed per instance
	if (camCtx_.fd == -1)
		InitCameraInt(&camCtx_.mode, true);

	// Allocate video memory
	AllocVMem(camCtx_);

	if(path.length() || path.empty())
	{
		CAMERA_UNLOCK;

		/* Spawn capture thread w/out viewfinder */
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

	if(IS_THREAD_HANDLE(hndl) || IS_FRAME_HANDLE(hndl))
	{
		ret = CCameraModule::StopVideoCapture(hndl);
	}

	camCtx_.hndl = kInvalidVidCapHndl;

	// Free video memory
	FreeVMem();

	return ret;
}

//----------------------------------------------------------------------------
Boolean CVIPCameraModule::PauseVideoCapture(const tVidCapHndl hndl, const Boolean display)
{
	CAMERA_LOCK;
	if ((hndl & kStreamingActive) && display)
		 EnableOverlay(camCtx_.fd, 0);
	CAMERA_UNLOCK;
	return CCameraModule::PauseVideoCapture(hndl, display);
}

//----------------------------------------------------------------------------
Boolean CVIPCameraModule::ResumeVideoCapture(const tVidCapHndl hndl)
{
	CAMERA_LOCK;
	if ((hndl & kStreamingActive) && overlaySurf.buffer != NULL) {
		SetOverlay(camCtx_.fd, &overlaySurf);
		EnableOverlay(camCtx_.fd, 1);
	}
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
	tFrameInfo 			frame	= {kCaptureFormatYUV420, pSurf->width, pSurf->height, 0, NULL, 0};
	struct tCaptureMode	oldMode	= camCtx_.mode;
	struct tCaptureMode	newMode = {kCaptureFormatYUV420, pSurf->width, pSurf->height, 1, 10};
	Boolean				visible = (overlaySurf.buffer != NULL && overlayEnabled) ? 1 : 0;
	Boolean				sameSize = newMode.width == oldMode.width && newMode.height == oldMode.height;
	Boolean				requeue = false;
	tCameraControls::iterator	it, sit;

	// Update cached control state for this CameraMPI instance
	for ( it = controlsCached->begin(), sit = camCtx_.controls->begin() ; sit < camCtx_.controls->end(); it++, sit++ )
	{
		(*it)->current = (*sit)->current;
	}

	// Switch to YUYV packed format for high-res capture modes
	if (pSurf->format == kPixelFormatYUYV422)
	{
		newMode.pixelformat = frame.pixelformat = kCaptureFormatRAWYUYV;
	}

	// Switch to custom square size format for rotated image capture
	if (pSurf->width < pSurf->height)
	{
		sameSize = false;
		newMode.width = frame.width = pSurf->height;
	}

	/* Stop viewfinder */
	if ((hndl & kStreamingActive) && visible)
		EnableOverlay(camCtx_.fd, 0);

	if (oldMode.pixelformat == frame.pixelformat && sameSize)
	{
		ret = CCameraModule::GetFrame(hndl, &frame);
		if (ret)
		{
			struct timeval now;
			gettimeofday(&now, NULL);
			#define SEC_USECS   1000000ULL
			#define DELTA_USECS  210000ULL
			U64 now64 = now.tv_sec * SEC_USECS + now.tv_usec;
			U64 frame64 = frame.timestamp.tv_sec * SEC_USECS + frame.timestamp.tv_usec;
			//210000 usec arrived at by test mashing Camera App Picture taking button
			if (now64 > frame64 + DELTA_USECS)
			{	
				CCameraModule::ReturnFrame(hndl, &frame);
				ret = CCameraModule::GetFrame(hndl, &frame);
				if (ret)
					requeue = true;
				else
					frame.data = NULL;
			} else {
				requeue = true;
			}
		}
	}
	else
		ret = CCameraModule::GrabFrame(hndl, &frame);

	if (ret == false)
	{
		sameSize = false;

		CAMERA_LOCK;

		/* Close camera fd */
		ret = CCameraModule::DeinitCameraInt(true);
		if (!ret)
			goto out;
	
		/* Re-open camera */
		ret = CCameraModule::InitCameraInt(&newMode, true);
		if (!ret)
			goto out;

		CAMERA_UNLOCK;

		// Update VIP control state for new V4L instance
		for ( it = controlsCached->begin() ; it < controlsCached->end(); it++ )
		{
			tControlInfo* ctrl = *it;
			SetCameraControl(ctrl, ctrl->current);
		}

		ret = CCameraModule::GrabFrame(hndl, &frame);
	}

	if (ret)
	{
		ret = RenderFrame(frame, pSurf, color_order);
	}

	if (requeue)
		CCameraModule::ReturnFrame(hndl, &frame);
	else
	if (frame.data)
		kernel_.Free(frame.data);

	CAMERA_LOCK;


	if ((hndl & kStreamingActive) && visible)
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
