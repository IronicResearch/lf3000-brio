#ifndef LF_BRIO_EVENTMGRMODULEPRIV_H
#define LF_BRIO_EVENTMGRMODULEPRIV_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		EventPriv.h
//
// Description:
//		Defines the interface for the private underlying Event module. 
//
//==============================================================================

#include <SystemTypes.h>
#include <CoreModule.h>
#include <DebugMPI.h>
#include "EventMPI.h"	// for tEventRegistrationFlags
#include <KernelMPI.h>

LF_BEGIN_BRIO_NAMESPACE()


// Constants
const CString	kEventModuleName	= "Event";
const tVersion	kEventModuleVersion	= 2;

// Function prototypes

//==============================================================================
class CEventModule : public ICoreModule {
public:	
	// core functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// Register & unregister listener chains
	VTABLE_EXPORT tErrType	RegisterEventListener(const IEventListener *pListener,
												tEventRegistrationFlags flags );
	VTABLE_EXPORT tErrType	UnregisterEventListener(const IEventListener *pListener);
	
	// Generate an event
	VTABLE_EXPORT tErrType	PostEvent(const IEventMessage &msg, 
									tEventPriority priority, 
									const IEventListener *pListener) const;
							
	// Hook in EventListener replacibility here
	VTABLE_EXPORT CEventListenerImpl* GenerateEventListenerImpl(
													const tEventType* pTypes, 
													U32 count) const;
													
private:
	// Limit object creation to the Module Manager interface functions
	CEventModule();
	virtual ~CEventModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void			::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));

public:
	static void* 		ButtonPowerUSBTask( void* arg );
	CDebugMPI			debug_;
	CKernelMPI			kernel_;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_EVENTMGRMODULEPRIV_H

// eof
