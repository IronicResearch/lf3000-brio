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
	typedef std::set<ConstListenerPtr>		Listeners;
	Listeners		listeners_;
	
public:
	//------------------------------------------------------------------------
	CEventManagerImpl() {}
	//------------------------------------------------------------------------
	void AddListener(const IEventListener* pListener)
	{
		listeners_.insert(pListener);
	}
	//------------------------------------------------------------------------
	tErrType RemoveListener(const IEventListener* pListener)
	{
		Listeners::const_iterator it = listeners_.find(pListener);
		if( it != listeners_.end() )
		{
			listeners_.erase(it);
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
						const IEventListener *pResponse) const
	{
		// 1) If there was a response listener provided (e.g., from
		//		a PlayAudio() or timer call), post to it.
		// 2) Post the message to all of the async/non-response listeners
		// TODO/tp: Optimize this by keeping a multimap of event types to
		// listeners???
		//
		PostEventToChain(const_cast<IEventListener*>(pResponse), msg);	//*1
		
		Listeners::iterator it = listeners_.begin();					//*2
		Listeners::iterator itEnd = listeners_.end();
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
	pinst->AddListener(pListener);
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
									const IEventListener *pResponse) const
{
	return pinst->PostEvent(msg, priority, pResponse);
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
