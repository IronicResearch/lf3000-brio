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

#define MODCOMP(x,y) ((x) < (y) ? (y-x) : (x-y))

//============================================================================
// MyTimerListener
//============================================================================
const tEventType kTimerTypes[] = { kAllTimerEvents };

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
												 	{{0, 0}, {0, 100000000}},
			                                     };
		TS_ASSERT( listener_->IsReset() );
		COneShotTimer	timer(props);
		kernel_->TaskSleep(200);
		TS_ASSERT( listener_->IsReset() );

	}
	
	//------------------------------------------------------------------------
	void testTimerFires( )
	{
		static const  tTimerProperties props = {TIMER_ABSTIME_SET,
												 	{{0, 0}, {0, 100000000}},
			                                    };
//		TS_ASSERT_EQUALS( kNoErr, event_->RegisterEventListener(listener_) );
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
	void testTimerPauseResume( )
	{
		static const  tTimerProperties props = {TIMER_ABSTIME_SET,
												 	{{0, 0}, {0, 100000000}},
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
	void testTimerRestart( )
	{
		static const  tTimerProperties props = {TIMER_ABSTIME_SET,
												 	{{0, 0}, {0, 100000000}},
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

	void testTimerElapsedTime( )
	{
		#define MS_TO_NS(x) (x*1000000)  // convert millisecond in nanosecond
		const int t_timer_0 = 10;  // Millisecond
		const int t_timer_exp = 100;  // Millisecond

		static const  tTimerProperties props = {TIMER_ABSTIME_SET,
												 {{0, MS_TO_NS(t_timer_exp)},  // Interval
												  {0, MS_TO_NS(t_timer_0)}}, // Initial expiration
   		                                       };
		const U32 kSleepInterval = 10; // Millisecond
		const U32 kDelta = 25;		   // Millisecond
		const U32 nIter = 20;
		const U32 i_start = 0;
		U32 elapsed;
		U32 remaining;
		tErrType err;

		TS_ASSERT( listener_->IsReset() );
		COneShotTimer	timer(props);
		timer.Start(props);
		
		for (U32 ii = i_start; ii < nIter; ++ii)
		{
			kernel_->TaskSleep(kSleepInterval);

			err = kernel_->GetTimerElapsedTime(timer.GetTimerHndl(), &elapsed);
			TS_ASSERT_EQUALS( kNoErr, err );
			
			err = kernel_->GetTimerRemainingTime(timer.GetTimerHndl(), &remaining);
			TS_ASSERT_EQUALS( kNoErr, err );
			
			TS_ASSERT_LESS_THAN_EQUALS(MODCOMP(elapsed,
												 ((ii+1) * kSleepInterval)),
												 kDelta + MS_TO_NS(t_timer_exp) );

			TS_ASSERT_LESS_THAN_EQUALS(MODCOMP(remaining,
												 ((nIter - (ii+1)) * kSleepInterval)),
												 kDelta + MS_TO_NS(t_timer_exp));
#if 0 // FIXME/BSK
		printf("\nii=%u  elapsed=%u remaining=%u\n",
				(unsigned int )ii, (unsigned int )elapsed, (unsigned int )remaining);
		fflush(stdout); 
#endif
			  
		}
		kernel_->TaskSleep(200);
		TS_ASSERT( !listener_->IsReset() );
		TS_ASSERT_EQUALS( timer.GetTimerHndl(), listener_->id_ );
		
	}
};
