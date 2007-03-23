// TestEventMgr.h

#include <cxxtest/TestSuite.h>
#include <boost/shared_array.hpp>
#include <EventMgrMPI.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <StringTypes.h>

					
// TODO/tp: Move to common header?
//============================================================================
// MyEventListener
//============================================================================
class MyEventListener : public IEventListener
{
public:
	//------------------------------------------------------------------------
	MyEventListener( const tEventType* ptypes, U32 count ) 
		: IEventListener(ptypes, count)
	{
	}
	//------------------------------------------------------------------------
	virtual tEventStatus Notify( const IEventMessage &msg )
	{
		U16 size		= msg.GetSizeInBytes();
		msg_			= boost::shared_array<U8>(new U8[size]);
		memcpy(msg_.get(), &msg, size);
		return kEventStatusOKConsumed;
	}
	//------------------------------------------------------------------------
	const IEventMessage* GetEventMsg() const
	{
		return reinterpret_cast<const IEventMessage*>(msg_.get());
	}
	//------------------------------------------------------------------------
	void Reset()
	{
		msg_.reset();
	}
	
private:
	//------------------------------------------------------------------------
	boost::shared_array<U8>	msg_;
};

//============================================================================
// MyUnitTestMsg
//============================================================================
class MyUnitTestMsg : public IEventMessage
{
public:
	MyUnitTestMsg(tEventType type, tRsrcHndl handle, U32 payload)
		: IEventMessage(type, handle), payload_(payload)
	{
	}
	virtual U16	GetSizeInBytes() const	{ return sizeof(MyUnitTestMsg); }
	U32	payload_;
};

//============================================================================
// MyUnitTestListener
//============================================================================
class MyUnitTestListener : public IEventListener
{
public:
	//------------------------------------------------------------------------
	MyUnitTestListener( const tEventType* ptypes, U32 count ) 
		: IEventListener(ptypes, count), 
		msg_(kUndefinedEventType, kUndefinedRsrcType, 0)
	{
		Reset();
	}
	//------------------------------------------------------------------------
	virtual tEventStatus Notify( const IEventMessage &msg )
	{
		U16 size		= msg.GetSizeInBytes();
		memcpy(&msg_, &msg, size);
		return kEventStatusOKConsumed;
	}
	//------------------------------------------------------------------------
	const MyUnitTestMsg& GetEventMsg() const
	{
		return msg_;
	}
	//------------------------------------------------------------------------
	void Reset()
	{
		memset(&msg_, 0, sizeof(MyUnitTestMsg));
	}
	//------------------------------------------------------------------------
	bool IsReset()
	{
		return msg_.GetEventType() == 0;
	}
	
private:
	//------------------------------------------------------------------------
	MyUnitTestMsg	msg_;
};


//============================================================================
// Constants
//============================================================================
const tRsrcHndl	kUnitTestRsrcHndl1 = 0x1234;
const tRsrcHndl	kUnitTestRsrcHndl2 = 0x2345;
const tRsrcHndl	kUnitTestRsrcHndl3 = 0x3456;
const tRsrcHndl	kUnitTestRsrcHndl4 = 0x4567;
const tRsrcHndl	kUnitTestRsrcHndl5 = 0x5678;
const tRsrcHndl	kUnitTestRsrcHndl6 = 0x6789;
const tRsrcHndl	kUnitTestRsrcHndl7 = 0x7890;
const tRsrcHndl	kUnitTestRsrcHndl8 = 0x8901;
const tRsrcHndl	kUnitTestRsrcHndl9 = 0x9012;

const U32 kPayload1 = 111;
const U32 kPayload2 = 222;
const U32 kPayload3 = 333;
const U32 kPayload4 = 444;
const U32 kPayload5 = 555;
const U32 kPayload6 = 666;
const U32 kPayload7 = 777;
const U32 kPayload8 = 888;
const U32 kPayload9 = 999;


const tEventPriority	kPriorityCritical	= 1;
const tEventPriority	kPriorityMedium		= 50;
const tEventPriority	kPriorityLow		= 200;

//============================================================================
// TestEventMgr functions
//============================================================================
class TestEventMgr : public CxxTest::TestSuite 
{
private:
	CEventMgrMPI*		eventmgr_;
public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		eventmgr_ = new CEventMgrMPI();
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete eventmgr_; 
	}
	
	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		TS_ASSERT( eventmgr_ != NULL );
		TS_ASSERT( eventmgr_->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		tVersion		version;
		CString			empty;
		ConstPtrCString	pName = &empty;
		ConstPtrCURI	pURI = &empty;
		
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->GetMPIVersion(version) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->GetMPIName(pName) );
		TS_ASSERT_EQUALS( version, MakeVersion(0, 1) );
		TS_ASSERT_EQUALS( *pName, "EventMgrMPI" );

		TS_ASSERT_EQUALS( kNoErr, eventmgr_->GetModuleVersion(version) );
		TS_ASSERT_EQUALS( version, MakeVersion(0, 1) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->GetModuleName(pName) );
		TS_ASSERT_EQUALS( *pName, "Event" );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->GetModuleOrigin(pURI) );
		TS_ASSERT_EQUALS( *pURI, "EventMgr FIXME" );
	}
	
	//------------------------------------------------------------------------
	void testNoListeners( )
	{
		// Create a listener but don't register it, then fire event
		// and make sure listener was not notified.
		const tEventType kHandledTypes[] = { kUnitTestEvent1 };
		MyUnitTestListener listener(kHandledTypes, ArrayCount(kHandledTypes));
		TS_ASSERT( listener.IsReset() );
		MyUnitTestMsg	msg1(kUnitTestEvent1, kUnitTestRsrcHndl1, kPayload1);
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
	}
	
	//------------------------------------------------------------------------
	void testRegisterUnregisterScope( )
	{
		// Verify that we can register and unregister listeners, and that
		// going out of scope causes a listener to unregister itself.
		MyUnitTestListener* pListener = NULL;
		{
			const tEventType kHandledTypes[] = { kUnitTestEvent1 };
			MyUnitTestListener listener(kHandledTypes, ArrayCount(kHandledTypes));
			pListener = &listener;
			TS_ASSERT_EQUALS( kNoErr, eventmgr_->RegisterEventListener(pListener) );
			TS_ASSERT_EQUALS( kNoErr, eventmgr_->UnregisterEventListener(pListener) );
			TS_ASSERT_EQUALS( kNoErr, eventmgr_->RegisterEventListener(pListener) );
		}
		TS_ASSERT_EQUALS( kEventListenerNotRegisteredErr, eventmgr_->UnregisterEventListener(pListener) );
	}
	
	//------------------------------------------------------------------------
	void testAsyncSingleListenerSingleEventWrongMsg( )
	{
		// Setup to listen for event1
		const tEventType kHandledTypes[] = { kUnitTestEvent1 };
		MyUnitTestListener listener(kHandledTypes, ArrayCount(kHandledTypes));
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->RegisterEventListener(&listener) );
		TS_ASSERT( listener.IsReset() );
		MyUnitTestMsg	msg2(kUnitTestEvent2, kUnitTestRsrcHndl2, kPayload2);
		
		// Post event2 and verify no notification
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg2, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
	}
	
	//------------------------------------------------------------------------
	void testAsyncSingleListenerSingleEvent( )
	{
		// Setup to listen for event1
		const tEventType kHandledTypes[] = { kUnitTestEvent1 };
		MyUnitTestListener listener(kHandledTypes, ArrayCount(kHandledTypes));
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->RegisterEventListener(&listener) );
		TS_ASSERT( listener.IsReset() );
		MyUnitTestMsg	msg1(kUnitTestEvent1, kUnitTestRsrcHndl1, kPayload1);
		
		// Post event1 and verify correct notification
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
		const MyUnitTestMsg& m = listener.GetEventMsg();
		TS_ASSERT_EQUALS( m.GetEventType(), kUnitTestEvent1 );
		TS_ASSERT_EQUALS( m.GetEventSource(), kUnitTestRsrcHndl1 );
		TS_ASSERT_EQUALS( m.payload_, kPayload1 );

		// Unregister listener, then post event and verify listener was not notified
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->UnregisterEventListener(&listener) );
		listener.Reset();
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
	}
	
	//------------------------------------------------------------------------
	void testDisableReenableTypes( )
	{
		// Try disabling & reenabling
		const tEventType kHandledTypes[] = { kUnitTestEvent3, kUnitTestEvent4, kUnitTestEvent5 };
		MyUnitTestListener listener(kHandledTypes, ArrayCount(kHandledTypes));
		TS_ASSERT_EQUALS( kEventTypeNotFoundErr, listener.DisableNotifyForEventType(kUnitTestEvent1) );
		TS_ASSERT_EQUALS( kEventTypeNotFoundErr, listener.ReenableNotifyForEventType(kUnitTestEvent1) );
		TS_ASSERT_EQUALS( kNoErr, listener.DisableNotifyForEventType(kUnitTestEvent3) );
		TS_ASSERT_EQUALS( kNoErr, listener.DisableNotifyForEventType(kUnitTestEvent3) );
		TS_ASSERT_EQUALS( kNoErr, listener.ReenableNotifyForEventType(kUnitTestEvent3) );
		TS_ASSERT_EQUALS( kEventTypeNotFoundErr, listener.ReenableNotifyForEventType(kUnitTestEvent3) );

		TS_ASSERT_EQUALS( kNoErr, listener.DisableNotifyForEventType(kUnitTestEvent4) );
		TS_ASSERT_EQUALS( kNoErr, listener.ReenableNotifyForEventType(kUnitTestEvent4) );
		TS_ASSERT_EQUALS( kEventTypeNotFoundErr, listener.ReenableNotifyForEventType(kUnitTestEvent4) );

		TS_ASSERT_EQUALS( kNoErr, listener.DisableNotifyForEventType(kUnitTestEvent5) );
		TS_ASSERT_EQUALS( kNoErr, listener.ReenableNotifyForEventType(kUnitTestEvent5) );
		TS_ASSERT_EQUALS( kEventTypeNotFoundErr, listener.ReenableNotifyForEventType(kUnitTestEvent5) );
	}
	
	//------------------------------------------------------------------------
	void testAsyncSingleListenerMultiEventEnableDisable( )
	{
		// Setup to listen for event1
		const tEventType kHandledTypes[] = { kUnitTestEvent3, kUnitTestEvent4, kUnitTestEvent5 };
		MyUnitTestListener listener(kHandledTypes, ArrayCount(kHandledTypes));
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->RegisterEventListener(&listener) );
		TS_ASSERT( listener.IsReset() );
		MyUnitTestMsg	msg3(kUnitTestEvent3, kUnitTestRsrcHndl3, kPayload3);
		MyUnitTestMsg	msg4(kUnitTestEvent4, kUnitTestRsrcHndl4, kPayload4);
		MyUnitTestMsg	msg5(kUnitTestEvent5, kUnitTestRsrcHndl5, kPayload5);
		
		// Post event3 and verify correct notification
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
		const MyUnitTestMsg& m = listener.GetEventMsg();
		TS_ASSERT_EQUALS( m.GetEventType(), kUnitTestEvent3 );
		TS_ASSERT_EQUALS( m.GetEventSource(), kUnitTestRsrcHndl3 );
		TS_ASSERT_EQUALS( m.payload_, kPayload3 );
		
		// Post event4 and verify correct notification
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg4, kPriorityCritical) );
		TS_ASSERT_EQUALS( m.GetEventType(), kUnitTestEvent4 );
		TS_ASSERT_EQUALS( m.GetEventSource(), kUnitTestRsrcHndl4 );
		TS_ASSERT_EQUALS( m.payload_, kPayload4 );
		
		// Post event5 and verify correct notification
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg5, kPriorityCritical) );
		TS_ASSERT_EQUALS( m.GetEventType(), kUnitTestEvent5 );
		TS_ASSERT_EQUALS( m.GetEventSource(), kUnitTestRsrcHndl5 );
		TS_ASSERT_EQUALS( m.payload_, kPayload5 );

		// Disable notification for event3 & fire, make sure no notification
		listener.Reset();
		listener.DisableNotifyForEventType(kUnitTestEvent3);
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->UnregisterEventListener(&listener) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );

		// Make sure other events still trigger
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg4, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
		listener.Reset();
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg5, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );

		// Reenable and event3 and verify triggers again
		listener.Reset();
		listener.ReenableNotifyForEventType(kUnitTestEvent3);
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );

		// Disable notification for event5 & fire, make sure no notification
		listener.Reset();
		listener.DisableNotifyForEventType(kUnitTestEvent5);
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->UnregisterEventListener(&listener) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg5, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );

		// Make sure other events still trigger
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
		listener.Reset();
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg4, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );

		// Reenable and event5 and verify triggers again
		listener.Reset();
		listener.ReenableNotifyForEventType(kUnitTestEvent5);
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg5, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );

		// Disable notification for all 3 events & fire, make sure no notifications
		listener.Reset();
		listener.DisableNotifyForEventType(kUnitTestEvent5);
		listener.DisableNotifyForEventType(kUnitTestEvent4);
		listener.DisableNotifyForEventType(kUnitTestEvent3);
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg4, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg5, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		
		// Reenable 5, make sure only 5 notifies
		listener.Reset();
		listener.ReenableNotifyForEventType(kUnitTestEvent5);
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg4, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg5, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
		
		// Reenable 3, make sure only 3 & 5 notify
		listener.Reset();
		listener.ReenableNotifyForEventType(kUnitTestEvent3);
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
		listener.Reset();
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg4, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg5, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
	}
	
	//------------------------------------------------------------------------
	void testAsyncSingleListenerAllEvents( )
	{
		// Setup to listen for all events and verify all trigger notifications
		const tEventType kHandledTypes[] = { kAllUnitTestEvents };
		MyUnitTestListener listener(kHandledTypes, ArrayCount(kHandledTypes));
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->RegisterEventListener(&listener) );
		TS_ASSERT( listener.IsReset() );
		MyUnitTestMsg	msg1(kUnitTestEvent1, kUnitTestRsrcHndl1, kPayload1);
		MyUnitTestMsg	msg3(kUnitTestEvent3, kUnitTestRsrcHndl3, kPayload3);
		MyUnitTestMsg	msg5(kUnitTestEvent5, kUnitTestRsrcHndl5, kPayload5);
		MyUnitTestMsg	msg7(kUnitTestEvent7, kUnitTestRsrcHndl7, kPayload7);
		MyUnitTestMsg	msg9(kUnitTestEvent9, kUnitTestRsrcHndl9, kPayload9);
		
		// Post event1 and verify correct notification
		const MyUnitTestMsg& m = listener.GetEventMsg();
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
		TS_ASSERT_EQUALS( m.GetEventType(), kUnitTestEvent1 );
		TS_ASSERT_EQUALS( m.GetEventSource(), kUnitTestRsrcHndl1 );
		TS_ASSERT_EQUALS( m.payload_, kPayload1 );

		// Post event3 and verify correct notification
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT_EQUALS( m.GetEventType(), kUnitTestEvent3 );
		TS_ASSERT_EQUALS( m.GetEventSource(), kUnitTestRsrcHndl3 );
		TS_ASSERT_EQUALS( m.payload_, kPayload5 );

		// Post event5 and verify correct notification
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg5, kPriorityCritical) );
		TS_ASSERT_EQUALS( m.GetEventType(), kUnitTestEvent5 );
		TS_ASSERT_EQUALS( m.GetEventSource(), kUnitTestRsrcHndl5 );
		TS_ASSERT_EQUALS( m.payload_, kPayload5 );

		// Post event7 and verify correct notification
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg7, kPriorityCritical) );
		TS_ASSERT_EQUALS( m.GetEventType(), kUnitTestEvent7 );
		TS_ASSERT_EQUALS( m.GetEventSource(), kUnitTestRsrcHndl7 );
		TS_ASSERT_EQUALS( m.payload_, kPayload7 );

		// Post event9 and verify correct notification
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg9, kPriorityCritical) );
		TS_ASSERT_EQUALS( m.GetEventType(), kUnitTestEvent9 );
		TS_ASSERT_EQUALS( m.GetEventSource(), kUnitTestRsrcHndl9 );
		TS_ASSERT_EQUALS( m.payload_, kPayload9 );
	}

	//------------------------------------------------------------------------
	void testAsyncSingleListenerAllEventsDisableSome( )
	{
		// Setup to listen for all events and verify all trigger notifications
		const tEventType kHandledTypes[] = { kAllUnitTestEvents };
		MyUnitTestListener listener(kHandledTypes, ArrayCount(kHandledTypes));
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->RegisterEventListener(&listener) );
		TS_ASSERT( listener.IsReset() );
		MyUnitTestMsg	msg1(kUnitTestEvent1, kUnitTestRsrcHndl1, kPayload1);
		MyUnitTestMsg	msg3(kUnitTestEvent3, kUnitTestRsrcHndl3, kPayload3);
		MyUnitTestMsg	msg9(kUnitTestEvent9, kUnitTestRsrcHndl9, kPayload9);
		
		// Disable notification for 3 & fire, make sure no notification for 3
		listener.Reset();
		listener.DisableNotifyForEventType(kUnitTestEvent3);
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
		listener.Reset();
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg9, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );

		// Reenable 3, make sure all notify
		listener.Reset();
		listener.ReenableNotifyForEventType(kUnitTestEvent3);
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
		listener.Reset();
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg9, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
		listener.Reset();
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );

		// Disable notification for multiple, make sure no notification for them
		listener.Reset();
		listener.DisableNotifyForEventType(kUnitTestEvent1);
		listener.DisableNotifyForEventType(kUnitTestEvent3);
		listener.DisableNotifyForEventType(kUnitTestEvent9);
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg9, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );

		// Reenable 3, make sure all it gets notification
		listener.Reset();
		listener.ReenableNotifyForEventType(kUnitTestEvent3);
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg9, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
	}
	
	//------------------------------------------------------------------------
	void testResponseListener( )
	{
	}
	
	//------------------------------------------------------------------------
	void testOverrideListener( )
	{
	}
	
	//------------------------------------------------------------------------
	void testListenerChains( )
	{
		// Chain 3 listners and verify correct one fires.
		// Then add "All" listener to head and verify it grabs all events.
	}
	
	//------------------------------------------------------------------------
	void testCriticalPostBlocks( )
	{
	}
	
	//------------------------------------------------------------------------
	void testNonPriorityDispatchesSwitchThreads( )
	{
	}
	
	//------------------------------------------------------------------------
	void testNonCriticalPostDoesntBlock( )
	{
	}
	
	//------------------------------------------------------------------------
	void disabled_testTimer( )
	{
		/*
		const tEventType kHandledTypes[] = { kTimerEvent };
		tListenerId			id;
		MyUnitTestListener		listener(kHandledTypes);
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->RegisterForResponseEvent(id, &listener_) );
		TS_ASSERT( listener.IsReset() );

		tTimerHndl			timer;
		tTimerProperties	props;
		props.milliseconds = 500;
		kernelmgr = boost::scoped_ptr(new CKernelMgrMPI());
		TS_ASSERT_EQUALS( kNoErr, kernelmgr->CreateTimer(timer, props) );
		TS_ASSERT( listener.IsReset() );
		usleep(1000);
		TS_ASSERT( !listener.IsReset() );
		const CTimerMsg* pMsg = reinterpret_cast<const CTimerMgs*>(listner.GetEventMsg());
		TS_ASSERT_EQUALS( kEventTimerFired, pMsg->GetEventType() );
		TS_ASSERT_EQUALS( kNoErr, kernelmgr->DestroyTimer(timer) );
		*/
	}
	
};
