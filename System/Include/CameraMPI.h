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
/// The Brio Camera MPI is the interface for camera access.  It consists of utility functions
/// for manipulating the camera hardware and primary functions for acquiring
/// images, video, and audio from the camera. The video capture functions are divided into low-level
/// functions for frame-by-frame capture, and high-level functions for capturing video with audio
/// automatically in a separate task thread. The current Camera MPI implementation supports
/// videos in the AVI format with the MJPEG video codec and raw PCM audio.  Audio is captured
/// in the WAV format containing raw PCM audio.  Still images are captured in JPEG format.
///
/// Still images produced by the camera are VGA (640x480) resolution JPEG/JFIF files with 24-bit color.
/// Audio produced by the camera is a PCM-encoded 16-bit mono channel sampled at 16000 Hz and stored
/// in a WAV container.  Videos produced by the camera are QVGA (320x240) resolution MJPEGs with
/// 16-bit mono PCM audio @ 16000Hz stored in an AVI container.
///
/// The Camera MPI is related to the Video MPI in that captured video is typically shown on a
/// display, as in a typical camera viewfinder. The displaying of a captured stream is
/// essentially video playback, with the video frames originating from a camera instead of
/// a file.
///
/// The low-level Camera MPI functions manage captures on an individual frame basis. A camera
/// resource is initialized by SetBuffers(), which reserves memory for frames to be exchanged
/// between the Camera MPI and the application. Video capturing is initiated by
/// StartVideoCapture(). Each captured frame is indexed by GetFrame(). The captured
/// frame is processed by the application (rendered to a display, written to a file, etc.),
/// and then returned to the frame buffer pool with ReturnFrame(). As long as this
/// GetFrame()-ReturnFrame() sequence continues, frames will be streamed to the
/// application. Capturing is halted with StopVideoCapture().
///
/// High-level Camera MPI functions are provided as counterpart to the Video and Audio MPIs.
/// This variation of the StartVideoCapture() MPI method will start a separate task thread to
/// capture the video automatically, performing the all the low-level frame-based tasks
/// internally. Capture can be paused with PauseVideoCapture(), and resumed with ResumeVideoCapture(),
/// or stopped by StopVideoCapture() which will terminate the task thread. Capture state info can be
/// queried by the IsCapturePaused() method.  For audio recordings, the API provides audio-only
/// equivalents to all of these methods.
///
/// General camera capability info such as supported resolutions, formats, and framerates can
/// be obtained with the GetCameraModes() method. Camera controls, such as contrast, brightness,
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

	/// Sets the default video path for subsequent StartVideoCapture() calls
	///
	/// \param path	Base path for files referenced by calls to StartVideoCapture()
	///
	/// \return Returns kNoErr on success.
	tErrType	SetCameraVideoPath(const CPath& path);

	/// Returns the base path set by SetCameraVideoPath().
	CPath*		GetCameraVideoPath();

	/// Sets the default still image path for subsequent SaveFrame()/OpenFrame() calls
	///
	/// \param path	Base path for files referenced by calls to SaveFrame()
	///
	/// \return Returns kNoErr on success.
	tErrType	SetCameraStillPath(const CPath& path);

	/// Returns the base path set by SetCameraStillPath().
	CPath*		GetCameraStillPath();

	/// Sets the default audio recording path for subsequent StartAudioCapture() calls
	///
	/// \param path	Base path for files referenced by calls to StartAudioCapture()
	///
	/// \return Returns kNoErr on success.
	tErrType	SetCameraAudioPath(const CPath& path);

	/// Returns the base path set by SetCameraAudioPath().
	CPath*		GetCameraAudioPath();

	/// Queries camera for supported resolution, format, and framerate.
	/// <pre>
	/// The existing camera widget supports these formats:
	/// 	-MJPG
	///		-YUYV
	///
	/// The existing camera widget supports these resolutions:
	/// 	-VGA	(640x480)
	/// 	-QVGA	(320x240)
	/// 	-QQVGA	(160x120)
	/// 	-CIF	(352x288)
	/// 	-QCIF	(176x144)
	///
	/// The existing camera widget supports these framerates:
	/// 	-MJPG (VGA, QVGA, QQVGA, CIF, QCIF) @ 30	fps
	/// 	-MJPG (VGA, QVGA, QQVGA, CIF, QCIF) @ 15	fps
	/// 	-MJPG (VGA, QVGA, QQVGA, CIF, QCIF) @ 1		fps
	/// 	-YUYV (QVGA)	@ 5		fps
	/// 	-YUYV (QQVGA)	@ 20	fps
	/// 	-YUYV (CIF)		@ 4		fps
	/// 	-YUYV (QCIF)	@ 15	fps
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
	///	-Gamma				(20 to 100; 75 default)
	///	-Backlight Compensation		(0 to 5, 0 default)
	///	-Power Line Frequency		(0 (Disabled), 1( 50 Hz), 2 (60 Hz); 2 default)
	///	-Sharpness			(0 to 10, 6 default)
	/// </pre>

	/// \param	controls	Control(s) supported by camera.
	///
	/// \return true on success.
	Boolean		GetCameraControls(tCameraControls &controls);

	/// Requests camera to apply the specified brightness, contrast, etc.
	///
	/// \param control		Control structure.  Id field must be valid as returned
	/// by GetCameraControls()
	///
	/// \param value		New value to apply
	///
	/// \return true on success.
	Boolean		SetCameraControl(const tControlInfo* control, const S32 value);

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

	/// GetFrame() retrieve next frame from capture stream.
	///
	/// \param	hndl	Video capture handle returned by StartVideoCapture()
	///
	/// \param frame	Image data returned from camera
	///
	/// \return Returns true on success.
	Boolean		GetFrame(const tVidCapHndl hndl, tFrameInfo *frame);

	/// RenderFrame() convert retrieved frame from camera format to a format suitable for
	/// rendering on the display.
	///
	/// \param frame	Image data returned from camera
	///
	/// \param pSurf	Display surface to render upon.  If NULL, the display will not be updated.
	///
	/// \param image	Raw render target.  If NULL, pSurf should be valid.  A non-NULL parameter
	/// will be filled with bitmap data which can be modified by the caller.
	///
	/// \return Returns true on success.
	Boolean		RenderFrame(tFrameInfo *frame, tVideoSurf *pSurf = NULL, tBitmapInfo *image = NULL);

	/// ReturnFrame() mark retrieved frame as processed and return it to the pool.
	///
	/// \param	hndl	Video capture handle returned by StartVideoCapture()
	///
	/// \param frame	Frame returned by GetFrame()
	///
	/// \return Returns true on success.
	Boolean		ReturnFrame(const tVidCapHndl hndl, const tFrameInfo *frame);

	/// StartVideoCapture() variation which spawns a thread for capture.
	/// If the host device has insufficient storage space to save a video
	/// recording, this call will fail.  This is true even if the caller
	/// is not recording video and wants to use the camera only as a viewfinder
	/// to capture still images.
	///
	/// \param	path	Video save file name relative to SetCameraVideoPath(),
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
	///
	/// \return kInvalidVidCapHndl on failure.
	tVidCapHndl	StartVideoCapture(const CPath& path, const Boolean audio, tVideoSurf* pSurf, tRect *rect);

	/// GrabFrame() takes a snapshot from the stream being captured in a separate
	/// thread via StartVideoCapture(const CPath&, const Boolean, tVideoSurf*, tRect *).
	/// This snapshot does not have to be returned, and will fail if the thread is
	/// recording video.
	///
	/// \param	hndl	Video capture handle returned by StartVideoCapture()
	///
	/// \param frame	As an input, used to request snapshot resolution (this can
	/// differ from the video stream).  As an output, used to return image data from camera.
	/// Note that frame->data MUST be KernelMPI.Free()'d by the caller after use.
	///
	/// \return Returns true on success.
	Boolean		GrabFrame(const tVidCapHndl hndl, tFrameInfo *frame);

	/// SaveFrame() saves a grabbed snapshot to the file system.
	///
	/// \param	path	JPEG save file name relative to SetCameraStillPath(),
	/// or full absolute path name if leading slash.
	///
	/// \param frame	Input: frame acquired by GrabFrame()
	///
	/// \return Returns true on success.
	Boolean		SaveFrame(const CPath &path, const tFrameInfo *frame);

	/// OpenFrame() retrieves a snapshot saved to the file system by SaveFrame()
	///
	/// \param	path	JPEG save file name relative to SetCameraStillPath(),
	/// or full absolute path name if leading slash.
	///
	/// \param frame	Output: JPEG image frame stored by SaveFrame().  On success,
	/// frame's fields are populated (width, height, size, and data).  Data is
	/// allocated on the stack by KernelMPI::Malloc(), and must be KernelMPI::Free()'d
	/// by the caller.
	///
	/// \return Returns true on success.
	Boolean		OpenFrame(const CPath &path, tFrameInfo *frame);

	/// PauseVideoCapture() pause an active capture.
	/// Pausing a recording is different from stopping a recording
	/// since a paused capture is still active, albeit on standby.
	/// All captures, paused or running, must be stopped to ensure
	/// a proper recording.
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
	Boolean		IsVideoCapturePaused(const tVidCapHndl hndl);

	/// StopVideoCapture() stop an active capture.
	///
	/// \param	hndl	Video capture handle returned by StartVideoCapture()
	///
	/// \return Returns true if successful.
	Boolean		StopVideoCapture(const tVidCapHndl hndl);

	/// StartAudioCapture() spawns a thread for audio-only capture.
	/// If the host device has insufficient storage space to save an audio
	/// recording, this call will fail.
	///
	/// \param	path	Audio WAV save file name relative to SetCameraAudioPath(),
	/// or full absolute path name if leading slash.
	///
	/// \return kInvalidAudCapHndl on failure.
	tAudCapHndl	StartAudioCapture(const CPath& path);

	/// PauseAudioCapture() pause an active capture.
	/// Pausing a recording is different from stopping a recording
	/// since a paused capture is still active, albeit on standby.
	/// All captures, paused or running, must be stopped to ensure
	/// a proper recording.
	///
	/// \param	hndl	Audio capture handle returned by
	/// StartAudioCapture()
	///
	/// \return Returns true if successful.
	Boolean		PauseAudioCapture(const tAudCapHndl hndl);

	/// ResumeAudioCapture() a paused capture.
	///
	/// \param	hndl	Audio capture handle returned by
	/// StartAudioCapture()
	///
	/// \return Returns true if successful.
	Boolean		ResumeAudioCapture(const tAudCapHndl hndl);

	/// IsAudioCapturePaused() checks if the specified capture is paused.
	///
	/// \param	hndl	Audio capture handle returned by
	/// StartAudioCapture()
	///
	/// \return Returns true if capture is currently paused.
	Boolean		IsAudioCapturePaused(const tAudCapHndl hndl);

	/// StopAudioCapture() stop an active capture.  If StopAudioCapture()
	/// is not called prior to the application exiting (either normally
	/// or abnormally), the recording will not be saved.
	///
	/// \param	hndl	Audio capture handle returned by StartAudioCapture()
	///
	/// \return Returns true if successful.
	Boolean		StopAudioCapture(const tAudCapHndl hndl);

private:
	class CCameraModule*	pModule_;
	U32					id_;
};

LF_END_BRIO_NAMESPACE()
#endif // LF_BRIO_CAMERAMPI_H

// EOF
