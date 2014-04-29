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
#include <DisplayPriv.h>
#include <DisplayMPI.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/v4l2-mediabus.h>
#include <lf3000/nxp-v4l2-media.h>
#include <lf3000/nx_alloc_mem.h>
#include <linux/lf1000/lf1000fb.h>

LF_BEGIN_BRIO_NAMESPACE()
//==============================================================================
// Defines
//==============================================================================

tCaptureMode QVGA = {kCaptureFormatYUV420, 320, 240, 1, 30};
tCaptureMode QSGA = {kCaptureFormatYUV420, 400, 300, 1, 30};
tCaptureMode  VGA = {kCaptureFormatYUV420, 640, 480, 1, 30};
tCaptureMode SVGA = {kCaptureFormatYUV420, 800, 600, 1, 30};
tCaptureMode SXGA = {kCaptureFormatYUV420, 1280,  960, 1, 15};
tCaptureMode UXGA = {kCaptureFormatYUV420, 1600, 1200, 1, 15};

tControlInfo CTRLS[] = {
	{kControlTypeBrightness, 0, 255, 128, 128},
	{kControlTypeContrast, 0, 255, 128, 128},
	{kControlTypeSaturation, 0, 255, 128, 128},
	{kControlTypeAutoWhiteBalance, 0, 1, 1, 1},
	{kControlTypeHorizontalFlip, 0, 1, 0, 0},
	{kControlTypeVerticalFlip, 0, 1, 0, 0},
	{kControlTypeRotate, 0, 1, 0, 0},
};

//----------------------------------------------------------------------------
inline void PackVidBuf(struct nxp_vid_buffer& vb, NX_VID_MEMORY_INFO* vm)
{
	vb.plane_num = 3;
	vb.phys[0] = vm->luPhyAddr;
	vb.phys[1] = vm->cbPhyAddr;
	vb.phys[2] = vm->crPhyAddr;
	vb.virt[0] = (char*)vm->luVirAddr;
	vb.virt[1] = (char*)vm->cbVirAddr;
	vb.virt[2] = (char*)vm->crVirAddr;
	vb.sizes[0] = vm->luStride * vm->imgHeight;
	vb.sizes[1] = vm->cbStride * vm->imgHeight;
	vb.sizes[2] = vm->crStride * vm->imgHeight;
//	vb.fds[0] = (int)((NX_MEMORY_INFO*)vm->privateDesc[0])->privateDesc;
//	vb.fds[1] = (int)((NX_MEMORY_INFO*)vm->privateDesc[1])->privateDesc;
//	vb.fds[2] = (int)((NX_MEMORY_INFO*)vm->privateDesc[2])->privateDesc;
	vb.fds[0] = (int)vm->privateDesc[0];
	vb.fds[1] = (int)vm->privateDesc[1];
	vb.fds[2] = (int)vm->privateDesc[2];

	// fixup planar buffer for 4K stride
	vb.phys[1] = vb.phys[0] + vm->luStride/2;
	vb.phys[2] = vb.phys[1] + vm->luStride * vm->imgHeight/2;
	vb.virt[1] = vb.virt[0] + vm->luStride/2;
	vb.virt[2] = vb.virt[1] + vm->luStride * vm->imgHeight/2;
}

//----------------------------------------------------------------------------
inline void PackVidBuf(struct nxp_vid_buffer& vb, NX_MEMORY_INFO* vm)
{
	vb.plane_num = 3;
	vb.phys[0] = vm->phyAddr;
	vb.phys[1] = vb.phys[0] + 4096/2;
	vb.phys[2] = vb.phys[1] + vm->size/2;
	vb.virt[0] = (char*)vm->virAddr;
	vb.virt[1] = vb.virt[0] + 4096/2;
	vb.virt[2] = vb.virt[1] + vm->size/2;
	vb.sizes[0] =
	vb.sizes[1] =
	vb.sizes[2] = vm->size;
	vb.fds[0] =
	vb.fds[1] =
	vb.fds[2] = (int)vm->privateDesc;
}

//----------------------------------------------------------------------------
inline void PackVidBuf(NX_VID_MEMORY_INFO* vm, NX_MEMORY_INFO* mi, int w, int h)
{
	vm->luPhyAddr = mi->phyAddr;
	vm->cbPhyAddr = vm->luPhyAddr + 4096/2;
	vm->crPhyAddr = vm->cbPhyAddr + 4096 * h/2;
	vm->luVirAddr = mi->virAddr;
	vm->cbVirAddr = vm->luVirAddr + 4096/2;
	vm->crVirAddr = vm->cbVirAddr + 4096 * h/2;
	vm->luStride =
	vm->cbStride =
	vm->crStride = 4096;
	vm->imgWidth  = w;
	vm->imgHeight = h;
	vm->privateDesc[0] =
	vm->privateDesc[1] =
	vm->privateDesc[2] = mi->privateDesc;
}

//----------------------------------------------------------------------------
inline void PackVidBuf(struct nxp_vid_buffer& vb, tCameraContext& ctx)
{
	vb.plane_num = 3;
	vb.fds[0] = vb.fds[1] = vb.fds[2] = ctx.fd;
	vb.sizes[0] = vb.sizes[1] = vb.sizes[2] = ctx.fi->line_length * ctx.mode.height;
	vb.phys[0] = ctx.fi->smem_start;
	vb.phys[1] = vb.phys[0] + ctx.fi->line_length/2;
	vb.phys[2] = vb.phys[1] + ctx.fi->line_length * ctx.mode.height/2;
	vb.virt[0] = (char*)ctx.vi->reserved[0];
	vb.virt[1] = vb.virt[0] + ctx.fi->line_length/2;
	vb.virt[2] = vb.virt[1] + ctx.fi->line_length * ctx.mode.height/2;
}

//----------------------------------------------------------------------------
inline void PackVidBuf(struct nxp_vid_buffer& vb, tVideoSurf* pSurf)
{
	vb.plane_num = 3;
	vb.phys[0] = (unsigned int)pSurf->buffer;
	vb.phys[1] = vb.phys[0] + pSurf->pitch/2;
	vb.phys[2] = vb.phys[1] + pSurf->pitch * pSurf->height/2;
	vb.virt[0] = (char*)pSurf->buffer;
	vb.virt[1] = vb.virt[0] + pSurf->pitch/2;
	vb.virt[2] = vb.virt[1] + pSurf->pitch * pSurf->height/2;
	vb.sizes[0] = pSurf->pitch * pSurf->height;
	vb.sizes[1] =
	vb.sizes[2] = pSurf->pitch * pSurf->height/2;
}

//----------------------------------------------------------------------------
inline void PackVidBuf(struct nxp_vid_buffer& vb, struct tFrameInfo* fi)
{
}

//----------------------------------------------------------------------------
inline void UnPackVidBuf(struct nxp_vid_buffer& vb, struct tFrameInfo* fi)
{
}

//----------------------------------------------------------------------------
inline void UnPackVidBuf(NX_VID_MEMORY_INFO* vm, tVideoSurf* pSurf)
{
	pSurf->buffer = (U8*)vm->luVirAddr;
	pSurf->pitch  = vm->luStride;
	pSurf->width  = vm->imgWidth;
	pSurf->height = vm->imgHeight;
	pSurf->format = kPixelFormatYUV420;
}

//----------------------------------------------------------------------------
inline void GetWindowPosition(tVideoSurf* pSurf, int* dx, int* dy)
{
	S16 x, y;
	U16 w, h;
	Boolean v;
	CDisplayMPI 	dispmgr;
	tDisplayHandle 	hvideo = dispmgr.GetCurrentDisplayHandle(kPixelFormatYUV420);

	dispmgr.GetWindowPosition(hvideo, x, y, w, h, v);
	*dx = x;
	*dy = y;
}

//----------------------------------------------------------------------------
inline void SetWindowScale(tVideoSurf* pSurf, int sw, int sh)
{
	CDisplayMPI 	dispmgr;
	tDisplayHandle 	hvideo = dispmgr.GetCurrentDisplayHandle(kPixelFormatYUV420);

	dispmgr.SetVideoScaler(hvideo, sw, sh, false);
}

//----------------------------------------------------------------------------
inline int GetZOrderPriority(void)
{
	int priority = 1;
	int fbdev = open("/dev/fb2", O_RDWR | O_SYNC);
	struct fb_var_screeninfo vinfo;
	if (fbdev > 0) {
		ioctl(fbdev, FBIOGET_VSCREENINFO, &vinfo);
		close(fbdev);
		priority = (vinfo.nonstd & (3<<LF1000_NONSTD_PRIORITY)) >> LF1000_NONSTD_PRIORITY;
	}
	return priority;
}

//----------------------------------------------------------------------------
inline void SetOverlayVisible(bool visible)
{
	int fbdev = open("/dev/fb2", O_RDWR | O_SYNC);
	if (fbdev > 0) {
		ioctl(fbdev, FBIOBLANK, (visible) ? 0 : 1);
		close(fbdev);
	}

}

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
	// Enumerate camera modes to match preferred default
	camCtx_.mode = QVGA;
	camCtx_.file = "/dev/video2";
	camCtx_.modes->push_back(new tCaptureMode(QVGA));
	camCtx_.modes->push_back(new tCaptureMode(QSGA));
	camCtx_.modes->push_back(new tCaptureMode( VGA));
	camCtx_.modes->push_back(new tCaptureMode(SVGA));
	camCtx_.modes->push_back(new tCaptureMode(SXGA));
	camCtx_.modes->push_back(new tCaptureMode(UXGA));
	for (int i = 0; i < sizeof(CTRLS)/sizeof(CTRLS[0]); i++) {
		camCtx_.controls->push_back(new tControlInfo(CTRLS[i]));
	}
	valid = InitCameraInt(&camCtx_.mode, false);
}

//----------------------------------------------------------------------------
Boolean CNXPCameraModule::InitCameraInt(const tCaptureMode* mode, bool reinit)
{
    struct V4l2UsageScheme us;
    struct nxp_vid_buffer  vb;
    NX_VID_MEMORY_INFO    *vm;

    memset(&us, 0, sizeof(us));
    us.useClipper0 = true;
    us.useClipper1 = true;
    us.useDecimator0 = true;
    us.useDecimator1 = true;
    us.useMlc0Video = true;

	// Init NXP V4L2 module
	nxphndl_ = v4l2_init(&us);
	dbg_.Assert(nxphndl_ != NULL, "%s: v4l2_init failed\n", __func__);

	v4l2_link(nxphndl_, nxp_v4l2_sensor1, nxp_v4l2_clipper1);
	v4l2_link(nxphndl_, nxp_v4l2_clipper1, nxp_v4l2_mlc0_video);

	v4l2_set_format(nxphndl_, nxp_v4l2_sensor1, mode->width, mode->height, PIXFORMAT_YUV422_PACKED);
	v4l2_set_format(nxphndl_, nxp_v4l2_clipper1, mode->width, mode->height, PIXFORMAT_YUV420_PLANAR);
	v4l2_set_format(nxphndl_, nxp_v4l2_mlc0_video, mode->width, mode->height, PIXFORMAT_YUV420_PLANAR);
	v4l2_set_crop(nxphndl_, nxp_v4l2_clipper1, 0, 0, mode->width, mode->height);
	v4l2_set_crop(nxphndl_, nxp_v4l2_mlc0_video, 0, 0, mode->width, mode->height);

	device_ = kCameraDefault;
	sensor_ = nxp_v4l2_sensor1;
	clipper_ = nxp_v4l2_clipper1;
	nxpvbuf_[0] = NULL;
	nxpmbuf_[0] = NULL;
	maxcnt_ = MAX_NXP_BUFS;
	overlay_ = false;
	streaming_ = false;

	return true;
}

//----------------------------------------------------------------------------
CNXPCameraModule::~CNXPCameraModule()
{
	tCaptureModes::iterator	mit;
	tCameraControls::iterator cit;

	DeinitCameraInt();

	for (cit = camCtx_.controls->begin(); cit != camCtx_.controls->end(); cit++) {
		delete *cit;
	}
	camCtx_.controls->clear();

	for (mit = camCtx_.modes->begin(); mit != camCtx_.modes->end(); mit++) {
		delete *mit;
	}
	camCtx_.modes->clear();
}

//----------------------------------------------------------------------------
Boolean CNXPCameraModule::DeinitCameraInt(bool reinit)
{
	StopVideoCapture(camCtx_.hndl);
	DeinitCameraBufferInt(&camCtx_);

	v4l2_unlink(nxphndl_, clipper_, nxp_v4l2_mlc0_video);
	v4l2_unlink(nxphndl_, sensor_, clipper_);

	// Exit NXP V4L2 module
	v4l2_exit(nxphndl_);

	return true;
}

//============================================================================
// NXP specific implementation
//============================================================================
//----------------------------------------------------------------------------
Boolean CNXPCameraModule::InitCameraBufferInt(tCameraContext *pCamCtx)
{
    struct nxp_vid_buffer  vb;
    NX_VID_MEMORY_INFO    *vm;
	NX_MEMORY_INFO    	  *mi;

	v4l2_reqbuf(nxphndl_, clipper_, maxcnt_);
	v4l2_reqbuf(nxphndl_, nxp_v4l2_mlc0_video, maxcnt_);

	for (int i = 0; i < maxcnt_; i++)
	{
		// Use ION memory for V4L buffers
	//	nxpvbuf_[i] = vm = NX_VideoAllocateMemory(64, 4096, camCtx_.mode.height, NX_MEM_MAP_TILED, FOURCC_MVS0);
		nxpmbuf_[i] = mi = NX_AllocateMemory(4096 * camCtx_.mode.height, 64);
		nxpvbuf_[i] = vm = new NX_VID_MEMORY_INFO;
		if (nxpmbuf_[i] == NULL)
			continue;
	//	PackVidBuf(vb, vm);
		PackVidBuf(vb, mi);
		PackVidBuf(vm, mi, camCtx_.mode.width, camCtx_.mode.height);

		v4l2_qbuf(nxphndl_, clipper_, vb.plane_num, i, &vb, -1, NULL);
	//	v4l2_qbuf(nxphndl_, nxp_v4l2_mlc0_video, vb.plane_num, i, &vb, -1, NULL);
	}

	index_ = 0;
	outcnt_ = 0;

	return true;
}
//----------------------------------------------------------------------------
Boolean CNXPCameraModule::DeinitCameraBufferInt(tCameraContext *pCamCtx)
{
	if (!nxpvbuf_[0])
		return true;

	if (streaming_)
		StopVideoCaptureInt(camCtx_.fd);

	for (int i = maxcnt_-1; i >= 0; i--)
	{
	//	NX_FreeVideoMemory((NX_VID_MEMORY_HANDLE)nxpvbuf_[i]);
		NX_FreeMemory((NX_MEMORY_HANDLE)nxpmbuf_[i]);
		delete (NX_VID_MEMORY_HANDLE)nxpvbuf_[i];
		nxpvbuf_[i] = NULL;
		nxpmbuf_[i] = NULL;
	}
	return true;
}
//----------------------------------------------------------------------------
Boolean CNXPCameraModule::SetBuffers(const U32 numBuffers)
{
	return true;
}
//----------------------------------------------------------------------------
tErrType CNXPCameraModule::EnumFormats(tCaptureModes& pModeList)
{
	pModeList = *camCtx_.modes;
	return kNoErr;
}

//----------------------------------------------------------------------------
Boolean CNXPCameraModule::SetCameraMode(const tCaptureMode* mode)
{
	v4l2_set_format(nxphndl_, sensor_, mode->width, mode->height, PIXFORMAT_YUV422_PACKED);
	v4l2_set_format(nxphndl_, clipper_, mode->width, mode->height, PIXFORMAT_YUV420_PLANAR);
	v4l2_set_format(nxphndl_, nxp_v4l2_mlc0_video, mode->width, mode->height, PIXFORMAT_YUV420_PLANAR);
	v4l2_set_crop(nxphndl_, clipper_, 0, 0, mode->width, mode->height);
	v4l2_set_crop(nxphndl_, nxp_v4l2_mlc0_video, 0, 0, mode->width, mode->height);

	camCtx_.mode = *mode;
	camCtx_.fmt.fmt.pix.width	= mode->width;
	camCtx_.fmt.fmt.pix.height	= mode->height;
	camCtx_.fmt.fmt.pix.pixelformat	= V4L2_PIX_FMT_YUV420;
	camCtx_.fps = mode->fps_denominator / mode->fps_numerator;

	return true;
}
//----------------------------------------------------------------------------
tErrType CNXPCameraModule::SetCurrentFormat(tCaptureMode* pMode)
{
	camCtx_.mode = *pMode;
	SetCameraMode(pMode);
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CNXPCameraModule::SetCurrentCamera(tCameraDevice device)
{
	if (device_ == device)
		return kNoErr;

	if (streaming_)
		StopVideoCaptureInt(camCtx_.fd);

	switch (device) {
	case kCameraFront:
		v4l2_unlink(nxphndl_, nxp_v4l2_clipper1, nxp_v4l2_mlc0_video);
		v4l2_unlink(nxphndl_, nxp_v4l2_sensor1, nxp_v4l2_clipper1);
		device_ = device;
		sensor_ = nxp_v4l2_sensor0;
		clipper_ = nxp_v4l2_clipper0;
		v4l2_link(nxphndl_, nxp_v4l2_sensor0, nxp_v4l2_clipper0);
		v4l2_link(nxphndl_, nxp_v4l2_clipper0, nxp_v4l2_mlc0_video);
		break;
	case kCameraDefault:
		v4l2_unlink(nxphndl_, nxp_v4l2_clipper0, nxp_v4l2_mlc0_video);
		v4l2_unlink(nxphndl_, nxp_v4l2_sensor0, nxp_v4l2_clipper0);
		device_ = device;
		sensor_ = nxp_v4l2_sensor1;
		clipper_ = nxp_v4l2_clipper1;
		v4l2_link(nxphndl_, nxp_v4l2_sensor1, nxp_v4l2_clipper1);
		v4l2_link(nxphndl_, nxp_v4l2_clipper1, nxp_v4l2_mlc0_video);
		break;
	default:
		return kInvalidParamErr;
	}

	v4l2_set_format(nxphndl_, sensor_, camCtx_.mode.width, camCtx_.mode.height, PIXFORMAT_YUV422_PACKED);
	v4l2_set_format(nxphndl_, clipper_, camCtx_.mode.width, camCtx_.mode.height, PIXFORMAT_YUV420_PLANAR);
	v4l2_set_crop(nxphndl_, clipper_, 0, 0, camCtx_.mode.width, camCtx_.mode.height);

	return kNoErr;
}
//----------------------------------------------------------------------------
Boolean	CNXPCameraModule::SetCameraControl(const tControlInfo* control, const S32 value)
{
	Boolean ret = false;
	int r = 0;

	switch (control->type) {
	case kControlTypeBrightness:
		r = v4l2_set_ctrl(nxphndl_, sensor_, V4L2_CID_BRIGHTNESS, value);
		break;
	case kControlTypeContrast:
		r = v4l2_set_ctrl(nxphndl_, sensor_, V4L2_CID_CONTRAST, value);
		break;
	case kControlTypeSaturation:
		r = v4l2_set_ctrl(nxphndl_, sensor_, V4L2_CID_SATURATION, value);
		break;
	case kControlTypeAutoWhiteBalance:
		r = v4l2_set_ctrl(nxphndl_, sensor_, V4L2_CID_AUTO_WHITE_BALANCE, value);
		break;
	case kControlTypeHorizontalFlip:
		r = v4l2_set_ctrl(nxphndl_, clipper_, V4L2_CID_HFLIP, value);
		break;
	case kControlTypeVerticalFlip:
		r = v4l2_set_ctrl(nxphndl_, clipper_, V4L2_CID_VFLIP, value);
		break;
	case kControlTypeRotate:
		r = v4l2_set_ctrl(nxphndl_, clipper_, V4L2_CID_ROTATE, value);
		break;
	default:
		ret = false;
		break;
	}

	if (r == 0) {
		tCameraControls::iterator	it;
		for (it = camCtx_.controls->begin(); it < camCtx_.controls->end(); it++) {
			if ( control->type == (*it)->type ) {
				(*it)->current = value;
				ret = true;
				break;
			}
		}
	}

	return ret;
}

#define V4L2_CID_MLC_VID_PRIORITY    (V4L2_CTRL_CLASS_USER + 21)
//----------------------------------------------------------------------------
Boolean CNXPCameraModule::InitCameraStartInt(tCameraContext *pCamCtx)
{
	v4l2_streamon(nxphndl_, clipper_);
	if (pCamCtx->surf) {
		int index = 0;
		int x, y;
		GetWindowPosition(pCamCtx->surf, &x, &y);
		v4l2_set_crop(nxphndl_, nxp_v4l2_mlc0_video, x, y, pCamCtx->surf->width, pCamCtx->surf->height);
		SetWindowScale(pCamCtx->surf, camCtx_.mode.width, camCtx_.mode.height);

		int priority = GetZOrderPriority();
		v4l2_set_ctrl(nxphndl_, nxp_v4l2_mlc0_video, V4L2_CID_MLC_VID_PRIORITY, priority);

		v4l2_streamon(nxphndl_, nxp_v4l2_mlc0_video);
		overlay_ = true;
	}
	streaming_ = true;
	return true;
}
//----------------------------------------------------------------------------
tVidCapHndl CNXPCameraModule::StartVideoCapture(const CPath& path, tVideoSurf* pSurf,
		IEventListener * pListener, const U32 maxLength, Boolean bAudio)
{
#if 0
	return CCameraModule::StartVideoCapture(path, NULL, pListener, maxLength, bAudio);
#else
	kernel_.LockMutex(mutex_);

	camCtx_.module	= this;
	camCtx_.path	= path;
	camCtx_.surf	= pSurf;
	camCtx_.bAudio	= bAudio && (path.length() > 0);
	camCtx_.maxLength = maxLength;
	camCtx_.reqLength = maxLength;
	camCtx_.pListener = pListener;

	camCtx_.fmt.fmt.pix.width	= camCtx_.mode.width;
	camCtx_.fmt.fmt.pix.height	= camCtx_.mode.height;
	camCtx_.fmt.fmt.pix.pixelformat	= V4L2_PIX_FMT_YUV420;
	camCtx_.fps = camCtx_.mode.fps_denominator / camCtx_.mode.fps_numerator;

	InitCameraBufferInt(&camCtx_);
	InitCameraStartInt(&camCtx_);
	camCtx_.surf	= NULL;
	InitCameraTask(&camCtx_);

	camCtx_.hndl = STREAMING_HANDLE((tVidCapHndl)nxphndl_);

	kernel_.UnlockMutex(mutex_);

	return camCtx_.hndl;
#endif
}

//----------------------------------------------------------------------------
Boolean CNXPCameraModule::StopVideoCaptureInt(int fd)
{
	v4l2_streamoff(nxphndl_, nxp_v4l2_mlc0_video);
	v4l2_streamoff(nxphndl_, clipper_);
	streaming_ = false;
	overlay_ = false;

	for (int i = 0; i < maxcnt_; i++) {
		int index = 0;
		v4l2_dqbuf(nxphndl_, nxp_v4l2_mlc0_video, 3, &index, NULL);
		v4l2_dqbuf(nxphndl_, clipper_, 3, &index, NULL);
	}
	outcnt_ = 0;

	return true;
}
//----------------------------------------------------------------------------
Boolean CNXPCameraModule::StopVideoCapture(const tVidCapHndl hndl)
{
#if 0
	return CCameraModule::StopVideoCapture(hndl);
#else
	kernel_.LockMutex(camCtx_.mThread);
	kernel_.LockMutex(mutex_);
	
	if (kInvalidVidCapHndl == camCtx_.hndl)
		goto done;

	DeInitCameraTask(&camCtx_);
	StopVideoCaptureInt(camCtx_.fd);
	DeinitCameraBufferInt(&camCtx_);

	camCtx_.hndl = kInvalidVidCapHndl;
	
done:
	kernel_.UnlockMutex(mutex_);
	kernel_.UnlockMutex(camCtx_.mThread);
	return true;
#endif
}

//----------------------------------------------------------------------------
Boolean CNXPCameraModule::PauseVideoCapture(const tVidCapHndl hndl, const Boolean display)
{
	// Disable viewfinder overlay updates until resumed
	if (display) {
		v4l2_streamoff(nxphndl_, nxp_v4l2_mlc0_video);
		SetOverlayVisible(true);
		overlay_ = false;
	}
	return CCameraModule::PauseVideoCapture(hndl, display);
}

//----------------------------------------------------------------------------
Boolean CNXPCameraModule::ResumeVideoCapture(const tVidCapHndl hndl)
{
	// Re-enable viewfinder overlay if it was paused
	if (camCtx_.bVPaused && outcnt_) {
		v4l2_streamon(nxphndl_, nxp_v4l2_mlc0_video);
		overlay_ = true;
	}
	return CCameraModule::ResumeVideoCapture(hndl);
}

//----------------------------------------------------------------------------
Boolean	CNXPCameraModule::SnapFrame(const tVidCapHndl hndl, const CPath &path)
{
	return CCameraModule::SnapFrame(hndl, path);
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
	tFrameInfo 	frame	= {kCaptureFormatYUV420, pSurf->width, pSurf->height, 0, NULL, 0};

	kernel_.LockMutex(camCtx_.mThread);
//	kernel_.LockMutex(mutex_);

	GetFrame(hndl, &frame);

	// Pack captured data into requested surface format
	RenderFrame(frame, pSurf, color_order);

	ReturnFrame(hndl, &frame);

//	kernel_.UnlockMutex(mutex_);
	kernel_.UnlockMutex(camCtx_.mThread);

	return true;
}

//----------------------------------------------------------------------------
Boolean CNXPCameraModule::PollFrame(const tVidCapHndl hndl)
{
	if (camCtx_.bPaused)
		return false;

	int index = 0;
	int flags = 0;
	kernel_.LockMutex(mutex_);
	for (index = 0; index < maxcnt_; index++) {
		int r = v4l2_query_buf(nxphndl_, clipper_, 3, index, &flags);
		if (r == 0 && (flags & V4L2_BUF_FLAG_DONE)) {
			kernel_.UnlockMutex(mutex_);
			return true;
		}
	}
	kernel_.UnlockMutex(mutex_);
	return false;
}

//----------------------------------------------------------------------------
Boolean	CNXPCameraModule::GetFrame(const tVidCapHndl hndl, tFrameInfo *frame)
{
	long long int timestamp = 0;
	NX_VID_MEMORY_INFO    *vm = (NX_VID_MEMORY_INFO*)nxpvbuf_[index_];
	struct nxp_vid_buffer  vb;

	if (camCtx_.bPaused)
		return false;

	kernel_.LockMutex(mutex_);

	v4l2_dqbuf(nxphndl_, clipper_, 3, &index_, NULL);
	v4l2_get_timestamp(nxphndl_, clipper_, &timestamp);
	vm = (NX_VID_MEMORY_INFO*)nxpvbuf_[index_];

	timestamp /= 1000LL;
	frame->timestamp.tv_sec 	= timestamp / 1000000LL;
	frame->timestamp.tv_usec 	= timestamp % 1000000LL;
	frame->index 	= index_;
	frame->data  	= (void*)vm->luVirAddr;
	frame->size  	= vm->luStride * vm->imgHeight;
	frame->width 	= vm->imgWidth;
	frame->height	= vm->imgHeight;
	frame->pixelformat = kCaptureFormatYUV420;

	if (overlay_) {
		PackVidBuf(vb, vm);
		v4l2_qbuf(nxphndl_, nxp_v4l2_mlc0_video, vb.plane_num, index_, &vb, -1, NULL);
		outcnt_++;
	}

	kernel_.UnlockMutex(mutex_);
	return true;
}

//----------------------------------------------------------------------------
Boolean	CNXPCameraModule::ReturnFrame(const tVidCapHndl hndl, const tFrameInfo *frame)
{
	struct nxp_vid_buffer  vb;
	NX_VID_MEMORY_INFO    *vm = (NX_VID_MEMORY_INFO*)nxpvbuf_[index_];

	if (camCtx_.bPaused)
		return false;

	kernel_.LockMutex(mutex_);

	PackVidBuf(vb, vm);
	v4l2_qbuf(nxphndl_, clipper_, vb.plane_num, index_, &vb, -1, NULL);

	if (overlay_ && outcnt_ >= maxcnt_) {
		int index = 0;
		v4l2_dqbuf(nxphndl_, nxp_v4l2_mlc0_video, 3, &index, NULL);
		outcnt_--;
	}

	kernel_.UnlockMutex(mutex_);
	return true;
}

//----------------------------------------------------------------------------
Boolean CNXPCameraModule::GrabFrame(const tVidCapHndl hndl, tFrameInfo *frame)
{
	void* buf;

	if (frame->width != camCtx_.mode.width || frame->height != camCtx_.mode.height)
		return false;

	kernel_.LockMutex(camCtx_.mThread);
//	kernel_.LockMutex(mutex_);

	// Make copy of captured data for legacy GrabFrame() compatibility
	GetFrame(hndl, frame);

	buf = kernel_.Malloc(frame->size);
	memcpy(buf, frame->data, frame->size);

	ReturnFrame(hndl, frame);

	// Caller releases copy buffer
	frame->data = buf;

//	kernel_.UnlockMutex(mutex_);
	kernel_.UnlockMutex(camCtx_.mThread);

	return true;
}

//----------------------------------------------------------------------------
tVideoSurf* CNXPCameraModule::GetCaptureVideoSurface(const tVidCapHndl hndl)
{
	static tVideoSurf     surf; // FIXME
	NX_VID_MEMORY_INFO    *vm = (NX_VID_MEMORY_INFO*)nxpvbuf_[index_];

	UnPackVidBuf(vm, &surf);

	return &surf;
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
