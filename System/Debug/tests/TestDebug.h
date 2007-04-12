// TestDebugMPI.h

#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <DebugMPI.h>
#include <UnitTestUtils.h>
#include <AudioTypes.h>
#include <ResourceTypes.h>

// For lots of text output, enable this:
//#define	LF_BRIO_VERBOSE_TEST_OUTPUT

LF_USING_BRIO_NAMESPACE()


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
		DebugMPI = new CDebugMPI(kGroupUnitTests);
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
			printf("MPI name is: %s\n", pName->c_str());
		
			err = DebugMPI->GetMPIVersion(version);
			printf("MPI Version is: %d\n", version);
			
			err = DebugMPI->GetModuleVersion( version );
			printf("Module version is: %d\n", version);
			
			err = DebugMPI->GetModuleName( pName );
			printf("Module name is: %s\n", pName->c_str());
			
			err = DebugMPI->GetModuleOrigin( pURI );
			printf("Module Origin name is: %s\n", pName->c_str());
		}		
#endif
	}
	
	//------------------------------------------------------------------------
	void testDebugOut2( )
	{
		TS_ASSERT_EQUALS( kDbgLvlValuable, DebugMPI->GetDebugLevel() );
		DebugMPI->SetDebugLevel( kDbgLvlCritical );
		TS_ASSERT_EQUALS( kDbgLvlCritical, DebugMPI->GetDebugLevel() );
		DebugMPI->SetDebugLevel( kDbgLvlNoteable );
		TS_ASSERT_EQUALS( kDbgLvlNoteable, DebugMPI->GetDebugLevel() );
		DebugMPI->SetDebugLevel( kDbgLvlValuable );
		TS_ASSERT_EQUALS( kDbgLvlValuable, DebugMPI->GetDebugLevel() );
	}
	
	//------------------------------------------------------------------------
	void testDebugOut( )
	{
		tDebugLevel level;
		
#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT			
		if ( DebugMPI->IsValid() ) {
		    printf("\nHello, world Debug Out Tests!\n");
		
			printf("Debug levels are: \n");
			printf("	kDbgLvlSilent = %d\n", kDbgLvlSilent);
			printf("	kDbgLvlCritical = %d\n", kDbgLvlCritical);
			printf("	kDbgLvlImportant = %d\n", kDbgLvlImportant);
			printf("	kDbgLvlValuable = %d\n", kDbgLvlValuable);
			printf("	kDbgLvlNoteable = %d\n", kDbgLvlNoteable);
			printf("	kDbgLvlVerbose = %d\n", kDbgLvlVerbose);
			
			printf("Default debug level: %d\n", DebugMPI->GetDebugLevel());
			printf("Should be: %d\n", kDbgLvlValuable);

			TS_ASSERT_EQUALS( kDbgLvlValuable, DebugMPI->GetDebugLevel() );
			
			DebugMPI->SetDebugLevel( kDbgLvlCritical );
			TS_ASSERT_EQUALS( kDbgLvlCritical, DebugMPI->GetDebugLevel() );
			printf("New debug level: ", DebugMPI->GetDebugLevel());
			
			printf("You shouldn't see anything here: ");
			DebugMPI->DebugOut(kDbgLvlNoteable, "This is a msg from DebugOut() at level %d\n", kDbgLvlNoteable);
		
			printf("\n");
			
			DebugMPI->SetDebugLevel( kDbgLvlNoteable );
			TS_ASSERT_EQUALS( kDbgLvlNoteable, DebugMPI->GetDebugLevel() );
			printf("New debug level: %d\n", DebugMPI->GetDebugLevel());
		
			printf("But you should see something here: ");
			DebugMPI->DebugOut(kDbgLvlValuable, "This is a msg from DebugOut() at level %d\n", kDbgLvlValuable);
		
			printf("\n");
	
			DebugMPI->SetDebugLevel( kDbgLvlSilent );
			TS_ASSERT_EQUALS( kDbgLvlSilent, DebugMPI->GetDebugLevel() );
			printf("New debug level: %d\n", DebugMPI->GetDebugLevel());
	
			printf("and not here: ");
			DebugMPI->DebugOut(kDbgLvlCritical, "This is a msg from DebugOut() at level %d\n", kDbgLvlCritical);
	
			printf("\n");
	
			DebugMPI->SetDebugLevel( kDbgLvlNoteable );
			TS_ASSERT_EQUALS( kDbgLvlNoteable, DebugMPI->GetDebugLevel() );
			printf("New debug level: %d\n", DebugMPI->GetDebugLevel());
	
			
			printf("Exiting Debug Out Test \n");
		}
#endif
	}
	
		//------------------------------------------------------------------------
	void testTimestamps( )
	{
		tDebugLevel level;
		
#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT			
		if ( DebugMPI->IsValid() ) {
		    printf("\nHello, world Timestamp Tests!\n");
		
			DebugMPI->EnableDebugOutTimestamp();
			
			DebugMPI->DebugOut(kDbgLvlCritical, "This is a timestamped msg from DebugOut() at level %d\n", kDbgLvlCritical);
	
			DebugMPI->DisableDebugOutTimestamp();
			
			DebugMPI->DebugOut(kDbgLvlCritical, "This is a non-timestamped msg from DebugOut() at level %d\n", kDbgLvlCritical);
	
			DebugMPI->EnableDebugOutTimestamp();
			
			DebugMPI->DebugOut(kDbgLvlCritical, "This is a another timestamped msg from DebugOut() at level %d\n", kDbgLvlCritical);
			DebugMPI->DebugOutLiteral(kDbgLvlCritical, "No timestamp for DebugOutLiteral()\n");
	
			DebugMPI->DisableDebugOutTimestamp();

			printf("Exiting Timestamp Tests \n");
		}
#endif
	}	

	//------------------------------------------------------------------------
	void testErrors( )
	{
#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT			
		if ( DebugMPI->IsValid() ) {
			DebugMPI->DebugOutErr(kDbgLvlCritical, kAudioCreateTaskErr, 
					"in CreateTask()\n");
			DebugMPI->DebugOutErr(kDbgLvlCritical, kResourceInvalidMPIIdErr, 
					"in LoadResource\n");
					
		}
#endif
	}
	
	//------------------------------------------------------------------------
	void testAsserts( )
	{
		tDebugLevel level;
		
#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT			
		if ( DebugMPI->IsValid() ) {
		    printf("\nHello, world Assert Tests!\n");
		
			printf("Testing assertions, you shouldn't see this: ");
			DebugMPI->Assert(true, "I'm not supposed to print!\n");
	
			printf("\n");
	
			DebugMPI->Assert(true, "I'm not supposed to print!\n");
	
			printf("You should see:  1 green, 2 electric, 3 spoon: ");
			DebugMPI->Assert(false, "Here they are: %d %s, %d %s, %d %s.\n", 
							1, "green", 2, "electric", 3, "spoon");
	
			printf("Exiting Assert Tests \n");
		}
#endif
	}	
};
