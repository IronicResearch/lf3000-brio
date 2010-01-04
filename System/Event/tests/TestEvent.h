// TestEvent.h

#include <cxxtest/TestSuite.h>
#include <boost/shared_array.hpp>
#include <EventMPI.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <StringTypes.h>
#include <UnitTestUtils.h>
#include <KernelMPI.h>

LF_USING_BRIO_NAMESPACE()
				
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
	MyUnitTestMsg(tEventType type, U32 payload)
		: IEventMessage(type), payload_(payload)
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
		msg_(kUndefinedEventType, 0)
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
// SlowUnitTestListener
//============================================================================
const tEventType kSlowHandledTypes[] = { kAllUnitTestEvents };
const int		kSlowDelay = 1000;
const int		kExpectedElapsedTimeDelta = 15;

class MySlowTestListener : public IEventListener
{
public:
	//------------------------------------------------------------------------
	MySlowTestListener( )
		: IEventListener(kSlowHandledTypes, ArrayCount(kSlowHandledTypes)),
		id_(kInvalidTaskHndl)
	{
	}
	//------------------------------------------------------------------------
	virtual tEventStatus Notify( const IEventMessage& /*msg*/ )
	{
		id_ = kernel.GetCurrentTask();
		kernel.TaskSleep(kSlowDelay);
		return kEventStatusOKConsumed;
	}
	
	//------------------------------------------------------------------------
	tTaskHndl	id_;
	CKernelMPI	kernel;
};


//============================================================================
// Constants
//============================================================================
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
const tEventPriority	kPriorityLow		= 200;

//============================================================================
// TestEvent functions
//============================================================================
class TestEvent : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CEventMPI*		eventmgr_;
public:
	//------------------------------------------------------------------------
	void setUp( )
	{
//		CDebugMPI	debug(kGroupUnitTests);
//		debug.SetDebugLevel(kDbgLvlVerbose);
		eventmgr_ = new CEventMPI();
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
		const CString*	pName;
		const CURI*		pURI;
		
		pName = eventmgr_->GetMPIName();
		TS_ASSERT_EQUALS( *pName, "EventMPI" );
		version = eventmgr_->GetModuleVersion();
		TS_ASSERT_EQUALS( version, 2 );
		pName = eventmgr_->GetModuleName();
		TS_ASSERT_EQUALS( *pName, "Event" );
		pURI = eventmgr_->GetModuleOrigin();
		TS_ASSERT_EQUALS( *pURI, "/LF/System/Event" );
	}
	
	//------------------------------------------------------------------------
	void testNoListeners( )
	{
		// Create a listener but don't register it, then fire event
		// and make sure listener was not notified.
		const tEventType kHandledTypes[] = { kUnitTestEvent1 };
		MyUnitTestListener listener(kHandledTypes, ArrayCount(kHandledTypes));
		TS_ASSERT( listener.IsReset() );
		MyUnitTestMsg	msg1(kUnitTestEvent1, kPayload1);
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
			TS_ASSERT_EQUALS( kEventListenerNotRegisteredErr, eventmgr_->UnregisterEventListener(pListener) );
			TS_ASSERT_EQUALS( kNoErr, eventmgr_->RegisterEventListener(pListener) );
		}
		TS_ASSERT_EQUALS( kEventListenerNotRegisteredErr, eventmgr_->UnregisterEventListener(pListener) );
	}
	
	//------------------------------------------------------------------------
	void testSingleListenerSingleEventWrongMsg( )
	{
		// Setup to listen for event1
		const tEventType kHandledTypes[] = { kUnitTestEvent1 };
		MyUnitTestListener listener(kHandledTypes, ArrayCount(kHandledTypes));
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->RegisterEventListener(&listener) );
		TS_ASSERT( listener.IsReset() );
		MyUnitTestMsg	msg2(kUnitTestEvent2, kPayload2);
		
		// Post event2 and verify no notification
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg2, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
	}
	
	//------------------------------------------------------------------------
	void testSingleListenerSingleEvent( )
	{
		// Setup to listen for event1
		const tEventType kHandledTypes[] = { kUnitTestEvent1 };
		MyUnitTestListener listener(kHandledTypes, ArrayCount(kHandledTypes));
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->RegisterEventListener(&listener) );
		TS_ASSERT( listener.IsReset() );
		MyUnitTestMsg	msg1(kUnitTestEvent1, kPayload1);
		
		// Post event1 and verify correct notification
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
		const MyUnitTestMsg& m = listener.GetEventMsg();
		TS_ASSERT_EQUALS( m.GetEventType(), kUnitTestEvent1 );
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
	void testSingleListenerMultiEventEnableDisable( )
	{
		// Setup to listen for event1
		const tEventType kHandledTypes[] = { kUnitTestEvent3, kUnitTestEvent4, kUnitTestEvent5 };
		MyUnitTestListener listener(kHandledTypes, ArrayCount(kHandledTypes));
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->RegisterEventListener(&listener) );
		TS_ASSERT( listener.IsReset() );
		MyUnitTestMsg	msg3(kUnitTestEvent3, kPayload3);
		MyUnitTestMsg	msg4(kUnitTestEvent4, kPayload4);
		MyUnitTestMsg	msg5(kUnitTestEvent5, kPayload5);
		
		// Post event3 and verify correct notification
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
		const MyUnitTestMsg& m = listener.GetEventMsg();
		TS_ASSERT_EQUALS( m.GetEventType(), kUnitTestEvent3 );
		TS_ASSERT_EQUALS( m.payload_, kPayload3 );
		
		// Post event4 and verify correct notification
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg4, kPriorityCritical) );
		TS_ASSERT_EQUALS( m.GetEventType(), kUnitTestEvent4 );
		TS_ASSERT_EQUALS( m.payload_, kPayload4 );
		
		// Post event5 and verify correct notification
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg5, kPriorityCritical) );
		TS_ASSERT_EQUALS( m.GetEventType(), kUnitTestEvent5 );
		TS_ASSERT_EQUALS( m.payload_, kPayload5 );

		// Disable notification for event3 & fire, make sure no notification
		listener.Reset();
		TS_ASSERT_EQUALS( kNoErr, listener.DisableNotifyForEventType(kUnitTestEvent3) );
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
		TS_ASSERT_EQUALS( kNoErr, listener.ReenableNotifyForEventType(kUnitTestEvent3) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );

		// Disable notification for event5 & fire, make sure no notification
		listener.Reset();
		TS_ASSERT_EQUALS( kNoErr, listener.DisableNotifyForEventType(kUnitTestEvent5) );
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
		TS_ASSERT_EQUALS( kNoErr, listener.ReenableNotifyForEventType(kUnitTestEvent5) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg5, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );

		// Disable notification for all 3 events & fire, make sure no notifications
		listener.Reset();
		TS_ASSERT_EQUALS( kNoErr, listener.DisableNotifyForEventType(kUnitTestEvent5) );
		TS_ASSERT_EQUALS( kNoErr, listener.DisableNotifyForEventType(kUnitTestEvent4) );
		TS_ASSERT_EQUALS( kNoErr, listener.DisableNotifyForEventType(kUnitTestEvent3) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg4, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg5, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		
		// Reenable 5, make sure only 5 notifies
		listener.Reset();
		TS_ASSERT_EQUALS( kNoErr, listener.ReenableNotifyForEventType(kUnitTestEvent5) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg4, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg5, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
		
		// Reenable 3, make sure only 3 & 5 notify
		listener.Reset();
		TS_ASSERT_EQUALS( kNoErr, listener.ReenableNotifyForEventType(kUnitTestEvent3) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
		listener.Reset();
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg4, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg5, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
	}
		
	//------------------------------------------------------------------------
	void testSingleListenerAllEvents( )
	{
		// Setup to listen for all events and verify all trigger notifications
		const tEventType kHandledTypes[] = { kAllUnitTestEvents };
		MyUnitTestListener listener(kHandledTypes, ArrayCount(kHandledTypes));
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->RegisterEventListener(&listener) );
		TS_ASSERT( listener.IsReset() );
		MyUnitTestMsg	msg1(kUnitTestEvent1, kPayload1);
		MyUnitTestMsg	msg3(kUnitTestEvent3, kPayload3);
		MyUnitTestMsg	msg5(kUnitTestEvent5, kPayload5);
		MyUnitTestMsg	msg7(kUnitTestEvent7, kPayload7);
		MyUnitTestMsg	msg9(kUnitTestEvent9, kPayload9);
		
		// Post event1 and verify correct notification
		const MyUnitTestMsg& m = listener.GetEventMsg();
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
		TS_ASSERT_EQUALS( m.GetEventType(), kUnitTestEvent1 );
		TS_ASSERT_EQUALS( m.payload_, kPayload1 );

		// Post event3 and verify correct notification
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT_EQUALS( m.GetEventType(), kUnitTestEvent3 );
		TS_ASSERT_EQUALS( m.payload_, kPayload3 );

		// Post event5 and verify correct notification
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg5, kPriorityCritical) );
		TS_ASSERT_EQUALS( m.GetEventType(), kUnitTestEvent5 );
		TS_ASSERT_EQUALS( m.payload_, kPayload5 );

		// Post event7 and verify correct notification
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg7, kPriorityCritical) );
		TS_ASSERT_EQUALS( m.GetEventType(), kUnitTestEvent7 );
		TS_ASSERT_EQUALS( m.payload_, kPayload7 );

		// Post event9 and verify correct notification
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg9, kPriorityCritical) );
		TS_ASSERT_EQUALS( m.GetEventType(), kUnitTestEvent9 );
		TS_ASSERT_EQUALS( m.payload_, kPayload9 );
	}

	//------------------------------------------------------------------------
	void testSingleListenerAllEventsDisableSome( )
	{
		// Setup to listen for all events and verify all trigger notifications
		const tEventType kHandledTypes[] = { kAllUnitTestEvents };
		MyUnitTestListener listener(kHandledTypes, ArrayCount(kHandledTypes));
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->RegisterEventListener(&listener) );
		TS_ASSERT( listener.IsReset() );
		MyUnitTestMsg	msg1(kUnitTestEvent1, kPayload1);
		MyUnitTestMsg	msg3(kUnitTestEvent3, kPayload3);
		MyUnitTestMsg	msg9(kUnitTestEvent9, kPayload9);
		
		// Disable notification for 3 & fire, make sure no notification for 3
		listener.Reset();
		TS_ASSERT_EQUALS( kNoErr, listener.DisableNotifyForEventType(kUnitTestEvent3) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
		listener.Reset();
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg9, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );

		// Reenable 3, make sure all notify
		listener.Reset();
		TS_ASSERT_EQUALS( kNoErr, listener.ReenableNotifyForEventType(kUnitTestEvent3) );
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
		TS_ASSERT_EQUALS( kNoErr, listener.DisableNotifyForEventType(kUnitTestEvent1) );
		TS_ASSERT_EQUALS( kNoErr, listener.DisableNotifyForEventType(kUnitTestEvent3) );
		TS_ASSERT_EQUALS( kNoErr, listener.DisableNotifyForEventType(kUnitTestEvent9) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg9, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );

		// Reenable 3, make sure all it gets notification
		listener.Reset();
		TS_ASSERT_EQUALS( kNoErr, listener.ReenableNotifyForEventType(kUnitTestEvent3) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg9, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( !listener.IsReset() );
	}
	
	//------------------------------------------------------------------------
	void testMultipleListeners( )
	{
		// Setup 3 listeners for event1
		const tEventType kHandledTypes[] = { kUnitTestEvent1 };
		MyUnitTestListener listener1(kHandledTypes, ArrayCount(kHandledTypes));
		MyUnitTestListener listener2(kHandledTypes, ArrayCount(kHandledTypes));
		MyUnitTestListener listener3(kHandledTypes, ArrayCount(kHandledTypes));
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->RegisterEventListener(&listener1) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->RegisterEventListener(&listener2) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->RegisterEventListener(&listener3) );
		MyUnitTestMsg	msg1(kUnitTestEvent1, kPayload1);
		
		// Post event1 and verify all 3 listeners got notified
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical) );
		TS_ASSERT( !listener1.IsReset() );
		TS_ASSERT( !listener2.IsReset() );
		TS_ASSERT( !listener3.IsReset() );
		listener1.Reset();
		listener2.Reset();
		listener3.Reset();
			
		// Unregister listener1 and verify other two still get notified
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->UnregisterEventListener(&listener1) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical) );
		TS_ASSERT( listener1.IsReset() );
		TS_ASSERT( !listener2.IsReset() );
		TS_ASSERT( !listener3.IsReset() );
		listener1.Reset();
		listener2.Reset();
		listener3.Reset();
			
		// Unregister listener3 and verify other two still get notified
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->UnregisterEventListener(&listener3) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical) );
		TS_ASSERT( listener1.IsReset() );
		TS_ASSERT( !listener2.IsReset() );
		TS_ASSERT( listener3.IsReset() );
	}
	
	//------------------------------------------------------------------------
	void testOverrideListener( )
	{
		// Setup override listener for event1
		const tEventType kHandledTypes[] = { kUnitTestEvent1 };
		MyUnitTestListener listener(kHandledTypes, ArrayCount(kHandledTypes));
		MyUnitTestMsg	msg1(kUnitTestEvent1, kPayload1);
		
		// Fire the event with no override
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical) );
		TS_ASSERT( listener.IsReset() );
		listener.Reset();
		
		// Fire the event to the override listener
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical, &listener) );
		TS_ASSERT( !listener.IsReset() );
	}
	
	//------------------------------------------------------------------------
	void testListenerChainErrors( )
	{
		// Chain 3 listners and verify correct one fires.
		// Then add "All" listener to head and verify it grabs all events.
		// Setup override listener for event1
		const tEventType kHandledTypes1[] = { kUnitTestEvent1 };
		const tEventType kHandledTypes2[] = { kUnitTestEvent2 };
		MyUnitTestListener listener1(kHandledTypes1, ArrayCount(kHandledTypes1));
		MyUnitTestListener listener2(kHandledTypes2, ArrayCount(kHandledTypes2));
		TS_ASSERT_EQUALS( kEventListenerCycleErr, listener1.SetNextListener(&listener1) );
		TS_ASSERT_EQUALS( kNoErr, listener1.SetNextListener(&listener2) );
		TS_ASSERT_EQUALS( kEventListenerCycleErr, listener2.SetNextListener(&listener1) );
	}
	
	//------------------------------------------------------------------------
	void testListenerChains( )
	{
		// Chain 3 listners and verify correct one fires.
		const tEventType kHandledTypes1[] = { kUnitTestEvent1 };
		const tEventType kHandledTypes2[] = { kUnitTestEvent2 };
		const tEventType kHandledTypes3[] = { kUnitTestEvent3 };
		const tEventType kHandledTypes4[] = { kUnitTestEvent4 };
		MyUnitTestListener listener1(kHandledTypes1, ArrayCount(kHandledTypes1));
		MyUnitTestListener listener2(kHandledTypes2, ArrayCount(kHandledTypes2));
		MyUnitTestListener listener3(kHandledTypes3, ArrayCount(kHandledTypes3));
		MyUnitTestListener listener4(kHandledTypes4, ArrayCount(kHandledTypes4));
		MyUnitTestMsg	msg1(kUnitTestEvent1, kPayload1);
		MyUnitTestMsg	msg2(kUnitTestEvent2, kPayload2);
		MyUnitTestMsg	msg3(kUnitTestEvent3, kPayload3);
		MyUnitTestMsg	msg4(kUnitTestEvent4, kPayload4);
		TS_ASSERT_EQUALS( kNoErr, listener1.SetNextListener(&listener2) );
		TS_ASSERT_EQUALS( kNoErr, listener2.SetNextListener(&listener3) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->RegisterEventListener(&listener1) );
		
		// Fire event1 and make sure only listener 1 is notified
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical) );
		TS_ASSERT( !listener1.IsReset() );
		TS_ASSERT( listener2.IsReset() );
		TS_ASSERT( listener3.IsReset() );
		listener1.Reset();
		listener2.Reset();
		listener3.Reset();
		
		// Fire event2 and make sure only listener 2 is notified
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg2, kPriorityCritical) );
		TS_ASSERT( listener1.IsReset() );
		TS_ASSERT( !listener2.IsReset() );
		TS_ASSERT( listener3.IsReset() );
		listener1.Reset();
		listener2.Reset();
		listener3.Reset();
		
		// Fire event1 and make sure only listener 3 is notified
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( listener1.IsReset() );
		TS_ASSERT( listener2.IsReset() );
		TS_ASSERT( !listener3.IsReset() );
		listener1.Reset();
		listener2.Reset();
		listener3.Reset();

		// Then reset listener 1 to All and verify 2 & 3 are no longer in chain
		TS_ASSERT_EQUALS( kNoErr, listener1.SetNextListener(&listener4) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg2, kPriorityCritical) );
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg3, kPriorityCritical) );
		TS_ASSERT( listener1.IsReset() );
		TS_ASSERT( listener2.IsReset() );
		TS_ASSERT( listener3.IsReset() );
		TS_ASSERT( listener4.IsReset() );

		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg4, kPriorityCritical) );
		TS_ASSERT( listener1.IsReset() );
		TS_ASSERT( !listener4.IsReset() ); 
	}
	
	//------------------------------------------------------------------------
	void testCriticalDispatchBlocksInSameThread( )
	{
		// Setup listener that delays and records the thread id
		MySlowTestListener listener;
		MyUnitTestMsg	msg1(kUnitTestEvent1, kPayload1);
		
		// Fire an event and verify we stay in same thread and block
		CKernelMPI	kernel;
		U32 start = kernel.GetElapsedTimeAsMSecs();
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityCritical, &listener) );
		U32 end = kernel.GetElapsedTimeAsMSecs();
		TS_ASSERT_EQUALS( listener.id_, kernel.GetCurrentTask() );
		TS_ASSERT_DELTA( start + kSlowDelay, end, kExpectedElapsedTimeDelta );
	}
	
	//------------------------------------------------------------------------
	void testNonPriorityDispatchesSwitchThreads( )
	{
		// Setup listener that delays and records the thread id
		MySlowTestListener listener;
		MyUnitTestMsg	msg1(kUnitTestEvent1, kPayload1);
		
		// Fire an event and verify we stay in same thread and block
		CKernelMPI	kernel;
		U32 start = kernel.GetElapsedTimeAsMSecs();
		TS_ASSERT_EQUALS( kNoErr, eventmgr_->PostEvent(msg1, kPriorityLow, &listener) );
		U32 end = kernel.GetElapsedTimeAsMSecs();
		kernel.TaskSleep(100);
		TS_ASSERT_DIFFERS( listener.id_, kInvalidTaskHndl );
		TS_ASSERT_DIFFERS( listener.id_, kernel.GetCurrentTask() );
		TS_ASSERT_DELTA( start, end, kExpectedElapsedTimeDelta );
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
