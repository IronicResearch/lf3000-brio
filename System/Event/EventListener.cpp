//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		EventListener.cpp
//
// Description:
//		Implements the underlying Event Manager module.
//
//============================================================================

#include <boost/scoped_ptr.hpp>
#include <EventListener.h>
#include <EventListenerPriv.h>
#include <EventMPI.h>
#include <SystemErrors.h>
//#include <KernelMPI.h>
#include <stdlib.h>
#include <string.h>


namespace Kernel
{
	void* Malloc(U32 size) { return malloc(size); }
}

//============================================================================
// CEventListenerImpl implementation class
//============================================================================
//----------------------------------------------------------------------------
CEventListenerImpl::CEventListenerImpl(const tEventType *eventList, U32 count)
	: mpNextListener(NULL), 
	mpEventList(NULL), mNumEvents(count),
	mpDisabledEventList(NULL), mNumDisabledEvents(0)
{
	U32 size = sizeof(tEventType) * count;
	mpEventList = reinterpret_cast<tEventType *>(Kernel::Malloc(size));
	mpDisabledEventList = reinterpret_cast<tEventType *>(Kernel::Malloc(size));
	memcpy(mpEventList, eventList, size);
	// TBD: sort the list so HandlesEvent() can do binary search?
	// TBD: remove duplicates?
}

//----------------------------------------------------------------------------
CEventListenerImpl::~CEventListenerImpl()
{
	
}

//----------------------------------------------------------------------------
const IEventListener* CEventListenerImpl::GetNextListener() const
{ 
	return mpNextListener; 
}

//----------------------------------------------------------------------------
void CEventListenerImpl::SetNextListener(const IEventListener* pListener) 
{ 
	mpNextListener = pListener;
}

//----------------------------------------------------------------------------
tErrType CEventListenerImpl::DisableNotifyForEventType(tEventType type)
{
	// 1) Check if event was already disabled, if so immediately return success
	// 2) If find event in event list, append it to the disabled list
	// 3) Return error if event not in original list
	//
	for( int ii = mNumDisabledEvents; ii > 0; --ii )					//*1
	{
		if( *(mpDisabledEventList + ii - 1) == type )
			return kNoErr;
	}
	
	tEventType allInGroup = (type | kWildcardNumSpaceTag);
	for( int ii = mNumEvents; ii > 0; --ii )							//*2
	{
		tEventType next = *(mpEventList + ii - 1);
		if( next == allInGroup || next == type )
		{
			*(mpDisabledEventList + mNumDisabledEvents) = type;
			++mNumDisabledEvents;
			return kNoErr;
		}
	}
	
	return kEventTypeNotFoundErr;											//*3
}

//----------------------------------------------------------------------------
tErrType CEventListenerImpl::ReenableNotifyForEventType(tEventType type)
{
	// If we find that the event was previously disabled, shift other
	// disabled events down to keep the list contiguous and decrement the
	// disabled events count.
	//
	tEventType *ptr = mpDisabledEventList + mNumDisabledEvents - 1;
	for( int ii = mNumDisabledEvents; ii > 0; --ii, --ptr )
	{
		if( *ptr == type )
		{
			for( int jj = ii; jj < mNumDisabledEvents; ++jj, ++ptr )
				*ptr = *(ptr + 1);
			--mNumDisabledEvents;
			return kNoErr;
		}
	}
	return kEventTypeNotFoundErr;
}

//----------------------------------------------------------------------------
Boolean CEventListenerImpl::HandlesEvent(tEventType type) const
{
	for( int ii = mNumDisabledEvents; ii > 0; --ii )
		if( *(mpDisabledEventList + ii - 1) == type )
			return false;
	
	tEventType allInGroup = (type | kWildcardNumSpaceTag);
	for( int ii = mNumEvents; ii > 0; --ii )
	{
		tEventType next = *(mpEventList + ii - 1);
		if( next == allInGroup || next == type )
			return true;
	}
	return false;
}
/*
void CEventListenerImpl::CatEventList()
{
	// get new total number of events
	U32 newNumEvents = mNumEvents + mpNextListener->mpListenerImpl->mNumEventsAll
	
	//delete old list and allocate memory for a new one
	delete [] mpEventListAll;
	mpEventListAll = new tEventType[newNumEvents];
	
	//copy our local list to the front
	memcpy( mpEventListAll, mpEventList, sizeof(tEventType*numEvents));
	
	//and conditionally copy the rest, ignoring duplicates
	U32 i, j, copyCount = 0;  //copyCount - makes sure we have no gaps, in the event of duplicates 
	for(i=0; i<mpNextListener->mpListenerImpl->mNumEventsAll; i++)
	{
		for(j=0; j<mNumEvents; j++)
		{
			if(mpNextListener->mpListenerImpl->mpEventListAll[i] == mpNumEvents[j])
			{
				break;				
			} 	
		}
		if(j>=mNumEvents)   //j<mNumEvents indicates duplicate 
		{
			mpEventListAll[mNumEvents+copyCount];
			copyCount++	;
		}	
	}		
}
*/


//============================================================================
// IEventListener
//============================================================================
//----------------------------------------------------------------------------
IEventListener::IEventListener(const tEventType* pTypes, U32 count)
{
	mpimpl = new CEventListenerImpl(pTypes, count);
}

//----------------------------------------------------------------------------
IEventListener::~IEventListener()
{
	boost::scoped_ptr<CEventMgrMPI> eventmgr(new CEventMgrMPI());
	eventmgr->UnregisterEventListener(this);
	delete mpimpl;
}

//----------------------------------------------------------------------------
const IEventListener* IEventListener::GetNextListener() const
{
	return mpimpl->GetNextListener();
}

//----------------------------------------------------------------------------
void IEventListener::SetNextListener(const IEventListener* pListener)
{
	mpimpl->SetNextListener(pListener);
}

//----------------------------------------------------------------------------
tErrType IEventListener::DisableNotifyForEventType(tEventType type)
{
	mpimpl->DisableNotifyForEventType(type);
}

//----------------------------------------------------------------------------
tErrType IEventListener::ReenableNotifyForEventType(tEventType type)
{
	mpimpl->ReenableNotifyForEventType(type);
}

// EOF
