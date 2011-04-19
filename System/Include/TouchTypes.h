#ifndef LF_BRIO_TOUCHTYPES_H
#define LF_BRIO_TOUCHTYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		TouchTypes.h
//
// Description:
//		Defines types for the Touch Manager module. 
//
//==============================================================================

#include <EventMessage.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <SystemTypes.h>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================	   
// Touch events
//==============================================================================
#define TOUCH_EVENTS					\
	(kTouchStateChanged)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupTouch), TOUCH_EVENTS)

const tEventType kAllTouchEvents = AllEvents(kGroupTouch);


//==============================================================================	   
// Touch errors
//==============================================================================
#define TOUCH_ERRORS				\
	(kTouchEmulationConfigErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupTouch), TOUCH_ERRORS)


//==============================================================================	   
// Touch defines
//==============================================================================
const U32	kTouchRateMin		= 1;	///< minimum touch rate (Hz)
const U32	kTouchRateMax		= 100;	///< maximum touch rate (Hz)
const U32	kTouchRateDefault	= 50;	///< default touch rate (Hz)

/// Enumerated type for SetTouchMode()/GetTouchMode()
enum tTouchMode {
	kTouchModeDefault,					///< default touch mode
	kTouchModeDrawing,					///< drawing touch mode
	kTouchModeCustom,					///< custom touch mode
	kTouchModePressure					///< pressure touch mode
};

/// Enumerated type for SetTouchParam()/GetTouchParam()
enum tTouchParam {
	kTouchParamSampleRate,				///< sample rate (Hz)
	kTouchParamDebounceDown,			///< debounce stylus down (samples)
	kTouchParamDebounceUp				///< debounce stylus up (samples)
};

// Table of tTouchParam parameters for Default touch mode
const U32	kTouchTableDefault[] = {
	50,								// sample rate (Hz)
	1,									// debounce stylus down (samples)
	1									// debounce stylus up (samples)
};

// Table of tTouchParam parameters for Drawing touch mode
const U32	kTouchTableDrawing[] = {
	100,								// sample rate (Hz)
	1,									// debounce stylus down (samples)
	1									// debounce stylus up (samples)
};

//==============================================================================	   
// Touch types
//==============================================================================
//------------------------------------------------------------------------------
struct tTouchData {
	U16 touchState;// 0=no touch, 1=touch (legacy); or = touch pressure (kTouchModePressure)
	S16 touchX, touchY;// Position in screen coords
	struct timeVal {
		S32 seconds;
		S32 microSeconds;
	} time;
};

//------------------------------------------------------------------------------
class CTouchMessage : public IEventMessage {
public:
	CTouchMessage( const tTouchData& data );
	virtual U16	GetSizeInBytes() const;
	tTouchData GetTouchState() const;
private:
	tTouchData	mData;
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_TOUCHTYPES_H

// eof
