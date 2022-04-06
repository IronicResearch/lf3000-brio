#include "CartridgeEventQueue.h"
#include "DebugMPI.h"

LF_BEGIN_BRIO_NAMESPACE()

CCartridgeEventQueue::CCartridgeEventQueue()
	:IEventListener(&kAllCartridgeEvents, 1),front_(0)
{
	LeapFrog::Brio::tMutexAttr mutexAttr;
	LeapFrog::Brio::tErrType error = kernel_.InitMutexAttributeObject(mutexAttr);
	error = kernel_.InitMutex(mutex_, mutexAttr);
}

CCartridgeEventQueue::~CCartridgeEventQueue()
{
	kernel_.DeInitMutex(mutex_);
}

std::vector<tCartridgeData> *CCartridgeEventQueue::GetQueue()
{
	kernel_.LockMutex(mutex_);
	std::vector<tCartridgeData> *r_queue = &eventVector_[front_];
	front_ ^= 0x1;
	eventVector_[front_].clear();
	kernel_.UnlockMutex(mutex_);
	return r_queue;
}

LeapFrog::Brio::tEventStatus CCartridgeEventQueue::Notify(const LeapFrog::Brio::IEventMessage &msgIn )
{
	tEventType event_type = msgIn.GetEventType();
	if(event_type == kCartridgeStateChanged)
	{
		kernel_.LockMutex(mutex_);
		CDebugMPI debug_mpi(kSystemDebugSigGroup);
		eventVector_[front_].push_back(((const CCartridgeMessage&)msgIn).GetCartridgeState());
		kernel_.UnlockMutex(mutex_); 
	}
	return kEventStatusOK;
}

LF_END_BRIO_NAMESPACE()
// EOF
