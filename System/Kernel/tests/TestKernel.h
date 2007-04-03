// TestKernel.h

#include <string>
#include <iostream>
#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <KernelMPI.h>
#include <UnitTestUtils.h>

using namespace std;

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
};
