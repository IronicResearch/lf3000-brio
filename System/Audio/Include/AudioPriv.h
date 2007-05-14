#ifndef LF_BRIO_AUDIOPRIVATE_H
#define LF_BRIO_AUDIOPRIVATE_H
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		AudioPriv.h
//
// Description:
//		Defines the private, hidden data structures used by AudioMgrMPI. 
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <StringTypes.h>
#include <CoreModule.h>
#include <KernelMPI.h>
#include <DebugMPI.h>
#include <AudioTask.h>
#include <AudioMsg.h>

//#include <RsrcTypes.h>
//#include <AudioTypes.h>
//#include <AudioRsrcs.h>
//#include <AudioMixer.h>
//#include <EventHandler.h>
LF_BEGIN_BRIO_NAMESPACE()


class IEventListener;

typedef U32 tRsrcHndl;

// Constants
const CString	kAudioModuleName	= "Audio";
const tVersion	kAudioModuleVersion	= 2;


//==============================================================================
class CAudioModule : public ICoreModule {
public:	
	// core functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// Overall Audio Control
	VTABLE_EXPORT tErrType	StartAudio( void );
	VTABLE_EXPORT tErrType	StopAudio( void );
	VTABLE_EXPORT tErrType	PauseAudio( void );
	VTABLE_EXPORT tErrType	ResumeAudio( void );

	VTABLE_EXPORT tAudioID PlayAudio( tRsrcHndl				hRsrc, 
										U8					volume, 
										tAudioPriority		priority,
										S8					pan, 
										IEventListener		*pHandler,
										tAudioPayload		payload,
										tAudioOptionsFlags	flags );

private:
	CKernelMPI* KernelMPI;
	CDebugMPI* DebugMPI;	
	tMessageQueueHndl hRecvMsgQueue_;
	tMessageQueueHndl hSendMsgQueue_;
	
	void SendCmdMessage( CAudioMsg& msg );
	tAudioID WaitForAudioID( void );
	
	// Limit object creation to the Module Manager interface functions
	CAudioModule();
	virtual ~CAudioModule();
	friend ICoreModule*	::CreateInstance(tVersion version);
	friend void			::DestroyInstance(ICoreModule*);
};

LF_END_BRIO_NAMESPACE()	
#endif		// LF_BRIO_AUDIOPRIVATE_H

// EOF	
