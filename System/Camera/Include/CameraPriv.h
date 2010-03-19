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
#include <AudioTypes.h>
#include <EventTypes.h>
#include <CameraTypes.h>
#include <VideoTypes.h>
#include <DebugMPI.h>
#include <KernelMPI.h>
#include <EventListener.h>
#include <KernelTypes.h>

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

#define IS_STREAMING_HANDLE(x) \
	((x) & kStreamingActive)

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
	Boolean						bStreaming;
	tMutex						mThread;

	class CCameraModule			*module;
};

struct tCaptureContext {
	tVideoSurf*			pSurf;
};

//==============================================================================
// External function prototypes
//==============================================================================
tErrType InitCameraTask(tCameraContext* pCtx);
tErrType DeInitCameraTask(tCameraContext* pCtx);

//==============================================================================
class CCameraModule : public ICoreModule {
public:
	// ICoreModule functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	VTABLE_EXPORT tErrType		SetCameraVideoPath(const CPath& path);
	VTABLE_EXPORT CPath*		GetCameraVideoPath();
	VTABLE_EXPORT tErrType		SetCameraStillPath(const CPath& path);
	VTABLE_EXPORT CPath*		GetCameraStillPath();
	VTABLE_EXPORT Boolean		GetCameraControls(tCameraControls &controls);
	VTABLE_EXPORT Boolean		SetCameraControl(const tControlInfo* control, const S32 value);
	VTABLE_EXPORT tVidCapHndl	StartVideoCapture(const CPath& path, tVideoSurf* pSurf);
	VTABLE_EXPORT Boolean		SnapFrame(const tVidCapHndl hndl, const CPath& path);
	VTABLE_EXPORT Boolean		RenderFrame(const CPath &path, tVideoSurf *pSurf);
	VTABLE_EXPORT Boolean		PauseVideoCapture(const tVidCapHndl hndl);
	VTABLE_EXPORT Boolean		ResumeVideoCapture(const tVidCapHndl hndl);
	VTABLE_EXPORT Boolean		IsVideoCapturePaused(const tVidCapHndl hndl);
	VTABLE_EXPORT Boolean		StopVideoCapture(const tVidCapHndl hndl);

private:
	CDebugMPI			dbg_;
	CKernelMPI			kernel_;
	tCameraContext		camCtx_;
	Boolean				valid;
	tMutex				mutex_;

	// Limit object creation to the Module Manager interface functions
	CCameraModule();
	virtual ~CCameraModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));
	friend void* CameraTaskMain(void* arg);

	// Implementation-specific functionality
	Boolean		InitCameraInt();
	Boolean		DeinitCameraInt();
	Boolean		GetCameraModes(tCaptureModes &modes);
	Boolean		SetCameraMode(const tCaptureMode* mode);
	Boolean		SetBuffers(const U32 numBuffers);
	tVidCapHndl	StartVideoCapture();
	Boolean		PollFrame(const tVidCapHndl hndl);
	Boolean		GetFrame(const tVidCapHndl hndl, tFrameInfo *frame);
	Boolean		RenderFrame(tFrameInfo *frame, tVideoSurf *pSurf, tBitmapInfo *image);
	Boolean		ReturnFrame(const tVidCapHndl hndl, const tFrameInfo *frame);
	Boolean		GrabFrame(const tVidCapHndl hndl, tFrameInfo *frame);
	Boolean		SaveFrame(const CPath &path, const tFrameInfo *frame);
	Boolean		OpenFrame(const CPath &path, tFrameInfo *frame);
};

LF_END_BRIO_NAMESPACE()
#endif // LF_BRIO_CAMERAPRIV_H

// EOF
