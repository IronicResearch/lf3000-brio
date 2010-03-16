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
/// The Camera MPI functions are provided as counterpart to the Video and Audio MPIs.
/// The StartVideoCapture() MPI method starts a separate task thread to capture video
/// automatically, performing the all the low-level frame-based tasks internally.
/// StartVideoCapture(), despite its name, does not necessarily preserve a video stream
/// to the file system.  It can also be used to instantiate a "viewfinder" which simply
/// renders to the display to be use in conjunction with still image capture.  Capture can
/// be paused with PauseVideoCapture(), and resumed with ResumeVideoCapture(),
/// or stopped by StopVideoCapture() which will terminate the task thread. Captures must be
/// stopped prior to application exit. Capture state info can be queried with the
/// IsVideoCapturePaused() method.  For audio recordings, the API provides audio-only
/// equivalents to all of these methods.
///
/// Camera controls, such as contrast, brightness, etc., can be obtained with the
/// GetCameraControls() method.
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
	/// \param	path	AVI save file name relative to SetCameraVideoPath(),
	/// or full absolute path name if a leading slash is present.  Blank
	/// parameter means don't save the capture to a file.
	///
	/// \param	pSurf	Pointer to video surface descriptor for rendering.
	/// NULL parameter means don't render the capture.  pSurf should reference
	/// a tDisplayHandle created with CDisplayMPI::CreateHandle() using
	/// kPixelFormatYUV420.
	///
	/// \return kInvalidVidCapHndl on failure.
	tVidCapHndl	StartVideoCapture(const CPath& path, tVideoSurf* pSurf);

	/// SnapFrame() takes a snapshot from the stream being captured in a separate
	/// thread via StartVideoCapture().
	/// This snapshot is automatically saved to the file system and is not processed
	/// by the application. SnapFrame() will fail if video recording is active.
	/// SnapFrame() can be called while video capturing (the viewfinder) is paused.
	///
	/// \param	hndl	Video capture handle returned by StartVideoCapture()
	///
	/// \param	path	JPEG save file name relative to SetCameraStillPath(),
	/// or full absolute path name if leading slash.
	///
	/// \return true on success.
	Boolean		SnapFrame(const tVidCapHndl hndl, const CPath &path);

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
	/// All captures, paused or running, must be stopped to ensure
	/// a proper recording on the filesystem.
	///
	/// \param	hndl	Video capture handle returned by
	/// StartVideoCapture()
	///
	/// \return true if successful.
	Boolean		PauseVideoCapture(const tVidCapHndl hndl);

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

	/// StopVideoCapture() stop an active capture.    If StopVideoCapture()
	/// is not called prior to the application exiting (either normally
	/// or abnormally), the recording will not be saved.
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

	/// StopAudioCapture() stop an active capture.  If StopAudioCapture()
	/// is not called prior to the application exiting (either normally
	/// or abnormally), the recording will not be saved.
	///
	/// \param	hndl	Audio capture handle returned by StartAudioCapture()
	///
	/// \return true if successful.
	Boolean		StopAudioCapture(const tAudCapHndl hndl);

private:
	class CCameraModule*	pModule_;
	U32					id_;
};

LF_END_BRIO_NAMESPACE()
#endif // LF_BRIO_CAMERAMPI_H

// EOF
