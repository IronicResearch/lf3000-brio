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
//		Defines the interface for the private underlying Video module. 
//
//==============================================================================

#include <SystemTypes.h>
#include <VideoTypes.h>
#include <CoreMPI.h>
#include <ResourceTypes.h>

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
    tVideoHndl	StartVideo(tRsrcHndl hRsrc);
	Boolean 	StopVideo(tVideoHndl hVideo);
	Boolean 	GetVideoFrame(tVideoHndl hVideo, void* pCtx);
	Boolean 	PutVideoFrame(tVideoHndl hVideo, void* pCtx);
 
private:
	class CVideoModule*	pModule_;
	U32					id_;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_VIDEOMPI_H

// EOF
