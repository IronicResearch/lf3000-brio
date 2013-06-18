#ifndef LF_BRIO_ACCELEROMETERMPI_H
#define LF_BRIO_ACCELEROMETERMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		AccelerometerMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the Accelerometer module.
//
//==============================================================================

#include <CoreMPI.h>
#include <AccelerometerTypes.h>
#include <EventMPI.h>
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
class CAccelerometerMPI : public ICoreMPI {
	/// \class CAccelerometerMPI
	///
	/// Accelerometer class to support posting CAccelerometerMessage events
	/// to registered listeners of kGroupAccelerometer type. Options to get/set
	/// sampling data rate and modes.
public:	
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;		
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	CAccelerometerMPI();
	virtual ~CAccelerometerMPI();
	
	/// Register event listener (via EventMPI) and enable continuous sampling mode
	tErrType 				RegisterEventListener(const IEventListener *pListener);

	/// Unregister event listener (via EventMPI) and disable all sampling
	tErrType 				UnregisterEventListener(const IEventListener *pListener);
	
	/// Returns true if an accelerometer device is present
	Boolean					IsAccelerometerPresent();

	/// Get accelerometer data
	tAccelerometerData		GetAccelerometerData() const;

	/// Get accelerometer orientation
	/// Returns the current orientation of the device's screen as a tDisplayOrientation
	/// type value. SetAccelerometerMode must have been previously called with
	/// kAccelerometerModeOrientation to guarantee accuracy of this function.
	S32						GetOrientation() const;

	/// Get accelerometer sample rate (Hz)
	U32						GetAccelerometerRate() const;

	/// Set accelerometer sample rate (Hz)
	/// Values are set to the nearest value of 100Hz/1, 100Hz/2, 100Hz/3, etc.
	/// [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 16, 20, 25, 33, 50, 100]
	tErrType				SetAccelerometerRate(U32 rate);

	/// Get accelerometer mode
	/// Disabled, Continuous sampling, One-shot sampling, Orientation changes
	tAccelerometerMode		GetAccelerometerMode() const;

	/// Set accelerometer mode
	tErrType				SetAccelerometerMode(tAccelerometerMode mode);
	
	/// Get the driver bias
	/// that is subtracted from each sample in the driver for calibration purposes
	tErrType				GetAccelerometerBias(S32& xoffset, S32& yoffset, S32& zoffset);
	
	/// Set the bias
	/// that is subtracted from each sample in the driver for calibration purposes
	///
	/// Note: The accelerometer bias is normally set by the driver based on
	/// manufacturing calibration data.
	///
	/// If a developer wishes to use their own calibration routine, they should
	/// zero these bias values before collecting calibration data in order to avoid
	/// cummulative errors when setting new bias values.
	///
	/// \return Returns kNoErr on success.
	tErrType				SetAccelerometerBias(S32 xoffset, S32 yoffset, S32 zoffset);
	
private:
	class CAccelerometerModule*	pModule_;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_ACCELEROMETERMPI_H

// eof
