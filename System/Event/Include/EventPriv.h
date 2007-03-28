#ifndef LF_BRIO_EVENTMGRMODULEPRIV_H
#define LF_BRIO_EVENTMGRMODULEPRIV_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		EventMgrModulePriv.h
//
// Description:
//		Defines the interface for the private underlyingEventMgr module. 
//
//==============================================================================

#include <CoreModule.h>
#include "EventMPI.h"	// for tEventRegistrationFlags


// Constants
const CString	kEventMgrModuleName		= "Event";
const tVersion	kEventMgrModuleVersion	= MakeVersion(0,1);

//==============================================================================
class CEventModule : public ICoreModule {
public:	
	// core functionality
	virtual Boolean		IsValid() const;
	virtual tErrType	GetModuleVersion(tVersion &version) const;
	virtual tErrType	GetModuleName(ConstPtrCString &pName) const;	
	virtual tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;

	// class-specific functionality
	CEventModule();
	virtual ~CEventModule();

	// Register & unregister listener chains
	virtual tErrType	RegisterEventListener(const IEventListener *pListener,
										tEventRegistrationFlags flags );
	virtual tErrType	UnregisterEventListener(const IEventListener *pListener);
	
	// Generate an event
	virtual tErrType	PostEvent(const IEventMessage &msg, 
							tEventPriority priority, 
							const IEventListener *pListener) const;
							
	// Hook in EventListener replacibility here
	virtual CEventListenerImpl* GenerateEventListenerImpl(const tEventType* pTypes, 
													U32 count) const;
};


#endif // LF_BRIO_EVENTMGRMODULEPRIV_H

// eof
