//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		EventListenerPriv.cpp
//
// Description:
//		Implements the underlying Event Manager module.
//
//============================================================================

#include <EventListener.h>
#include <EventListenerPriv.h>
#include <EventMPI.h>
#include <EventPriv.h>
#include <SystemErrors.h>
//#include <KernelMPI.h>
#include <stdlib.h>// FIXME: remove when include KernelMPI
#include <string.h>// FIXME: remove when include KernelMPI


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
tErrType CEventListenerImpl::SetNextListener(const IEventListener* pListener) 
{
	// Validate self not already in chain
	if( this == pListener->mpimpl )
		return kEventListenerCycleErr;
	for( const IEventListener* pNext = pListener->mpimpl->GetNextListener(); 
			pNext; pNext = pNext->mpimpl->GetNextListener() )
	{
		if( pNext->mpimpl == this )
			return kEventListenerCycleErr;
	}
	mpNextListener = pListener;
	return kNoErr;
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

// EOF
