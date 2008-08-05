#ifndef LF_BRIO_TIMER_H
#define LF_BRIO_TIMER_H

//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		Timer.h
//
// Description:
//		CTimer class that integrates KernelMPI timers with the Event Manager.
//
//==============================================================================

// System includes
#include <SystemTypes.h>
#include <KernelMPI.h>
#include <EventMPI.h>

#include <iostream>
using namespace std;

LF_BEGIN_BRIO_NAMESPACE()


#define kTimerEventPriority 128	// asynchronous event message for min impact


//==============================================================================	   
// Timer events
//==============================================================================
#define TIMER_EVENTS					\
	(kTimerFiredEvent)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupTimer), TIMER_EVENTS)

const tEventType kAllTimerEvents = AllEvents(kGroupTimer);


//==============================================================================	   
// CTimerMessage
//==============================================================================
//------------------------------------------------------------------------------
class CTimerMessage : public IEventMessage
{
public:
	explicit CTimerMessage(tTimerHndl hndl) : 
				IEventMessage(kTimerFiredEvent), hndl_(hndl) {}
	virtual U16	GetSizeInBytes() const	{ return sizeof(CTimerMessage); }
	tTimerHndl	GetTimerHndl() const	{ return hndl_; }
private:
	tTimerHndl	hndl_;
};



//==============================================================================	   
// COneShotTimer
//==============================================================================
//static void TimerCallback(tTimerHndl hndl);

//static tTimerHndl tHndl_1;

//const static tTimerProperties props = {TIMER_RELATIVE_SET,
//										{0, 0, 0, 0},
//			                            TimerCallback,
//			                            &tHndl_1};

class COneShotTimer
{
public:
	//--------------------------------------------------------------------------
	explicit COneShotTimer(const tTimerProperties& props)
	{
 		timer_ = kernel_.CreateTimer(TimerCallback, props, (const char *)0 );
		if (timer_ == kInvalidTimerHndl)
			throw ("CTimer creation failed");
	}
	
	//--------------------------------------------------------------------------
	~COneShotTimer()
	{
		kernel_.DestroyTimer(timer_);
	}
	
	//--------------------------------------------------------------------------
	tTimerHndl GetTimerHndl() const
	{
		return timer_;
	}
	
	//--------------------------------------------------------------------------
	void Start(const tTimerProperties& props)
	{
		// Calling Start() in succession restarts the timer with its
		// original timer period.
		kernel_.StartTimer(timer_, props);
	}
	
	//--------------------------------------------------------------------------
	void Pause(saveTimerSettings& saveValue)
	{
		kernel_.PauseTimer(timer_, saveValue);
	}
	
	//--------------------------------------------------------------------------
	void Resume(saveTimerSettings& saveValue)
	{
		kernel_.ResumeTimer(timer_,  saveValue);
	}
	
	//--------------------------------------------------------------------------
	tErrType GetTimerElapsedTimeInMilliSec(U32* ms) const
	{
    	tErrType err = kNoErr;

		err = kernel_.GetTimerElapsedTime(timer_, ms);
		return err;
	}
	
	//--------------------------------------------------------------------------
	tErrType GetTimerRemainingTimeInMilliSec(U32* ms) const
	{
    	tErrType err = kNoErr;

		err = kernel_.GetTimerRemainingTime(timer_, ms);
		return err;
	}
	
	//--------------------------------------------------------------------------
private:

	// Callback method declared static to avoid C++ namespace warnings 
	static void TimerCallback(tTimerHndl hndl) // 
	{
		// Event MPI object needs to be persistent, however this method is static
		static CEventMPI	event;
		CTimerMessage	msg(hndl);
		event.PostEvent(msg, kTimerEventPriority );
	}
	
	//--------------------------------------------------------------------------
	CKernelMPI	kernel_;
	tTimerHndl  timer_;
};

//==============================================================================
// Basic audio types
//==============================================================================


LF_END_BRIO_NAMESPACE()	
#endif		// LF_BRIO_TIMER_H

// EOF	
