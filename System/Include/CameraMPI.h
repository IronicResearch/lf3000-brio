#ifndef LF_BRIO_CAMERAMPI_H
#define LF_BRIO_CAMERAMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		CameraMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the Camera module.
//
//==============================================================================

#include <SystemTypes.h>
#include <CameraTypes.h>
#include <VideoTypes.h>
#include <CoreMPI.h>
#include <EventListener.h>

LF_BEGIN_BRIO_NAMESPACE()

/// \class CCameraMPI
///
/// The Brio Camera MPI is the interface for camera access.  It consists of low-level
/// functions for manipulating the camera hardware, and higher-level functions for acquiring
/// images (video) from the camera. The video capture functions are divided into low-level
/// functions for frame-by-frame capture, and high-level functions for capturing video
/// automatically in a separate task thread. The current Camera MPI implementation supports
/// videos in the AVI format with the MJPEG video codec and PCM audio (TBD).
///
/// The Camera MPI is related to the Video MPI in that captured video is typically shown on a
/// display, as in a typical camera viewfinder. The displaying of a captured stream is
/// essentially video playback, with the video frames originating from a camera as opposed
/// to a file.
///
/// The low-level Camera MPI functions manage captures on an individual frame basis. A camera
/// resource is initialized by SetBuffers(), which reserves memory for frames to be exchanged
/// between the Camera MPI and the application. Video capturing is initiated by
/// StartVideoCapture(). Each captured frame is indexed by GetCaptureFrame(). The captured
/// frame is processed by the application (rendered to a display, written to a file, etc.),
/// and then returned to the frame buffer pool with PutCaptureFrame(). As long as this
/// GetCaptureFrame()-PutCaptureFrame() sequences continues, frames will be streamed to the
/// device. Capturing is halted with StopVideoCapture().
///
/// High-level Camera MPI functions are provided as counterpart to the Video and Audio MPIs.
/// This variation of the StartVideoCapture() MPI method will start a separate task thread to
/// capture the video automatically, performing the all the low-level frame-based tasks
/// internally. Capture can be paused with PauseVideoCapture(), and resumed with ResumeVideoCapture(),
/// or stopped by StopVideoCapture() which will terminate the task thread. Capture state info can be
/// queried by the IsCapturePaused() method.
///
/// General camera capability info such as supported resolutions, formats, and framerates can
/// be obtained with the GetCameraInfo() method. Camera controls, such as contrast, brightness,
/// etc., can be obtained with the GetCameraControls() method.
///

//==============================================================================
class CCameraMPI : public ICoreMPI {
public:
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	CCameraMPI();
	virtual ~CCameraMPI();

	// MPI-specific functionality

	/// Sets the default resource path for subsequent StartVideoCapture() calls
	///
	/// \param path	Base path for files referenced by calls to StartVideoCapture()
	///
	/// \return Returns kNoErr on success.
	tErrType	SetCameraResourcePath(const CPath& path);

	/// Returns the base path set by SetCameraResourcePath().
	CPath*		GetCameraResourcePath();

	/// Queries camera for supported resolution, format, and framerate.
	/// <pre>
	/// The existing camera widget supports these formats:
	/// 	-MJPG
	///
	/// The existing camera widget supports these resolutions:
	/// 	-VGA	(640x480)
	/// 	-QVGA	(320x240)
	/// 	-QQVGA	(160x120)
	/// 	-QCIF	(176x144)
	/// 	-CIF	(352x288)
	///
	/// The existing camera widget supports these framerates:
	/// 	-30	fps
	/// 	-15	fps
	/// 	-1	fps
	/// </pre>

	/// \param	modes		Modes(s) supported by camera (e.g., VGA,JPEG,30fps).
	///
	/// \return true on success.
	Boolean		GetCameraModes(tCaptureModes &modes);

	/// Requests camera to apply the specified resolution, format, framerate, etc.
	///
	/// \param	mode		Mode to activate
	///
	/// \return true on success.
	Boolean		SetCameraMode(const tCaptureMode* mode);

	/// Queries camera for application controls, such as contrast, brightness, etc.
	/// <pre>
	/// The existing camera widget supports these controls:
	///	-Brightness			(0 to 255; 138 default)
	///	-Contrast			(0 to 50; 5 default)
	///	-Saturation			(0 to 200; 195 default)
	///	-Hue				(-180 to 180; 0 default)
	///	-Brightness			(0 to 255; 138 default)
	///	-Gamma				(20 to 100; 75 default)
	///	-Backlight Compensation		(0 to 5, 0 default)
	///	-Power Line Frequency		(Disabled, 50 Hz, 60 Hz; 60 Hz default)
	///	-Sharpness			(0 to 10, 6 default)
	/// </pre>

	/// \param	controls	Control(s) supported by camera.
	///
	/// \return true on success.
	Boolean		GetCameraControls(tCameraControls &controls);

	/// Requests camera to apply the specified resolution, format, framerate, etc.
	///
	/// \return true on success.
	Boolean		SetCameraControls(const tControlInfo* control);

	/// Allocates buffers for passing between the application and camera.  Must be called
	/// before StartVideoCapture()
	///
	/// \param	numBuffers		Number of buffers to allocate.  Higher framerates require
	/// more buffers to prevent dropped frames.
	///
	/// \return true on success.
	Boolean		SetBuffers(const U32 numBuffers);

	/// StartVideoCapture() variation which begins capture for subsequent low-level calls.
	///
	/// \return Returns kInvalidVidCapHndl on failure.
	tVidCapHndl	StartVideoCapture();

	/// PollFrame() check if new frame is available from capture stream.
	///
	/// \param	hndl	Video capture handle returned by StartVideoCapture()
	///
	/// \return Returns true if a frame is ready.
	Boolean		PollFrame(const tVidCapHndl hndl);

	/// GetFrame() retrieve next frame from capture stream. Also used to retrieve a snapshot
	/// from stream being captured in a separate thread via
	/// StartVideoCapture(const CPath&, const Boolean, tVideoSurf*, tRect *).
	/// This snapshot does not have to be returned with PutFrame().
	///
	/// \param	hndl	Video capture handle returned by StartVideoCapture()
	///
	/// \param frame	Image data returned from camera
	///
	/// \return Returns true on success.
	Boolean		GetFrame(const tVidCapHndl hndl, tFrameInfo *frame);

	/// PutFrame() mark retrieved frame as processed and return it to the pool.
	///
	/// \param	hndl	Video capture handle returned by StartVideoCapture()
	///
	/// \param frame	Frame returned by GetFrame()
	///
	/// \return Returns true on success.
	Boolean		PutFrame(const tVidCapHndl hndl, const tFrameInfo *frame);


	/// StartVideoCapture() variation which spawns a thread for capture.
	///
	/// \param	path	Video save file name relative to SetCameraResourcePath(),
	/// or full absolute path name if leading slash.  NULL parameter means
	/// don't save the capture to a file.
	///
	/// \param	audio	Flag to determine if audio should be captured along with
	/// the video stream.  false means don't capture audio.  If audio is true,
	/// all data will be saved in an audio-video interleaved (AVI) file.
	///
	/// \param	pSurf	Pointer to video surface descriptor for rendering.
	/// NULL parameter means don't render the capture.
	///
	/// \param	rect	Display rectangle to be re-drawn (should correspond to
	/// the CDisplayMPI used to initialize pSurf)
	tVidCapHndl	StartVideoCapture(const CPath& path, const Boolean audio, tVideoSurf* pSurf, tRect *rect);

	/// PauseVideoCapture() pause an active capture.
	///
	/// \param	hndl	Video capture handle returned by
	/// StartVideoCapture(const CPath&, const Boolean, tVideoSurf*, tRect *)
	///
	/// \return Returns true if successful.
	Boolean		PauseVideoCapture(const tVidCapHndl hndl);

	/// ResumeVideoCapture() a paused capture.
	///
	/// \param	hndl	Video capture handle returned by
	/// StartVideoCapture(const CPath&, const Boolean, tVideoSurf*, tRect *)
	///
	/// \return Returns true if successful.
	Boolean		ResumeVideoCapture(const tVidCapHndl hndl);

	/// IsCapturePaused() checks if the specified capture is paused.
	///
	/// StartVideoCapture(const CPath&, const Boolean, tVideoSurf*, tRect *)
	/// \param	hndl	Video capture handle returned by
	/// \return Returns true if capture is currently paused.
	Boolean		IsCapturePaused(const tVidCapHndl hndl);

	/// StopVideoCapture() stop an active capture.
	///
	/// \param	hndl	Video capture handle returned by StartVideoCapture()
	///
	/// \return Returns true if successful.
	Boolean		StopVideoCapture(const tVidCapHndl hndl);

private:
	class CCameraModule*	pModule_;
	U32					id_;
};

LF_END_BRIO_NAMESPACE()
#endif // LF_BRIO_CAMERAMPI_H

// EOF