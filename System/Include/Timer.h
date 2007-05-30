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

#define kFoo 20
#define kTimerEventPriority 21

//==============================================================================	   
// CTimerMessage
//==============================================================================
//------------------------------------------------------------------------------
class CTimerMessage : public IEventMessage
{
public:
	explicit CTimerMessage(tTimerHndl hndl) : 
				IEventMessage(kFoo, 0), hndl_(hndl) {}
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

//const static tTimerProperties props = {TIMER_ABSTIME_SET,
//										{0, 0, 0, 0},
//			                            TimerCallback,
//			                            &tHndl_1};

class COneShotTimer
{
public:
	//--------------------------------------------------------------------------
	explicit COneShotTimer(const tTimerProperties& props)
//	COneShotTimer(const tTimerProperties& props)
	{
 		pfnTimerCallback pfn = TimerCallback; // FIXME Original TP
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
	U32 GetTimerElapsedTimeInMilliSec() const
	{
		return kernel_.GetTimerElapsedTime(timer_);
	}
	
	//--------------------------------------------------------------------------
	U32 GetTimerRemainingTimeInMilliSec() const
	{
		return kernel_.GetTimerRemainingTime(timer_);
	}
	
// TPprivate:
	//--------------------------------------------------------------------------
private:
	static void TimerCallback(tTimerHndl hndl) // 
	{
		CEventMPI	event;
		CTimerMessage	msg(hndl);
		event.PostEvent(msg, kTimerEventPriority );
#if 0 // FIXME/BSK
//		printf("TimerCallback function was called Timer tTimerHndl=0x%x \n", hndl);  		
		timeval timePrint;
		gettimeofday( &timePrint, NULL );
		printf("%d.%d   TimerCallback  \n", timePrint.tv_sec, timePrint.tv_usec / 1000 );
		fflush(stdout);
#endif
//tErrType PostEvent(const IEventMessage &msg,
// 				tEventPriority priority, const IEventListener *pResponse)
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
