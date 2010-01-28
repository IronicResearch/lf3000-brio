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

#include <linux/videodev2.h>

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
	Boolean						audio;
	tVideoSurf					*surf;
	tRect						*rect;

	tTaskHndl					hCameraThread;

	Boolean						bPaused;
	Boolean						bStreaming;
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
	VTABLE_EXPORT tErrType		SetCameraResourcePath(const CPath& path);
	VTABLE_EXPORT CPath*		GetCameraResourcePath();
	VTABLE_EXPORT Boolean		GetCameraModes(tCaptureModes &modes);
	VTABLE_EXPORT Boolean		SetCameraMode(const tCaptureMode* mode);
	VTABLE_EXPORT Boolean		GetCameraControls(tCameraControls &controls);
	VTABLE_EXPORT Boolean		SetCameraControl(const tControlInfo* control, const S32 value);
	VTABLE_EXPORT Boolean		SetBuffers(const U32 numBuffers);
	VTABLE_EXPORT tVidCapHndl	StartVideoCapture();
	VTABLE_EXPORT Boolean		PollFrame(const tVidCapHndl hndl);
	VTABLE_EXPORT Boolean		GetFrame(const tVidCapHndl hndl, tFrameInfo *frame);
	VTABLE_EXPORT Boolean		RenderFrame(tFrameInfo *frame, tVideoSurf *pSurf, tBitmapInfo *image);
	VTABLE_EXPORT Boolean		ReturnFrame(const tVidCapHndl hndl, const tFrameInfo *frame);
	VTABLE_EXPORT tVidCapHndl	StartVideoCapture(const CPath& path, Boolean audio, tVideoSurf* pSurf, tRect* rect);
	VTABLE_EXPORT Boolean		GrabFrame(const tVidCapHndl hndl, tFrameInfo *frame);
	VTABLE_EXPORT Boolean		PauseVideoCapture(const tVidCapHndl hndl);
	VTABLE_EXPORT Boolean		ResumeVideoCapture(const tVidCapHndl hndl);
	VTABLE_EXPORT Boolean		IsCapturePaused(const tVidCapHndl hndl);
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

	// Implementation-specific functionality
	Boolean				InitCameraInt();
	Boolean				DeinitCameraInt();
};

LF_END_BRIO_NAMESPACE()
#endif // LF_BRIO_CAMERAPRIV_H

// EOF
