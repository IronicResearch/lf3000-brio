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
#include <queue>
#include <TouchTypes.h>
LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
class CTouchEventQueue : public IEventListener
{
public:
	CTouchEventQueue();
	~CTouchEventQueue();
	
	std::queue<tTouchData> popQueue();
	
private:
	tEventStatus Notify(const IEventMessage &msgIn);
	CKernelMPI kernel_;
	tMutex mutex_;
	std::queue<tTouchData> eventQueue_;
};

LF_END_BRIO_NAMESPACE()
#endif //LF_BRIO_TOUCH_EVENT_QUEUE_H
