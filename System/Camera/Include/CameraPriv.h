#ifndef LF_BRIO_CAMERAPRIV_H
#define LF_BRIO_CAMERAPRIV_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		CameraPriv.h
//
// Description:
//		Defines the interface for the private underlying Camera manager module.
//
//==============================================================================
#include <SystemTypes.h>
#include <CoreModule.h>
#include <EventTypes.h>
#include <CameraTypes.h>
#include <VideoTypes.h>
#include <DebugMPI.h>
#include <KernelMPI.h>
#include <EventMPI.h>
#include <MicrophoneMPI.h>
#include <MicrophoneTypes.h>
#include <EventListener.h>
#include <KernelTypes.h>
#include <UsbHost.h>
#include <queue>

/*
 * This is a workaround for the old (broken) headers installed on lightning-release
 * and the stale nfsroot on emplhwbuild01.leapfrog.com.
 */
#ifdef __STRICT_ANSI__
#undef __STRICT_ANSI__
#define FOO__STRICT_ANSI__
#endif
#include <asm/types.h>
#ifdef FOO__STRICT_ANSI__
#define __STRICT_ANSI__
#undef FOO__STRICT_ANSI__
#endif

#include <linux/videodev2.h>
#include <linux/fb.h>
#include <jpeglib.h>

#include <alsa/asoundlib.h>
#include <AVIWrapper.h>
#include <sndfile.h>

#include <linux/fb.h>
#if !defined(EMULATION)
#include <vmem.h>
#endif

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Constants
//==============================================================================
const CString			kUSBCameraModuleName	= "CameraUSB";
const tVersion			kUSBCameraModuleVersion	= 2;
const CString			kVIPCameraModuleName	= "CameraVIP";
const tVersion			kVIPCameraModuleVersion	= 2;
const CString			kEmulCameraModuleName	= "CameraEmul";
const tVersion			kEmulCameraModuleVersion	= 2;
const CString			kNXPCameraModuleName	= "CameraNXP";
const tVersion			kNXPCameraModuleVersion	= 2;
const tEventPriority	kCameraEventPriority	= 0;
const tDebugLevel		kCameraDebugLevel		= kDbgLvlVerbose;

const tVidCapHndl		kStreamingActive		= 0x80000000;
const tVidCapHndl		kStreamingThread		= 0x40000000;
const tVidCapHndl		kStreamingFrame			= 0x20000000;	// frame-by-frame, i.e., no thread

const U64	MIN_FREE	= 0x100000ULL;		/* 1MB. NOTE: UBIFS is internally padded,
											 * so this isn't strictly needed */

#define IS_STREAMING_HANDLE(x) \
	(((x) & kStreamingActive) && (camCtx_.hndl == x))

#define IS_THREAD_HANDLE(x) \
	((x) & kStreamingThread)

#define IS_FRAME_HANDLE(x) \
	((x) & kStreamingFrame)

#define STREAMING_HANDLE(x) \
	((x) | kStreamingActive)

#define THREAD_HANDLE(x) \
	((x) | kStreamingThread)

#define FRAME_HANDLE(x) \
	((x) | kStreamingFrame)

//==============================================================================
// Typedefs
//==============================================================================

// How to render JPEGs
typedef enum {
	JPEG_SLOW	= JDCT_ISLOW,		/* sw IDCT & jpeg_read_scanlines() - best for photos */
	JPEG_FAST	= JDCT_IFAST,		/* sw IDCT & jpeg_read_scanlines() */
	JPEG_FLOAT	= JDCT_FLOAT,		/* sw IDCT & jpeg_read_scanlines() */
#if !defined(EMULATION)
	JPEG_HW1	= JDCT_HW,			/* hw IDCT & jpeg_read_scanlines() */
	JPEG_HW2						/* hw IDCT & jpeg_read_coefficients() - best for viewfinder */
#endif
} JPEG_METHOD;

// Frame info
struct tFrameInfo {
	tCaptureFormat	pixelformat;
	U16				width;
	U16				height;
	U32				index;
	void *			data;
	U32				size;
	struct timeval	timestamp;
};

enum tBitmapFormat {
	kBitmapFormatError = 0,
	kBitmapFormatGrayscale8,
	kBitmapFormatRGB888,
	kBitmapFormatYCbCr888,
};

// Bitmap image (processed frame) info
struct tBitmapInfo {
	tBitmapFormat	format;
	U16				width;
	U16				height;
	U16				depth;
	U8 *			data;
	U32				size;
	JSAMPARRAY		buffer;		//row pointers
};

struct tCameraContext {
	const char					*file;		// e.g., "/dev/video0"
	int							fd;			// file descriptor

	tVidCapHndl					hndl;

	struct v4l2_format 			fmt;		// to track the currently selected format
	double						fps;

	struct v4l2_capability		cap;
	tCaptureModes				*modes;
	tCaptureMode				mode;

	tCameraControls				*controls;

	struct v4l2_buffer			buf;
	U32							numBufs;
	void						**bufs;

	CPath						path;
	tVideoSurf					*surf;

	tTaskHndl					hCameraThread;

	Boolean						bPaused;
	Boolean						bVPaused;
	tMutex						mThread;
	tMutex						mThread2;
	U32							reqLength;		// length in seconds requested by app
	U32							maxLength;		// length in seconds available on FS

	class CCameraModule			*module;
	IEventListener				*pListener;

	Boolean						bAudio;			// capture audio option

	std::queue<tFrameInfo>		qframes;		// queue for CameraTaskRender() thread
	tFrameInfo					*frame;
	tBitmapInfo					*image;
	JPEG_METHOD					method;
	tVideoSurf					*paSurf;
	tDisplayHandle				*paHndl;
	bool						bDoubleBuffered;

	CDebugMPI					*dbg;
	CKernelMPI					*kernel;

	struct fb_fix_screeninfo 	*fi;
	struct fb_var_screeninfo 	*vi;
};

struct tCaptureContext {
	tVideoSurf*			pSurf;
};

struct tMicrophoneContext {
	snd_pcm_t *				pcm_handle;
	snd_async_handler_t *	ahandler;
	snd_pcm_hw_params_t	*	hwparams;
	snd_pcm_sw_params_t *	swparams;

	int						fd[2];		/* pipe for ALSA callback->file output*/
	unsigned short *		poll_buf;	/* to transfer from pipe to file */

	unsigned int			rate;		/* sampling rate from alsa-lib, not HW */
	unsigned int			channels;	/* sampling rate from alsa-lib, not HW */
	snd_pcm_format_t		format;
	int						sbits;		/* sample width (significant bits) */

	tAudCapHndl				hndl;

	tTaskHndl				hMicThread;

	CPath					path;
	Boolean					bPaused;
	U32						reqLength;		// length in seconds requested by app
	U32						maxLength;		// length in seconds available on FS

	IEventListener			*pListener;

	tMutex					dlock;

	unsigned int			period_time;	// period time in usec for callbacks
	snd_pcm_uframes_t		period_size;

	unsigned int			bytesRead;		// bytes read per callback
	unsigned int			bytesWritten;	// bytes written total
	unsigned int			counter;		// block counter
	unsigned int			block_size;		// block size in bytes = period_size * 2

	snd_pcm_status_t* 		status;
	snd_timestamp_t 		tstamp;

	S32						threshold;		// threshold for trigger event
	S32						duration;		// duration for trigger event
	S32						clipCount;		// clip count for trigger event
	S32						rateAdjust;		// sample rate adjustment

	CDebugMPI				*dbg;
};

struct tIDCTContext {
	const char					*file;		// e.g., "/dev/idct"
	int							fd;			// file descriptor
	void						*reg;		// mmaped area
	volatile U32				*reg32;		// register access
};

//==============================================================================
// External function prototypes
//==============================================================================
tErrType InitCameraTask(tCameraContext* pCtx);
tErrType DeInitCameraTask(tCameraContext* pCtx);

//==============================================================================
class CCameraModule : public ICoreModule {

	// Internal listener for converting mic events to cam events.
	class MicrophoneListener : public IEventListener
	{
		private:
			tEventType	reason;
			IEventListener *destListener;
			CEventMPI *pEvent;

		public:
			MicrophoneListener(IEventListener *dest, CEventMPI *event);
			tEventStatus Notify(const IEventMessage& Imsg);
			tEventStatus Reset();
		};


public:
	// ICoreModule functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const	=0;
	virtual const CString*	GetModuleName() const		=0;
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	VTABLE_EXPORT Boolean		IsCameraPresent();
	VTABLE_EXPORT tErrType		SetCameraVideoPath(const CPath& path);
	VTABLE_EXPORT CPath*		GetCameraVideoPath();
	VTABLE_EXPORT tErrType		SetCameraStillPath(const CPath& path);
	VTABLE_EXPORT CPath*		GetCameraStillPath();
	VTABLE_EXPORT tErrType		SetCameraAudioPath(const CPath& path);
	VTABLE_EXPORT CPath*		GetCameraAudioPath();
	VTABLE_EXPORT Boolean		GetCameraControls(tCameraControls &controls);
	VTABLE_EXPORT Boolean		SetCameraControl(const tControlInfo* control, const S32 value);
	VTABLE_EXPORT tVidCapHndl	StartVideoCapture(const CPath& path, tVideoSurf* pSurf,\
													IEventListener * pListener, const U32 maxLength, const Boolean audio);
	VTABLE_EXPORT Boolean		SnapFrame(const tVidCapHndl hndl, const CPath& path);
	VTABLE_EXPORT Boolean		GetFrame(const tVidCapHndl hndl, U8* pixels, tColorOrder color_order);
	VTABLE_EXPORT Boolean		GetFrame(const tVidCapHndl hndl, tVideoSurf *pSurf, tColorOrder color_order);
	VTABLE_EXPORT Boolean		RenderFrame(const CPath &path, tVideoSurf *pSurf);
	VTABLE_EXPORT Boolean		PauseVideoCapture(const tVidCapHndl hndl, const Boolean display);
	VTABLE_EXPORT Boolean		ResumeVideoCapture(const tVidCapHndl hndl);
	VTABLE_EXPORT Boolean		IsVideoCapturePaused(const tVidCapHndl hndl);
	VTABLE_EXPORT Boolean		StopVideoCapture(const tVidCapHndl hndl);

	VTABLE_EXPORT tAudCapHndl	StartAudioCapture(const CPath& path, IEventListener * pListener,\
								                    const U32 maxLength, const Boolean paused);
	VTABLE_EXPORT Boolean		PauseAudioCapture(const tAudCapHndl hndl);
	VTABLE_EXPORT Boolean		ResumeAudioCapture(const tAudCapHndl hndl);
	VTABLE_EXPORT Boolean		IsAudioCapturePaused(const tAudCapHndl hndl);
	VTABLE_EXPORT Boolean		StopAudioCapture(const tAudCapHndl hndl);

	VTABLE_EXPORT Boolean		SetMicrophoneParam(enum tMicrophoneParam param, S32 value);
	VTABLE_EXPORT S32			GetMicrophoneParam(enum tMicrophoneParam param);

	VTABLE_EXPORT tErrType		EnumFormats(tCaptureModes& pModeList);
	VTABLE_EXPORT tErrType		SetCurrentFormat(tCaptureMode* pMode);
	VTABLE_EXPORT tCaptureMode*	GetCurrentFormat();

	VTABLE_EXPORT tErrType 		SetCurrentCamera(tCameraDevice device);
	VTABLE_EXPORT tCameraDevice GetCurrentCamera();

	// surface access and locking
	VTABLE_EXPORT tVideoSurf* 	GetCaptureVideoSurface(const tVidCapHndl hndl);
	VTABLE_EXPORT Boolean 		LockCaptureVideoSurface(const tVidCapHndl hndl);
	VTABLE_EXPORT Boolean 		UnLockCaptureVideoSurface(const tVidCapHndl hndl);

private:
	MicrophoneListener	*micListener_;
	CDebugMPI			dbg_;
	CKernelMPI			kernel_;
	CEventMPI			event_;
	CMicrophoneMPI		*microphone_;
	tCameraContext		camCtx_;
	tMicrophoneContext	micCtx_;
	tIDCTContext		idctCtx_;

	Boolean				valid;
	tMutex				mutex_;
	tCameraDevice		device_;
	boost::shared_ptr<CUsbHost> usbHost_;

	CPath				vpath;
	CPath				spath;
	tMutex				dlock;

	CCameraModule();
	virtual ~CCameraModule();
	friend void* CameraTaskMain(void* arg);
	friend void* CameraTaskRender(void* arg);

	// Implementation-specific functionality
	virtual Boolean		InitCameraInt(const tCaptureMode* mode, bool reinit = false);
	virtual Boolean		DeinitCameraInt(bool reinit = false);
	Boolean		GetCameraModes(tCaptureModes &modes);
	virtual Boolean		SetCameraMode(const tCaptureMode* mode);
	virtual Boolean		SetBuffers(const U32 numBuffers);
	virtual Boolean		PollFrame(const tVidCapHndl hndl);
	virtual Boolean		GetFrame(const tVidCapHndl hndl, tFrameInfo *frame);
	Boolean		RenderFrame(tFrameInfo *frame, tVideoSurf *pSurf, tBitmapInfo *image, const JPEG_METHOD method);
	Boolean		RenderFrame(tFrameInfo &frame, tVideoSurf *pSurf, tColorOrder color_order);
	virtual Boolean		ReturnFrame(const tVidCapHndl hndl, const tFrameInfo *frame);
	virtual Boolean		GrabFrame(const tVidCapHndl hndl, tFrameInfo *frame);
	Boolean		SaveFrame(const CPath &path, const tFrameInfo *frame);
	Boolean		OpenFrame(const CPath &path, tFrameInfo *frame);
	Boolean		SnapFrameRGB(const tVidCapHndl hndl, const CPath &path);
	Boolean		SnapFrameJPG(const tVidCapHndl hndl, const CPath &path);
	Boolean		CompressFrame(tFrameInfo *frame, int stride);

	virtual Boolean 	InitCameraBufferInt(tCameraContext *pCamCtx);
	virtual Boolean 	DeinitCameraBufferInt(tCameraContext *pCamCtx);
	virtual Boolean 	InitCameraStartInt(tCameraContext *pCamCtx);
	virtual Boolean 	StopVideoCaptureInt(int fd);

#if 0
	tErrType	InitMicInt();
	tErrType	DeinitMicInt();
	int			XlateAudioFormatAVI(snd_pcm_format_t fmt);
	int			XlateAudioFormatSF(snd_pcm_format_t fmt);
	Boolean		StartAudio(Boolean reset = true);
	Boolean		WriteAudio(avi_t *avi);
	Boolean		WriteAudio(SNDFILE *wav);
	Boolean		FlushAudio();
	Boolean		StopAudio();

	friend void* MicTaskMain(void* arg);
	friend tErrType InitMicTask(CCameraModule* module);
	friend tErrType DeInitMicTask(CCameraModule* module);
#endif
	void		InitLut();
	tErrType	InitIDCTInt();
	tErrType	DeinitIDCTInt();
	tErrType	StartIDCT(S16 *ptr);
	tErrType	RetrieveIDCT(S16 *ptr);
	/* inline function */
	void DecompressAndPaint(struct jpeg_decompress_struct *cinfo, tVideoSurf *surf);
	void SetScaler(int width, int height, bool centered);

	friend class CUSBCameraModule;
	friend class CVIPCameraModule;
	friend class CEmulCameraModule;
	friend class CNXPCameraModule;
};

//==============================================================================
// USB-specific functionality
class CUSBCameraModule : public CCameraModule {
	// internal listener for hotplug events
	class CameraListener : public IEventListener
	{
		CUSBCameraModule*	pMod;
		Boolean			running;
	public:
		CameraListener(CUSBCameraModule* ctx);
		~CameraListener();
		tEventStatus Notify(const IEventMessage& msg);
	};
public:
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;

private:
	CPath				sysfs;		// e.g., "/sys/class/usb_device/usbdev1.2/"
	CPath				devname;
	CPath				devpath;
	CameraListener		*listener_;
	friend Boolean EnumCameraCallback(const CPath& path, void* pctx);
	friend Boolean EnumVideoCallback(const CPath& path, void* pctx);

	// Limit object creation to the Module Manager interface functions
	CUSBCameraModule();
	virtual ~CUSBCameraModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));
};

//==============================================================================
// LF2000 VIP-specific functionality
class CVIPCameraModule : public CCameraModule {
public:
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;

	VTABLE_EXPORT tVidCapHndl	StartVideoCapture(const CPath& path, tVideoSurf* pSurf,\
													IEventListener * pListener, const U32 maxLength, const Boolean audio);
	VTABLE_EXPORT Boolean		StopVideoCapture(const tVidCapHndl hndl);
	VTABLE_EXPORT Boolean		SnapFrame(const tVidCapHndl hndl, const CPath &path);
	VTABLE_EXPORT Boolean		GetFrame(const tVidCapHndl hndl, U8* pixels, tColorOrder color_order);
	VTABLE_EXPORT Boolean		GetFrame(const tVidCapHndl hndl, tVideoSurf *pSurf, tColorOrder color_order);
	VTABLE_EXPORT Boolean		PauseVideoCapture(const tVidCapHndl hndl, const Boolean display);
	VTABLE_EXPORT Boolean		ResumeVideoCapture(const tVidCapHndl hndl);
	VTABLE_EXPORT tErrType		EnumFormats(tCaptureModes& pModeList);

private:
	tVideoSurf	overlaySurf;
	Boolean		overlayEnabled;
	tCameraControls	*controlsCached;

	struct fb_fix_screeninfo 	fi;
	struct fb_var_screeninfo 	vi;
	int							fd;
	int							fdvmem;
#if !defined(EMULATION)
	VM_IMEMORY 					vm;
#endif

	void		AllocVMem(tCameraContext& camCtx);
	void		FreeVMem();
	Boolean 	EnableOverlay(int fd, int enable);

	// Limit object creation to the Module Manager interface functions
	CVIPCameraModule();
	virtual ~CVIPCameraModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));
	friend class CNXPCameraModule;
};


//==============================================================================
// LF3000 NXP-specific functionality
class CNXPCameraModule : public CCameraModule {

public:
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;

	VTABLE_EXPORT tVidCapHndl	StartVideoCapture(const CPath& path, tVideoSurf* pSurf,\
													IEventListener * pListener, const U32 maxLength, const Boolean audio);
	VTABLE_EXPORT Boolean		StopVideoCapture(const tVidCapHndl hndl);
	VTABLE_EXPORT Boolean		SnapFrame(const tVidCapHndl hndl, const CPath &path);
	VTABLE_EXPORT Boolean		GetFrame(const tVidCapHndl hndl, U8* pixels, tColorOrder color_order);
	VTABLE_EXPORT Boolean		GetFrame(const tVidCapHndl hndl, tVideoSurf *pSurf, tColorOrder color_order);
	VTABLE_EXPORT Boolean		PauseVideoCapture(const tVidCapHndl hndl, const Boolean display);
	VTABLE_EXPORT Boolean		ResumeVideoCapture(const tVidCapHndl hndl);
	VTABLE_EXPORT tErrType		EnumFormats(tCaptureModes& pModeList);
	VTABLE_EXPORT tErrType		SetCurrentFormat(tCaptureMode* pMode);
	VTABLE_EXPORT tErrType 		SetCurrentCamera(tCameraDevice device);
	VTABLE_EXPORT Boolean		SetCameraControl(const tControlInfo* control, const S32 value);
	VTABLE_EXPORT tVideoSurf* 	GetCaptureVideoSurface(const tVidCapHndl hndl);

private:
	void*						nxphndl_;
	void*						nxpvbuf_[3];
	void*						nxpmbuf_[3];
	int  						index_;
	int							outcnt_;
	int  						clipper_;
	int  						sensor_;
	bool 						overlay_;

	// Limit object creation to the Module Manager interface functions
	CNXPCameraModule();
	virtual ~CNXPCameraModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));

	// Implementation-specific functionality
	Boolean		InitCameraInt(const tCaptureMode* mode, bool reinit = false);
	Boolean		DeinitCameraInt(bool reinit = false);
	Boolean		SetCameraMode(const tCaptureMode* mode);
	Boolean		SetBuffers(const U32 numBuffers);
	Boolean		PollFrame(const tVidCapHndl hndl);
	Boolean		GetFrame(const tVidCapHndl hndl, tFrameInfo *frame);
	Boolean		ReturnFrame(const tVidCapHndl hndl, const tFrameInfo *frame);
	Boolean		GrabFrame(const tVidCapHndl hndl, tFrameInfo *frame);

	Boolean 	InitCameraBufferInt(tCameraContext *pCamCtx);
	Boolean 	DeinitCameraBufferInt(tCameraContext *pCamCtx);
	Boolean 	InitCameraStartInt(tCameraContext *pCamCtx);
	Boolean 	StopVideoCaptureInt(int fd);

};

//==============================================================================
// Emulation-specific functionality
class CEmulCameraModule : public CCameraModule {

public:
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;

private:

	// Limit object creation to the Module Manager interface functions
	CEmulCameraModule();
	virtual ~CEmulCameraModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));

	VTABLE_EXPORT tVidCapHndl	StartVideoCapture(const CPath& path, tVideoSurf* pSurf,\
													IEventListener * pListener, const U32 maxLength, const Boolean audio);

	tVideoSurf	videoSurface_;
};

LF_END_BRIO_NAMESPACE()
#endif // LF_BRIO_CAMERAPRIV_H

// EOF
