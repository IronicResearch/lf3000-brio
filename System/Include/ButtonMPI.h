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

	/// Register event listener via CEventMPI and enables continuous sampling mode.
	tErrType RegisterEventListener(const IEventListener *pListener);
	/// Unregister event listener via CEventMPI and disables all sampling.
	tErrType UnregisterEventListener(const IEventListener *pListener);

	/// Get button state (up or down) and transition.
	tButtonData		GetButtonState() const;
	/// Get button state (up or down), transition and a time stamp for the transition.
	tButtonData2	GetButtonState2() const;

	/// Get touch state (touch or no touch) and the x, y co-ordinates.
	tTouchData		GetTouchState() const;

	/// Get touch sample rate (Hz). This shows how often the touch state is checked.
	U32				GetTouchRate() const;

	/// Set touch sample rate (Hz). This sets how often the touch state is checked.
	/// Values are set to the nearest value of 50Hz/1, 50Hz/2, 50Hz/3, etc.
	/// Possible values are:
	/// [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 16, 20, 25, 33, 50]
	tErrType		SetTouchRate(U32 rate);

	/// Get touch mode (enum tTouchMode)
	tTouchMode		GetTouchMode() const;

	/// Set touch mode (enum tTouchMode)
	tErrType		SetTouchMode(tTouchMode mode);

	/// Get touch parameter value (enum tTouchParam)
	U32				GetTouchParam(tTouchParam param) const;

	/// Set touch parameter value (enum tTouchParam)
	tErrType		SetTouchParam(tTouchParam param, U32 value);

    /// Gets the orientation that events use to interpret the DPad buttons,
    /// either landscape (Kdpadlandscape), or portrait(Kdpadportrait).
	tDpadOrientation	GetDpadOrientation();

	/// Sets the orientation that events use to interpret the DPad buttons,
    /// either landscape (Kdpadlandscape), or portrait(Kdpadportrait).
	tErrType			SetDpadOrientation(tDpadOrientation dpad_orientation);

private:
	class CButtonModule*	pModule_;
};


LF_END_BRIO_NAMESPACE()
#endif // LF_BRIO_BUTTONMPI_H

// eof
