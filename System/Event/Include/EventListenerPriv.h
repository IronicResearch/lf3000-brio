#ifndef LF_BRIO_EVENTLISTENERPRIV_H
#define LF_BRIO_EVENTLISTENERPRIV_H
//==============================================================================
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		EventListenerPriv.h
//
// Description:
//		Base class for all event listeners. 
//
//==============================================================================

#include <SystemTypes.h>

class IEventListener;	// forward declaration


//============================================================================
// Implementation class
//============================================================================
class CEventListenerImpl
{
	// Mirrors the interface of IEventListener so that class can pass
	// calls through to this private implementation class.
	//
public:
	CEventListenerImpl(const tEventType* pTypes, U32 numEvents);
	virtual ~CEventListenerImpl();
	const IEventListener* GetNextListener() const;
	void				SetNextListener(const IEventListener* pListener);
	tErrType			DisableNotifyForEventType(tEventType type);
	tErrType			ReenableNotifyForEventType(tEventType type);
	Boolean				HandlesEvent(tEventType type) const;
private:
	const IEventListener*	mpNextListener;
	tEventType*				mpEventList;
	tEventType*				mpDisabledEventList;
	const U16				mNumEvents;
	U16						mNumDisabledEvents;
};



#endif // LF_BRIO_EVENTLISTENERPRIV_H
