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

//============================================================================
// Test functions
//============================================================================
class TestKernelMPI_Emulation : public CxxTest::TestSuite, TestSuiteBase
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
	void ptintf_test_info( char *pName )
	{
		static int testNum = 1;
		if( testNum == 1 )
		{
			printf("\n");
			printf(".");
		}
		printf("Number=%3d Test Name = %s\n", testNum++, pName );
		fflush(stdout);
	}	
};

//EOF
