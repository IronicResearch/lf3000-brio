//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		AccelerometerEventQueue.cpp
//
// Description:
//		Implements utility class CAccelerometerEventQueue.
//
//==============================================================================

#include "AccelerometerEventQueue.h"

LF_BEGIN_BRIO_NAMESPACE()

CAccelerometerEventQueue::CAccelerometerEventQueue()
	:IEventListener(&kAllAccelerometerEvents, 1),front_(0)
{
	tMutexAttr mutexAttr;
	kernel_.InitMutexAttributeObject(mutexAttr);
	kernel_.InitMutex(mutex_, mutexAttr);
}

CAccelerometerEventQueue::~CAccelerometerEventQueue()
{
	kernel_.DeInitMutex(mutex_);
}

std::vector<tAccelerometerData> *CAccelerometerEventQueue::GetQueue()
{
	kernel_.LockMutex(mutex_);
	std::vector<tAccelerometerData> *r_queue = &eventVector_[front_];
	front_ ^= 0x1;
	eventVector_[front_].clear();
	kernel_.UnlockMutex(mutex_);
	return r_queue;
}

LeapFrog::Brio::tEventStatus CAccelerometerEventQueue::Notify(const IEventMessage &msgIn )
{
	tEventType event_type = msgIn.GetEventType();
	if(event_type == kAccelerometerDataChanged)
	{
		kernel_.LockMutex(mutex_);
		eventVector_[front_].push_back(((const CAccelerometerMessage&)msgIn).GetAccelerometerData());
		kernel_.UnlockMutex(mutex_); 
	}
	return kEventStatusOK;
}

LF_END_BRIO_NAMESPACE()
// EOF
