#include "TouchEventQueue.h"

LF_BEGIN_BRIO_NAMESPACE()

CTouchEventQueue::CTouchEventQueue()
	:IEventListener(&kAllTouchEvents, 1)
{
	tMutexAttr mutexAttr;
	kernel_.InitMutexAttributeObject(mutexAttr);
	kernel_.InitMutex(mutex_, mutexAttr);
}

CTouchEventQueue::~CTouchEventQueue()
{
	kernel_.DeInitMutex(mutex_);
}

std::queue<CTouchMessage> CTouchEventQueue::popQueue()
{
	std::queue<CTouchMessage> r_queue;
	kernel_.LockMutex(mutex_);
	swap(r_queue, eventQueue_);
	kernel_.UnlockMutex(mutex_);
	return r_queue;
}

LeapFrog::Brio::tEventStatus CTouchEventQueue::Notify(const IEventMessage &msgIn )
{
	tEventType event_type = msgIn.GetEventType();
	if(event_type == kTouchStateChanged)
	{
		kernel_.LockMutex(mutex_);
		eventQueue_.push((const CTouchMessage&)msgIn);
		kernel_.UnlockMutex(mutex_); 
	}
	return kEventStatusOK;
}

LF_END_BRIO_NAMESPACE()
// EOF
