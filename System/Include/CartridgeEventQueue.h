#ifndef LF_BRIO_CARTRIDGE_EVENT_QUEUE_H
#define LF_BRIO_CARTRIDGE_EVENT_QUEUE_H

//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		CartridgeEventQueue.h
//
// Description:
//		Defines utility class CCartridgeEventQueue.
//		Register this event listener, and it will queue up all the cartridge events.
//		Regularly use popQueue at a time of convenience to process the events.
//		Remember to unregister it when you don't want to queue up cartridge events.
//
//==============================================================================

#include <EventListener.h>
#include <KernelMPI.h>
#include <vector>
#include <CartridgeTypes.h>
LF_BEGIN_BRIO_NAMESPACE()

/// \class CCartridgeEventQueue
///
/// Helper class for polling cartridge events.
///
/// This class exists as a simplified way to poll for Cartridge Events and not
/// have to worry about missing anything.
///
/// Create an instance of this class either on the stack or the heap.
/// Register it as a listener.
///
/// When it's time to check for cartridge presses, call GetQueue().  Process the
/// cartridge events in the queue.  Most often there will only be one event in
/// the queue, but if you're update loop is slow there will be more.  No timestamps
/// are provided.
///
/// When you're done, unregister the listener.
///
/// Make sure if you're not listening to cartridge events to unregister the listener
/// temporarily (eg when paused).  Or else the cartridge events during pause will
/// show up in the list.  Even if you do an extra GetQueue(), the vector growth
/// will stay.

//==============================================================================
class CCartridgeEventQueue : public IEventListener
{
public:
	CCartridgeEventQueue();
	~CCartridgeEventQueue();

	/// Returns pointer to front vector of Cartridge events
	///
	/// Use this to iterate through the events that have occured.
	/// This automatically clears the other vector, and all future events
	/// will go into that vector (double buffer).
	///
	/// \warning DO NOT CALL DELETE on this pointer
	std::vector<tCartridgeData> *GetQueue();

	tEventStatus Notify(const IEventMessage &msgIn);
private:
	CKernelMPI kernel_;
	tMutex mutex_;
	std::vector<tCartridgeData> eventVector_[2];
	int front_;
};

/// Alternate constructor for queue which consumes input events,
/// instead of propagating its events to other listeners.
class CCartridgeEventQueue2 : public CCartridgeEventQueue
{
	tEventStatus Notify(const IEventMessage &msgIn) {
		CCartridgeEventQueue::Notify(msgIn);
		return kEventStatusOKConsumed;
	}
};

LF_END_BRIO_NAMESPACE()
#endif //LF_BRIO_CARTRIDGE_EVENT_QUEUE_H
