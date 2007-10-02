//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		Event.cpp
//
// Description:
//		Implements the underlying Event Manager module.
//
//============================================================================

#include <SystemTypes.h>
#include <StringTypes.h>
#include <SystemErrors.h>

#include <KernelMPI.h>
#include <DebugMPI.h>
#include <EventListener.h>
#include <EventListenerPriv.h>
#include <EventMessage.h>
#include <EventMPI.h>
#include <EventPriv.h>

LF_BEGIN_BRIO_NAMESPACE() 

const CURI	kModuleURI	= "/LF/System/Event";


//============================================================================
// Informational functions
//============================================================================
//----------------------------------------------------------------------------
tVersion CEventModule::GetModuleVersion() const
{
	return kEventModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CEventModule::GetModuleName() const
{
	return &kEventModuleName;
}

//----------------------------------------------------------------------------
const CURI* CEventModule::GetModuleOrigin() const
{
	return &kModuleURI;
}


//============================================================================
// Event dispatching message
//============================================================================
namespace
{
	class CEventDispatchMessage : public CMessage
	{
	public:
		CEventDispatchMessage(const IEventMessage *m = NULL, 
							const IEventListener *r = NULL)
		: pMsg(m), pResponse(r)
		{
			messageSize = sizeof(CEventDispatchMessage);
		}
		const IEventMessage*	pMsg;
		const IEventListener*	pResponse;
	};
	const U32 kEventDispatchMessageSize = sizeof(CEventDispatchMessage);
	
	char*	g_msgQueueName = "/eventDispatchQueue";
}	
	
//============================================================================
// Module internal state
//============================================================================
//----------------------------------------------------------------------------
class CEventManagerImpl
{
	// Assumptions: The number of listeners in the system at any one time
	// is not huge (< 20), so little is done in the way of optimization.
	//
	// NOTE: This class is not in an unnamed namespace because it is a friend
	// of CEventListenerImpl.
private:
	//------------------------------------------------------------------------
	const IEventListener**	ppListeners_;
	U32						numListeners_;
	U32						listSize_;
	mutable CKernelMPI		kernel_;
	CDebugMPI				debug_;
	tMessageQueueHndl		hMsgQueueFG_ ;
	
	enum { kGrowBySize = 2 };
	static tMessageQueueHndl	g_hMsgQueueBG_ ;
	static bool 				g_threadRun_;
	static bool					g_threadRunning_;
	static tTaskHndl			g_hThread_;
	
	//============================================================================
	// Event dispatching task
	//============================================================================
	static void* EventDispatchTask( void* arg )
	{
		// 1) Create a msg queue that allows this thread to receive messages
		// 2) Task loop, receive and dispatch messages
		//
		CEventManagerImpl*	pThis = reinterpret_cast<CEventManagerImpl*>(arg);
		CKernelMPI	kernel;
		CDebugMPI	debug(kGroupEvent);
	
		tMessageQueuePropertiesPosix props = 								//*1
		{
		    0,                          // msgProperties.blockingPolicy;  
		    g_msgQueueName,    			// msgProperties.nameQueue
		    B_S_IRUSR,                    // msgProperties.mode 
		    B_O_RDONLY,    				// msgProperties.oflag  
		    0,                          // msgProperties.priority
		    0,                          // msgProperties.mq_flags
		    8,                          // msgProperties.mq_maxmsg
		    kEventDispatchMessageSize, 	// msgProperties.mq_msgsize
		    0                           // msgProperties.mq_curmsgs
		};
		tErrType err = kernel.OpenMessageQueue(g_hMsgQueueBG_, props, NULL);
		debug.AssertNoErr(err, "EventDispatchTask(): Failed to create msg queue!\n" );
		debug.DebugOut(kDbgLvlVerbose, "EventDispatchTask() Message queue created %d\n", 
						static_cast<int>(g_hMsgQueueBG_));
	
		CEventDispatchMessage	msg;										//*2

		g_threadRunning_ = true;
		while (g_threadRun_)
		{
#if 0	// FIXME/dm: Not ready to timeout message queue on embedded target
			// Wake up every 250ms to see if we need to exit, otherwise wake up on msg and process it.
			err = kernel.ReceiveMessageOrWait(g_hMsgQueueBG_, &msg, kEventDispatchMessageSize, 250);
#else
			err = kernel.ReceiveMessage(g_hMsgQueueBG_, &msg, kEventDispatchMessageSize);
#endif
			if ( err != kConnectionTimedOutErr ) {
		    	debug.AssertNoErr(err, "EventDispatchTask(): Receive message!\n" );
		    	pThis->PostEventImpl(*(msg.pMsg), msg.pResponse);
		    }
		}
		
		g_threadRunning_ = false;
		return NULL;
	}

public:
	//------------------------------------------------------------------------
	CEventManagerImpl() 
		: ppListeners_(NULL), numListeners_(0), listSize_(0), 
		debug_(kGroupEvent), hMsgQueueFG_(kInvalidMessageQueueHndl)

	{		
		// 1) Create the message queue for communicating with the
		//    background dispatching thread
		// 2) Create the background event dispatching thread
		//
		CDebugMPI	debug(kGroupEvent);
		debug.DebugOut(kDbgLvlVerbose, "CEventManagerImpl::ctor: Initializing Event Manager\n");

		tMessageQueuePropertiesPosix props = 							//*1
		{
		    0,                          	// msgProperties.blockingPolicy;  
		    g_msgQueueName,    				// msgProperties.nameQueue
		    B_S_IRWXU,                    	// msgProperties.mode 
		    B_O_WRONLY|B_O_CREAT|B_O_TRUNC, // msgProperties.oflag  
		    0,                          	// msgProperties.priority
		    0,                          	// msgProperties.mq_flags
		    8,                          	// msgProperties.mq_maxmsg
		    kEventDispatchMessageSize,  	// msgProperties.mq_msgsize
		    0                           	// msgProperties.mq_curmsgs
		};
		
		tErrType err = kernel_.OpenMessageQueue(hMsgQueueFG_, props, NULL);

		debug.AssertNoErr(err, "CEventManagerImpl::ctor: Failed to create message queue!\n");
		debug.DebugOut(kDbgLvlVerbose, "CEventManagerImpl::ctor: Message queue created %d\n", 
						static_cast<int>(hMsgQueueFG_));

		tTaskProperties properties;										//*2

		properties.TaskMainFcn = CEventManagerImpl::EventDispatchTask;
		properties.pTaskMainArgValues = this;
		err = kernel_.CreateTask(g_hThread_, properties, NULL);

		debug.Assert( kNoErr == err, "CEventManagerImpl::ctor: Failed to create EventDispatchTask!\n" );

		// Wait for thread to start up...
		while ( !g_threadRunning_ ) {
			kernel_.TaskSleep(10);
		}
}
	
	//------------------------------------------------------------------------
	~CEventManagerImpl() 
	{
		CDebugMPI	debug(kGroupEvent);
		debug.DebugOut(kDbgLvlVerbose, "CEventManagerImpl::dtor: Event Manager going away...\n");

		delete ppListeners_;
		g_threadRun_ = false;
		while ( g_threadRunning_ ) {
			kernel_.TaskSleep(10);
		}
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
		// TODO/tp: Will the number of listeners ever vary enough to be worth
		// reclaiming memory?  Unlikely, but you can add some profiling info.
		// If we find the listener in the list, shift other listeners down
		// down to keep the list contiguous and decrement the listeners count.
		if (ppListeners_) {	
			const IEventListener** ptr = ppListeners_ + (numListeners_ - 1);		
			for( U32 ii = numListeners_; ii > 0; --ii, --ptr )
			{
				if( *ptr == pListener )
				{
					for( U32 jj = ii; jj < numListeners_; ++jj, ++ptr )
						*ptr = *(ptr + 1);
					--numListeners_;
					return kNoErr;
				}
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
			if( pListener->pimpl_->HandlesEvent(msg.GetEventType())
					&& pListener->Notify(msg) == kEventStatusOKConsumed )
				break;
			pListener = const_cast<IEventListener*>
								(pListener->pimpl_->GetNextListener());
		}
	}
	//------------------------------------------------------------------------
	tErrType PostEventImpl(const IEventMessage &msg, 
						const IEventListener *pResponse) const
	{
		// 1) If there was a response listener provided (e.g., from
		//		a PlayAudio() or timer call), post to it.
		// 2) Otherwise, post the message to all of the async/non-response listeners
		// 3) TODO/tp: If you want the ability to install event "monitors" that are
		//    able to see all response events (even though the monitors don't make
		//    the calls that generate these events), you will need to clone the
		//    ppListeners_ & numListeners_ variables for a list of "monitors"
		//    that are registered through a new event manager member function
		//    (with the same signatures as the RegisterEventListener()/
		//    UnregisterEventListener() functions).
		// TODO/tp: Speed could be optimized by keeping a multimap of event types to
		// listeners, but only consider it if profiling indicates a need.
		//
		if (pResponse)
			PostEventToChain(const_cast<IEventListener*>(pResponse), msg);	//*1
		else
		{
			const IEventListener** ptr = ppListeners_ + (numListeners_ - 1);//*2
			for( U32 ii = numListeners_; ii > 0; --ii, --ptr )
				PostEventToChain(const_cast<IEventListener*>(*ptr), msg);
		}
		// TODO: copy and modify the "else" clause immediately above to call
		// all global event "monitors".
		return kNoErr;
	}
	//------------------------------------------------------------------------
	tErrType PostEvent(const IEventMessage &msg, 
						tEventPriority priority, 
						const IEventListener *pResponse) const
	{
		// NOTE: 0 is the highest priority, 255 the lowest.  
		// 1) For high priority requests, call through on the poster's thread
		//  to minimize overhead.
		// 2) For low priority requests, dispatch through the background thread.
		//
		if (priority < 128)												//*1
			return PostEventImpl(msg, pResponse);

		// FIXME: copy msg to buffer!
		CEventDispatchMessage	dmsg(&msg, pResponse);					//*2
		tErrType err = kernel_.SendMessage(hMsgQueueFG_, dmsg);
		debug_.AssertNoErr(err, "PostEvent -- SendMessage failed!\n");
		return err;
	}
};

//----------------------------------------------------------------------------
tMessageQueueHndl	CEventManagerImpl::g_hMsgQueueBG_ = kInvalidMessageQueueHndl;
bool 				CEventManagerImpl::g_threadRun_   = true;
bool 				CEventManagerImpl::g_threadRunning_  = false;
tTaskHndl			CEventManagerImpl::g_hThread_	  = kInvalidTaskHndl;
	
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
											tEventRegistrationFlags /*flags*/)
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

LF_END_BRIO_NAMESPACE()


//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()

static CEventModule*	sinst = NULL;

extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion /*version*/)
	{
		if( sinst == NULL )
			sinst = new CEventModule;
		return sinst;
	}
		
	//------------------------------------------------------------------------
	void DestroyInstance(ICoreModule* /*ptr*/)
	{
	//		assert(ptr == sinst);
		delete sinst;
		sinst = NULL;
	}
}
#endif	// LF_MONOLITHIC_DEBUG

// EOF
