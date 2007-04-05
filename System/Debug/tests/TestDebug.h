// TestDebugMPI.h

#include <string>
#include <iostream>
#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <DebugMPI.h>
#include <UnitTestUtils.h>

// For lots of text output, enable this:
//#define	LF_BRIO_VERBOSE_TEST_OUTPUT

using namespace std;

//============================================================================
// TestDebugMPI functions
//============================================================================
class TestDebug : public CxxTest::TestSuite, TestSuiteBase
{
private:
public:
	CDebugMPI*		DebugMPI;
	//------------------------------------------------------------------------
	void setUp( )
	{
		DebugMPI = new CDebugMPI();
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete DebugMPI; 
	}

	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		TS_ASSERT( DebugMPI != NULL );
		TS_ASSERT( DebugMPI->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		tVersion		version;
		CString			empty;
		CURI			emptyu;
		ConstPtrCString	pName = &empty;
		ConstPtrCURI	pURI = &emptyu;
		
		if ( DebugMPI->IsValid() ) {
			TS_ASSERT_EQUALS( kNoErr, DebugMPI->GetMPIVersion(version) );
			TS_ASSERT_EQUALS( kNoErr, DebugMPI->GetMPIName(pName) );
			TS_ASSERT_EQUALS( version, MakeVersion(0, 1) );
			TS_ASSERT_EQUALS( *pName, "DebugMPI" );
	
			TS_ASSERT_EQUALS( kNoErr, DebugMPI->GetModuleVersion(version) );
			TS_ASSERT_EQUALS( version, MakeVersion(0, 1) );
			TS_ASSERT_EQUALS( kNoErr, DebugMPI->GetModuleName(pName) );
			TS_ASSERT_EQUALS( *pName, "Debug" );
			TS_ASSERT_EQUALS( kNoErr, DebugMPI->GetModuleOrigin(pURI) );
			TS_ASSERT_EQUALS( *pURI, "Debug Module Origin URI" );
		}
	}
	
	void testDumpCoreInfo( )
	{
		tErrType err;
		Boolean fValid;
		tVersion version;
		ConstPtrCString pName;
		ConstPtrCURI pURI;

#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT			
		if ( DebugMPI->IsValid() ) {
			err = DebugMPI->GetMPIName( pName );
			std::cout << "MPI name is: " << pName->c_str() << endl;
		
			err = DebugMPI->GetMPIVersion(version);
			std::cout << "MPI Version is: " << version << endl;
			
			err = DebugMPI->GetModuleVersion( version );
			std::cout << "Module version is: " << version << endl;
			
			err = DebugMPI->GetModuleName( pName );
			std::cout << "Module name is: " << pName->c_str() << endl;
			
			err = DebugMPI->GetModuleOrigin( pURI );
			std::cout << "Module Origin name is: " << pName->c_str() << endl;
		}		
#endif
	}
	
	//------------------------------------------------------------------------
	void testDebugOut( )
	{
		tDebugLevel level;
		
#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT			
		if ( DebugMPI->IsValid() ) {
		    std::cout << endl << "Hello, world Debug Out Tests!\n";
		
			cout << "Debug levels are: " << endl;
			cout << "	kDbgLvlSilent = " << kDbgLvlSilent  << endl;
			cout << "	kDbgLvlCritical = " << kDbgLvlCritical << endl;
			cout << "	kDbgLvlImportant = " << kDbgLvlImportant << endl;
			cout << "	kDbgLvlValuable = " << kDbgLvlValuable << endl;
			cout << "	kDbgLvlNoteable = " << kDbgLvlNoteable << endl;
			cout << "	kDbgLvlVerbose = " << kDbgLvlVerbose << endl;
			
			cout << "Default debug level: " << DebugMPI->GetDebugLevel() << endl;
			cout << "Should be: " << kDbgLvlValuable << endl;
	
			TS_ASSERT_EQUALS( kDbgLvlValuable, DebugMPI->GetDebugLevel() );
			
			DebugMPI->SetDebugLevel( kDbgLvlCritical );
			TS_ASSERT_EQUALS( kDbgLvlCritical, DebugMPI->GetDebugLevel() );
			cout << "New debug level: " << DebugMPI->GetDebugLevel() << endl;
			
			cout << "You shouldn't see anything here: ";
			DebugMPI->DebugOut((tDebugSignature)kTestMPISig, kDbgLvlNoteable, (const char *)"This is a msg from DebugOut() at level %d\n", kDbgLvlNoteable);
		
			cout << endl;
			
			DebugMPI->SetDebugLevel( kDbgLvlNoteable );
			TS_ASSERT_EQUALS( kDbgLvlNoteable, DebugMPI->GetDebugLevel() );
			cout << "New debug level: " << DebugMPI->GetDebugLevel() << endl;
		
			cout << "But you should see something here: ";
			DebugMPI->DebugOut((tDebugSignature)kTestMPISig, kDbgLvlValuable, (const char *)"This is a msg from DebugOut() at level %d\n", kDbgLvlValuable);
		
			cout << endl;
	
			DebugMPI->SetDebugLevel( kDbgLvlSilent );
			TS_ASSERT_EQUALS( kDbgLvlSilent, DebugMPI->GetDebugLevel() );
			cout << "New debug level: " << DebugMPI->GetDebugLevel() << endl;
	
			cout << "and not here: ";
			DebugMPI->DebugOut((tDebugSignature)kTestMPISig, kDbgLvlCritical, (const char *)"This is a msg from DebugOut() at level %d\n", kDbgLvlCritical);
	
			cout << endl;
	
			DebugMPI->SetDebugLevel( kDbgLvlNoteable );
			TS_ASSERT_EQUALS( kDbgLvlNoteable, DebugMPI->GetDebugLevel() );
			cout << "New debug level: " << DebugMPI->GetDebugLevel() << endl;
	
			
			cout << "Exiting Debug Out Test " << endl;
		}
#endif
	}
	
	//------------------------------------------------------------------------
	void testAsserts( )
	{
		tDebugLevel level;
		
#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT			
		if ( DebugMPI->IsValid() ) {
		    std::cout << endl << "Hello, world Assert Tests!\n";
		
			cout << "Testing assertions, you shouldn't see this: ";
			DebugMPI->Assert(true, "I'm not supposed to print!\n");
	
			cout << endl;
	
			DebugMPI->Assert(true, "I'm not supposed to print!\n");
	
			cout << "But you should see this: ";
			DebugMPI->Assert(false, "I'm supposed to print!\n");
	
			cout << endl;
	
			cout << "This should containt the numbers 1, 2, 3: ";
			DebugMPI->Assert(false, "Here they are: %d, %d, %d.\n", 1, 2, 3);
	
			cout << "This should containt the words:  green, electric, spoon: ";
			DebugMPI->Assert(false, "Here they are: %s, %s, %s.\n", "green", "electric", "spoon");
	
			cout << "Exiting Assert Tests " << endl;
		}
#endif
	}

		//------------------------------------------------------------------------
	void testTimestamps( )
	{
		tDebugLevel level;
		
#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT			
		if ( DebugMPI->IsValid() ) {
		    std::cout << endl << "Hello, world Timestamp Tests!\n";
		
			DebugMPI->EnableDebugOutTimestamp();
			
			DebugMPI->DebugOut((tDebugSignature)kTestMPISig, kDbgLvlCritical, (const char *)"This is a timestamped msg from DebugOut() at level %d\n", kDbgLvlCritical);
	
			DebugMPI->DisableDebugOutTimestamp();
			
			DebugMPI->DebugOut((tDebugSignature)kTestMPISig, kDbgLvlCritical, (const char *)"This is a non-timestamped msg from DebugOut() at level %d\n", kDbgLvlCritical);
	
			DebugMPI->EnableDebugOutTimestamp();
			
			DebugMPI->DebugOut((tDebugSignature)kTestMPISig, kDbgLvlCritical, (const char *)"This is a another timestamped msg from DebugOut() at level %d\n", kDbgLvlCritical);
	
			cout << "Exiting Timestamp Tests " << endl;
		}
#endif
	}	
};
