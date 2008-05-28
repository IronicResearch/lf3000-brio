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
	/// or full absolute path name if leading slash
	///
	/// \param	pathAudio	Audio file name relative to SetVideoResourcePath(),
	/// or full absolute path name if leading slash
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
	/// \param	pTime	Pointer to video time stamp struct for frame seeking.
	///
	/// \return	Returns true on success.
	Boolean 	SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pTime);
 
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

private:
	class CVideoModule*	pModule_;
	U32					id_;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_VIDEOMPI_H

// EOF
