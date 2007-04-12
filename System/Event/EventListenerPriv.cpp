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

#include <SystemTypes.h>
#include <EventListener.h>
#include <EventListenerPriv.h>
#include <EventMPI.h>
#include <EventPriv.h>
#include <SystemErrors.h>
//#include <KernelMPI.h>
#include <stdlib.h>// FIXME: remove when include KernelMPI
#include <string.h>// FIXME: remove when include KernelMPI
LF_BEGIN_BRIO_NAMESPACE()


namespace Kernel
{
	void* Malloc(U32 size) { return malloc(size); }
}

//============================================================================
// CEventListenerImpl implementation class
//============================================================================
//----------------------------------------------------------------------------
CEventListenerImpl::CEventListenerImpl(const tEventType *eventList, U32 count)
	: pNextListener_(NULL), 
	pEventList_(NULL), numEvents_(count),
	pDisabledEventList_(NULL), numDisabledEvents_(0)
{
	U32 size = sizeof(tEventType) * count;
	pEventList_ = reinterpret_cast<tEventType *>(Kernel::Malloc(size));
	pDisabledEventList_ = reinterpret_cast<tEventType *>(Kernel::Malloc(size));
	memcpy(pEventList_, eventList, size);
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
	return pNextListener_; 
}

//----------------------------------------------------------------------------
tErrType CEventListenerImpl::SetNextListener(const IEventListener* pListener) 
{
	// Validate self not already in chain
	if( this == pListener->pimpl_ )
		return kEventListenerCycleErr;
	for( const IEventListener* pNext = pListener->pimpl_->GetNextListener(); 
			pNext; pNext = pNext->pimpl_->GetNextListener() )
	{
		if( pNext->pimpl_ == this )
			return kEventListenerCycleErr;
	}
	pNextListener_ = pListener;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CEventListenerImpl::DisableNotifyForEventType(tEventType type)
{
	// 1) Check if event was already disabled, if so immediately return success
	// 2) If find event in event list, append it to the disabled list
	// 3) Return error if event not in original list
	//
	for( int ii = numDisabledEvents_; ii > 0; --ii )					//*1
	{
		if( *(pDisabledEventList_ + ii - 1) == type )
			return kNoErr;
	}
	
	tEventType allInGroup = (type | kWildcardNumSpaceTag);
	for( int ii = numEvents_; ii > 0; --ii )							//*2
	{
		tEventType next = *(pEventList_ + ii - 1);
		if( next == allInGroup || next == type )
		{
			*(pDisabledEventList_ + numDisabledEvents_) = type;
			++numDisabledEvents_;
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
	tEventType *ptr = pDisabledEventList_ + numDisabledEvents_ - 1;
	for( int ii = numDisabledEvents_; ii > 0; --ii, --ptr )
	{
		if( *ptr == type )
		{
			for( int jj = ii; jj < numDisabledEvents_; ++jj, ++ptr )
				*ptr = *(ptr + 1);
			--numDisabledEvents_;
			return kNoErr;
		}
	}
	return kEventTypeNotFoundErr;
}

//----------------------------------------------------------------------------
Boolean CEventListenerImpl::HandlesEvent(tEventType type) const
{
	for( int ii = numDisabledEvents_; ii > 0; --ii )
		if( *(pDisabledEventList_ + ii - 1) == type )
			return false;
	
	tEventType allInGroup = (type | kWildcardNumSpaceTag);
	for( int ii = numEvents_; ii > 0; --ii )
	{
		tEventType next = *(pEventList_ + ii - 1);
		if( next == allInGroup || next == type )
			return true;
	}
	return false;
}

LF_END_BRIO_NAMESPACE()
// EOF
