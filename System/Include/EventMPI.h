#ifndef LF_BRIO_EVENTMPI_H
#define LF_BRIO_EVENTMPI_H
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
#include <EventTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
class CEventMPI : public ICoreMPI {
public:	
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;		
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

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
	class CEventModule*	pModule_;

	// Disable copy semantics
	CEventMPI(const CEventMPI&);
	CEventMPI& operator=(const CEventMPI&);
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_EVENTMGRMPI_H

// eof
