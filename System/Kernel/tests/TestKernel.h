// TestKernel.h

#include <string>
#include <iostream>
#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <KernelMPI.h>
#include <UnitTestUtils.h>

using namespace std;
LF_USING_BRIO_NAMESPACE()

typedef struct{
	int numTest;
	char *testDescription;
}thread_arg_t;

    void *myTask(void* threadArg)
    {
		printf("myTask was created and started\n");
	
		int my_policy;
		struct sched_param my_param;
        thread_arg_t *sentData;

		sentData = (thread_arg_t *)threadArg;
		printf("----Test=#%d %s \n", sentData->numTest, sentData->testDescription);   
		
		pthread_getschedparam(pthread_self(),
						  &my_policy,
						  &my_param);
		
		printf("\nThread routine running at %s Priority is %d\n",
		(my_policy == SCHED_FIFO ? "FIFO"
		:(my_policy == SCHED_RR ? "RR"
		:(my_policy == SCHED_OTHER ? "OTHER"
		:"unknown"))),
		my_param.sched_priority);						   	
		fflush( stdout );
		
		sleep(2);	// In order to test Cancel function
    }
//============================================================================
// TestAudioMgr functions
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

	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		TS_ASSERT( KernelMPI != NULL );
		TS_ASSERT( KernelMPI->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		tVersion		version;
		CString			empty;
		ConstPtrCString	pName = &empty;
		ConstPtrCURI	pURI = &empty;
		
		TS_ASSERT_EQUALS( kNoErr, KernelMPI->GetMPIVersion(version) );
		TS_ASSERT_EQUALS( kNoErr, KernelMPI->GetMPIName(pName) );
		TS_ASSERT_EQUALS( version, MakeVersion(0, 1) );
		TS_ASSERT_EQUALS( *pName, "KernelMPI" );

		TS_ASSERT_EQUALS( kNoErr, KernelMPI->GetModuleVersion(version) );
		TS_ASSERT_EQUALS( version, MakeVersion(0, 1) );
		TS_ASSERT_EQUALS( kNoErr, KernelMPI->GetModuleName(pName) );
		TS_ASSERT_EQUALS( *pName, "Kernel" );
		TS_ASSERT_EQUALS( kNoErr, KernelMPI->GetModuleOrigin(pURI) );
		TS_ASSERT_EQUALS( *pURI, "/Somewhere/KernelModule" );
	}
	
	void testDumpCoreInfo( )
	{
		tErrType err;
		Boolean fValid;
		tVersion version;
		ConstPtrCString pName;
		ConstPtrCURI pURI;
		
	    std::cout << "Hello, world!\n";
		
		fValid = KernelMPI->IsValid();
		std::cout << "MPI IsValid = " << fValid << endl;
		
		err = KernelMPI->GetMPIName( pName );
		std::cout << "MPI name is: " << *pName << endl;
	
		err = KernelMPI->GetMPIVersion(version);
		std::cout << "MPI Version is: " << version << endl;
		
		err = KernelMPI->GetModuleVersion( version );
		std::cout << "Module version is: " << version << endl;
		
		err = KernelMPI->GetModuleName( pName );
		std::cout << "Module name is: " << *pName << endl;
		
		err = KernelMPI->GetModuleOrigin( pURI );
		std::cout << "Module Origin name is: " << *pURI << endl;
		
	}

	void testCreateTask()
	{
//--------------------------------------------------------------		
// 	For reference:
//		struct tTaskProperties 
//		{
//			U32 				priority;	            // 1
//			tAddr				stackAddr;				// 2
//			U32 				stackSize;				// 3	
//			tTaskMainFcn_posix	TaskMainFcn;            // 4
//			U32					taskMainArgCount;       // 5
//			tPtr				pTaskMainArgValues;		// 6						
//			tTaskStartupMode	startupMode;	        // 7
//			tTaskSchedPolicy	schedulingPolicy;		// 8
//			U32					schedulingInterval;     // 9
//   		int                 inheritSched;           // 10
//		};
//----------------------------------------------------------------

		const CURI *pTaskURI = NULL;
		tTaskProperties pProperties;
		tTaskHndl *pHndl;
        thread_arg_t threadArg;
        int testNumber = 1;
 #if 0
 //-----------------	Test 1	----------------------------        
        threadArg.numTest = testNumber++;
        char *message1 = "Default Properties";
        threadArg.testDescription = message1;

		pProperties.TaskMainFcn = (void* (*)(void*))myTask;
		pProperties.pTaskMainArgValues = &threadArg;
		
		TS_ASSERT_EQUALS( kNoErr, KernelMPI->CreateTask( NULL, &pProperties, pHndl) );
		TS_ASSERT_EQUALS( kNoErr, KernelMPI->JoiningThreads( *pHndl, NULL ) );

//-----------------	Test 2	----------------------------        
        threadArg.numTest = testNumber++;
        char *message3 = "Set Priority and Policcy";
        threadArg.testDescription = message3;

		pProperties.priority = 50;
    	pProperties.schedulingPolicy = SCHED_FIFO;

        pProperties.TaskMainFcn = (void* (*)(void*))myTask;
		pProperties.pTaskMainArgValues = &threadArg;
		
		TS_ASSERT_EQUALS( kNoErr, KernelMPI->CreateTask( NULL, &pProperties, pHndl) );
		TS_ASSERT_EQUALS( kNoErr, KernelMPI->JoiningThreads( *pHndl, NULL ) );
//-----------------	Test 3	----------------------------        

        threadArg.numTest = testNumber++;
        char *message2 = "Cancel Thread";
        threadArg.testDescription = message2;

		pProperties.priority = 50;
    	pProperties.schedulingPolicy = SCHED_FIFO;

        pProperties.TaskMainFcn = (void* (*)(void*))myTask;
		pProperties.pTaskMainArgValues = &threadArg;
		
		TS_ASSERT_EQUALS( kNoErr, KernelMPI->CreateTask( NULL, &pProperties, pHndl) );
		TS_ASSERT_EQUALS( kNoErr, KernelMPI->CancelTask( *pHndl ) );
#endif
//-----------------	Test 4	----------------------------        
        U32 size = 0x5000;
        tPtr pPtr = NULL;
        
		TS_ASSERT_EQUALS( kNoErr, KernelMPI->Malloc( size, pPtr ) );
		TS_ASSERT_EQUALS( kNoErr, KernelMPI->Free( pPtr ) );
	}		
};
