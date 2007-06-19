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
	VTABLE_EXPORT Boolean 		PutVideoFrame(tVideoHndl hVideo, void* pCtx);

private:
	CDebugMPI			dbg_;

	// Limit object creation to the Module Manager interface functions
	CVideoModule();
	virtual ~CVideoModule();
	friend ICoreModule*	::CreateInstance(tVersion version);
	friend void			::DestroyInstance(ICoreModule*);
	
	// Implementation-specific functionality
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_VIDEOPRIV_H

// EOF
