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
#include <TouchTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
class CButtonMPI : public ICoreMPI {
	/// \class CButtonMPI
	///
	/// Button manager class for posting CButtonMessage events on button 
	/// presses or releases to registered listeners of kGroupButton type. 
	/// Button on/off states and transitions are bitwise encoded in 
	/// tButtonData fields, with time-stamp info logged to additional 
	/// tButtonData2 fields.
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
	
	/// register handler
	tErrType RegisterEventListener(const IEventListener *pListener);
	/// unregister handler
	tErrType UnregisterEventListener(const IEventListener *pListener);
	
	/// Get button state
	tButtonData		GetButtonState() const;
	tButtonData2	GetButtonState2() const;

	/// Get touch sample rate (Hz)
	U32				GetTouchRate() const;

	/// Set touch sample rate (Hz)
	/// Values are set to the nearest value of 100Hz/1, 100Hz/2, 100Hz/3, etc.
	/// The values that possible are:
	/// [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 16, 20, 25, 33, 50, 100]
	tErrType		SetTouchRate(U32 rate);

private:
	class CButtonModule*	pModule_;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_BUTTONMPI_H

// eof
