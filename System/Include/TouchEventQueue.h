#ifndef LF_BRIO_TOUCH_EVENT_QUEUE_H
#define LF_BRIO_TOUCH_EVENT_QUEUE_H

//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		TouchEventQueue.h
//
// Description:
//		Defines utility class CTouchEventQueue.
//		Register this event listener, and it will queue up all the touch events.
//		Regularly use popQueue at a time of convenience to process the events.
//		Remember to unregister it when you don't want to queue up button events.
//
//==============================================================================

#include <EventListener.h>
#include <KernelMPI.h>
#include <vector>
#include <TouchTypes.h>
LF_BEGIN_BRIO_NAMESPACE()

/// \class CTouchEventQueue
/// 
/// Helper class for polling touch events.
///
/// This class exists as a simplified way to poll for touch, release, and move
/// events in a queue without missing anything.
///
/// Create an instance of this class either on the stack or the heap.
/// Register it as a listener.
///
/// When it's time to check for touch events, call GetQueue().  Process the
/// touch events in the queue.  Most often there will only be one event in
/// the queue, but if your update loop is slow there will be more.  If you
/// need to know the timing of the touch events, the timestamps are included.
///
/// When you're done, unregister the listener.
///
/// Make sure if you're not listening to touch events to unregister the listener
/// temporarily (eg when paused).  Or else the buttons pressed during pause will
/// show up in the list.  Even if you do an extra GetQueue(), the vector growth
/// will stay.

//==============================================================================
class CTouchEventQueue : public IEventListener
{
public:
	CTouchEventQueue();
	~CTouchEventQueue();
	
	//// Returns pointer to front vector of touch events
	///
	/// Use this to iterate through the events that have occured.
	/// This automatically clears the other vector, and all future events
	/// will go into that vector (double buffer).
	///
	/// \warning DO NOT CALL DELETE on this pointer
	std::vector<tTouchData> *GetQueue();
	
	tEventStatus Notify(const IEventMessage &msgIn);
private:
	CKernelMPI kernel_;
	tMutex mutex_;
	std::vector<tTouchData> eventVector_[2];
	int front_;
};

/// Alternate constructor for queue which consumes input events,
/// instead of propagating its events to other listeners.
class CTouchEventQueue2 : public CTouchEventQueue
{
	tEventStatus Notify(const IEventMessage &msgIn) {
		CTouchEventQueue::Notify(msgIn);
		return kEventStatusOKConsumed;
	}
};

LF_END_BRIO_NAMESPACE()
#endif //LF_BRIO_TOUCH_EVENT_QUEUE_H
