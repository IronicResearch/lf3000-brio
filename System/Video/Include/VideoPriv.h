#ifndef LF_BRIO_VIDEOPRIV_H
#define LF_BRIO_VIDEOPRIV_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		VideoPriv.h
//
// Description:
//		Defines the interface for the private underlying Video manager module. 
//
//==============================================================================
#include <SystemTypes.h>
#include <CoreModule.h>
#include <AudioTypes.h>
#include <EventTypes.h>
#include <VideoTypes.h>
#include <DebugMPI.h>
#include <EventListener.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Constants
//==============================================================================
const CString			kVideoModuleName	= "Video";
const tVersion			kVideoModuleVersion	= 2;
const tEventPriority	kVideoEventPriority	= 0;
const tDebugLevel		kVideoDebugLevel	= kDbgLvlImportant;

//==============================================================================
// Typedefs
//==============================================================================

struct tVideoContext {
	tVideoHndl			hVideo;
	tAudioID			hAudio;
	CPath&				pathAudio;
	tVideoSurf*			pSurfVideo;
	IEventListener*		pListener;
	Boolean				bLooped;
	Boolean				bDropFramed;
	Boolean				bPaused;
	Boolean				bPlaying;
};

//==============================================================================
// External function prototypes
//==============================================================================

tErrType InitVideoTask( tVideoContext* pCtx );
tErrType DeInitVideoTask( void );

//==============================================================================
class CVideoModule : public ICoreModule {
public:	
	// ICoreModule functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	VTABLE_EXPORT tErrType		SetVideoResourcePath(const CPath& path);
	VTABLE_EXPORT CPath*		GetVideoResourcePath() const;
	VTABLE_EXPORT tVideoHndl	StartVideo(const CPath& path);
	VTABLE_EXPORT tVideoHndl	StartVideo(const CPath& path, tVideoSurf* pSurf, Boolean bLoop = false, IEventListener* pListener = NULL);
	VTABLE_EXPORT tVideoHndl	StartVideo(const CPath& path, const CPath& pathAudio, tVideoSurf* pSurf, Boolean bLoop = false, IEventListener* pListener = NULL);
    VTABLE_EXPORT Boolean     	StopVideo(tVideoHndl hVideo);
	VTABLE_EXPORT Boolean 		GetVideoFrame(tVideoHndl hVideo, void* pCtx);
	VTABLE_EXPORT Boolean 		PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pCtx);
	VTABLE_EXPORT Boolean 		GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo);
	VTABLE_EXPORT Boolean 		GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime);
	VTABLE_EXPORT Boolean 		SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop);
	VTABLE_EXPORT Boolean 		SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx);
	VTABLE_EXPORT Boolean 		PauseVideo(tVideoHndl hVideo);
	VTABLE_EXPORT Boolean 		ResumeVideo(tVideoHndl hVideo);
	VTABLE_EXPORT Boolean 		IsVideoPaused(tVideoHndl hVideo);
	VTABLE_EXPORT Boolean 		IsVideoPlaying(tVideoHndl hVideo);
	VTABLE_EXPORT Boolean 		IsVideoLooped(tVideoHndl hVideo);

private:
	CDebugMPI			dbg_;

	// Limit object creation to the Module Manager interface functions
	CVideoModule();
	virtual ~CVideoModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));
	
	// Implementation-specific functionality
    tVideoHndl			StartVideoInt(const CPath& path);
    Boolean				InitVideoInt(tVideoHndl hVideo);
    void				DeInitVideoInt(tVideoHndl hVideo);
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_VIDEOPRIV_H

// EOF
