#ifndef LF_BRIO_HWCONTROLLER_EVENT_QUEUE_H_
#define LF_BRIO_HWCONTROLLER_EVENT_QUEUE_H_

//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		HWControllerEventQueue.h
//
// Description:
//		Defines utility class HWControllerEventQueue.
//		Register this event listener, and it will queue up all the controller events.
//		Regularly use popQueue at a time of convenience to process the events.
//		Remember to unregister it when you don't want to queue up button events.
//
//==============================================================================

#include <EventListener.h>
#include <KernelMPI.h>
#include <vector>
#include <Hardware/HWControllerEventMessage.h>

LF_BEGIN_BRIO_NAMESPACE()

/// \class HWControllerEventQueue
/// NOTE: For use with LeapTV applications ONLY.
/// Helper class for polling controller events.
///
/// This class exists as a simplified way to poll for controller events and not
/// have to worry about missing anything.
///
/// Create an instance of this class either on the stack or the heap.
/// Register it as a listener.
///
/// When it's time to check for controller events, call GetQueue().  Process the
/// events in the queue.  Most often there will only be one event in
/// the queue, but if you're update loop is slow there will be more.  If you
/// need to know the timing of the event, the timestamps are included.
///
/// When you're done, unregister the listener.
///
/// Make sure if you're not listening to events to unregister the listener
/// temporarily (eg when paused).  Or else the events during pause will
/// show up in the list.  Even if you do an extra GetQueue(), the vector growth
/// will stay.

//==============================================================================
class HWControllerEventQueue : public IEventListener
{
public:
	HWControllerEventQueue();
	~HWControllerEventQueue();

	/// Returns pointer to front vector of controller events
	///
	/// Use this to iterate through the events that have occurred.
	/// This automatically clears the other vector, and all future events
	/// will go into that vector (double buffer).
	///
	/// \warning DO NOT CALL DELETE on this pointer
	std::vector<LF::Hardware::HWControllerEventMessage> *GetQueue();

	tEventStatus Notify(const IEventMessage &msgIn);
private:
	CKernelMPI kernel_;
	tMutex mutex_;
	std::vector<LF::Hardware::HWControllerEventMessage> eventVector_[2];
	int front_;
};

/// Alternate constructor for queue which consumes input events,
/// instead of propagating its events to other listeners.
class HWControllerEventQueue2 : public HWControllerEventQueue
{
	tEventStatus Notify(const IEventMessage &msgIn) {
		HWControllerEventQueue::Notify(msgIn);
		return kEventStatusOKConsumed;
	}
};

LF_END_BRIO_NAMESPACE()

#endif /* LF_BRIO_HWCONTROLLER_EVENT_QUEUE_H_ */
