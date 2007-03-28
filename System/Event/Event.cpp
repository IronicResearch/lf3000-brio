//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		EventMgrModule.cpp
//
// Description:
//		Implements the underlying Event Manager module.
//
//============================================================================

#include <StringTypes.h>
#include <SystemErrors.h>

//#include <KernelMPI.h>
#include <EventListener.h>
#include <EventListenerPriv.h>
#include <EventMessage.h>
#include <EventMPI.h>
#include <EventPriv.h>

#include <stdlib.h>	// FIXME: remove when include KernelMPI

const CURI	kModuleURI	= "EventMgr FIXME";


//============================================================================
// Informational functions
//============================================================================
//----------------------------------------------------------------------------
tErrType CEventModule::GetModuleVersion(tVersion &version) const
{
	version = kEventMgrModuleVersion;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CEventModule::GetModuleName(ConstPtrCString &pName) const
{
	pName = &kEventMgrModuleName;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CEventModule::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	pURI = &kModuleURI;
	return kNoErr;
}


//============================================================================
// Module internal state
//============================================================================
//----------------------------------------------------------------------------
class CEventManagerImpl
{
	// Assumptions: The number of listeners in the system at any one time
	// is not huge (< 20), so little is done in the way of optimization.
private:
	//------------------------------------------------------------------------
	const IEventListener**	ppListeners_;
	U32						numListeners_;
	U32						listSize_;
	enum { kGrowBySize = 2 };
	
public:
	//------------------------------------------------------------------------
	CEventManagerImpl() 
		: ppListeners_(NULL), numListeners_(0), listSize_(0)
	{
	}
	//------------------------------------------------------------------------
	void AddListener(const IEventListener* pListener)
	{
		if( numListeners_ >= listSize_ )
		{
			listSize_ += kGrowBySize;
			U32 size = listSize_ * sizeof(const IEventListener*);
			const IEventListener** temp = reinterpret_cast<const IEventListener**>(malloc(size));
			if( numListeners_ )
			{
				memcpy(temp, ppListeners_, numListeners_ * sizeof(const IEventListener*));
				free(ppListeners_);
			}
			ppListeners_ = temp;

		}
		const IEventListener** next = ppListeners_ + numListeners_;
		*next = pListener;
		++numListeners_;
	}
	//------------------------------------------------------------------------
	tErrType RemoveListener(const IEventListener* pListener)
	{
		// TODO: Will the number of listeners ever vary enough to be worth
		// reclaiming memory?
		// If we find the listener in the list, shift other listeners down
		// down to keep the list contiguous and decrement the listeners count.
		const IEventListener** ptr = ppListeners_ + (numListeners_ - 1);		
		for( U32 ii = numListeners_; ii > 0; --ii, --ptr )
		{
			if( *ptr == pListener )
			{
				for( int jj = ii; jj < numListeners_; ++jj, ++ptr )
					*ptr = *(ptr + 1);
				--numListeners_;
				return kNoErr;
			}
		}
		return kEventListenerNotRegisteredErr;
	}
	//------------------------------------------------------------------------
	void PostEventToChain( IEventListener *pListener, 
							const IEventMessage &msg ) const
	{
		while( pListener )
		{
			if( pListener->mpimpl->HandlesEvent(msg.GetEventType())
					&& pListener->Notify(msg) == kEventStatusOKConsumed )
				break;
			pListener = const_cast<IEventListener*>
								(pListener->mpimpl->GetNextListener());
		}
	}
	//------------------------------------------------------------------------
	tErrType PostEvent(const IEventMessage &msg, 
						tEventPriority priority, 
						const IEventListener *pResponse) const
	{
		// 1) If there was a response listener provided (e.g., from
		//		a PlayAudio() or timer call), post to it.
		// 2) Post the message to all of the async/non-response listeners
		// TODO/tp: Optimize this by keeping a multimap of event types to
		// listeners???
		//
		PostEventToChain(const_cast<IEventListener*>(pResponse), msg);	//*1
		
		const IEventListener** ptr = ppListeners_ + (numListeners_ - 1);//*2
		for( U32 ii = numListeners_; ii > 0; --ii, --ptr )
			PostEventToChain(const_cast<IEventListener*>(*ptr), msg);
		return kNoErr;
	}
};
	
namespace
{
	CEventManagerImpl* pinst = NULL;
}


//============================================================================
// Ctor & dtor
//============================================================================
//----------------------------------------------------------------------------
CEventModule::CEventModule()
{
	if (pinst == NULL)
		pinst = new CEventManagerImpl;
}

//----------------------------------------------------------------------------
CEventModule::~CEventModule()
{
	delete pinst;
}

//----------------------------------------------------------------------------
Boolean	CEventModule::IsValid() const
{
	return (pinst != NULL) ? true : false;
}


//============================================================================
// Event registration
//============================================================================
//----------------------------------------------------------------------------
tErrType CEventModule::RegisterEventListener(const IEventListener *pListener,
											tEventRegistrationFlags flags)
{
	pinst->AddListener(pListener);
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CEventModule::UnregisterEventListener(const IEventListener *pListener)
{
	return pinst->RemoveListener(pListener);
}



//============================================================================
// Event generation
//============================================================================
//----------------------------------------------------------------------------
tErrType CEventModule::PostEvent(const IEventMessage &msg, 
									tEventPriority priority, 
									const IEventListener *pResponse) const
{
	return pinst->PostEvent(msg, priority, pResponse);
}


//============================================================================
// Event generation
//============================================================================
//----------------------------------------------------------------------------
CEventListenerImpl* CEventModule::GenerateEventListenerImpl(
						const tEventType* pTypes, U32 count) const
{
	return new CEventListenerImpl(pTypes, count);
}


//============================================================================
// Instance management interface for the Module Manager
//============================================================================

static CEventModule*	sinst = NULL;
//------------------------------------------------------------------------
extern "C" tVersion ReportVersion()
{
	// TBD: ask modules for this or incorporate verion into so file name
	return kEventMgrModuleVersion;
}
	
//------------------------------------------------------------------------
extern "C" ICoreModule* CreateInstance(tVersion version)
{
	if( sinst == NULL )
		sinst = new CEventModule;
	return sinst;
}
	
//------------------------------------------------------------------------
extern "C" void DestroyInstance(ICoreModule* ptr)
{
//		assert(ptr == sinst);
	delete sinst;
	sinst = NULL;
}


// EOF
