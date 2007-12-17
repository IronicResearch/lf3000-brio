// TestDebugMPI.h

#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <DebugMPI.h>
#include <UnitTestUtils.h>
#include <AudioTypes.h>
#include <KernelTypes.h>
//#include <ResourceTypes.h>

// For lots of text output, enable this:
// BSK/FIXME
//#define	LF_BRIO_VERBOSE_TEST_OUTPUT

LF_USING_BRIO_NAMESPACE()

void ptintf_test_info( char *pName );

//#define LF_BRIO_VERBOSE_TEST_OUTPUT
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
		const CString*	pName;
		const CURI*		pURI;
		
		if ( DebugMPI->IsValid() ) {
			pName = DebugMPI->GetMPIName();
			TS_ASSERT_EQUALS( *pName, "DebugMPI" );
			version = DebugMPI->GetModuleVersion();
			TS_ASSERT_EQUALS( version, 2 );
			pName = DebugMPI->GetModuleName();
			TS_ASSERT_EQUALS( *pName, "Debug" );
			pURI = DebugMPI->GetModuleOrigin();
			TS_ASSERT_EQUALS( *pURI, "Debug Module Origin URI" );
		}
	}
	
	void testDumpCoreInfo( )
{
#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT			
		ptintf_test_info( "testDumpCoreInfo" );
		tVersion version;
		ConstPtrCString pName;
		ConstPtrCURI pURI;

		if ( DebugMPI->IsValid() ) {
			pName = DebugMPI->GetMPIName();
			printf("MPI name is: %s\n", pName->c_str());
		
			version = DebugMPI->GetModuleVersion();
			printf("Module version is: %d\n", version);
			
			pName = DebugMPI->GetModuleName();
			printf("Module name is: %s\n", pName->c_str());
			
			pURI = DebugMPI->GetModuleOrigin();
			printf("Module Origin name is: %s\n", pURI->c_str());
		}		
#endif
	}
	
	//------------------------------------------------------------------------
	void testDebugOut2( )
	{
//		ptintf_test_info( "testDebugOut2" );
		// NOTE/tp: disabled first test because we initialize debug output
		// to silent in unit tests.
//		TS_ASSERT_EQUALS( kDbgLvlValuable, DebugMPI->GetDebugLevel() );
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
		
#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT			
		tDebugLevel level;

		ptintf_test_info( "testDebugOut" );
		if ( DebugMPI->IsValid() ) {
		    printf("\nHello, world Debug Out Tests!\n");
		
			printf("Debug levels are: \n");
			printf("	kDbgLvlSilent =    %d\n", kDbgLvlSilent);
			printf("	kDbgLvlCritical =  %d\n", kDbgLvlCritical);
			printf("	kDbgLvlImportant = %d\n", kDbgLvlImportant);
			printf("	kDbgLvlValuable =  %d\n", kDbgLvlValuable);
			printf("	kDbgLvlNoteable =  %d\n", kDbgLvlNoteable);
			printf("	kDbgLvlVerbose =   %d\n", kDbgLvlVerbose);
			
			printf("\nDefault debug level: %d\n", DebugMPI->GetDebugLevel());
			printf("\n Should be: %d\n", kDbgLvlValuable);

			TS_ASSERT_EQUALS( kDbgLvlValuable, DebugMPI->GetDebugLevel() );
			
			DebugMPI->SetDebugLevel( kDbgLvlCritical );
			TS_ASSERT_EQUALS( kDbgLvlCritical, DebugMPI->GetDebugLevel() );
			printf("\nNew debug level: %d\n", DebugMPI->GetDebugLevel());
			
			printf("\nYou shouldn't see anything here: ");
			DebugMPI->DebugOut(kDbgLvlNoteable, "\nThis is a msg from DebugOut() at level kDbgLvlNoteable=%d\n", kDbgLvlNoteable);
		
			printf("\n");
			
			DebugMPI->SetDebugLevel( kDbgLvlNoteable );
			TS_ASSERT_EQUALS( kDbgLvlNoteable, DebugMPI->GetDebugLevel() );
			printf("\nNew debug level kDbgLvlNoteable: %d\n", DebugMPI->GetDebugLevel());
		
			printf("But you should see something here: ");

			DebugMPI->DebugOut(kDbgLvlValuable, "This is a msg from DebugOut() at level kDbgLvlValuable=%d\n", kDbgLvlValuable);
		
			printf("\n");
	
			DebugMPI->SetDebugLevel( kDbgLvlSilent );
			TS_ASSERT_EQUALS( kDbgLvlSilent, DebugMPI->GetDebugLevel() );
			printf("New debug level kDbgLvlSilent: %d\n", DebugMPI->GetDebugLevel());
	
			printf("and not here: ");
			DebugMPI->DebugOut(kDbgLvlCritical, "This is a msg from DebugOut() at level %d\n", kDbgLvlCritical);
	
			printf("\n");
	
			DebugMPI->SetDebugLevel( kDbgLvlNoteable );
			TS_ASSERT_EQUALS( kDbgLvlNoteable, DebugMPI->GetDebugLevel() );
			printf("New debug level kDbgLvlNoteable: %d\n", DebugMPI->GetDebugLevel());
			printf("But you should see something here: ");
			DebugMPI->DebugOut(kDbgLvlCritical, "This is a msg from DebugOut() at level %d\n", kDbgLvlCritical);
			
			printf("\nExiting Debug Out Test \n");
		}
#endif
	}
	
		//------------------------------------------------------------------------
	void testTimestamps( )
	{
		
#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT			
		tDebugLevel level;

		ptintf_test_info( "testTimestamps" );
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
		ptintf_test_info( "testErrors" );
		if ( DebugMPI->IsValid() ) {
			DebugMPI->DebugOutErr(kDbgLvlCritical, kAudioCreateTaskErr, 
					"in CreateTask()\n");
			DebugMPI->DebugOutErr(kDbgLvlCritical, kDirectoryNotEmptyErr, 
					"in LoadResource\n");
					
		}
#endif
	}
	
	//------------------------------------------------------------------------
	void testAsserts( )
	{
		
#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT			
		tDebugLevel level;
		ptintf_test_info( "testAsserts" );

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
	
	void testSetGetDebugLevel()
	{
		 const tDebugSignature kMyApp = kFirstCartridge1DebugSig;
		 CDebugMPI dbgMPI1(kMyApp);
		 CDebugMPI dbgMPI2(kMyApp);
// FIXME/BSK remove verbose print
//		ptintf_test_info( "testSetGetDebugLevel" );

//		 printf("Original DebugLevel first instance %d\n", dbgMPI1.GetDebugLevel());
//		 printf("Original DebugLevel second instance %d\n", dbgMPI2.GetDebugLevel());

		 dbgMPI1.SetDebugLevel(kDbgLvlCritical);
		 dbgMPI2.SetDebugLevel(kDbgLvlCritical);

// FIXME/BSK remove verbose print
//		 printf("Afrer first change DebugLevel first instance %d\n", dbgMPI1.GetDebugLevel());
//		 printf("Afrer first change DebugLevel second instance %d\n", dbgMPI2.GetDebugLevel());
		 
		 TS_ASSERT_EQUALS( dbgMPI1.GetDebugLevel(), dbgMPI2.GetDebugLevel() );
		 dbgMPI1.SetDebugLevel(kDbgLvlImportant);
		 
// FIXME/BSK remove verbose print
//		 printf("Afrer second change DebugLevel first instance %d\n", dbgMPI1.GetDebugLevel());
//		 printf("Afrer second change DebugLevel second instance %d\n", dbgMPI2.GetDebugLevel());

		 TS_ASSERT(dbgMPI1.GetDebugLevel()!=dbgMPI2.GetDebugLevel());
		 dbgMPI1.Assert(dbgMPI1.GetDebugLevel()!=dbgMPI2.GetDebugLevel(), "The debug levels are the same!");
	}


	// =========================================================
		void ptintf_test_info( char *pName )
		{
			static int testNum = 1;
			if( testNum == 1 )
			{
				printf("\n\n");
				printf(".");
			}
			printf("-----------------#%3d Test Name = %s--------------------------\n", testNum++, pName );
			fflush(stdout);
		}

};
