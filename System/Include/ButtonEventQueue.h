#ifndef LF_BRIO_BUTTON_EVENT_QUEUE_H
#define LF_BRIO_BUTTON_EVENT_QUEUE_H

//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		ButtonEventQueue.h
//
// Description:
//		Defines utility class CButtonEventQueue.
//		Register this event listener, and it will queue up all the button events.
//		Regularly use popQueue at a time of convenience to process the events.
//		Remember to unregister it when you don't want to queue up button events.
//
//==============================================================================

#include <EventListener.h>
#include <KernelMPI.h>
#include <vector>
#include <ButtonTypes.h>
LF_BEGIN_BRIO_NAMESPACE()

//==============================================================================
class CButtonEventQueue : public IEventListener
{
public:
	CButtonEventQueue();
	~CButtonEventQueue();
	
	//Returns the address of the front buffer
	//DO NOT CALL DELETE on this pointer
	std::vector<tButtonData2> *GetQueue();
	
private:
	tEventStatus Notify(const IEventMessage &msgIn);
	CKernelMPI kernel_;
	tMutex mutex_;
	std::vector<tButtonData2> eventVector_[2];
	int front_;
};

LF_END_BRIO_NAMESPACE()	
#endif //LF_BRIO_BUTTON_EVENT_QUEUE_H
