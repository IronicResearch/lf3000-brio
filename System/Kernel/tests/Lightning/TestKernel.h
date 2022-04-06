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

void ptintf_test_info( char *pName );
//------------------------------------------------------------------------------------------
 
}  // workspace

LF_USING_BRIO_NAMESPACE()

//============================================================================
// Test functions
//============================================================================
class TestKernelMPI_Board : public CxxTest::TestSuite, TestSuiteBase
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

 	void xtestWasCreated( )
	{
		ptintf_test_info("testWasCreated");

		TS_ASSERT( KernelMPI != NULL );
		TS_ASSERT( KernelMPI->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void xtestCoreMPI( )
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

	void xtestGetHRTAsUsec()
	{
		ptintf_test_info("testGetHRTAsUsec. Only Board Test ");

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
	
	void xtestGetElapsedAsSec()
	{
		ptintf_test_info("testGetElapsedAsSec. Only Board Test. Test takes 10 sec");

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

	void xtestGetElapsedTimeAsStructure()
	{
		ptintf_test_info("testGetElapsedTimeAsStructure. Only Board Test. Test takes 10 sec");

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
//		    printf("i= %4d DT=%u\n", i, (unsigned int)(curTime.sec - curTimePrev.sec));

			TS_ASSERT_EQUALS( err, ((tErrType)0) );
		
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
