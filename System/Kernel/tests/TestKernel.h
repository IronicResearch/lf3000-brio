// TestKernel.h

#include <cxxtest/TestSuite.h>

#include <signal.h>   	// for timer_create
#include <time.h> 		// for timer create
#include <sys/time.h>

#include <SystemEvents.h>
#include <UnitTestUtils.h>
#include <KernelTypes.h>
#include <EventListener.h>
#include <SystemTypes.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <KernelMPI.h>
#include <CoreTypes.h> 
#include <EventMPI.h>
//#include <Timer.h>

using namespace std;

namespace
{
typedef struct{
	int numTest;
	char testDescription[128];
}thread_arg_t;

	char *message[3] =  { 	"Properties are default",
	 						"Properties are set - Test 2",
	 						"Properties are set - Test 3"
	};						 

	static void *myTask(void* arg)
	{
		thread_arg_t *ptr = (thread_arg_t *)arg; 
		TS_ASSERT( !strcmp(message[ ptr->numTest ], ptr->testDescription ) );
		sleep(1); 
	}	
//---------------------------------------------------------
// Timer testing fuctions
//----------------------------------------------------------
	tTimerHndl hndlTimer_1;
	tTimerHndl hndlTimer_2;
	tTimerHndl hndlTimer_3;

	int counterTimer_1 = 0;
	int counterTimer_2 = 0;
	int counterTimer_3 = 0;

	void myTask_Timer_1(tTimerHndl tHndl)
	{
		counterTimer_1++;
#if 0 // FIXME /BSK
		printf("\nmyTask_Timer_1 - The timer called me %d tHndl 0x%x\n", counterTimer_1, tHndl );
		fflush(stdout);
#endif
	}	

	void myTask_Timer_2(tTimerHndl tHndl)
	{
		counterTimer_2++;
#if 0		// FIXME /BSK
		printf("myTask_Timer_2 - The timer called me %d tHndl 0x%x\n", counterTimer_2, tHndl );
		fflush(stdout);
#endif
	}	

	void myTask_Timer_3(tTimerHndl tHndl)
	{
		counterTimer_3++;
#if 0	// FIXME /BSK
		printf("myTask_Timer_3 - The timer called me %d tHndl 0x%x\n", counterTimer_3, tHndl );
		fflush(stdout);
#endif
	}	
}

LF_USING_BRIO_NAMESPACE()
//============================================================================
// MyTimerListener
//============================================================================

//#ifdef TESTTIMERSEVENT

//#define kTimerFiredEvent 0 // FIXME/BSK

//tEventType kTimerTypes[] = { kTimerFiredEvent };

//class MyTimerListener : public IEventListener
//{
//public:
	//------------------------------------------------------------------------
//	MyTimerListener(const tEventType* pTypes, U32 count, tTimerHndl id)
//	MyTimerListener(unsigned long  * pTypes, unsigned long count, tTimerHndl id)
//		: IEventListener( pTypes, count),
//		id_(id)
//		: IEventListener( kTimerTypes, ArrayCount(kTimerTypes)),
//		id_(kInvalidTimerHndl)
//	{
//	}
	//------------------------------------------------------------------------
//	virtual tEventStatus Notify( const IEventMessage &msg )
//	{
//		const CTimerMessage& m = reinterpret_cast<const CTimerMessage&>(msg);
//		id_ = m.GetTimerHndl();
//		return kEventStatusOKConsumed;
//	}
	//------------------------------------------------------------------------
//	bool IsReset( ) const
//	{
//		return id_ == kInvalidTimerHndl;
//	}
	//------------------------------------------------------------------------
//	tTimerHndl	id_;
//}; 

//#endif // TESTTIMERSEVENT
// Remove ----------------------------------------------
// Remove ----------------------------------------------
//---------------------------------------------------------
// Task testing fuctions
//----------------------------------------------------------


//============================================================================
// Test functions
//============================================================================
class TestKernelMPI : public CxxTest::TestSuite, TestSuiteBase
{
private:

//#ifdef TESTTIMERSEVENT
//    CKernelMPI*			kernel_;
//	MyTimerListener*	listener_;
//	CEventMPI*			event_;
//#else    
//#endif //TESTTIMERSEVENT 

public:
	CKernelMPI*		KernelMPI;
	//------------------------------------------------------------------------
	void setUp( )
	{
		KernelMPI = new CKernelMPI();
//		event_ = new CEventMPI();
				
//		listener_ = new MyTimerListener(kTimerTypes,
//		                                ArrayCount(kTimerTypes),
//		                                 kInvalidTimerHndl);
//		event_->RegisterEventListener(listener_);
//		kernel_ = new CKernelMPI();;
	}
//------------------------------------------------------------------------
	void tearDown( )
	{
//		delete event_;
//		delete listener_;
//		delete kernel_;
		delete KernelMPI; 
	}

//#ifndef TESTTIMERSEVENT
 	void testWasCreated( )
	{
		TS_ASSERT( KernelMPI != NULL );
		TS_ASSERT( KernelMPI->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;
		
		pName = KernelMPI->GetMPIName();
		TS_ASSERT_EQUALS( *pName, "KernelMPI" );
		version = KernelMPI->GetModuleVersion();
		TS_ASSERT_EQUALS( version, 2 );
		pName = KernelMPI->GetModuleName();
		TS_ASSERT_EQUALS( *pName, "Kernel" );
		pURI = KernelMPI->GetModuleOrigin();
		TS_ASSERT_EQUALS( *pURI, "/LF/System/Kernel" );
	}
	
	void testCreateTask()
	{
		const CURI *pTaskURI = NULL;
		tTaskHndl pHndl_1;
		tTaskHndl pHndl_2;
		tTaskHndl pHndl_3;
        thread_arg_t threadArg;
        int testNumber = 0;
 	    tTaskProperties pProperties;
        tPtr status = NULL;
/*
 struct tTaskProperties {
	U32 				priority;	            // 1
	tAddr				stackAddr;				// 2
	U32 				stackSize;				// 3	
	tTaskMainFcn_posix	TaskMainFcn;            // 4
	U32					taskMainArgCount;       // 5
	tPtr				pTaskMainArgValues;		// 6						
	tTaskStartupMode	startupMode;	        // 7
	tTaskSchedPolicy	schedulingPolicy;		// 8
	U32					schedulingInterval;     // 9
    int                 inheritSched;           // 10
 */
 
//-----------------	Test 1	----------------------------        
        char *message1 = "Properties are default";

		pProperties.TaskMainFcn = (void* (*)(void*))myTask;
    	strcpy(threadArg.testDescription, message[ testNumber ] );
		threadArg.numTest = testNumber++;
		pProperties.pTaskMainArgValues = &threadArg;
		
		TS_ASSERT_EQUALS( kNoErr,KernelMPI->CreateTask( pHndl_1,
		 	    		 (const tTaskProperties )pProperties, NULL) );

		TS_ASSERT_EQUALS( pProperties.priority, KernelMPI->GetTaskPriority( pHndl_1 ) ); 
		TS_ASSERT_EQUALS( pProperties.schedulingPolicy, KernelMPI->GetTaskSchedulingPolicy( pHndl_1 ) ); 

		TS_ASSERT_EQUALS( kNoErr, KernelMPI->JoinTask( pHndl_1, status));

//-----------------	Test 2	----------------------------        
    char *message2 = "Properties are set";
 // 	priority;				// 1	
	pProperties.priority = 50;
 
 //	stackAddr;				// 2
	const unsigned PTHREAD_STACK_ADDRESS = 0xABC;
//	pProperties.stackAddr = (tAddr ) malloc(PTHREAD_STACK_ADDRESS + 0x4000);

//	stackSize;				// 3	
    const unsigned PTHREAD_STACK_MINIM = 0x10000;
//	pProperties.stackSize = PTHREAD_STACK_MIN + 0x4000;

//	TaskMainFcn;			// 4
	pProperties.TaskMainFcn = (void* (*)(void*))myTask;

//	taskMainArgCount & pTaskMainArgValues		// 5 & 6
    	strcpy(threadArg.testDescription, message[ testNumber ] );
		threadArg.numTest = testNumber++;

//	startupMode;			// 7

//	pProperties.startupMode 

//	schedulingPolicy;		// 8
    pProperties.schedulingPolicy = SCHED_FIFO;

//	schedulingInterval;		// 9
//  pProperties.schedulingInterval 
		TS_ASSERT_EQUALS( kNoErr,KernelMPI->CreateTask( pHndl_2,
		 	    		 (const tTaskProperties )pProperties, NULL) );

		TS_ASSERT_EQUALS( pProperties.priority, KernelMPI->GetTaskPriority( pHndl_2 ) ); 
		TS_ASSERT_EQUALS( pProperties.schedulingPolicy, KernelMPI->GetTaskSchedulingPolicy( pHndl_2 ) ); 

		TS_ASSERT_EQUALS( kNoErr, KernelMPI->JoinTask( pHndl_2, status));


//-----------------	Test 3	----------------------------        
	    char *message3 = "Properties are set";

    	strcpy(threadArg.testDescription, message[ testNumber ] );
		threadArg.numTest = testNumber++;


		pProperties.priority = 50;
    	pProperties.schedulingPolicy = SCHED_FIFO;

        pProperties.TaskMainFcn = (void* (*)(void*))myTask;
		pProperties.pTaskMainArgValues = &threadArg;
		
		TS_ASSERT_EQUALS( kNoErr,KernelMPI->CreateTask( pHndl_3,
		 	    		 (const tTaskProperties )pProperties, NULL) );

		TS_ASSERT_EQUALS( pProperties.priority, KernelMPI->GetTaskPriority( pHndl_3 ) ); 
		TS_ASSERT_EQUALS( pProperties.schedulingPolicy, KernelMPI->GetTaskSchedulingPolicy( pHndl_3 ) ); 

		TS_ASSERT_EQUALS( kNoErr, KernelMPI->JoinTask( pHndl_3, status));

		TS_ASSERT_EQUALS( kNoErr, KernelMPI->CancelTask( pHndl_1 ) );
		TS_ASSERT_EQUALS( kNoErr, KernelMPI->CancelTask( pHndl_2 ) );
		TS_ASSERT_EQUALS( kNoErr, KernelMPI->CancelTask( pHndl_3 ) );
	}

	void testTaskSleep()
	{
		U32 msec = 50;
		U32 threshold = 10;
		struct timespec t1, t2;
		int dt_msec;

		clock_gettime( CLOCK_REALTIME, &t1) ; 
		KernelMPI->TaskSleep( msec );	
		clock_gettime( CLOCK_REALTIME, &t1) ; 
		dt_msec = t2.tv_sec * 1000 + t1.tv_nsec / 1000000 - 
			     (t2.tv_sec * 1000 + t1.tv_nsec / 1000000);
			      
 		TS_ASSERT_LESS_THAN(dt_msec, msec + threshold); 
	}
			
	
	void testMemory()
	{
        U32 size = 0x5000;
        tPtr pPtr = NULL;
        
//        TS_WARN("TODO: Test Memory!");
        pPtr = KernelMPI->Malloc(size);
//		TS_ASSERT_DIFFERS( pPtr, static_cast<tPtr>(kNull) );
//		TS_ASSERT_EQUALS( kNoErr, KernelMPI->Free( pPtr ) );
        
  		TS_ASSERT_DIFFERS( (void*)NULL, pPtr );
  		TS_ASSERT_EQUALS( kNoErr, KernelMPI->Free( pPtr ) );
	}
		
	void TestCreateMessageQueue_1() 
	{
		tErrType err;
		tMessageQueueHndl hndl;
		
		const tMessageQueuePropertiesPosix msgProperties = 
		{
    		0,                          // msgProperties.blockingPolicy;  
    		"/test_q_1",                  // msgProperties.nameQueue
    		S_IRWXU,                    // msgProperties.mode 
    		O_RDWR|O_CREAT|O_TRUNC,       // msgProperties.oflag  
    		0,                          // msgProperties.priority
    		0,                          // msgProperties.mq_flags
    		10,                          // msgProperties.mq_maxmsg
    		sizeof(CEventMessage),      // msgProperties.mq_msgsize
    		0                           // msgProperties.mq_curmsgs
		};

//        TS_WARN("TODO: Test Create/Destroy Message Queue_1!");
		err = KernelMPI->OpenMessageQueue(hndl,
							msgProperties,
							(const char* )NULL );
		TS_ASSERT_EQUALS( kNoErr, err );							
		if( !err )
		{
			err = KernelMPI->CloseMessageQueue( hndl, msgProperties );
			TS_ASSERT_EQUALS( kNoErr, err );
//			err = KernelMPI->UnlinkMessageQueue( msgProperties.nameQueue );
//			TS_ASSERT_EQUALS( kNoErr, err );
		}
	}
			
	void TestCreateMessageQueue_2() 
	{
		tErrType err;
		tMessageQueueHndl hndl;
		
		const tMessageQueuePropertiesPosix msgProperties = 
		{
    		0,                          // msgProperties.blockingPolicy;  
    		"/test_q_2",                // msgProperties.nameQueue
    		S_IRWXU,                    // msgProperties.mode 
    		O_RDWR|O_CREAT|O_TRUNC,     // msgProperties.oflag  
    		0,                          // msgProperties.priority
    		0,                          // msgProperties.mq_flags
    		10,                         // msgProperties.mq_maxmsg
    		sizeof(CEventMessage)/2,    // msgProperties.mq_msgsize
    		0                           // msgProperties.mq_curmsgs
		};

//        TS_WARN("TODO: Test Create/Destroy Message Queue_2!");
		err = KernelMPI->OpenMessageQueue(hndl,
							msgProperties,
							(const char* )NULL );
		TS_ASSERT_EQUALS( kNoErr, err );							
		if( !err )
		{
			err = KernelMPI->CloseMessageQueue( hndl, msgProperties );
			TS_ASSERT_EQUALS( kNoErr, err );
//			err = KernelMPI->UnlinkMessageQueue( msgProperties.nameQueue );
//			TS_ASSERT_EQUALS( kNoErr, err );
		}								
	}

	void xtestCreateTimer()
	{
//TS_WARN("TODO: Test Create/Destroy Timer!");
		
// NOTE:
//typedef struct timer_arg{	pfnTimerCallback pfn; tPtr arg;} callbackData;
//struct tTimerProperties { int type; struct itimerspec timeout; callbackData callback;};
		const static tTimerProperties props_1 = {TIMER_ABSTIME_SET,
												 	{0, 0, 0, 0},
		                                       	};

		const static tTimerProperties props_2 = {TIMER_ABSTIME_SET,
						 							{0, 0, 0, 0},
			                                   	};

		const static tTimerProperties props_3 = {TIMER_ABSTIME_SET,
					 								{0, 0, 0, 0},
			                                   	};
//typedef void (*pfnTimerCallback)(tTimerHndl arg)		
		hndlTimer_1 = KernelMPI->CreateTimer(myTask_Timer_1, props_1, (const char *)0 );
		TS_ASSERT_DIFFERS( 0, hndlTimer_1 );

		hndlTimer_2 = KernelMPI->CreateTimer(myTask_Timer_2, props_2, (const char *)0 );
		TS_ASSERT_DIFFERS( 0, hndlTimer_2 );

		hndlTimer_3 = KernelMPI->CreateTimer(myTask_Timer_3, props_3, (const char *)0 );
		TS_ASSERT_DIFFERS( 0, hndlTimer_3 );

//		TS_ASSERT_DIFFERS( hndl, static_cast<hndl>( kNull ));
//		tErrType err = KernelMPI->DestroyTimer( hndlTimer );
//		TS_ASSERT_EQUALS( err, ((tErrType)0) );
	}

	void xtestStartStopTimer()
	{
		tTimerProperties props = {0};
		tErrType err;
		props.type = TIMER_RELATIVE_SET; 	
		// Timer period
    	props.timeout.it_interval.tv_sec = 0;
		props.timeout.it_interval.tv_nsec = 500000000L;
		// Timer expirated
		props.timeout.it_value.tv_sec = 1;
		props.timeout.it_value.tv_nsec = 0;
		err = KernelMPI->StartTimer( hndlTimer_1, props );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

		err = KernelMPI->StartTimer( hndlTimer_2, props );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

		err = KernelMPI->StartTimer( hndlTimer_3, props );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

		err = KernelMPI->StopTimer( hndlTimer_1 );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

		err = KernelMPI->StopTimer( hndlTimer_2 );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

		err = KernelMPI->StopTimer( hndlTimer_3 );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

	}		
	void xtestResetTimerRelative()
	{
//        TS_WARN("TODO: Test Reset Timer Relative!");
		tTimerProperties props = {0};
		tErrType err;
		props.type = TIMER_RELATIVE_SET; 	
		// Timer period
    	props.timeout.it_interval.tv_sec = 0;
		props.timeout.it_interval.tv_nsec = 500000000L;
		// Timer expirated
		props.timeout.it_value.tv_sec = 1;
		props.timeout.it_value.tv_nsec = 0;
		err = KernelMPI->ResetTimer( hndlTimer_1, props );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

		err = KernelMPI->ResetTimer( hndlTimer_2, props );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

		err = KernelMPI->ResetTimer( hndlTimer_3, props );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );
	}

	void xtestPauseResumeTimer()
	{
		tErrType err;

		saveTimerSettings save_1 = {0};
		saveTimerSettings save_2 = {0};
		saveTimerSettings save_3 = {0};

		err = KernelMPI->PauseTimer( hndlTimer_1, save_1 );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

		err = KernelMPI->PauseTimer( hndlTimer_2, save_2 );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

		err = KernelMPI->PauseTimer( hndlTimer_3, save_3 );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );
		
		err = KernelMPI->ResumeTimer( hndlTimer_1, save_1 );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

		err = KernelMPI->ResumeTimer( hndlTimer_2, save_2 );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

		err = KernelMPI->ResumeTimer( hndlTimer_3, save_3 );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );
			
	}
		

    void xtestDestroyTimer()
    {
//        TS_WARN("TODO: Test Destroy Timer!");
		tErrType err;
		struct timespec sleeptime = { 1, 0 };
		const int limit = 3; 

		while (( counterTimer_1 < limit && counterTimer_2 < limit && counterTimer_3 < limit ))
		{
			nanosleep( &sleeptime, NULL );  
		}

		if( hndlTimer_1 != 0 )
		{	
			err = KernelMPI->DestroyTimer( hndlTimer_1 );
			TS_ASSERT_EQUALS( err, ((tErrType)0) );
		}
		
		if( hndlTimer_1 != 0 )
		{
			err = KernelMPI->DestroyTimer( hndlTimer_2 );
			TS_ASSERT_EQUALS( err, ((tErrType)0) );
		}
		if( hndlTimer_1 != 0 )
		{
			err = KernelMPI->DestroyTimer( hndlTimer_3 );
			TS_ASSERT_EQUALS( err, ((tErrType)0) );
		}
    }

//----------------------------------------------------------------
//----------------------------------------------------------------
// Testing Timers with Events
//----------------------------------------------------------------

//	void xtestTimerNotFired( )			
//	{
//
//		static tTimerHndl tHndl_4;
//		static const  tTimerProperties props_4 = {TIMER_ABSTIME_SET,
//												 	{0, 100, 0, 0},
//			                                     };
//		tTimerProperties	props_4 = {0};
//TP		props.msec = 100;
//		TS_ASSERT( listener_->IsReset() );
// 		COneShotTimer timer(props_4);
//    CKernelMPI* kernel_ = KernelMPI;
//		KernelMPI->TaskSleep(200);
//		TS_ASSERT( listener_->IsReset() );

//	}

//	void xxxtestTimerFires( )
//	{
//		static tTimerHndl tHndl_4;
//		const static tTimerProperties props_5 = {TIMER_ABSTIME_SET,
//												 	{0, 100, 0, 0},
//			                                       	};
//		tTimerProperties	props;
// 		props.msec = 100;
//		TS_ASSERT( listener_->IsReset() );
//		COneShotTimer	timer(props_5);
//		timer.Start(props_5);
//		kernel_->TaskSleep(50);
//		TS_ASSERT( listener_->IsReset() );
//		kernel_->TaskSleep(100);
//		TS_ASSERT( !listener_->IsReset() );
//		TS_ASSERT_EQUALS( timer.GetTimerHndl(), listener_->id_ );
//	}
	
//	void xxxtestTimerPauseResume( )
//	{
//		static tTimerHndl tHndl_6;
//		const static tTimerProperties props_6 = {TIMER_ABSTIME_SET,
//												 	{0, 100, 0, 0},
//			                                    };
//		saveTimerSettings saveValue;
//TP		tTimerProperties	props;
//TP		props.msec = 100;
//		TS_ASSERT( listener_->IsReset() );
//		COneShotTimer	timer(props_6);
//		timer.Start(props_6);
//		kernel_->TaskSleep(50);
//		TS_ASSERT( listener_->IsReset() );
//		timer.Pause(saveValue);
//		kernel_->TaskSleep(100);
//		TS_ASSERT( listener_->IsReset() );
//		timer.Resume(saveValue);
//		kernel_->TaskSleep(100);
//		TS_ASSERT( !listener_->IsReset() );
//		TS_ASSERT_EQUALS( timer.GetTimerHndl(), listener_->id_ );
//	}
	
	//------------------------------------------------------------------------
//	void xxxtestTimerRestart( )
//	{
//		static tTimerHndl tHndl_7;
//		const static tTimerProperties props_7 = {TIMER_ABSTIME_SET,
//												 	{0, 100, 0, 0},
//			                                      };
//TP		tTimerProperties	props;
//TP		props.msec = 100;

//		TS_ASSERT( listener_->IsReset() );
//		COneShotTimer	timer(props_7);
//		for (int ii = 0; ii < 3; ++ii )
//		{
//			timer.Start(props_7);
//   CKernelMPI* kernel_ = KernelMPI;
//			kernel_->TaskSleep(50);
//			TS_ASSERT( listener_->IsReset() );
//		}
//    CKernelMPI* kernel_ = KernelMPI;
//		kernel_->TaskSleep(100);
//		TS_ASSERT( !listener_->IsReset() );
//		TS_ASSERT_EQUALS( timer.GetTimerHndl(), listener_->id_ );
//	}
	
	//------------------------------------------------------------------------
//	void xxxtestTimerElapsedTime( )
//	{
//		static tTimerHndl tHndl_8;
//		const static tTimerProperties props_8 = {TIMER_ABSTIME_SET,
//												 	{0, 100, 0, 0},
//			                                       	};
//TP		tTimerProperties	props;
//TP		props.msec = 1000;
//		TS_ASSERT( listener_->IsReset() );
//		COneShotTimer	timer(props_8);
//		U32 start = kernel_->GetElapsedTime();
//		timer.Start(props_8);
//		const int kSleepInterval = 100;
//		const int kDelta = 10;
//		int i = 0; // FIXME/BSK
//		for (int ii = i; ii < 9; ++ii)
//		{
//			kernel_->TaskSleep(kSleepInterval);
//			U32 elapsed = timer.GetTimerElapsedTimeInMilliSec();
//			U32 remaining = timer.GetTimerRemainingTimeInMilliSec();
//			TS_ASSERT_DELTA( elapsed, ii * kSleepInterval, kDelta );
//			TS_ASSERT_DELTA( remaining, (10 - ii) * kSleepInterval, kDelta );
//		}
//		kernel_->TaskSleep(200);
//		TS_ASSERT( !listener_->IsReset() );
//		TS_ASSERT_EQUALS( timer.GetTimerHndl(), listener_->id_ );
//	}
};
//EOF






