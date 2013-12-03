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
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/v4l2-mediabus.h>
#include <lf3000/nxp-v4l2-media.h>
#include <lf3000/nx_alloc_mem.h>

LF_BEGIN_BRIO_NAMESPACE()
//==============================================================================
// Defines
//==============================================================================

tCaptureMode QVGA = {kCaptureFormatYUV420, 400, 300, 1, 30};
tCaptureMode SVGA = {kCaptureFormatYUV420, 800, 600, 1, 30};
tCaptureMode UXGA = {kCaptureFormatYUV420, 1600, 1200, 1, 30};

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
	vb.fds[0] = (int)((NX_MEMORY_INFO*)vm->privateDesc[0])->privateDesc;
	vb.fds[1] = (int)((NX_MEMORY_INFO*)vm->privateDesc[1])->privateDesc;
	vb.fds[2] = (int)((NX_MEMORY_INFO*)vm->privateDesc[2])->privateDesc;
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
	camCtx_.modes->push_back(new tCaptureMode(SVGA));
	camCtx_.modes->push_back(new tCaptureMode(UXGA));
	valid = true;

	// Use YUV framebuffer memory for V4L buffers
	camCtx_.fi = new struct fb_fix_screeninfo;
	camCtx_.vi = new struct fb_var_screeninfo;
	camCtx_.fd = open("/dev/fb2", O_RDWR);
	ioctl(camCtx_.fd, FBIOGET_FSCREENINFO, camCtx_.fi);
	ioctl(camCtx_.fd, FBIOGET_VSCREENINFO, camCtx_.vi);
	camCtx_.vi->reserved[0] = (unsigned int)mmap(0, camCtx_.fi->smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, camCtx_.fd, 0);

	memset(&vb, 0, sizeof(vb));
	PackVidBuf(vb, camCtx_);

	// Use ION memory for V4L buffers
	nxpvbuf_ = vm = NX_VideoAllocateMemory(64, camCtx_.mode.width, camCtx_.mode.height, NX_MEM_MAP_TILED, FOURCC_MVS0);
	PackVidBuf(vb, vm);

	v4l2_reqbuf(nxphndl_, nxp_v4l2_clipper1, 1);
	v4l2_qbuf(nxphndl_, nxp_v4l2_clipper1, vb.plane_num, 0, &vb, -1, NULL);

	v4l2_reqbuf(nxphndl_, nxp_v4l2_mlc0_video, 1);
	v4l2_qbuf(nxphndl_, nxp_v4l2_mlc0_video, vb.plane_num, 0, &vb, -1, NULL);

	device_ = kCameraDefault;
	sensor_ = nxp_v4l2_sensor1;
	clipper_ = nxp_v4l2_clipper1;
}

//----------------------------------------------------------------------------
CNXPCameraModule::~CNXPCameraModule()
{
//	DeinitCameraInt();
	munmap((void*)camCtx_.vi->reserved[0], camCtx_.fi->smem_len);
	close(camCtx_.fd);
	delete camCtx_.vi;
	delete camCtx_.fi;

	NX_FreeVideoMemory((NX_VID_MEMORY_HANDLE)nxpvbuf_);

	v4l2_unlink(nxphndl_, clipper_, nxp_v4l2_mlc0_video);
	v4l2_unlink(nxphndl_, sensor_, clipper_);

	// Exit NXP V4L2 module
	v4l2_exit(nxphndl_);
}

//============================================================================
// NXP specific implementation
//============================================================================
//----------------------------------------------------------------------------
Boolean CNXPCameraModule::InitCameraInt(const tCaptureMode* mode, bool reinit)
{
	return true;
}
//----------------------------------------------------------------------------
Boolean CNXPCameraModule::DeinitCameraInt(bool reinit)
{
	return true;
}
//----------------------------------------------------------------------------
Boolean CNXPCameraModule::InitCameraBufferInt(tCameraContext *pCamCtx)
{
	return true;
}
//----------------------------------------------------------------------------
Boolean CNXPCameraModule::DeinitCameraBufferInt(tCameraContext *pCamCtx)
{
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
	v4l2_set_format(nxphndl_, sensor_, mode->width, mode->height, PIXCODE_YUV420_PLANAR);
	v4l2_set_format(nxphndl_, clipper_, mode->width, mode->height, PIXFORMAT_YUV420_PLANAR);
	v4l2_set_crop(nxphndl_, clipper_, 0, 0, mode->width, mode->height);
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
	struct nxp_vid_buffer  vb;
	NX_VID_MEMORY_INFO    *vm = (NX_VID_MEMORY_INFO*)nxpvbuf_;

	if (device_ == device)
		return kNoErr;

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

	v4l2_set_format(nxphndl_, sensor_, camCtx_.mode.width, camCtx_.mode.height, PIXCODE_YUV420_PLANAR);
	v4l2_set_format(nxphndl_, clipper_, camCtx_.mode.width, camCtx_.mode.height, PIXFORMAT_YUV420_PLANAR);
	v4l2_set_crop(nxphndl_, clipper_, 0, 0, camCtx_.mode.width, camCtx_.mode.height);

	PackVidBuf(vb, vm);
	v4l2_reqbuf(nxphndl_, clipper_, 1);
	v4l2_qbuf(nxphndl_, clipper_, vb.plane_num, 0, &vb, -1, NULL);

	return kNoErr;
}

//----------------------------------------------------------------------------
Boolean CNXPCameraModule::InitCameraStartInt(tCameraContext *pCamCtx)
{
	v4l2_streamon(nxphndl_, clipper_);
	if (pCamCtx->surf) {
		int index = 0;
		v4l2_dqbuf(nxphndl_, clipper_, 3, &index, NULL);
		v4l2_set_crop(nxphndl_, nxp_v4l2_mlc0_video, 0, 0, pCamCtx->surf->width, pCamCtx->surf->height);
		v4l2_streamon(nxphndl_, nxp_v4l2_mlc0_video);
	}
	return true;
}
//----------------------------------------------------------------------------
tVidCapHndl CNXPCameraModule::StartVideoCapture(const CPath& path, tVideoSurf* pSurf,
		IEventListener * pListener, const U32 maxLength, Boolean bAudio)
{
//	return CCameraModule::StartVideoCapture(path, pSurf, pListener, maxLength, bAudio);

	camCtx_.module	= this;
	camCtx_.path	= path;
	camCtx_.surf	= pSurf;
	camCtx_.bAudio	= bAudio && (path.length() > 0);
	camCtx_.maxLength = maxLength;
	camCtx_.reqLength = maxLength;
	camCtx_.pListener = pListener;

	InitCameraStartInt(&camCtx_);
	InitCameraTask(&camCtx_);
	return STREAMING_HANDLE((tVidCapHndl)nxphndl_);
}

//----------------------------------------------------------------------------
Boolean CNXPCameraModule::StopVideoCaptureInt(int fd)
{
	v4l2_streamoff(nxphndl_, nxp_v4l2_mlc0_video);
	v4l2_streamoff(nxphndl_, clipper_);
	return true;
}
//----------------------------------------------------------------------------
Boolean CNXPCameraModule::StopVideoCapture(const tVidCapHndl hndl)
{
//	return CCameraModule::StopVideoCapture(hndl);
	StopVideoCaptureInt(camCtx_.fd);
	DeInitCameraTask(&camCtx_);
	return true;
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
	return false; //CCameraModule::SnapFrame(hndl, path);
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
	return false; //CCameraModule::GetFrame(hndl, pSurf, color_order);
}

//----------------------------------------------------------------------------
Boolean CNXPCameraModule::PollFrame(const tVidCapHndl hndl)
{
	return true;
}

//----------------------------------------------------------------------------
Boolean	CNXPCameraModule::GetFrame(const tVidCapHndl hndl, tFrameInfo *frame)
{
	int index = 0;
	long long int timestamp = 0;
	NX_VID_MEMORY_INFO    *vm = (NX_VID_MEMORY_INFO*)nxpvbuf_;

	v4l2_dqbuf(nxphndl_, clipper_, 3, &index, NULL);
	v4l2_get_timestamp(nxphndl_, clipper_, &timestamp);

	timestamp /= 1000LL;
	frame->timestamp.tv_sec 	= timestamp / 1000000LL;
	frame->timestamp.tv_usec 	= timestamp % 1000000LL;
	frame->index 	= index;
	frame->data  	= (void*)vm->luVirAddr;
	frame->size  	= vm->luStride * vm->imgHeight;
	frame->width 	= vm->imgWidth;
	frame->height	= vm->imgHeight;

	return true;
}

//----------------------------------------------------------------------------
Boolean	CNXPCameraModule::ReturnFrame(const tVidCapHndl hndl, const tFrameInfo *frame)
{
	struct nxp_vid_buffer  vb;
	NX_VID_MEMORY_INFO    *vm = (NX_VID_MEMORY_INFO*)nxpvbuf_;

	PackVidBuf(vb, vm);
	v4l2_qbuf(nxphndl_, clipper_, vb.plane_num, 0, &vb, -1, NULL);
	return true;
}

//----------------------------------------------------------------------------
Boolean CNXPCameraModule::GrabFrame(const tVidCapHndl hndl, tFrameInfo *frame)
{
	return CCameraModule::GrabFrame(hndl, frame);
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
