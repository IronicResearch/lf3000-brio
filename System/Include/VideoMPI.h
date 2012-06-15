#ifndef LF_BRIO_VIDEOMPI_H
#define LF_BRIO_VIDEOMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		VideoMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the Video module. 
//
//==============================================================================

#include <SystemTypes.h>
#include <VideoTypes.h>
#include <CoreMPI.h>
#include <EventListener.h>

LF_BEGIN_BRIO_NAMESPACE()

/// \class CVideoMPI
///
/// The Brio Video MPI is the interface for video playback functionality. It consists of 
/// low-level functions for handling individual video frames, high-level functions for 
/// playing video automatically in its own task thread, and state query functions. The 
/// current Video MPI implementation supports videos in the Ogg container format with 
/// the Theora video codec, and the AVI container format with the MJPEG codec for
/// compatibility with the new CameraMPI.
/// 
/// The low-level Video MPI functions manage videos on individual frame basis. A video 
/// resource is opened and initialized by StartVideo(). Each video frame is indexed by  
/// GetVideoFrame(), or alternately by time stamp with SyncVideoFrame(). The video frame 
/// is decoded into YUV format by PutVideoFrame() into a display surface created by 
/// the Display MPI. The display surface buffer address and dimensions are specified 
/// using a video surface descriptor. The application would then use an event loop to 
/// control video playback by outputting each frame at at time such as video splash 
/// screen animation, or in sync at selected time stamp intervals with a playing audio 
/// track. After all video frames are output, the video resource is closed with StopVideo().
///
/// High-level Video MPI functions are provided as counterpart to the Audio MPI. This 
/// variation of the StartVideo() MPI method will start a seprate task thread to play the 
/// video automatically, performing the all the low-level frame-based tasks internally. 
/// An optional looping parameter is available for looping video playback. An optional 
/// event listener parameter is available for posting state messages like when the video 
/// is done playing in a non-looping case. Playback can be paused with PauseVideo(), and 
/// resumed with ResumeVideo(), or stopped by StopVideo() which will terminate the task 
/// thread. Playback state info can be queried by IsVideoPaused(), IsvideoPlaying(), and 
/// IsVideoLooped() methods.
///
/// General video property info such as width, height, and FPS rate can be obtained in 
/// GetVideoInfo() method. Time stamp and frame index data can be queried for the current 
/// video frame with the GetVideoTime() method, which is applicable for either the 
/// low-level frame-based methods or the high-level running task thread.
///

//==============================================================================
class CVideoMPI : public ICoreMPI {
public:	
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;		
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;
			
	// class-specific functionality
	CVideoMPI();
	virtual ~CVideoMPI();

	// MPI-specific functionality
	
	/// Sets the default video resource path for subsequent StartVideo() calls
	///
	/// \param path	Base path for files referenced by calls to StartVideo()
	///
	/// \return Returns kNoErr on success.
	tErrType	SetVideoResourcePath(const CPath& path);

	/// Returns the base path set by SetVideoResourcePath().
	CPath*		GetVideoResourcePath() const;
	
	/// Start video stream after opening resource
	///
	/// StartVideo() variation which opens the video resource for subsequent low-level calls.
	///
	/// \param	path	Video file name relative to SetVideoResourcePath(),
	/// or full absolute path name if leading slash
	///
	/// \return	Returns valid video handle on success,
	/// or kInvalidVideoHndl on failure.
	tVideoHndl	StartVideo(const CPath& path);

	/// StartVideo() variation which creates a thread for playing video
	///
	/// \param	path	Video file name relative to SetVideoResourcePath(),
	/// or full absolute path name if leading slash
	///
	/// \param	pSurf	Pointer to video surface descriptor for rendering
	///
	/// \param	bLoop	Option for looping video
	///
	/// \param	pListener	Pointer to Event listener for video done callbacks
	tVideoHndl	StartVideo(const CPath& path, tVideoSurf* pSurf, Boolean bLoop = false, IEventListener* pListener = NULL);

	/// StartVideo() variation which creates a thread for playing video with audio
	///
	/// \param	path	Video file name relative to SetVideoResourcePath(),
	/// or full absolute path name if leading slash. Ogg, AVI extensions supported.
	///
	/// \param	pathAudio	Audio file name relative to SetVideoResourcePath(),
	/// or full absolute path name if leading slash. The audio file may be the
	/// same as the video file parameter for interleaved format videos, or may
	/// be a separate audio file track. Ogg, AVI, WAV extensions supported. 
	///
	/// \param	pSurf	Pointer to video surface descriptor for rendering
	///
	/// \param	bLoop	Option for looping video
	///
	/// \param	pListener	Pointer to Event listener for video done callbacks
	tVideoHndl	StartVideo(const CPath& path, const CPath& pathAudio, tVideoSurf* pSurf, Boolean bLoop = false, IEventListener* pListener = NULL);

	/// Stop video stream to close resource
	///
	/// \param	hVideo	Handle to video returned by StartVideo()
	///
	/// \return	Returns true on success.
	Boolean 	StopVideo(tVideoHndl hVideo);

	/// Get the next frame from video stream (pCtx not used)
	///
	/// \param	hVideo	Handle to video returned by StartVideo()
	///
	/// \param	pCtx	(not used)
	///
	/// \return	Returns true on success.
	Boolean 	GetVideoFrame(tVideoHndl hVideo, void* pCtx);

	/// Output the video frame to display surface (pCtx surface descriptor)
	///
	/// \param	hVideo	Handle to video returned by StartVideo()
	///
	/// \param	pSurf	Pointer to video surface descriptor for rendering
	///
	/// \return	Returns true on success.
	Boolean 	PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pSurf);
	
	/// Get info properties about video stream (width, height, fps)
	///
	/// \param	hVideo	Handle to video returned by StartVideo()
	///
	/// \param	pInfo	Pointer to video info struct to be filled in
	///
	/// \return	Returns true on success.
	Boolean		GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo);
	
	/// Get time stamp of current video frame (frame number, time in milliseconds)
	///
	/// \param	hVideo	Handle to video returned by StartVideo()
	///
	/// \param	pTime	Pointer to video timestamp stuct to be filled in
	///
	/// \return	Returns true on success.
	Boolean 	GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime);
	
	/// Get the next video frame synced at time stamp, dropping inbetween frames as directed
	///
	/// \param	hVideo	Handle to video returned by StartVideo()
	///
	/// \param	pTime	Pointer to video timestamp struct to be filled in,
	/// or synchronized with if drop-framing enabled.
	///
	/// \param	bDrop	Option for drop-frame synchronization
	///
	/// \return	Returns true on success.
	Boolean 	SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pTime, Boolean bDrop = false);

	/// Seek to the video frame at selected time stamp
	///
	/// \param	hVideo	Handle to video returned by StartVideo()
	///
	/// \param	pTime	Pointer to video time stamp struct for frame seeking.  Only the frame is used, the time is ignored.
	///
	/// \return	Returns true on success.
	Boolean 	SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pTime);

	/// Seek to the video frame at selected time stamp
	///
	/// \param	hVideo	Handle to video returned by StartVideo()
	///
	/// \param	pTime	Pointer to video time stamp struct for frame seeking.  Only the frame is used, the time is ignored.
	///
	/// \param bUpdateVideoDisplay	If true, the display will be updated with the new video position.
	///
	/// \return	Returns true on success.
	Boolean 	SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pTime, Boolean bUpdateVideoDisplay);
	
	/// Seek to the nearest video keyframeframe before or at selected time stamp
	///
	/// \param	hVideo	Handle to video returned by StartVideo()
	///
	/// \param	pTime	Pointer to video time stamp struct for frame seeking.  Only the frame is used, the time is ignored.
	///
	/// \param bUpdateVideoDisplay	If true, the display will be updated with the new video position.
	///
	/// \return	Returns true on success.
	Boolean 	SeekVideoKeyFrame(tVideoHndl hVideo, tVideoTime* pTime, Boolean bUpdateVideoDisplay = false);
 
	/// Pause video
	///
	/// \param	hVideo	Handle to video returned by StartVideo()
	///
	/// \return	Returns true on success.
	Boolean 	PauseVideo(tVideoHndl hVideo);

	/// Resume paused video
	///
	/// \param	hVideo	Handle to video returned by StartVideo()
	///
	/// \return	Returns true on success.
	Boolean 	ResumeVideo(tVideoHndl hVideo);

	/// Is video paused?
	///
	/// \param	hVideo	Handle to video returned by StartVideo()
	///
	/// \return	Returns true if video playback thread is paused.
	Boolean 	IsVideoPaused(tVideoHndl hVideo);

	/// Is video playing?
	///
	/// \param	hVideo	Handle to video returned by StartVideo()
	///
	/// \return	Returns true if video playback thread is playing.
	Boolean 	IsVideoPlaying(tVideoHndl hVideo);

	/// Is video looped?
	///
	/// \param	hVideo	Handle to video returned by StartVideo()
	///
	/// \return	Returns true if video playback thread is looping.
	Boolean 	IsVideoLooped(tVideoHndl hVideo);

	/// Get video handle of currently playing video
	///
	/// \return	Handle to video returned by StartVideo() if active
	tVideoHndl 	GetCurrentVideoHandle(void);

	/// Get length of video
	///
	/// \param	path		Video file name relative to SetVideoResourcePath(),
	/// or full absolute path name if leading slash. Ogg, AVI extensions supported.
	///
	/// \param	maxLength	Maximum length (in seconds) of video file.
	///
	/// \return	The length of the video in milliseconds.  If hVideo is invalid, then it returns -1.
	S64 		GetVideoLength(const CPath& path, int maxLength);

	S64 		GetVideoLength(tVideoHndl hVideo);

private:
	class CVideoModule*	pModule_;
	U32					id_;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_VIDEOMPI_H

// EOF
