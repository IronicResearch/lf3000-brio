//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		HWControllerEventQueue.cpp
//
// Description:
//		Defines utility class HWControllerEventQueue.
//		Register this event listener, and it will queue up the controller events.
//		Regularly use popQueue at a time of convenience to process the events.
//		Remember to unregister it when you don't want to queue up button events.
//
//==============================================================================

#include <Hardware/HWControllerEventQueue.h>
#include <Hardware/HWControllerEventMessage.h>
#include <Hardware/HWControllerTypes.h>

const LeapFrog::Brio::tEventType
  kHWControllerQueueListenerTypes[] = {LF::Hardware::kHWControllerAccelerometerDataChanged,
		  	  	  	  	  	  	  	   LF::Hardware::kHWControllerAccelerometerOrientationChanged,
		  	  	  	  	  	  	  	   LF::Hardware::kHWControllerButtonStateChanged,
		  	  	  	  	  	  	  	   LF::Hardware::kHWControllerAnalogStickDataChanged
									  };

LF_BEGIN_BRIO_NAMESPACE()

HWControllerEventQueue::HWControllerEventQueue()
	:IEventListener(kHWControllerQueueListenerTypes, ArrayCount(kHWControllerQueueListenerTypes)),front_(0)
{
	LeapFrog::Brio::tMutexAttr mutexAttr;
	kernel_.InitMutexAttributeObject(mutexAttr);
	kernel_.InitMutex(mutex_, mutexAttr);
}

HWControllerEventQueue::~HWControllerEventQueue()
{
	kernel_.DeInitMutex(mutex_);
}

std::vector<LF::Hardware::HWControllerEventMessage> *HWControllerEventQueue::GetQueue()
{
	kernel_.LockMutex(mutex_);
	std::vector<LF::Hardware::HWControllerEventMessage> *r_queue = &eventVector_[front_];
	front_ ^= 0x1;
	eventVector_[front_].clear();
	kernel_.UnlockMutex(mutex_);
	return r_queue;
}

LeapFrog::Brio::tEventStatus HWControllerEventQueue::Notify(const LeapFrog::Brio::IEventMessage &msgIn )
{
	const LF::Hardware::HWControllerEventMessage&
		  msg = reinterpret_cast<const LF::Hardware::HWControllerEventMessage&>(msgIn);

	kernel_.LockMutex(mutex_);
	eventVector_[front_].push_back(msg);
	kernel_.UnlockMutex(mutex_);

	return kEventStatusOK;
}

LF_END_BRIO_NAMESPACE()
// EOF




