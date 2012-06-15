#ifndef LF_BRIO_ACCELEROMETER_EVENT_QUEUE_H
#define LF_BRIO_ACCELEROMETER_EVENT_QUEUE_H

//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		AccelerometerEventQueue.h
//
// Description:
//		Defines utility class CAccelerometerEventQueue.
//		Register this event listener, and it will queue up accelerometer events.
//		Regularly use GetQueue() at a time of convenience to process the events.
//		Remember to unregister it when you don't want to queue up more events.
//
//==============================================================================

#include <EventListener.h>
#include <KernelMPI.h>
#include <AccelerometerTypes.h>
#include <vector>
LF_BEGIN_BRIO_NAMESPACE()

/// \class CAccelerometerEventQueue
///
/// Helper class for polling accelerometer events.
///
/// This class exists as a simplified way to poll for Accelerometer Events and not
/// have to worry about missing anything.
///
/// Create an instance of this class either on the stack or the heap.
/// Register it as a listener.
///
/// When it's time to check for accelerometer data, call GetQueue(), then process
/// the events in the queue.  Most often there will only be one event in
/// the queue, but if your update loop is slow there will be more. Timestamps
/// are already present in each input data record.
///
/// When you're done, unregister the listener.
///
/// Make sure if you're not listening to events to unregister the listener
/// temporarily (eg, when paused), or else the events during pause will
/// show up in the list. Even if you do an extra GetQueue(), the vector growth
/// will stay.

//==============================================================================
class CAccelerometerEventQueue : public IEventListener
{
public:
	CAccelerometerEventQueue();
	~CAccelerometerEventQueue();

	/// Returns pointer to front vector of Accelerometer events
	///
	/// Use this to iterate through the events that have occured.
	/// This automatically clears the other vector, and all future events
	/// will go into that vector (double buffer).
	///
	/// \warning DO NOT CALL DELETE on this pointer
	std::vector<tAccelerometerData> *GetQueue();

	tEventStatus Notify(const IEventMessage &msgIn);
private:
	CKernelMPI kernel_;
	tMutex mutex_;
	std::vector<tAccelerometerData> eventVector_[2];
	int front_;
};

/// Alternate constructor for queue which consumes input events,
/// instead of propagating its events to other listeners.
class CAccelerometerEventQueue2 : public CAccelerometerEventQueue
{
	tEventStatus Notify(const IEventMessage &msgIn) {
		CAccelerometerEventQueue::Notify(msgIn);
		return kEventStatusOKConsumed;
	}
};

LF_END_BRIO_NAMESPACE()
#endif //LF_BRIO_ACCELEROMETER_EVENT_QUEUE_H
