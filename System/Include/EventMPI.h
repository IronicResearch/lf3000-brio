#ifndef LF_BRIO_EVENTMGRMPI_H
#define LF_BRIO_EVENTMGRMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		EventMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the Event module. 
//
//==============================================================================

#include <SystemTypes.h>
#include <CoreMPI.h>
#include <EventListener.h>
#include <EventMessage.h>


// Types
typedef U32		tListenerId;
typedef U32		tEventRegistrationFlags;
typedef U8		tEventPriority;

const tListenerId kNoListener = (tListenerId)(0);


//==============================================================================
class CEventMPI : public ICoreMPI {
public:	
	// core functionality
	virtual	Boolean		IsValid() const;
	virtual tErrType	GetMPIVersion(tVersion &version) const;		   
	virtual tErrType	GetMPIName(ConstPtrCString &pName) const;		
	virtual tErrType	GetModuleVersion(tVersion &version) const;
	virtual tErrType	GetModuleName(ConstPtrCString &pName) const;	
	virtual tErrType	GetModuleOrigin(ConstPtrCURI &pURI) const;

	// class-specific functionality
	CEventMPI();
	virtual ~CEventMPI();

	// Register & unregister listener chains
	tErrType	RegisterEventListener(const IEventListener *pListener,
										tEventRegistrationFlags flags = 0);
	tErrType	UnregisterEventListener(const IEventListener *pListener);
	
	// Generate an event
	tErrType	PostEvent(const IEventMessage &msg, 
						tEventPriority priority,
						const IEventListener *pResponseListener = NULL) const;
private:
	class CEventModule*	mpModule;
};


#endif // LF_BRIO_EVENTMGRMPI_H

// eof
