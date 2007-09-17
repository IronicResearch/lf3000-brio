#ifndef LF_BRIO_PLATFORMPRIV_H
#define LF_BRIO_PLATFORMPRIV_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		PlatformPriv.h
//
// Description:
//		Defines the interface for the private underlying Platform manager module. 
//
//==============================================================================
#include <SystemTypes.h>
#include <CoreModule.h>
#include <EventTypes.h>
#include <PlatformTypes.h>
#include <DebugMPI.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
// Constants
//==============================================================================
const CString			kPlatformModuleName	= "Platform";
const tVersion			kPlatformModuleVersion	= 2;
const tEventPriority	kPlatformEventPriority	= 0;	//TBD/tp: make replacable?


//==============================================================================
class CPlatformModule : public ICoreModule {
public:	
	// ICoreModule functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	VTABLE_EXPORT tPlatformData	GetPlatformState() const;

private:
	void				InitModule( );
	void				DeinitModule();
	CDebugMPI			dbg_;

	// Limit object creation to the Module Manager interface functions
	CPlatformModule();
	virtual ~CPlatformModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_PLATFORMPRIV_H

// eof
