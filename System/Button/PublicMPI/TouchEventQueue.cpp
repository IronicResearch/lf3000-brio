#include "TouchEventQueue.h"

LF_BEGIN_BRIO_NAMESPACE()

CTouchEventQueue::CTouchEventQueue()
	:IEventListener(&kAllTouchEvents, 1),front_(0)
{
	tMutexAttr mutexAttr;
	kernel_.InitMutexAttributeObject(mutexAttr);
	kernel_.InitMutex(mutex_, mutexAttr);
}

CTouchEventQueue::~CTouchEventQueue()
{
	kernel_.DeInitMutex(mutex_);
}

std::vector<tTouchData> *CTouchEventQueue::GetQueue()
{
	kernel_.LockMutex(mutex_);
	std::vector<tTouchData> *r_queue = &eventVector_[front_];
	front_ ^= 0x1;
	eventVector_[front_].clear();
	kernel_.UnlockMutex(mutex_);
	return r_queue;
}

LeapFrog::Brio::tEventStatus CTouchEventQueue::Notify(const IEventMessage &msgIn )
{
	tEventType event_type = msgIn.GetEventType();
	if(event_type == kTouchStateChanged)
	{
		kernel_.LockMutex(mutex_);
		eventVector_[front_].push_back(((const CTouchMessage&)msgIn).GetTouchState());
		kernel_.UnlockMutex(mutex_); 
	}
	return kEventStatusOK;
}

LF_END_BRIO_NAMESPACE()
// EOF
