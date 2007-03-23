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
#include <EventMgrMPI.h>
#include <StringTypes.h>


// Constants
const CString	kEventMgrModuleName		= "Event";
const tVersion	kEventMgrModuleVersion	= MakeVersion(0,1);

//==============================================================================
class CEventMgrModule : public ICoreModule {
public:	
	// core functionality
	virtual Boolean		IsValid() const;
	virtual tErrType	GetModuleVersion(tVersion &version) const;
	virtual tErrType	GetModuleName(ConstPtrCString &pName) const;	
	virtual tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;

	// class-specific functionality
	CEventMgrModule();
	virtual ~CEventMgrModule();

	// Register & unregister listener chains
	tErrType	RegisterEventListener(const IEventListener *pListener,
										tEventRegistrationFlags flags );
	tErrType	RegisterResponseEventListener(tListenerId &id, 
										const IEventListener *pListener,
										tEventRegistrationFlags flags);
	tErrType	UnregisterEventListener(const IEventListener *pListener);
	
	// Generate an event
	tErrType	PostEvent(const IEventMessage &msg, 
							tEventPriority priority, 
							const IEventListener *pListener,
							tListenerId id) const;
};


#endif // LF_BRIO_EVENTMGRMODULEPRIV_H

// eof
