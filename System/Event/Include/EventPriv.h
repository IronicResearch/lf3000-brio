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
#include "EventMPI.h"	// for tEventRegistrationFlags
LF_BEGIN_BRIO_NAMESPACE()


// Constants
const CString	kEventModuleName	= "Event";
const tVersion	kEventModuleVersion	= MakeVersion(0,1);

//==============================================================================
class CEventModule : public ICoreModule {
public:	
	// core functionality
	virtual Boolean		IsValid() const;
	virtual tErrType	GetModuleVersion(tVersion &version) const;
	virtual tErrType	GetModuleName(ConstPtrCString &pName) const;	
	virtual tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;

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
	friend ICoreModule*	::CreateInstance(tVersion version);
	friend void			::DestroyInstance(ICoreModule*);
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_EVENTMGRMODULEPRIV_H

// eof
