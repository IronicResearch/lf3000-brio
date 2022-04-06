#include "ButtonEventQueue.h"

LF_BEGIN_BRIO_NAMESPACE()

CButtonEventQueue::CButtonEventQueue()
	:IEventListener(&kAllButtonEvents, 1),front_(0)
{
	LeapFrog::Brio::tMutexAttr mutexAttr;
	kernel_.InitMutexAttributeObject(mutexAttr);
	kernel_.InitMutex(mutex_, mutexAttr);
}

CButtonEventQueue::~CButtonEventQueue()
{
	kernel_.DeInitMutex(mutex_);
}

std::vector<tButtonData2> *CButtonEventQueue::GetQueue()
{
	kernel_.LockMutex(mutex_);
	std::vector<tButtonData2> *r_queue = &eventVector_[front_];
	front_ ^= 0x1;
	eventVector_[front_].clear();
	kernel_.UnlockMutex(mutex_);
	return r_queue;
}

LeapFrog::Brio::tEventStatus CButtonEventQueue::Notify(const LeapFrog::Brio::IEventMessage &msgIn )
{
	tEventType event_type = msgIn.GetEventType();
	if(event_type == kButtonStateChanged)
	{
		kernel_.LockMutex(mutex_);
		eventVector_[front_].push_back(((const CButtonMessage&)msgIn).GetButtonState2());
		kernel_.UnlockMutex(mutex_); 
	}
	return kEventStatusOK;
}

LF_END_BRIO_NAMESPACE()
// EOF
