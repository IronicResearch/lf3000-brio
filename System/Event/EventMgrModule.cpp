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
#include <EventMgrMPI.h>
#include <EventMgrModulePriv.h>


const CURI	kModuleURI	= "EventMgr FIXME";


//============================================================================
// Informational functions
//============================================================================
//----------------------------------------------------------------------------
tErrType CEventMgrModule::GetModuleVersion(tVersion &version) const
{
	version = kEventMgrModuleVersion;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CEventMgrModule::GetModuleName(ConstPtrCString &pName) const
{
	pName = &kEventMgrModuleName;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CEventMgrModule::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	pURI = &kModuleURI;
	return kNoErr;
}


//============================================================================
// Module internal state
//============================================================================
#include <vector>
#include <map>
#include <set>
//----------------------------------------------------------------------------
class CEventManagerImpl
{
	// Assumptions: The number of listeners in the system at any one time
	// is not huge (< 20), so little is done in the way of optimization.
	// TODO: Once debugged in unit tests, replace STL collection classes
private:
	//------------------------------------------------------------------------
	typedef const IEventListener*	ConstListenerPtr;
	typedef std::map<tListenerId, ConstListenerPtr>	IdMap;
	typedef std::set<ConstListenerPtr>				Listeners;
	tListenerId		nextListener_;
	IdMap			idToListenerMap_;
	Listeners		async_;
	Listeners		response_;
	
public:
	//------------------------------------------------------------------------
	CEventManagerImpl() : nextListener_(0) {}
	//------------------------------------------------------------------------
	void AddAsyncListener(const IEventListener* pListener)
	{
		async_.insert(pListener);
	}
	//------------------------------------------------------------------------
	tListenerId	AddResponseListener(const IEventListener* pListener)
	{
		response_.insert(pListener);
		++nextListener_;
		idToListenerMap_.insert(IdMap::value_type(nextListener_, pListener));
		return nextListener_;
	}
	//------------------------------------------------------------------------
	tErrType RemoveListener(const IEventListener* pListener)
	{
		Listeners::iterator it = response_.find(pListener);
		if( it != response_.end() )
		{
			response_.erase(it);
			IdMap::iterator itb = idToListenerMap_.begin();
			IdMap::iterator ite = idToListenerMap_.end();
			for( ; itb != ite; ++itb )
			{
				if( itb->second == pListener )
				{
					idToListenerMap_.erase(itb);
					break;
				}
			}
			return kNoErr;
		}
		it = async_.find(pListener);
		if( it != async_.end() )
		{
			async_.erase(it);
			return kNoErr;
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
								(pListener->GetNextListener());
		}
	}
	//------------------------------------------------------------------------
	tErrType PostEvent(const IEventMessage &msg, 
						tEventPriority priority, 
						const IEventListener *pListener,
						tListenerId id) const
	{
		// 1) Look for a response listener associated with this msg
		// 1a) If there was an override handler provided (e.g., from
		//		a client PlayAudio() call that provided a handler param)
		//		use it alone.
		// 1b) Otherwise if a listener handle was provided, use it
		//		FIXME/tp: Can we remove this and just use the override
		//		as both that and the response???
		// 2) Post the message to any response listener
		// 3) Post the message to all of the async listeners
		// TODO: Optimize this by keeping a multimap of event types to
		// listeners???
		//
		const IEventListener* response = NULL;							//*1
		if( pListener != NULL )
			response = pListener;										//*1a
		else if( id != kNoListener )
		{
			IdMap::const_iterator it = idToListenerMap_.find(id);		//*1b
			if( it == idToListenerMap_.end() )
				return kEventListenerNotRegisteredErr;
			response = it->second;
		}
		
		PostEventToChain(const_cast<IEventListener*>(response), msg);	//*2
		
		Listeners::iterator it = async_.begin();						//*3
		Listeners::iterator itEnd = async_.end();
		for( ; it != itEnd; ++it )
			PostEventToChain(const_cast<IEventListener*>(*it), msg);
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
CEventMgrModule::CEventMgrModule()
{
	if (pinst == NULL)
		pinst = new CEventManagerImpl;
}

//----------------------------------------------------------------------------
CEventMgrModule::~CEventMgrModule()
{
	delete pinst;
}

//----------------------------------------------------------------------------
Boolean	CEventMgrModule::IsValid() const
{
	return (pinst != NULL) ? true : false;
}


//============================================================================
// Event registration
//============================================================================
//----------------------------------------------------------------------------
tErrType CEventMgrModule::RegisterEventListener(const IEventListener *pListener,
											tEventRegistrationFlags flags)
{
	pinst->AddAsyncListener(pListener);
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CEventMgrModule::RegisterResponseEventListener(tListenerId &id, 
											const IEventListener *pListener,
											tEventRegistrationFlags flags)
{
	id = pinst->AddResponseListener(pListener);
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CEventMgrModule::UnregisterEventListener(const IEventListener *pListener)
{
	return pinst->RemoveListener(pListener);
}



//============================================================================
// Event generation
//============================================================================
//----------------------------------------------------------------------------
tErrType CEventMgrModule::PostEvent(const IEventMessage &msg, 
									tEventPriority priority, 
									const IEventListener *pListener,
									tListenerId id) const
{
	return pinst->PostEvent(msg, priority, pListener, id);
}



//============================================================================
// Instance management interface for the Module Manager
//============================================================================
extern "C"
{
	static CEventMgrModule*	sinst = NULL;
	//------------------------------------------------------------------------
	tVersion ReportVersion()
	{
		// TBD: ask modules for this or incorporate verion into so file name
		return kEventMgrModuleVersion;
	}
	
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion version)
	{
		if( sinst == NULL )
			sinst = new CEventMgrModule;
		return sinst;
	}
	
	//------------------------------------------------------------------------
	void DestroyInstance(ICoreModule* ptr)
	{
//		assert(ptr == sinst);
		delete sinst;
		sinst = NULL;
	}
}

// EOF
