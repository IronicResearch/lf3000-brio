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
#include <EventTypes.h>
#include <VideoTypes.h>
#include <DebugMPI.h>
#include <ResourceTypes.h>

LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
// Constants
//==============================================================================
const CString			kVideoModuleName	= "Video";
const tVersion			kVideoModuleVersion	= 2;
const tEventPriority	kVideoEventPriority	= 0;

//==============================================================================
// Typedefs
//==============================================================================


//==============================================================================
class CVideoModule : public ICoreModule {
public:	
	// ICoreModule functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
    VTABLE_EXPORT tVideoHndl	StartVideo(tRsrcHndl hRsrc);
    VTABLE_EXPORT Boolean     	StopVideo(tVideoHndl hVideo);
	VTABLE_EXPORT Boolean 		GetVideoFrame(tVideoHndl hVideo, void* pCtx);
	VTABLE_EXPORT Boolean 		PutVideoFrame(tVideoHndl hVideo, tVideoSurf* pCtx);
	VTABLE_EXPORT Boolean 		GetVideoInfo(tVideoHndl hVideo, tVideoInfo* pInfo);
	VTABLE_EXPORT Boolean 		GetVideoTime(tVideoHndl hVideo, tVideoTime* pTime);
	VTABLE_EXPORT Boolean 		SyncVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx, Boolean bDrop);
	VTABLE_EXPORT Boolean 		SeekVideoFrame(tVideoHndl hVideo, tVideoTime* pCtx);

private:
	CDebugMPI			dbg_;

	// Limit object creation to the Module Manager interface functions
	CVideoModule();
	virtual ~CVideoModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));
	
	// Implementation-specific functionality
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_VIDEOPRIV_H

// EOF
