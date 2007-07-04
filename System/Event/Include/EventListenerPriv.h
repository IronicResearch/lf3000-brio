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
LF_BEGIN_BRIO_NAMESPACE()

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
	virtual const IEventListener* GetNextListener() const;
	virtual tErrType	SetNextListener(const IEventListener* pListener);
	virtual tErrType	DisableNotifyForEventType(tEventType type);
	virtual tErrType	ReenableNotifyForEventType(tEventType type);
	virtual Boolean		HandlesEvent(tEventType type) const;
private:
	const IEventListener*	pNextListener_;
	tEventType*				pEventList_;
	tEventType*				pDisabledEventList_;
	const U16				numEvents_;
	U16						numDisabledEvents_;

	// Disable copy semantics
	CEventListenerImpl(const CEventListenerImpl&);
	CEventListenerImpl& operator=(const CEventListenerImpl&);
};


LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_EVENTLISTENERPRIV_H
