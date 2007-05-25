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
// Test functions
//============================================================================
class TestKernelMPI : public CxxTest::TestSuite, TestSuiteBase
{
private:
public:
	CKernelMPI*		KernelMPI;
	//------------------------------------------------------------------------
	void setUp( )
	{
		KernelMPI = new CKernelMPI();
	}
//------------------------------------------------------------------------
	void tearDown( )
	{
		delete KernelMPI; 
	}

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

	void testCreateTimer()
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

	void testStartStopTimer()
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
	void testResetTimerRelative()
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

	void testPauseResumeTimer()
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
		

    void testDestroyTimer()
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

    //==============================================================================
	// Mutexes
	//==============================================================================
    // Initializes a mutex with the attributes specified in the specified mutex attribute object
    void testInit_DeInit_Mutex()
    {
		tErrType err;
		//Note: typedef pthread_mutexattr_t tMutexAttr
		//Note: typedef pthread_mutex_t     tMutex;
		tMutex    mutex = PTHREAD_MUTEX_INITIALIZER;
		tMutex    mutex2;
		tMutex    mutex3;
	
 	 	tMutexAttr   mta;
		
		//C reate a default mutex attribute
    	err = KernelMPI->InitMutexAttributeObject( mta );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

  		// Create the mutex using the NULL attributes (default)
//  		rc = pthread_mutex_init(&mutex3, NULL);
       	err = KernelMPI->InitMutex( mutex3, NULL );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

  		// Create the mutex using a mutex attributes object
       	err = KernelMPI->InitMutex( mutex2, &mta );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

  		// Destroy three mutexes
       	err = KernelMPI->DeInitMutex( mutex );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

       	err = KernelMPI->DeInitMutex( mutex2 );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

       	err = KernelMPI->DeInitMutex( mutex3 );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );
    }
	
    // Destroys a mutex. It was tested in the 'testInit_DeInit_Mutex'
    void xtestDeInitMutex()
    {
		tErrType err;
		tMutex mutex;
        err = KernelMPI->DeInitMutex( mutex );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );
    }
	
    // Obtains the priority ceiling of a mutex attribute object
    // This function will not be used on the borad 
    void xtestGetMutexPriorityCeiling()
	{
        // err = KernelMPI->GetMutexPriorityCeiling( const tMutex& mutex );
    }
	
    // Sets the priority ceiling attribute of a mutex attribute object
    // This function will not be used on the borad 
    void xtestSetMutexPriorityCeiling()
    {
 		// err = KernelMPI->SetMutexPriorityCeiling( tMutex& mutex, S32 prioCeiling, S32* pOldPriority = NULL );
    }
	
     	// Locks an unlocked mutex
    void testLockMutex()
    {
		tErrType err;

		tMutex mutex = PTHREAD_MUTEX_INITIALIZER;

        err = KernelMPI->LockMutex( mutex );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

		err = KernelMPI->UnlockMutex( mutex );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );
       		
       	err = KernelMPI->DeInitMutex( mutex );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );
    }
	
    	// Tries to lock a not xtested
	void testTryLockMutex_1()
    {
		tErrType err;
		tMutex mutex;

   		err = KernelMPI->InitMutex( mutex, NULL );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

		err = KernelMPI->TryLockMutex( mutex );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );
			
   		err = KernelMPI->DeInitMutex( mutex );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );
			
			// See http://www.yolinux.com/TUTORIALS/LinuxTutorialPosixThreads.html
			
//			pthread_mutex_lock(&mutex_1);

//   		while ( pthread_mutex_trylock(&mutex_2) )  /* Test if already locked   */
//    		{
//       			pthread_mutex_unlock(&mutex_1);  /* Free resource to avoid deadlock */
//       			...
       			/* stall here   */
//       			...
//       			pthread_mutex_lock(&mutex_1);
//    		}
//    		count++;
//    		pthread_mutex_unlock(&mutex_1);
//    		pthread_mutex_unlock(&mutex_2);
    
// 			
        }
	
    // Unlocks a mutex. It was tested
    void xtestUnlockMutex()
    {
        	// err = KernelMPI->UnlockMutex( tMutex& mutex );
    }

	//==============================================================================
	// Conditions
	//==============================================================================
    // Unblocks all threads that are waiting on a condition variable
        void xtestBroadcastCond()
        {
        	// err = KernelMPI->BroadcastCond( tCond& cond );
        }	
    
    // Destroys a condition variable attribute object
        void xtestDestroyCond()
        {
        	// err = KernelMPI->DestroyCond( tCond& cond );
        }
    
    // Initializes a condition variable with the attributes specified in the
    // specified condition variable attribute object
        void xtestInitCond()
        {
        	// err = KernelMPI->InitCond( tCond& cond, const tCondAttr& attr );
        }
    
    // Unblocks at least one thread waiting on a condition variable
        void xtestSignalCond()
        {
        	// err = KernelMPI->SignalCond( tCond& cond );
        }
    
    // Automatically unlocks the specified mutex, and places the calling thread into a wait state
    // FIXME: pAbstime var
        void xtestTimedWaitOnCond()
        {
        	// err = KernelMPI->TimedWaitOnCond( tCond& cond, tMutex& mutex, const tTimeSpec* pAbstime );
        }
    
    // Automatically unlocks the specified mutex, and places the calling thread into a wait state
        void xtestWaitOnCond()
        {
        	// err = KernelMPI->WaitOnCond( tCond& cond, tMutex& mutex );
        	
        }	
    
    // Destroys a condition variable
        void xtestDestroyCondAttr()
        {
        	// err = KernelMPI->DestroyCondAttr( tCondAttr& attr );
        }
    
    // Obtains the process-shared setting of a condition variable attribute object
        void xtestGetCondAttrPShared()
        {
        	// err = KernelMPI->GetCondAttrPShared( const tCondAttr& attr, int* pShared );
        }
    
    // Initializes a condition variable attribute object    
        void xtestInitCondAttr()
        {
        	// err = KernelMPI->InitCondAttr( tCondAttr& attr );
        }
    
    // Sets the process-shared attribute in a condition variable attribute object
    // to either PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE
        void xtestSetCondAttrPShared()
        {
        	// err = KernelMPI->SetCondAttrPShared( tCondAttr* pAttr, int shared );
        }

};
//EOF
