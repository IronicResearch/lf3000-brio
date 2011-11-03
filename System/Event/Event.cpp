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
#include <string.h>
#include <sys/stat.h>

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
			messagePriority = 0;
			messageReserved = 0;
		}
		const IEventMessage*	pMsg;
		const IEventListener*	pResponse;
	};
	const U32 kEventDispatchMessageSize = sizeof(CEventDispatchMessage);
	
	const char*	g_msgQueueName = "/eventDispatchQueue";

	const tEventPriority	kEventListenerEventPriority	= 0;
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
	static tTaskHndl			g_hBPUThread_;
	static tTaskHndl			g_hCartThread_;
	static tMutex				g_mutexEventList_;
	
	//============================================================================
	// Event dispatching task
	//============================================================================
	static void* EventDispatchTask( void* arg )
	{
		// 1) Create a msg queue that allows this thread to receive messages
		// 2) Task loop, receive and dispatch messages
		//
		CEventManagerImpl*	pThis = reinterpret_cast<CEventManagerImpl*>(arg);
		
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
		tErrType err = pThis->kernel_.OpenMessageQueue(g_hMsgQueueBG_, props, NULL);
		pThis->debug_.AssertNoErr(err, "EventDispatchTask(): Failed to create msg queue!\n" );
		pThis->debug_.DebugOut(kDbgLvlVerbose, "EventDispatchTask() Message queue created %d\n", 
						static_cast<int>(g_hMsgQueueBG_));
		CEventDispatchMessage	msg;										//*2

		g_threadRunning_ = true;
		while (g_threadRun_)
		{
			// use timeout for cancellation point
			err = pThis->kernel_.ReceiveMessageOrWait(g_hMsgQueueBG_, &msg, kEventDispatchMessageSize, 100);
	        pthread_testcancel();
			if ( err != kConnectionTimedOutErr && g_threadRun_ ) {
		    	pThis->debug_.AssertNoErr(err, "EventDispatchTask(): Receive message!\n" );
		    	pThis->PostEventImpl(*(msg.pMsg), msg.pResponse);
				pThis->kernel_.Free((void *)msg.pMsg);
		    }
		}
		
		// Note: Before exiting this thread, we must make sure that all of the
		// messages that we malloc'd are freed.
		pThis->debug_.DebugOut(kDbgLvlValuable, "EventDispatchTask() exiting %p\n", pThis);
		pThis->kernel_.CloseMessageQueue(g_hMsgQueueBG_, props);

		g_threadRunning_ = false;
		return pThis;
	}

public:
	static CEventModule*			pEventModule_;
	
	//------------------------------------------------------------------------
	CEventManagerImpl() 
		: ppListeners_(NULL), numListeners_(0), listSize_(0), 
		debug_(kGroupEvent), hMsgQueueFG_(kInvalidMessageQueueHndl)

	{		
		// 1) Create the message queue for communicating with the
		//    background dispatching thread
		// 2) Create the background event dispatching thread
		//
#ifdef DEBUG
		debug_.SetDebugLevel(kDbgLvlVerbose);
#endif
		debug_.DebugOut(kDbgLvlVerbose, "CEventManagerImpl::ctor: Initializing Event Manager\n");
		
		// Skip user event threads if daemon process
		char buf[256] = "\0";
	    if (readlink("/proc/self/exe", buf, sizeof(buf)) != -1)
		{
	    	if (strcasestr(buf, "Daemon") != NULL)
	    	{
	    		debug_.DebugOut(kDbgLvlImportant, "CEventManagerImpl::ctor: No user events for daemon process %s\n", buf);
	    		return;
	    	}
		}

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
		debug_.AssertNoErr(err, "CEventManagerImpl::ctor: Failed to create message queue!\n");
		debug_.DebugOut(kDbgLvlVerbose, "CEventManagerImpl::ctor: Message queue created %d\n", 
						static_cast<int>(hMsgQueueFG_));

		tTaskProperties properties;										//*2

		properties.TaskMainFcn = CEventManagerImpl::EventDispatchTask;
		properties.taskMainArgCount = 1;
		properties.pTaskMainArgValues = this;
		err = kernel_.CreateTask(g_hThread_, properties, NULL);
		
		debug_.Assert( kNoErr == err, "CEventManagerImpl::ctor: Failed to create EventDispatchTask!\n" );

		// Wait for thread to start up...
		while ( !g_threadRunning_ ) {
			kernel_.TaskSleep(10);
		}

#ifndef EMULATION
		// Load tslib pre-emptively during module creation to serialize plugin loading
		struct stat stbf;
		if (stat("/flags/notslib", &stbf) != 0) {
			tHndl handle;
			struct tsdev* tsl = NULL;
			LoadTSLib(pEventModule_, &handle, &tsl);
		}
#endif

		// Create additional Button/Power/USB driver polling thread
		properties.TaskMainFcn = CEventModule::ButtonPowerUSBTask;
		properties.pTaskMainArgValues = pEventModule_;
		g_hBPUThread_ = kInvalidTaskHndl;
		err = kernel_.CreateTask(g_hBPUThread_, properties, NULL);
		debug_.Assert( kNoErr == err, "CEventManagerImpl::ctor: Failed to create ButtonPowerUSBTask!\n" );
		debug_.DebugOut(kDbgLvlValuable, "Start ButtonPowerUSB Task ... !\n");

		// Create Cartridge thread for handling cart hotswap
		properties.TaskMainFcn = CEventModule::CartridgeTask;
		properties.pTaskMainArgValues = pEventModule_;
		g_hCartThread_ = kInvalidTaskHndl;
		err = kernel_.CreateTask(g_hCartThread_, properties, NULL);
		debug_.Assert( kNoErr == err, "CEventManagerImpl::ctor: Failed to create CartridgeTask!\n" );
		debug_.DebugOut(kDbgLvlValuable, "Start Cartridge task ...!\n");
	}
	
	//------------------------------------------------------------------------
	~CEventManagerImpl() 
	{
		void* 		retval = NULL;
		int			counter = 10;
		
		// Terminate Button/Power/USB driver polling thread first
		pEventModule_->bThreadRun_ = false;
		if (g_hCartThread_ != kInvalidTaskHndl) {
			debug_.DebugOut(kDbgLvlValuable, "%s: Terminating cart thread\n", __FUNCTION__);
			kernel_.JoinTask(g_hCartThread_, retval);
		}
		if (g_hBPUThread_ != kInvalidTaskHndl) {
			debug_.DebugOut(kDbgLvlValuable, "%s: Terminating button thread\n", __FUNCTION__);
			kernel_.JoinTask(g_hBPUThread_, retval);
		}
		// Terminate thread normally and dispose of listener list afterwards
		g_threadRun_ = false;
		debug_.DebugOut(kDbgLvlValuable, "%s: Terminating message thread\n", __FUNCTION__);
		while (g_threadRunning_ && --counter > 0)
			kernel_.TaskSleep(10);
		if (g_hThread_ != kInvalidTaskHndl)
			kernel_.JoinTask(g_hThread_, retval);
		debug_.DebugOut(kDbgLvlValuable, "%s: counter=%d, retval=%p\n", __FUNCTION__, counter, retval);

		if (ppListeners_)
			free(ppListeners_);
	}
	
	//------------------------------------------------------------------------
	void AddListener(const IEventListener* pListener, tEventRegistrationFlags flags)
	{
#ifdef DEBUG
		// Detect duplicate listeners, and re-register at end of list
		if (ppListeners_)
		{
			const IEventListener** ptr = ppListeners_;
			for (int i = 0; i < numListeners_; i++, ptr++)
			{
				if (pListener == *ptr)
				{
					debug_.DebugOut(kDbgLvlCritical, "%s: duplicate listener %p at index %d\n", __FUNCTION__, pListener, i);
					//RemoveListener(pListener);
					break;
				}
			}
		}
#endif
		pListener->pimpl_->eventRegistrationFlags = flags;
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
		if(numListeners_)
		{
			const IEventListener** last = ppListeners_ + numListeners_ - 1;
			if((*last)->pimpl_->eventRegistrationFlags & kEventRegistrationStayLastBitMask)
			{
				const IEventListener** next = ppListeners_ + numListeners_;
				*next = *last;
				*last = pListener;
			} else {
				const IEventListener** next = ppListeners_ + numListeners_;
				*next = pListener;
			}
		} else {
			const IEventListener** next = ppListeners_ + numListeners_;
			*next = pListener;
		}

		++numListeners_;
		debug_.DebugOut(kDbgLvlValuable, "%s: added listener %p, number %d\n", __FUNCTION__, pListener, (unsigned)numListeners_);
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
		
		debug_.DebugOut(kDbgLvlValuable, "%s: could not remove listener %p, number %d\n", __FUNCTION__, pListener, (unsigned)numListeners_);
		return kEventListenerNotRegisteredErr;
	}
	//------------------------------------------------------------------------
	void PostEventToChain( IEventListener *pListener, 
							const IEventMessage &msg ) const
	{
		while( pListener )
		{
			if( pListener->pimpl_->HandlesEvent(msg.GetEventType()) )
			{
				int ret = pListener->Notify(msg);
#ifdef DEBUG				
				debug_.DebugOut(kDbgLvlVerbose, "%s: posted to listener %p, type %08X, ret %d\n", __FUNCTION__, pListener, (unsigned)msg.GetEventType(), ret);
#endif
				if (ret == kEventStatusOKConsumed )
					break;
			}
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

		int size = msg.GetSizeInBytes();
		IEventMessage *msgCopy = (IEventMessage *)kernel_.Malloc(size);
		debug_.Assert(msgCopy != 0, "PostEvent -- failed to copy message!\n");
		memcpy(msgCopy, &msg, size);

		CEventDispatchMessage	dmsg(msgCopy, pResponse);					//*2
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
tTaskHndl			CEventManagerImpl::g_hBPUThread_	  = kInvalidTaskHndl;
tTaskHndl			CEventManagerImpl::g_hCartThread_	  = kInvalidTaskHndl;
tMutex				CEventManagerImpl::g_mutexEventList_ = PTHREAD_MUTEX_INITIALIZER;
CEventModule*		CEventManagerImpl::pEventModule_ = NULL;
	
namespace
{
	CEventManagerImpl* pinst = NULL;
}



//============================================================================
// Ctor & dtor
//============================================================================
//----------------------------------------------------------------------------
CEventModule::CEventModule()
: debug_(kGroupEvent), bThreadRun_(true)
{
	CEventManagerImpl::pEventModule_ = this;
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
	pinst->AddListener(pListener, flags);
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
