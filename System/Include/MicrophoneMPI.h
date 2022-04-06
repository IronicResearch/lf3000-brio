#ifndef LF_BRIO_MICROPHONEMPI_H
#define LF_BRIO_MICROPHONEMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		MicrophoneMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the Microphone module.
//
//==============================================================================

#include <SystemTypes.h>
#include <MicrophoneTypes.h>
#include <CoreMPI.h>
#include <EventListener.h>

LF_BEGIN_BRIO_NAMESPACE()
/// \class CMicrophoneMPI
///
/// <B>For use with Firmware 3.x or later only.</B>
///
/// Originally a component of the Camera MPI, the Brio Microphone MPI is the interface for microphone access.
///	It consists of utility functions for manipulating the microphone hardware and primary functions for acquiring
/// audio from the microphone.  Audio is captured in the WAV format containing raw PCM audio.
///
/// Audio produced by the camera is a PCM-encoded 16-bit mono channel sampled at 16000 Hz and stored
/// in a WAV container.
///
/// The Microphone MPI functions are provided as a counterpart to the Audio MPI.  The StartAudioCapture() MPI
/// method starts a separate task thread to capture audio automatically, performing all the low-level frame-based tasks internally.
/// StartVideoCapture(), despite its name, does not necessarily preserve a video stream
/// to the file system.  Audio can be paused with PauseAudioCapture(), and resumed with ResumeAudioCapture(),
/// or stopped by StopAudioCapture() which will terminate the task thread. Recordings must be
/// stopped prior to application exit. Capture state info can be queried with the
/// IsAudioCapturePaused() method.
///
///	<B>WARNING:</B> The Audio recording API is not power-fail safe.  This means that
///	the application is responsible for cleaning up any incomplete recordings.  A recording
///	is considered complete if it is stopped by any of the following means:
///	<ul>
///	<li>Synchronously, by the application via StopAudioCapture()</li>
///	<li>Asyncrhonously, by the Microphone MPI</li>
///	<ul>
///		<li>If the timeout specified in StartAudioCapture() elapses</li>
///		<li>If the file system usage quota is hit (disk full)</li>
///		<li>If the microphone is physically disconnected</li>
///	</ul>
///	</ul>
///	As long as the application registers an IEventListener when invoking StartAudioCapture(), it will be
/// notified of any recording termination (tCaptureStoppedMsg, tCaptureTimeoutMsg, tCaptureQuotaHitMsg, tMicrophoneRemovedMsg).
/// Using an IEventListener, with potential assistance from AtomicFile (fopenAtomic()), it is possible for
///	the application to maintain consistent recordings.

//==============================================================================
class CMicrophoneMPI : public ICoreMPI {
public:
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	CMicrophoneMPI();
	virtual ~CMicrophoneMPI();

	// MPI-specific functionality

	/// Determines if the microphone widget is present
	///
	/// \return true if the widget is installed.
	Boolean		IsMicrophonePresent();

	/// Sets the default audio recording path for subsequent StartAudioCapture() calls
	///
	/// \param path	Base path for files referenced by calls to StartAudioCapture()
	///
	/// \return Returns kNoErr on success.
	tErrType	SetAudioPath(const CPath& path);

	/// Returns the base path set by SetAudioPath().
	CPath*		GetAudioPath();

	/// StartAudioCapture() spawns a thread for audio-only capture.
	/// If the host device has insufficient storage space to save an audio
	/// recording, this call will fail.
	///
	/// \param	path	WAV save file name relative to SetAudioPath(),
	/// or full absolute path name if leading slash.
	///
	/// \param	pListener	Pointer to Event listener for callbacks from the
	/// audio capture subsystem.  Callbacks will be issued when recording is
	/// stopped asynchronously because the file system is full.
	///	<B>WARNING:</B> Although this parameter is allowed to be NULL, it is <B>STRONGLY</B>
	///	suggested to always supply a valid listener.  Tracking the signaled CMicrophoneEventMessage
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

	// Internal Brio use only. Not for use by developers.
	// Used by CameraMPI to write audio to avi stream.
	unsigned int 	CameraWriteAudio(void* avi);

private:
	class CMicrophoneModule*	pModule_;
	U32					id_;
};

LF_END_BRIO_NAMESPACE()
#endif // LF_BRIO_MICROPHONEMPI_H

// EOF
