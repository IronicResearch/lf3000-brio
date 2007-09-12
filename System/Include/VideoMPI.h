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
	tErrType	SetVideoResourcePath(const CPath& path);
	CPath*		GetVideoResourcePath() const;
	
	/// Start video stream after opening resource
	tVideoHndl	StartVideo(const CPath& path);
	tVideoHndl	StartVideo(const CPath& path, tVideoSurf* pSurf, Boolean bLoop = false, IEventListener* pListener = NULL);
	tVideoHndl	StartVideo(const CPath& path, const CPath& pathAudio, tVideoSurf* pSurf, Boolean bLoop = false, IEventListener* pListener = NULL);

	/// Stop video stream to close resource
	Boolean 	StopVideo(tVideoHndl hVideo);

	/// Get the next frame from video stream (pCtx not used)
	Boolean 	GetVideoFrame(tVideoHndl hVideo, void* pCtx);

	/// Output the video frame to display surface (pCtx surface descriptor)
	Boolean 	PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pSurf);
	
	/// Get info properties about video stream (width, height, fps)
	Boolean		GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo);
	
	/// Get time stamp of current video frame (frame number, time in milliseconds)
	Boolean 	GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime);
	
	/// Get the next video frame synced at time stamp, dropping inbetween frames as directed
	Boolean 	SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pTime, Boolean bDrop = false);

	/// Seek to the video frame at selected time stamp
	Boolean 	SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pTime);
 
	/// Pause video
	Boolean 	PauseVideo(tVideoHndl hVideo);

	/// Resume paused video
	Boolean 	ResumeVideo(tVideoHndl hVideo);

	/// Is video paused?
	Boolean 	IsVideoPaused(tVideoHndl hVideo);

	/// Is video playing?
	Boolean 	IsVideoPlaying(tVideoHndl hVideo);

	/// Is video looped?
	Boolean 	IsVideoLooped(tVideoHndl hVideo);

private:
	class CVideoModule*	pModule_;
	U32					id_;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_VIDEOMPI_H

// EOF
