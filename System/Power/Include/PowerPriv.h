#ifndef LF_BRIO_POWERPRIV_H
#define LF_BRIO_POWERPRIV_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		PowerPriv.h
//
// Description:
//		Defines the interface for the private underlying Power manager module. 
//
//==============================================================================
#include <SystemTypes.h>
#include <CoreModule.h>
#include <EventTypes.h>
#include <PowerTypes.h>
#include <DebugMPI.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
// Constants
//==============================================================================
const CString			kPowerModuleName	= "Power";
const tVersion			kPowerModuleVersion	= 2;
const tEventPriority	kPowerEventPriority	= 0;	//TBD/tp: make replacable?


//==============================================================================
class CPowerModule : public ICoreModule {
public:	
	// ICoreModule functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	VTABLE_EXPORT tPowerData	GetPowerState() const;

private:
	void				InitModule( );
	void				DeinitModule();
	CDebugMPI			dbg_;

	// Limit object creation to the Module Manager interface functions
	CPowerModule();
	virtual ~CPowerModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_POWERPRIV_H

// eof
