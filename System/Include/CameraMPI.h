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
/// images, video, and audio from the camera.  The current Camera MPI implementation supports
/// videos in the AVI format with the MJPEG video codec and raw PCM audio.  Audio is captured
/// in the WAV format containing raw PCM audio.  Still images are captured in JPEG format.
///
/// Still images produced by the camera are VGA (640x480) resolution JPEG/JFIF files with 24-bit color.
/// Audio produced by the camera is a PCM-encoded 16-bit mono channel sampled at 16000 Hz and stored
/// in a WAV container.  Videos produced by the camera are QVGA (320x240) resolution MJPEGs with
/// 16-bit mono PCM audio @ 16000Hz stored in an AVI container.
///
/// The Camera MPI is related to the Video MPI in that captured video is typically shown on a
/// display as in a typical camera viewfinder. The displaying of a captured stream is
/// essentially video playback, with the video frames originating from a camera instead of
/// a file.
///
/// The Camera MPI functions are provided as counterparts to the Video and Audio MPIs.
/// The StartVideoCapture() MPI method starts a separate task thread to capture video
/// automatically, performing the all the low-level frame-based tasks internally.
/// StartVideoCapture(), despite its name, does not necessarily preserve a video stream
/// to the file system.  It can also be used to instantiate a "viewfinder" which simply
/// renders to the display to be use in conjunction with still image capture.  Capture can
/// be paused with PauseVideoCapture(), and resumed with ResumeVideoCapture(),
/// or stopped by StopVideoCapture() which will terminate the task thread. Recordings must be
/// stopped prior to application exit. Capture state info can be queried with the
/// IsVideoCapturePaused() method.  For audio recordings, the API provides audio-only
/// equivalents to all of these methods.
///
/// Camera controls, such as contrast, brightness, etc., can be obtained with the
/// GetCameraControls() method.
///
///	<B>WARNING:</B> The Audio and Video recording APIs are not power-fail safe.  This means that
///	the application is responsible for cleaning up any incomplete recordings.  A recording
///	is considered complete if it is stopped by any of the following means:
///	<ul>
///	<li>Synchronously, by the application via StopAudioCapture() / StopVideoCapture()</li>
///	<li>Asyncrhonously, by the Camera MPI</li>
///	<ul>
///		<li>If the timeout specified in StartAudioCapture() / StartVideoCapture() elapses</li>
///		<li>If the file system usage quota is hit (disk full)</li>
///		<li>If the camera is physically disconnected</li>
///	</ul>
///	</ul>
///	As long as the application registers an IEventListener when invoking StartAudioCapture() /
///	StartVideoCapture(), it will be notified of any recording termination (tCaptureStoppedMsg,
///	tCaptureTimeoutMsg, tCaptureQuotaHitMsg, tCameraRemovedMsg).
/// Using an IEventListener, with potential assistance from AtomicFile (fopenAtomic()), it is possible for
///	the application to maintain consistent recordings.

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

	/// Determines if the camera widget is present
	///
	/// \return true if the widget is installed.
	Boolean		IsCameraPresent();

	/// Sets the default video path for subsequent StartVideoCapture() calls
	///
	/// \param path	Base path for files referenced by calls to StartVideoCapture()
	///
	/// \return Returns kNoErr on success.
	tErrType	SetCameraVideoPath(const CPath& path);

	/// Returns the base path set by SetCameraVideoPath().
	CPath*		GetCameraVideoPath();

	/// Sets the default still image path for subsequent SnapFrame()/RenderFrame() calls
	///
	/// \param path	Base path for files referenced by calls to SnapFrame()
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

	/// Queries camera for image sensor controls, such as contrast, brightness, etc.
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

	/// Spawns a thread for capturing frames from the camera.
	/// If the host device has insufficient storage space to save a video
	/// recording, this call will fail.  This is true even if the caller
	/// is not recording video and wants to use the camera only as a viewfinder
	/// to capture still images.
	///
	/// \param	pSurf	Pointer to video surface descriptor for rendering.
	/// NULL parameter means don't render the capture.  pSurf should reference
	/// a tDisplayHandle created with CDisplayMPI::CreateHandle() using
	/// kPixelFormatYUV420. If the passed in surface width and height are different
	/// from the created surface dimensions, the YUV layer's HW scaler will 
	/// scale up (or down) the pSurf parameter's size to the surface size.
	///
	/// \param	pListener	Pointer to Event listener for callbacks from the
	/// video capture subsystem.  Callbacks will occur when recording is asynchronously
	/// stopped if the file system is full or the maximum record length has been reached.
	///	<B>WARNING:</B> Although this parameter is allowed to be NULL, it is <B>STRONGLY</B>
	///	suggested to always supply a valid listener.  Tracking the signaled CCameraEventMessage
	/// is required to ensure consistent recordings.  A recording that has been started
	///	but never stopped (synchronously or asynchronously) is invalid and <B>MUST</B> be deleted.
	///
	/// \param	path	AVI save file name relative to SetCameraVideoPath(),
	/// or full absolute path name if a leading slash is present.  Blank
	/// parameter "" means don't save the capture to a file.  Blank, "",
	/// is the default parameter.
	///
	/// \param	maxLength	Maximum length in seconds to record.  A value of 0 means
	/// unlimited recording.
	///
	/// \param	audio	Option to capture video with audio (true) or video only (false).
	/// Default = true.
	///
	/// \return kInvalidVidCapHndl on failure.
	tVidCapHndl	StartVideoCapture(tVideoSurf* pSurf, IEventListener * pListener = NULL,
			const CPath& path = "", const U32 maxLength = 0, const Boolean audio = true);

	/// SnapFrame() takes a snapshot from the stream being captured in a separate
	/// thread via StartVideoCapture().
	/// This snapshot is automatically saved to the file system and is not processed
	/// by the application. SnapFrame() will fail if video recording is active.
	/// SnapFrame() can be called while video capturing (the viewfinder) is paused.
	///
	/// \param	hndl	Video capture handle returned by StartVideoCapture()
	///
	/// \param	path	JPEG save file name relative to SetCameraStillPath(),
	/// or full absolute path name if leading slash. If '.png' extension is used,
	/// the image will be converted to RGB and saved as PNG file instead of JPEG.
	///
	/// \return true on success.
	Boolean		SnapFrame(const tVidCapHndl hndl, const CPath &path);

	/// GetFrame() takes a snapshot from the stream being captured in a separate
	/// thread via StartVideoCapture().
	/// This snapshot is automatically automatically decompressed from JPEG
	/// to an RGB bitmap and saved to the memory location provided by the caller.
	/// GetFrame() can be called while video capturing (the viewfinder) is paused.
	///
	/// \param	hndl	Video capture handle returned by StartVideoCapture()
	///
	/// \param	pixels	Buffer to hold the captured frame.  The buffer must be
	/// large enough to hold a 24-bpp VGA image, i.e., 640*480*3 bytes.  The format
	/// of the buffer as populated by GetFrame() is a typical left-to-right,
	/// top-to-bottom raster scan, and the pixels themselves are simply R-G-B.
	/// - Inter-pixel order [row,column]:
	/// 	- [0,0] [0,1] [0,2] ... [0, 639] [1, 0] [1,1] ... [479,638] [479,639]
	///
	/// \param color_order	Choose the order of the colors (particularly the red and the blue channels).
	///	- kOpenGlRgb (good if you're making a OpenGL texture of frame, also slightly faster)
	///		- Intra-pixel order (Byte 1, Byte 2, Byte 3):
	/// 		- RRRRRRRR GGGGGGGG BBBBBBBB
	///	- kDisplayRgb (good if you're manually putting this on screen with a display handle or BlitBuffer)
	///		- Intra-pixel order (Byte 1, Byte 2, Byte 3):
	/// 		- BBBBBBBB GGGGGGGG RRRRRRRR
	///
	/// \return true on success.
	Boolean		GetFrame(const tVidCapHndl hndl, U8 *pixels, tColorOrder color_order = kOpenGlRgb);

	/// RenderFrame() render an image saved with SnapFrame() to the host device's display.
	///
	/// \param	path	JPEG save file name relative to SetCameraStillPath(),
	/// or full absolute path name if leading slash.
	///
	/// \param	pSurf	Pointer to video surface descriptor for rendering.
	/// pSurf should reference a tDisplayHandle created with CDisplayMPI::CreateHandle()
	/// using kPixelFormatYUV420.
	///
	/// \return Returns true on success.
	Boolean		RenderFrame(const CPath &path, tVideoSurf *pSurf);

	/// PauseVideoCapture() pause an active capture.
	/// Pausing a capture is different from stopping a capture
	/// since a paused capture is still active, albeit on standby.
	/// All recordings, paused or running, must be stopped to ensure
	/// proper video file creation on the filesystem.
	///
	/// \param	hndl	Video capture handle returned by
	/// StartVideoCapture()
	///
	/// \param display	Pause updates to the video surface registered
	/// with StartVideoCapture().  Recording to AVI is paused unconditionally.
	///
	/// \return true if successful.
	Boolean		PauseVideoCapture(const tVidCapHndl hndl, const Boolean display=true);

	/// ResumeVideoCapture() a paused capture.
	///
	/// \param	hndl	Video capture handle returned by
	/// StartVideoCapture()
	///
	/// \return true if successful.
	Boolean		ResumeVideoCapture(const tVidCapHndl hndl);

	/// IsCapturePaused() checks if the specified capture is paused.
	///
	/// \param	hndl	Video capture handle returned by StartVideoCapture()
	///
	/// \return true if capture is currently paused.
	Boolean		IsVideoCapturePaused(const tVidCapHndl hndl);

	/// StopVideoCapture() stop an active capture.  Recordings must be stopped
	/// prior to application exit (either normal or abnormal).  The application
	/// can stop recording by calling StopVideoCapture(), or Brio will stop
	/// recording when either the maximum length specified in StartVideoCapture()
	/// is reached, or the recording consumes all available space in the
	/// file system.  If the recording is not stopped by either the application
	/// or Brio, it will not be saved.
	///
	/// \param	hndl	Video capture handle returned by StartVideoCapture()
	///
	/// \return true if successful.
	Boolean		StopVideoCapture(const tVidCapHndl hndl);

	/// StartAudioCapture() spawns a thread for audio-only capture.
	/// If the host device has insufficient storage space to save an audio
	/// recording, this call will fail.
	///
	/// \param	path	WAV save file name relative to SetCameraAudioPath(),
	/// or full absolute path name if leading slash.
	///
	/// \param	pListener	Pointer to Event listener for callbacks from the
	/// audio capture subsystem.  Callbacks will be issued when recording is
	/// stopped asynchronously because the file system is full.
	///	<B>WARNING:</B> Although this parameter is allowed to be NULL, it is <B>STRONGLY</B>
	///	suggested to always supply a valid listener.  Tracking the signaled CCameraEventMessage
	/// is required to ensure consistent recordings.  A recording that has been started
	///	but never stopped (synchronously or asynchronously) is invalid and <B>MUST</B> be deleted.
	///
	/// \param	maxLength	Maximum length in seconds to record.  A value of 0
	/// means unlimited recording.
	///
	/// \param	paused		Start audio capture in paused state. If set true,
	/// the application *must* call \ref ResumeAudioCapture() to begin recording 
	/// audio when ready. Otherwise recording begins immediately.
	///
	/// \return kInvalidAudCapHndl on failure.
	tAudCapHndl	StartAudioCapture(const CPath& path, IEventListener * pListener, const U32 maxLength = 0, 
			const Boolean paused = false);

	/// PauseAudioCapture() pause an active capture.
	/// Pausing a recording is different from stopping a recording
	/// since a paused capture is still active, albeit on standby.
	/// All recordings, paused or running, must be stopped to ensure
	/// proper audio file creation.
	///
	/// \param	hndl	Audio capture handle returned by
	/// StartAudioCapture()
	///
	/// \return true if successful.
	Boolean		PauseAudioCapture(const tAudCapHndl hndl);

	/// ResumeAudioCapture() a paused capture.
	///
	/// \param	hndl	Audio capture handle returned by
	/// StartAudioCapture()
	///
	/// \return true if successful.
	Boolean		ResumeAudioCapture(const tAudCapHndl hndl);

	/// IsAudioCapturePaused() checks if the specified capture is paused.
	///
	/// \param	hndl	Audio capture handle returned by
	/// StartAudioCapture()
	///
	/// \return true if capture is currently paused.
	Boolean		IsAudioCapturePaused(const tAudCapHndl hndl);

	/// StopAudioCapture() stop an active capture.  Recordings must be stopped
	/// prior to application exit (either normal or abnormal).  The application
	/// can stop recording by calling StopAudioCapture(), or Brio will stop
	/// recording when either the maximum length specified in StartAudioCapture()
	/// is reached, or the recording consumes all available space in the
	/// file system.  If the recording is not stopped by either the application
	/// or Brio, it will not be saved.
	///
	/// \param	hndl	Audio capture handle returned by StartAudioCapture()
	///
	/// \return true if successful.
	Boolean		StopAudioCapture(const tAudCapHndl hndl);

	/// SetMicrophoneParam() sets microphone parameter for audio capture session.
	/// \param	param	Microphone paramater type
	/// \param	value	Microphone parameter value
	/// \return			True if parameter is valid, False otherwise.
	Boolean		SetMicrophoneParam(enum tMicrophoneParam param, S32 value);

	/// GetMicrophoneParam() gets microphone parameter for audio capture session.
	/// \param	param	Microphone paramater type
	/// \return			Microphone parameter value
	S32			GetMicrophoneParam(enum tMicrophoneParam param);

private:
	class CCameraModule*	pModule_;
	U32					id_;
};

LF_END_BRIO_NAMESPACE()
#endif // LF_BRIO_CAMERAMPI_H

// EOF
