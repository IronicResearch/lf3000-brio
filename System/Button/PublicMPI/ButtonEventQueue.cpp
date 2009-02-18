#include "ButtonEventQueue.h"

LF_BEGIN_BRIO_NAMESPACE()

CButtonEventQueue::CButtonEventQueue()
	:IEventListener(&kAllButtonEvents, 1)
{
	LeapFrog::Brio::tMutexAttr mutexAttr;
	kernel_.InitMutexAttributeObject(mutexAttr);
	kernel_.InitMutex(mutex_, mutexAttr);
}

CButtonEventQueue::~CButtonEventQueue()
{
	kernel_.DeInitMutex(mutex_);
}

std::queue<CButtonMessage> CButtonEventQueue::popQueue()
{
	std::queue<CButtonMessage> r_queue;
	kernel_.LockMutex(mutex_);
	swap(r_queue, eventQueue_);
	kernel_.UnlockMutex(mutex_);
	return r_queue;
}

LeapFrog::Brio::tEventStatus CButtonEventQueue::Notify(const LeapFrog::Brio::IEventMessage &msgIn )
{
	tEventType event_type = msgIn.GetEventType();
	if(event_type == kButtonStateChanged)
	{
		kernel_.LockMutex(mutex_);
		eventQueue_.push((const CButtonMessage&)msgIn);
		kernel_.UnlockMutex(mutex_); 
	}
	return kEventStatusOK;
}

LF_END_BRIO_NAMESPACE()
// EOF
