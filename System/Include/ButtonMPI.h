#ifndef LF_BRIO_BUTTONMPI_H
#define LF_BRIO_BUTTONMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		ButtonMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the Button Manager module. 
//
//==============================================================================

#include <CoreMPI.h>
#include <ButtonTypes.h>
#include <EventMPI.h>
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
class CButtonMPI : public ICoreMPI {
public:	
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;		
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	CButtonMPI();
	virtual ~CButtonMPI();
	
	// register handler
	tErrType RegisterEventListener(const IEventListener *pListener);
	// unregister handler
	tErrType UnregisterEventListener(const IEventListener *pListener);
	
	// Get button state
	tButtonData		GetButtonState() const;
	tButtonData2	GetButtonState2() const;

private:
	class CButtonModule*	pModule_;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_BUTTONMPI_H

// eof
