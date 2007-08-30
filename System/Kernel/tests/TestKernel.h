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
#include <errno.h>

using namespace std;
LF_USING_BRIO_NAMESPACE()


namespace
{
#define MS_TO_NS(x) (x*1000000) 	
	typedef struct
	{
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
		return (void *)NULL;
	}	
	void ptintf_test_info( char *pName );
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
		counterTimer_1--;
#if 0 // FIXME /BSK
		printf("\nmyTask_Timer_1 - The timer called me %d tHndl 0x%x\n",
			counterTimer_1, (unsigned int )tHndl );
		fflush(stdout);

#endif
	}	

	void myTask_Timer_2(tTimerHndl tHndl)
	{
		counterTimer_2--;
#if 0		// FIXME /BSK
		printf("myTask_Timer_2 - The timer called me %d tHndl 0x%x\n",
			counterTimer_2, (unsigned int )tHndl );
		fflush(stdout);
#endif
	}	

	void myTask_Timer_3(tTimerHndl tHndl)
	{
		counterTimer_3--;
#if 0	// FIXME /BSK
		printf("myTask_Timer_3 - The timer called me %d tHndl 0x%x\n",
			 counterTimer_3, (unsigned int )tHndl );
		fflush(stdout);
#endif
		pthread_exit((void *)2007);
	}
//-----------------------------------------------------------------------------------------
//		Testing WaitOnCond() function
//------------------------------------------------------------------------------------------
int        conditionMet = 0;

#define NTHREADS    5

void *threadfunc_broadcast(void *parm);
static tCond      cond_broadcast  = PTHREAD_COND_INITIALIZER;
static tMutex     mutex_broadcast = PTHREAD_MUTEX_INITIALIZER;

void *threadfunc_broadcast(void *parm)
{
	tErrType err;
//	printf("Thread created\n");
//	fflush( stdout );

	sleep(3);

	conditionMet = 1;
    err = pthread_cond_broadcast(&cond_broadcast);
	TS_ASSERT_EQUALS( err, ((tErrType)0) );
	return (void *)NULL;
}
//------------------------------------------------------------------------------------------
 
}  // workspace

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
		ptintf_test_info("testWasCreated");

		TS_ASSERT( KernelMPI != NULL );
		TS_ASSERT( KernelMPI->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		ptintf_test_info("testCoreMPI");

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
	

	//==============================================================================
	// FileSystem
	//==============================================================================
	//------------------------------------------------------------------------------
    CPath GetSourceFile()
    {
#ifndef EMULATION
    	return "/usr/include/stdio.h";
#else
    	CPath curFile = __FILE__;
		if (curFile[0] == '.')
		{
			char buf[1024];
			getcwd(buf, sizeof(buf));
			curFile = buf + curFile.substr(1);
		}
    	return curFile;
#endif
    }
    
	//------------------------------------------------------------------------------
    CPath GetSourceDirectory()
    {
    	CPath curDir;
    	CPath curFile = GetSourceFile();
    	size_t idx = curFile.rfind('/');
    	if (idx > 0)
    		curDir = curFile.substr(0, idx);
    	return curDir;
    }
    
	//------------------------------------------------------------------------------
    void testIsDirectory()
    {
		ptintf_test_info("testIsDirectory");

		TS_ASSERT_EQUALS(KernelMPI->IsDirectory(""), false);
		TS_ASSERT_EQUALS(KernelMPI->IsDirectory("/"), true);
		TS_ASSERT_EQUALS(KernelMPI->IsDirectory("/usr"), true);
		TS_ASSERT_EQUALS(KernelMPI->IsDirectory("/usr/"), true);
		TS_ASSERT_EQUALS(KernelMPI->IsDirectory("/home/non-existing-file"), false);
		TS_ASSERT_EQUALS(KernelMPI->IsDirectory(GetSourceFile()), false);
		TS_ASSERT_EQUALS(KernelMPI->IsDirectory(GetSourceDirectory()), true);
    }

	//------------------------------------------------------------------------------
    void testFilesInDirectory()
    {
		ptintf_test_info("testFilesInDirectory");

    	std::vector<CPath>	files;
    	size_t kZero = 0;
    	files = KernelMPI->GetFilesInDirectory("");
		TS_ASSERT_EQUALS(files.size(), kZero);
    	files = KernelMPI->GetFilesInDirectory("/usr");
		TS_ASSERT_DIFFERS(files.size(), kZero);
    	files = KernelMPI->GetFilesInDirectory("/usr/");
		TS_ASSERT_DIFFERS(files.size(), kZero);
    	files = KernelMPI->GetFilesInDirectory("/home/non-existing-file");
    	TS_ASSERT_EQUALS(files.size(), kZero);
    	files = KernelMPI->GetFilesInDirectory(GetSourceFile());
    	TS_ASSERT_EQUALS(files.size(), kZero);
    	
    	CPath curDir = GetSourceDirectory();
    	files = KernelMPI->GetFilesInDirectory(curDir);
    	TS_ASSERT_DIFFERS(files.size(), kZero);
    	bool found = false;
    	for (size_t ii = 0; ii < files.size(); ++ii)
    	{
    		if (files[ii] == GetSourceFile())
    			found = true;
    	}
    	TS_ASSERT(found == true);
	}

    
    void testGetElapsedTime()
	{
		ptintf_test_info("testGetElapsedTime");

//		tErrType err;
//		U32 pUs;
//		const U64 threshold = 20000; 

       // No errors are defined for the 'gettimeofday()' function
       // that used for GetElapsedTime
		U32 tAsMSecs = KernelMPI->GetElapsedTimeAsMSecs();
		U64 tAsUSecs = KernelMPI->GetElapsedTimeAsUSecs();
		TS_ASSERT_LESS_THAN_EQUALS( tAsMSecs * 1000, tAsUSecs );
//		U64 delta = tAsUSecs - (U64 )tAsMSecs * 1000;
		//FIXME/BSK
		//	This test will fail in the emulation mode due overflowing	
		//	TS_ASSERT_LESS_THAN_EQUALS(	delta, threshold );
			
	}
	
	void testCreateTask()
	{
		ptintf_test_info("testCreateTask");

//		const CURI *pTaskURI = NULL;
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
//        char *message1 = "Properties are default";

		pProperties.TaskMainFcn = (void* (*)(void*))myTask;
    	strcpy(threadArg.testDescription, message[ testNumber ] );
		threadArg.numTest = testNumber++;
		pProperties.pTaskMainArgValues = &threadArg;
		
		TS_ASSERT_EQUALS( kNoErr,KernelMPI->CreateTask( pHndl_1,
		 	    		 (const tTaskProperties )pProperties, NULL) );

		TS_ASSERT_EQUALS( pProperties.priority, KernelMPI->GetTaskPriority( pHndl_1 ) ); 
		TS_ASSERT_EQUALS( pProperties.schedulingPolicy, KernelMPI->GetTaskSchedulingPolicy( pHndl_1 ) ); 

		TS_ASSERT_EQUALS( kNoErr, KernelMPI->JoinTask( pHndl_1, status));

// These tests will always fail in emulation because non-privliged
// users are not allowed to set SCHED_FIFO.
#ifndef EMULATION
//-----------------	Test 2	----------------------------        
//    char *message2 = "Properties are set";
 // 	priority;				// 1	
	pProperties.priority = 50;
 
 //	stackAddr;				// 2
//	const unsigned PTHREAD_STACK_ADDRESS = 0xABC;
//	pProperties.stackAddr = (tAddr ) malloc(PTHREAD_STACK_ADDRESS + 0x4000);

//	stackSize;				// 3	
//    const unsigned PTHREAD_STACK_MINIM = 0x10000;
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
//	    char *message3 = "Properties are set";

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
//		TS_ASSERT_EQUALS((int )status -2007, 0 );   // FIXME/BSK

		TS_ASSERT_EQUALS( kNoErr, KernelMPI->CancelTask( pHndl_1 ) );
		TS_ASSERT_EQUALS( kNoErr, KernelMPI->CancelTask( pHndl_2 ) );
		TS_ASSERT_EQUALS( kNoErr, KernelMPI->CancelTask( pHndl_3 ) );
#endif
	}

	void testTaskSleep()
	{
		ptintf_test_info("testTaskSleep");

		U32 msec = 50;
		U32 threshold = 10;
		struct timespec t1, t2;
		U32 dt_msec;

		clock_gettime( CLOCK_REALTIME, &t1) ; 
		KernelMPI->TaskSleep( msec );	
		clock_gettime( CLOCK_REALTIME, &t1) ; 
		dt_msec = t2.tv_sec * 1000 + t1.tv_nsec / 1000000 - 
			     (t2.tv_sec * 1000 + t1.tv_nsec / 1000000);
			      
 		TS_ASSERT_LESS_THAN(dt_msec, msec + threshold); 
	}
			
	
	void testMemory()
	{
		ptintf_test_info("testMemory");

        U32 size = 0x5000;
        tPtr pPtr = NULL;
        
//        TS_WARN("TODO: Test Memory!");
        pPtr = KernelMPI->Malloc(size);
//		TS_ASSERT_DIFFERS( pPtr, static_cast<tPtr>(kNull) );
//		TS_ASSERT_EQUALS( kNoErr, KernelMPI->Free( pPtr ) );
        
  		TS_ASSERT_DIFFERS( (void*)NULL, pPtr );
  		KernelMPI->Free( pPtr );
	}
		
	void TestCreateMessageQueue_1() 
	{
		ptintf_test_info("TestCreateMessageQueue_1");

		tErrType err;
		tMessageQueueHndl hndl;
		
		const tMessageQueuePropertiesPosix msgProperties = 
		{
    		0,                          	// msgProperties.blockingPolicy;  
    		"/test_q_1",                  	// msgProperties.nameQueue
    		S_IRWXU,                    	// msgProperties.mode 
    		B_O_RDWR|B_O_CREAT|B_O_TRUNC,   // msgProperties.oflag  
    		0,                          	// msgProperties.priority
    		0,                          	// msgProperties.mq_flags
    		10,                          	// msgProperties.mq_maxmsg
    		sizeof(CEventMessage),      	// msgProperties.mq_msgsize
    		0                           	// msgProperties.mq_curmsgs
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
		ptintf_test_info("TestCreateMessageQueue_2");

		tErrType err;
		tMessageQueueHndl hndl;
		
		const tMessageQueuePropertiesPosix msgProperties = 
		{
    		0,                          	// msgProperties.blockingPolicy;  
    		"/test_q_2",                	// msgProperties.nameQueue
    		S_IRWXU,                    	// msgProperties.mode 
    		B_O_RDWR|B_O_CREAT|B_O_TRUNC,	// msgProperties.oflag  
    		0,                          	// msgProperties.priority
    		0,                          	// msgProperties.mq_flags
    		10,                         	// msgProperties.mq_maxmsg
    		sizeof(CEventMessage)/2,    	// msgProperties.mq_msgsize
    		0                           	// msgProperties.mq_curmsgs
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
		ptintf_test_info("testCreateTimer");

//TS_WARN("TODO: Test Create/Destroy Timer!");
		
// NOTE:
//typedef struct timer_arg{	pfnTimerCallback pfn; tPtr arg;} callbackData;
//struct tTimerProperties { int type; struct itimerspec timeout; callbackData callback;};
		const static tTimerProperties props_1 = {TIMER_ABSTIME_SET,
												 	{{0, 0}, {0, 0}},};

		const static tTimerProperties props_2 = {TIMER_ABSTIME_SET,
												 	{{0, 0}, {0, 0}},};

		const static tTimerProperties props_3 = {TIMER_ABSTIME_SET,
												 	{{0, 0}, {0, 0}},};

//typedef void (*pfnTimerCallback)(tTimerHndl arg)		
		hndlTimer_1 = KernelMPI->CreateTimer(myTask_Timer_1, props_1, (const char *)0 );
		TS_ASSERT_DIFFERS( kInvalidHndl, hndlTimer_1 );

		hndlTimer_2 = KernelMPI->CreateTimer(myTask_Timer_2, props_2, (const char *)0 );
		TS_ASSERT_DIFFERS( kInvalidHndl, hndlTimer_2 );

		hndlTimer_3 = KernelMPI->CreateTimer(myTask_Timer_3, props_3, (const char *)0 );
		TS_ASSERT_DIFFERS( kInvalidHndl, hndlTimer_3 );

//		TS_ASSERT_DIFFERS( hndl, static_cast<hndl>( kNull ));
//		tErrType err = KernelMPI->DestroyTimer( hndlTimer );
//		TS_ASSERT_EQUALS( err, ((tErrType)0) );
	}

	void testStartStopTimer()
	{
		ptintf_test_info("testStartStopTimer");

//-------------------------------------------------------------------
#if 0 // FIXME/BSK
		// Testing clock_gettime
		struct timespec mytime;
		for(int i=1; i < 10; i++ )
		{
			clock_gettime(CLOCK_REALTIME, &mytime);
			printf("CLOCK_GETTIME values:\n");
			printf("Sec=%u Nsec=%u\n", (unsigned int )mytime.tv_sec,
										(unsigned int )mytime.tv_nsec);
			fflush(stdout);
		}		 
#endif
//------------------------------------------------------------------

		tTimerProperties props = {TIMER_ABSTIME_SET,
									{{0, 0}, {0, 0}},};
		tErrType err;

		props.type = TIMER_RELATIVE_SET; 	

//----------------------------------------------------------------------------------
//  Case #1 Timer establishes the repetition value
    	props.timeout.it_interval.tv_sec = 0;
		props.timeout.it_interval.tv_nsec = MS_TO_NS(500); //500000000L;  // 0.5 sec
		// Timer expirated
		props.timeout.it_value.tv_sec = 1;
		props.timeout.it_value.tv_nsec = 0;
		
		struct timespec sleeptime;
		unsigned int dt = 0;
		sleeptime.tv_sec = 0;
		sleeptime.tv_nsec = MS_TO_NS(20); // 10 ms
        const int num_interval = 3; 
		counterTimer_1 = num_interval; 
		err = KernelMPI->StartTimer( hndlTimer_1, props );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

#if 1 // FIXME/BSK
		while( counterTimer_1 )
		{
			nanosleep( &sleeptime, NULL );
			dt += sleeptime.tv_nsec;
		}
#endif

		TS_ASSERT_LESS_THAN_EQUALS( (unsigned int )(dt),
			 (unsigned int )(MS_TO_NS(1000) + (num_interval-1) * MS_TO_NS(500) + MS_TO_NS(20)) );

//----------------------------------------------------------------------------------
//  Case #2 Timer establishes only the initial expiration time
    	props.timeout.it_interval.tv_sec = 0;
		props.timeout.it_interval.tv_nsec = 0; 
		// Timer expirated
		props.timeout.it_value.tv_sec = 1;
		props.timeout.it_value.tv_nsec = 0;
		
		dt = 0;
		sleeptime.tv_sec = 0;
		sleeptime.tv_nsec = MS_TO_NS(10); // 10 ms

		counterTimer_2 = 1; 


		err = KernelMPI->StartTimer( hndlTimer_2, props );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

#if 1 // FIXME/BSK
		while( counterTimer_2 )
		{
			nanosleep( &sleeptime, NULL );
			dt += sleeptime.tv_nsec;
		}
#endif
		TS_ASSERT_LESS_THAN_EQUALS( (unsigned int )(dt),
			 (unsigned int )(MS_TO_NS(1000) + MS_TO_NS(20)) );


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
		ptintf_test_info("testResetTimerRelative");

//        TS_WARN("TODO: Test Reset Timer Relative!");
		tTimerProperties props = {TIMER_ABSTIME_SET,
									{{0, 0}, {0, 0}},};
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
		ptintf_test_info("testPauseResumeTimer");

		tErrType err;

		saveTimerSettings save_1 = {{0, 0}, {0, 0}};
		saveTimerSettings save_2 = {{0, 0}, {0, 0}};
		saveTimerSettings save_3 = {{0, 0}, {0, 0}};

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
		
   void testGetTimerElapsed_OR_Remaining_Time()
    {
		ptintf_test_info("testGetTimerElapsed_OR_Remaining_Time");

		U32 elapsed;
		U32 remaining;
		tErrType err;

		err = KernelMPI->GetTimerElapsedTime(hndlTimer_1, &elapsed);
		TS_ASSERT_EQUALS( kNoErr, err );
		err = KernelMPI->GetTimerRemainingTime(hndlTimer_1, &remaining);
		TS_ASSERT_EQUALS( kNoErr, err );

//		errno = 0;
//		err = KernelMPI->GetTimerElapsedTime(hndlTimer_1+20, &elapsed);
//		TS_ASSERT_EQUALS( EINVAL, errno );

//		errno = 0;
//		err = KernelMPI->GetTimerRemainingTime(hndlTimer_1+20, &remaining);
//		TS_ASSERT_EQUALS( EINVAL, errno );

   	// err = KernelMPI->GetCondAttrPShared( const tCondAttr& attr, int* pShared );
    }

     void testDestroyTimer()
    {
		ptintf_test_info("testDestroyTimer");
//		sleep(2);
//        TS_WARN("TODO: Test Destroy Timer!");
		tErrType err;

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
		ptintf_test_info("testInit_DeInit_Mutex");

		tErrType err;
		//Note: typedef pthread_mutexattr_t tMutexAttr
		//Note: typedef pthread_mutex_t     tMutex;
		tMutex    mutex = PTHREAD_MUTEX_INITIALIZER;
		tMutex    mutex2;
		tMutex    mutex3;
	
 	 	tMutexAttr   mta;
		const tMutexAttr attr = {0};
		
		//Create a default mutex attribute
    	err = KernelMPI->InitMutexAttributeObject( mta );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

  		// Create the mutex using the NULL attributes (default)
//  		rc = pthread_mutex_init(&mutex3, NULL);
       	err = KernelMPI->InitMutex( mutex3, attr );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

  		// Create the mutex using a mutex attributes object
       	err = KernelMPI->InitMutex( mutex2, mta );
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
		ptintf_test_info("xtestDeInitMutex");

		tErrType err;
		tMutex mutex;
        
        err = KernelMPI->DeInitMutex( mutex );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );
    }
	
    // Obtains the priority ceiling of a mutex attribute object
    // This function will not be used on the borad 
    void xtestGetMutexPriorityCeiling()
	{
		ptintf_test_info("xtestGetMutexPriorityCeiling");

		//tErrType err;
        //err = KernelMPI->GetMutexPriorityCeiling( const tMutex& mutex );
    }
	
    // Sets the priority ceiling attribute of a mutex attribute object
    // This function will not be used on the borad 
    void xtestSetMutexPriorityCeiling()
    {
		ptintf_test_info("xtestSetMutexPriorityCeiling");

		//tErrType err;
 		//err = KernelMPI->SetMutexPriorityCeiling( tMutex& mutex, S32 prioCeiling, S32* pOldPriority = NULL );
    }
	
     // Locks an unlocked mutex
    void testLockMutex()
    {
		ptintf_test_info("testLockMutex");

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
		ptintf_test_info("testTryLockMutex_1");

		tErrType err;
		tMutex mutex;
		const tMutexAttr attr = {0};

   		err = KernelMPI->InitMutex( mutex, attr );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

		err = KernelMPI->TryLockMutex( mutex );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );
			
		err = KernelMPI->UnlockMutex( mutex );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

   		err = KernelMPI->DeInitMutex( mutex );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );
			
	}
	
    // Unlocks a mutex. It was tested
    void xtestUnlockMutex()
    {
		ptintf_test_info("xtestUnlockMutex");
     	// err = KernelMPI->UnlockMutex( tMutex& mutex );
    }

	//==============================================================================
	// Conditions
	//-----------------------------------------------------------------------------
	//	Note:
	//			typedef pthread_cond_t      tCond;
	// 			typedef pthread_condattr_t  tCondAttr;
	//
	//==============================================================================
     // Initializes a condition variable with the attributes specified in the
    // specified condition variable attribute object
        void testInitCond()
        {
			ptintf_test_info("testInitCond");

			tErrType err;
        	tCond cond;
        	const tCondAttr attr = {0};
        	
        	err = KernelMPI->InitCond( cond, attr );
			TS_ASSERT_EQUALS( err, ((tErrType)0) );
        	err = KernelMPI->DestroyCond( cond );
			TS_ASSERT_EQUALS( err, ((tErrType)0) );
        }
 
    // Destroys a condition variable attribute object. Tested above 
    void xtestDestroyCond()
    {
		ptintf_test_info("xtestDestroyCond");
    	// err = KernelMPI->DestroyCond( tCond& cond );
    }

// Initializes a condition variable attribute object    
    void testInitCondAttr()
    {
		ptintf_test_info("testInitCondAttr");
		tErrType err;
    	tCondAttr attr;

		err = KernelMPI->InitCondAttr( attr );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

    	err = KernelMPI->DestroyCondAttr( attr );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

    }

// Destroys a condition variable Tested
    void xtestDestroyCondAttr()
    {
		ptintf_test_info("xtestDestroyCondAttr");
    	// err = KernelMPI->DestroyCondAttr( tCondAttr& attr );
    }

// Unblocks all threads that are waiting on a condition variable
        void testBroadcastCond()
        {
			ptintf_test_info("testBroadcastCond");
			tErrType err;
        	static tCond cond = PTHREAD_COND_INITIALIZER;        	
        	
        	err = KernelMPI->BroadcastCond( cond );
			TS_ASSERT_EQUALS( err, ((tErrType)0) );

         	err = KernelMPI->DestroyCond( cond );
			TS_ASSERT_EQUALS( err, ((tErrType)0) );
 
         }	
    
    // Unblocks at least one thread waiting on a condition variable
        void testSignalCond()
        {
			ptintf_test_info("testSignalCond");
			tErrType err;
        	tCond cond = PTHREAD_COND_INITIALIZER;        	
        	
        	err = KernelMPI->SignalCond( cond );
			TS_ASSERT_EQUALS( err, ((tErrType)0) );

         	err = KernelMPI->DestroyCond( cond );
			TS_ASSERT_EQUALS( err, ((tErrType)0) );
 
        	// err = KernelMPI->SignalCond( tCond& cond );
    }

// Automatically unlocks the specified mutex, and places the calling thread into a wait state
// FIXME: pAbstime var
    void testTimedWaitOnCond()
    {
		ptintf_test_info("testTimedWaitOnCond");
		tErrType err;
    	static tCond cond = PTHREAD_COND_INITIALIZER;  
		static tMutex mutex = PTHREAD_MUTEX_INITIALIZER;
		const int sleepDuration = 2;
		
//			tTimeSpec timeout1, timeout2; 
//			time(&timeout1.tv_sec);
//			timeout1.tv_sec += sleepDuration;
		struct timeval now;		// time when it's started waiting
		struct timeval end;		// time when it's ended waiting
		struct timespec timeout;	// timeout value for wait function

		gettimeofday( &now, NULL );			
		timeout.tv_sec =  now.tv_sec + sleepDuration;
		timeout.tv_nsec = now.tv_usec * 1000;

    	err = KernelMPI->TimedWaitOnCond( cond, mutex, &timeout );
		TS_ASSERT_EQUALS( (int )err, ETIMEDOUT );

		gettimeofday( &end, NULL );			
        TS_ASSERT_LESS_THAN_EQUALS( end.tv_sec - now.tv_sec, 3 );
           
     	err = KernelMPI->DestroyCond( cond );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

   		err = KernelMPI->DeInitMutex( mutex );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );

    	//pthread_cond_timedwait(&cond, &mutex, pAbstime );
    }

// Automatically unlocks the specified mutex, and places the calling thread into a wait state
    void testWaitOnCond()
    {
		ptintf_test_info("testWaitOnCond");
		tErrType err;

//  			int                   rc=0;
 // 			int                   i;
		tTaskHndl		threadid;
		tTaskProperties props;
		props.TaskMainFcn = threadfunc_broadcast;
//			tPtr status = NULL;
			
			//    rc = pthread_create(&threadid[i], NULL, threadfunc_broadcast, NULL);
			err = KernelMPI->CreateTask( threadid,
						 (const tTaskProperties )props,	NULL );
			TS_ASSERT_EQUALS( err, ((tErrType)0) );

		err = KernelMPI->LockMutex( mutex_broadcast );
		TS_ASSERT_EQUALS( err, ((tErrType)0) );
		
		while (!conditionMet) 
		{
    			err = KernelMPI->WaitOnCond( cond_broadcast, mutex_broadcast );
				TS_ASSERT_EQUALS( err, ((tErrType)0) );
  		}

 			err = KernelMPI->UnlockMutex( mutex_broadcast );
			TS_ASSERT_EQUALS( err, ((tErrType)0) );
    	
    		err = KernelMPI->DeInitMutex( mutex_broadcast );
			TS_ASSERT_EQUALS( err, ((tErrType)0) );
    	
    		err = KernelMPI->DestroyCond( cond_broadcast );
			TS_ASSERT_EQUALS( err, ((tErrType)0) );

        }	
    
 
 
    // Obtains the process-shared setting of a condition variable attribute object
    void xtestGetCondAttrPShared()
    {
		ptintf_test_info("xtestGetCondAttrPShared");
    	// err = KernelMPI->GetCondAttrPShared( const tCondAttr& attr, int* pShared );
    }


// Sets the process-shared attribute in a condition variable attribute object
// to either PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE
    void xtestSetCondAttrPShared()
    {
		ptintf_test_info("xtestSetCondAttrPShared");
    	// err = KernelMPI->SetCondAttrPShared( tCondAttr* pAttr, int shared );
    }

	void testGetHRTAsUsec()
	{
		ptintf_test_info("testGetHRTAsUsec.");

		U32 uSec;
		U32 uSecPrev;
		tErrType err;
		struct timespec sleeptime;

		sleeptime.tv_sec = 0;
		sleeptime.tv_nsec = 10000;
		unsigned dt = 10200;
		
		err = KernelMPI->GetHRTAsUsec(uSec);
		TS_ASSERT_EQUALS( err, ((tErrType)0) );
		uSecPrev = uSec;

		for(int i = 0; i < 10; i++ )
		{
			nanosleep( &sleeptime, NULL ); 
			err = KernelMPI->GetHRTAsUsec(uSec);
			TS_ASSERT_EQUALS( err, ((tErrType)0) );
			
			if(uSec <= uSecPrev)
			{
 				uSecPrev = uSec;
				continue;
			}		
			TS_ASSERT_EQUALS( err, ((tErrType)0) );
            TS_ASSERT_LESS_THAN_EQUALS(	(uSec - uSecPrev), dt);        	

#if 0 // FIXME/BSK
			printf("uSec=%u dt=%u\n", (unsigned int)uSec, (unsigned int)(uSec - uSecPrev));
			fflush(stdout);
#endif

			uSecPrev = uSec;
		}	
	}
	
	void testGetElapsedAsSec()
	{
		ptintf_test_info("testGetElapsedAsSec. Test takes 10 sec");

		U32 sec;
		U32 secPrev;
		tErrType err;
		struct timespec sleeptime;

		sleeptime.tv_sec = 1;
		sleeptime.tv_nsec = 0;
		float dt = 1.1;
		
		err = KernelMPI->GetElapsedAsSec(sec);
		TS_ASSERT_EQUALS( err, ((tErrType)0) );
		secPrev = sec;

		for(int i = 0; i < 10; i++ )
		{
			nanosleep( &sleeptime, NULL ); 
			err = KernelMPI->GetElapsedAsSec(sec);
			TS_ASSERT_EQUALS( err, ((tErrType)0) );
//		    printf("i=%5d Cur=%u Prev=%u\n", i, (unsigned int)sec, (unsigned int)secPrev);

//			if(sec <= secPrev)
//			{
// 				secPrev = sec;
//				continue;
//			}		
			TS_ASSERT_EQUALS( err, ((tErrType)0) );
            TS_ASSERT_LESS_THAN_EQUALS(	(sec - secPrev), dt);        	
			secPrev = sec;
		}	

	}

	void testGetElapsedTimeAsStructure()
	{
		ptintf_test_info("testGetElapsedTimeAsStructure. Test takes 10 sec");

		tErrType err;
		Current_Time curTime;
		Current_Time curTimePrev;
		
		err = KernelMPI->GetElapsedTimeAsStructure(curTime);
		TS_ASSERT_EQUALS( err, ((tErrType)0) );
		curTimePrev = curTime;

		struct timespec sleeptime;
		sleeptime.tv_sec = 1;
		sleeptime.tv_nsec = 0;

		for(int i = 0; i < 10; i++ )
		{
			nanosleep( &sleeptime, NULL );
			err = KernelMPI->GetElapsedTimeAsStructure(curTime);
			TS_ASSERT_EQUALS( err, ((tErrType)0) );
#if 0 // FIXME/BSK
		    printf("i= %4d DT=%u\n", i, (unsigned int)(curTime.sec - curTimePrev.sec));
#endif
			if( curTime.sec - curTimePrev.sec < 0 )
        	{
				curTimePrev = curTime;
				continue;		        	
        	}
        
        	TS_ASSERT_LESS_THAN_EQUALS(Abs(curTime.sec -  curTimePrev.sec),1);
        	TS_ASSERT_LESS_THAN_EQUALS(Abs(curTime.min -  curTimePrev.min),59);
        	TS_ASSERT_LESS_THAN_EQUALS(Abs(curTime.hour - curTimePrev.hour),23);
        	TS_ASSERT_LESS_THAN_EQUALS(Abs(curTime.mday - curTimePrev.mday),30);
        	TS_ASSERT_LESS_THAN_EQUALS(Abs(curTime.mon -  curTimePrev.mon),11);
        	TS_ASSERT_LESS_THAN_EQUALS(Abs(curTime.year - curTimePrev.year),1);

			curTimePrev = curTime;

#if 0 // FIXME/BSK
			printf("\n\ni=%d.  Current RTC date/time is %d-%d-%d, %02d:%02d:%02d.\n",
				i, curTime.mday, curTime.mon, curTime.year + 1900,
					curTime.hour, curTime.min, curTime.sec);
			fflush(stdout);		
#endif        
		}	

	}
// =========================================================
	void ptintf_test_info( char *pName )
	{
		static int testNum = 1;
		if( testNum == 1 )
		{
			printf("\n");
			printf(".");
		}
		printf("#%3d Test Name = %s\n", testNum++, pName );
		fflush(stdout);
	}	
};


//EOF
