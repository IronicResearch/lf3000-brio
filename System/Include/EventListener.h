#ifndef LF_BRIO_EVENTLISTENER_H
#define LF_BRIO_EVENTLISTENER_H
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		EventListener.h
//
// Description:
//		Base class for all event listeners. 
//
//==============================================================================

#include <SystemTypes.h>
#include <SystemEvents.h>

LF_BEGIN_BRIO_NAMESPACE()

class IEventMessage;		// forward declaration

enum tEventStatus {
		kEventStatusOK,
		kEventStatusOKConsumed,
		kEventStatusERROR
};

//==============================================================================
class IEventListener { 
	// The IEventListener-derived objects passed to SetNextListener() are
	// owned by client code and should not be deleted by this class.
	// The tEventType* list passed to the ctor is not assumed to be persistent
	// (IEventListener makes a copy of these in its ctor).
	// The tEventType* list passed to the ctor must be the complete list of
	// events handled by the listener, DisableNotifyForEventType() &
	// ReenableNotifyForEventType() work off of this list.
	//
	// NOTE: The Notify() member function's "msg" parameter is passed by 
	// reference, and the lifetime of that message is not guaranteed beyond
	// the return from the Notify() function.  So if you need access to that
	// message outside of the Notify() function, you must copy that information
	// out of the message for later use.
	//
public:
	IEventListener(const tEventType* pTypes, U32 count);
	virtual ~IEventListener();
	virtual tEventStatus	Notify(const IEventMessage& msg) = 0;
	const IEventListener*	GetNextListener() const;
	tErrType				SetNextListener(const IEventListener* pListener);
	tErrType				DisableNotifyForEventType(tEventType type);
	tErrType				ReenableNotifyForEventType(tEventType type);

private:
	class CEventListenerImpl *pimpl_;
	friend class CEventListenerImpl;
	friend class CEventManagerImpl;

	// Disable copy semantics
	IEventListener(const IEventListener&);
	IEventListener& operator=(const IEventListener&);
};


/*
// Sends the event message to a queue for asynchronous processing
class CSendMessageEventHandler : public IEventListener {
public:
	CSendMessageEventHandler() { ; }
   virtual	~CSendMessageEventHandler() { ; }

	virtual tErrType	Notify(tEventType event, const IEventMessage *pMsg,
							tEventContext callerContext);
	void		SetMessageQueue(tMessageQueueHndl hndl) { mhMsgQueue = hndl; }

protected:
	tMessageQueueHndl	mhMsgQueue;
};
*/

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_EVENTLISTENER_H

// EOF
