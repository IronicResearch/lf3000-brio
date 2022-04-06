#ifndef LF_BRIO_BUTTON_EVENT_QUEUE_H
#define LF_BRIO_BUTTON_EVENT_QUEUE_H

//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		ButtonEventQueue.h
//
// Description:
//		Defines utility class CButtonEventQueue.
//		Register this event listener, and it will queue up all the button events.
//		Regularly use popQueue at a time of convenience to process the events.
//		Remember to unregister it when you don't want to queue up button events.
//
//==============================================================================

#include <EventListener.h>
#include <KernelMPI.h>
#include <vector>
#include <ButtonTypes.h>
LF_BEGIN_BRIO_NAMESPACE()

/// \class CButtonEventQueue
/// 
/// Helper class for polling button events.
///
/// This class exists as a simplified way to poll for button presses and not
/// have to worry about missing anything.
///
/// Create an instance of this class either on the stack or the heap.
/// Register it as a listener.
///
/// When it's time to check for button presses, call GetQueue().  Process the
/// button events in the queue.  Most often there will only be one event in
/// the queue, but if you're update loop is slow there will be more.  If you
/// need to know the timing of the button presses, the timestamps are included.
///
/// When you're done, unregister the listener.
///
/// Make sure if you're not listening to button events to unregister the listener
/// temporarily (eg when paused).  Or else the buttons pressed during pause will
/// show up in the list.  Even if you do an extra GetQueue(), the vector growth
/// will stay.

//==============================================================================
class CButtonEventQueue : public IEventListener
{
public:
	CButtonEventQueue();
	~CButtonEventQueue();
	
	/// Returns pointer to front vector of button events
	///
	/// Use this to iterate through the events that have occured.
	/// This automatically clears the other vector, and all future events
	/// will go into that vector (double buffer).
	///
	/// \warning DO NOT CALL DELETE on this pointer
	std::vector<tButtonData2> *GetQueue();

	tEventStatus Notify(const IEventMessage &msgIn);
private:
	CKernelMPI kernel_;
	tMutex mutex_;
	std::vector<tButtonData2> eventVector_[2];
	int front_;
};

/// Alternate constructor for queue which consumes input events,
/// instead of propagating its events to other listeners.
class CButtonEventQueue2 : public CButtonEventQueue
{
	tEventStatus Notify(const IEventMessage &msgIn) {
		CButtonEventQueue::Notify(msgIn);
		return kEventStatusOKConsumed;
	}
};

LF_END_BRIO_NAMESPACE()	
#endif //LF_BRIO_BUTTON_EVENT_QUEUE_H
