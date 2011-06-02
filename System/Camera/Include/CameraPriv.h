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
#include <EventListener.h>
#include <KernelTypes.h>
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
#include <jpeglib.h>

#include <alsa/asoundlib.h>
#include <AVIWrapper.h>
#include <sndfile.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Constants
//==============================================================================
const CString			kCameraModuleName		= "Camera";
const tVersion			kCameraModuleVersion	= 2;
const tEventPriority	kCameraEventPriority	= 0;
const tDebugLevel		kCameraDebugLevel		= kDbgLvlImportant;

const tVidCapHndl		kStreamingActive		= 0x80000000;
const tVidCapHndl		kStreamingThread		= 0x40000000;
const tVidCapHndl		kStreamingFrame			= 0x20000000;	// frame-by-frame, i.e., no thread

const U64	MIN_FREE	= 1*1024*1024;		/* NOTE: UBIFS is internally padded,
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
	JPEG_HW1	= JDCT_HW,			/* hw IDCT & jpeg_read_scanlines() */
	JPEG_HW2						/* hw IDCT & jpeg_read_coefficients() - best for viewfinder */
} JPEG_METHOD;

// Image capture format.  Uncompressed formats are possible - these would be equivalent to
// DisplayTypes:tPixelFormat.  Since JPEG is compressed, it's not a pixel format in the
// proper sense
enum tCaptureFormat {
	kCaptureFormatError = 0,
	kCaptureFormatMJPEG,
	kCaptureFormatRAWYUYV,
};

// Three components: image format, image resolution, and video frame rate, determine
// the camera's capture mode
struct tCaptureMode {
	tCaptureFormat	pixelformat;
	U16				width;
	U16				height;
	U32				fps_numerator;
	U32				fps_denominator;
};

typedef std::vector<tCaptureMode *> tCaptureModes;

// Frame info
struct tFrameInfo {
	tCaptureFormat	pixelformat;
	U16				width;
	U16				height;
	U32				index;
	void *			data;
	U32				size;
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
	// internal listener for hotplug events
	class CameraListener : public IEventListener
	{
		CCameraModule*	pMod;
		Boolean			running;
	public:
		CameraListener(CCameraModule* ctx);
		~CameraListener();
		tEventStatus Notify(const IEventMessage& msg);
	};
public:
	// ICoreModule functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;
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

private:
	CPath				sysfs;		// e.g., "/sys/class/usb_device/usbdev1.2/"
	CameraListener		*listener_;
	CDebugMPI			dbg_;
	CKernelMPI			kernel_;
	CEventMPI			event_;
	tCameraContext		camCtx_;
	tMicrophoneContext	micCtx_;
	tIDCTContext		idctCtx_;

	Boolean				valid;
	tMutex				mutex_;

	// Limit object creation to the Module Manager interface functions
	CCameraModule();
	virtual ~CCameraModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));
	friend void* CameraTaskMain(void* arg);
	friend void* CameraTaskRender(void* arg);
	friend Boolean EnumCameraCallback(const CPath& path, void* pctx);

	// Implementation-specific functionality
	Boolean		InitCameraInt();
	Boolean		DeinitCameraInt();
	Boolean		GetCameraModes(tCaptureModes &modes);
	Boolean		SetCameraMode(const tCaptureMode* mode);
	Boolean		SetBuffers(const U32 numBuffers);
	tVidCapHndl	StartVideoCapture();
	Boolean		PollFrame(const tVidCapHndl hndl);
	Boolean		GetFrame(const tVidCapHndl hndl, tFrameInfo *frame);
	Boolean		RenderFrame(tFrameInfo *frame, tVideoSurf *pSurf, tBitmapInfo *image, const JPEG_METHOD method);
	Boolean		ReturnFrame(const tVidCapHndl hndl, const tFrameInfo *frame);
	Boolean		GrabFrame(const tVidCapHndl hndl, tFrameInfo *frame);
	Boolean		SaveFrame(const CPath &path, const tFrameInfo *frame);
	Boolean		OpenFrame(const CPath &path, tFrameInfo *frame);
	Boolean		SnapFrameRGB(const tVidCapHndl hndl, const CPath &path);

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

	void		InitLut();
	tErrType	InitIDCTInt();
	tErrType	DeinitIDCTInt();
	tErrType	StartIDCT(S16 *ptr);
	tErrType	RetrieveIDCT(S16 *ptr);
	/* inline function */
	void DecompressAndPaint(struct jpeg_decompress_struct *cinfo, tVideoSurf *surf);

};

LF_END_BRIO_NAMESPACE()
#endif // LF_BRIO_CAMERAPRIV_H

// EOF
