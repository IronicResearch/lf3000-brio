// TestTimer.h

#include <cxxtest/TestSuite.h>
#include <sys/time.h>
#include <EventMPI.h>
#include <Timer.h>
#include <UnitTestUtils.h>
#include <KernelMPI.h>
#include <GroupEnumeration.h>


LF_USING_BRIO_NAMESPACE()
				
#if 1 // FIXME/BSK
	struct timeval timePrint;
#endif

//============================================================================
// MyTimerListener
//============================================================================
// FIXME/BSK											  
const tEventType kTimerFiredEvent = AllEvents(kGroupTimer);
const tEventType kTimerTypes[] = { kTimerFiredEvent };

class MyTimerListener : public IEventListener
{
public:
	//------------------------------------------------------------------------
	MyTimerListener( )
		: IEventListener(kTimerTypes, ArrayCount(kTimerTypes)),
		id_(kInvalidTimerHndl)
	{
#if 0 // FIXME/BSK
		printf("MyTimerListener constructor\n");
		fflush(stdout);
#endif
	}

//------------------------------------------------------------------------
	virtual tEventStatus Notify( const IEventMessage &msg )
	{
		const CTimerMessage& m = reinterpret_cast<const CTimerMessage&>(msg);
		id_ = m.GetTimerHndl();

#if 0 // FIXME/BSK
		printf("tEventStatus Notify id=%u\n",id_);
		fflush(stdout);
#endif
		return kEventStatusOKConsumed;
	}
	//------------------------------------------------------------------------
	bool IsReset( ) const
	{
		return id_ == kInvalidTimerHndl;
	}
	
	//------------------------------------------------------------------------
	tTimerHndl	id_;
};


//============================================================================
// TestEvent functions
//============================================================================
class TestEventTimer : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CKernelMPI*			kernel_;
	MyTimerListener*	listener_;
	CEventMPI*			event_;
public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		kernel_ = new CKernelMPI();
		event_ = new CEventMPI();
		listener_ = new MyTimerListener();
		event_->RegisterEventListener(listener_);
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete listener_;
		delete event_; 
		delete kernel_; 
	}
	
	//------------------------------------------------------------------------
	void testTimerNotFired( )
	{
		static const  tTimerProperties props = {TIMER_ABSTIME_SET,
												 	{0, 0, 0, 100000000},
			                                     };
		TS_ASSERT( listener_->IsReset() );
		COneShotTimer	timer(props);
		kernel_->TaskSleep(200);
		TS_ASSERT( listener_->IsReset() );

	}
	
	//------------------------------------------------------------------------
	void xtestTimerFires( )
	{
		static const  tTimerProperties props = {TIMER_ABSTIME_SET,
												 	{0, 0, 0, 100000000},
			                                    };
		TS_ASSERT( listener_->IsReset() );
		COneShotTimer	timer(props);
		timer.Start(props);

#if 0 // FIXME/BSK
		gettimeofday( &timePrint, NULL );
		printf("\n%d.%d   testTimerFires 1 \n", timePrint.tv_sec, timePrint.tv_usec / 1000 );
		fflush(stdout);
#endif		

		kernel_->TaskSleep(50);
		TS_ASSERT( listener_->IsReset() );
		kernel_->TaskSleep(100);

#if 0 // FIXME/BSK
		gettimeofday( &timePrint, NULL );
		printf("%d.%d   testTimerFires 2 \n", timePrint.tv_sec, timePrint.tv_usec / 1000 );
		fflush(stdout);
#endif		

		TS_ASSERT( !listener_->IsReset() );
		TS_ASSERT_EQUALS( timer.GetTimerHndl(), listener_->id_ );

#if 0 // FIXME/BSK
		gettimeofday( &timePrint, NULL );
		printf("%d.%d   testTimerFires 3 \n", timePrint.tv_sec, timePrint.tv_usec / 1000 );
		fflush(stdout);
#endif		
	}
	
	//------------------------------------------------------------------------
	void xtestTimerPauseResume( )
	{
		static const  tTimerProperties props = {TIMER_ABSTIME_SET,
												 	{0, 0, 0, 100000000},
			                                    };
		saveTimerSettings saveValue;
		TS_ASSERT( listener_->IsReset() );
		COneShotTimer	timer(props);
		timer.Start(props);
		kernel_->TaskSleep(50);
		TS_ASSERT( listener_->IsReset() );
		timer.Pause(saveValue);
		kernel_->TaskSleep(100);
		TS_ASSERT( listener_->IsReset() );
		timer.Resume(saveValue);
		kernel_->TaskSleep(100);
		TS_ASSERT( !listener_->IsReset() );
		TS_ASSERT_EQUALS( timer.GetTimerHndl(), listener_->id_ );
	}
	
	//------------------------------------------------------------------------
	void xtestTimerRestart( )
	{
		static const  tTimerProperties props = {TIMER_ABSTIME_SET,
												 	{0, 0, 0, 100000000},
			                                     };
		TS_ASSERT( listener_->IsReset() );
		COneShotTimer	timer(props);
		for (int ii = 0; ii < 3; ++ii )
		{
			timer.Start(props);
			kernel_->TaskSleep(50);
			TS_ASSERT( listener_->IsReset() );
		}
		kernel_->TaskSleep(100);
		TS_ASSERT( !listener_->IsReset() );
		TS_ASSERT_EQUALS( timer.GetTimerHndl(), listener_->id_ );
	}
	
	//------------------------------------------------------------------------
	void xtestTimerElapsedTime( )
	{
		static const  tTimerProperties props = {TIMER_ABSTIME_SET,
												 	{0, 0, 0, 100000000},
			                                     };
		U32 pUs;
		TS_ASSERT( listener_->IsReset() );
		COneShotTimer	timer(props);
		U32 start = kernel_->GetElapsedTime( &pUs );
		timer.Start(props);
		const int kSleepInterval = 100;
		const int kDelta = 10;
		int i = 0;
		for (int ii = i; ii < 9; ++ii)
		{
			kernel_->TaskSleep(kSleepInterval);
			U32 elapsed = kernel_->GetTimerElapsedTime(timer.GetTimerHndl());
			U32 remaining = kernel_->GetTimerRemainingTime(timer.GetTimerHndl());
			TS_ASSERT_DELTA( elapsed, ii * kSleepInterval, kDelta );
			TS_ASSERT_DELTA( remaining, (10 - ii) * kSleepInterval, kDelta );
		}
		kernel_->TaskSleep(200);
		TS_ASSERT( !listener_->IsReset() );
		TS_ASSERT_EQUALS( timer.GetTimerHndl(), listener_->id_ );
	}
	
};
