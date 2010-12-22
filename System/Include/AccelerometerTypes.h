#ifndef LF_BRIO_ACCELEROMETERTYPES_H
#define LF_BRIO_ACCELEROMETERTYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		AccelerometerTypes.h
//
// Description:
//		Defines types for the Accelerometer module.
//
//==============================================================================

#include <EventMessage.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================	   
// Accelerometer events
//==============================================================================
#define ACCELEROMETER_EVENTS					\
	(kAccelerometerDataChanged)					\
	(kOrientationChanged)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupAccelerometer), ACCELEROMETER_EVENTS)

const tEventType kAllAccelerometerEvents = AllEvents(kGroupAccelerometer);


//==============================================================================	   
// Accelerometer errors
//==============================================================================
#define ACCELEROMETER_ERRORS				\
	(kAccelerometerEmulationConfigErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupAccelerometer), ACCELEROMETER_ERRORS)


//==============================================================================	   
// Accelerometer defines
//==============================================================================
const U32	kAccelerometerRateMin		= 1;	///< minimum sample rate (Hz)
const U32	kAccelerometerRateMax		= 100;	///< maximum sample rate (Hz)
const U32	kAccelerometerRateDefault	= 10;	///< default sample rate (Hz)

/// Enumerated type for SetAccelerometerMode()/GetAccelerometerMode()
enum tAccelerometerMode {
	kAccelerometerModeDisabled,					///< disabled sampling mode
	kAccelerometerModeContinuous,				///< continuous data sampling mode
	kAccelerometerModeOneShot,					///< one-shot data sampling mode
	kAccelerometerModeOrientation				///< orientation mode
};

//==============================================================================
// Accelerometer data types
//==============================================================================
//------------------------------------------------------------------------------
struct tAccelerometerData {
	S32 accelX;
	S32 accelY;
	S32 accelZ;
	struct timeVal {
		S32 seconds;
		S32 microSeconds;
	} time;
};

//------------------------------------------------------------------------------
class CAccelerometerMessage : public IEventMessage {
public:
	CAccelerometerMessage( const tAccelerometerData& data );
	virtual U16			GetSizeInBytes() const;
	tAccelerometerData 	GetAccelerometerData() const;
private:
	tAccelerometerData 	mData;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_ACCELEROMETERTYPES_H

// eof
