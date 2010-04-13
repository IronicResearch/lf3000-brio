// TestDebugMPI.h

#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <DebugMPI.h>
#include <UnitTestUtils.h>
#include <AudioTypes.h>
#include <KernelTypes.h>
//#include <ResourceTypes.h>


LF_USING_BRIO_NAMESPACE()

#define LF_BRIO_VERBOSE_TEST_OUTPUT

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
		PRINT_TEST_NAME();
		
		TS_ASSERT( DebugMPI != NULL );
		TS_ASSERT( DebugMPI->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		PRINT_TEST_NAME();
		
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
		PRINT_TEST_NAME();
		
#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT
		tVersion version;
		ConstPtrCString pName;
		ConstPtrCURI pURI;
		CDebugMPI dbg(kTestSuiteDebugSig);

		printf("size of CDebugMPI object is %d\n", sizeof(CDebugMPI));
//		printf("member variable pModule_ offset is %d\n", (unsigned int)&dbg.pModule_ - (unsigned int)&dbg);		
//		printf("member variable sig_ offset is %d\n", (unsigned int)&dbg.sig_ - (unsigned int)&dbg);		

		
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
	void testDebugOut( )
	{
		PRINT_TEST_NAME();
		
#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT			
		tDebugLevel level;

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
			
			DebugMPI->DebugOut(kDbgLvlNoteable, "\nThis msg should not be displayed ! since DebugOut() at level kDbgLvlNoteable=%d\n", kDbgLvlNoteable);
		
			printf("\n");
			
			DebugMPI->SetDebugLevel( kDbgLvlNoteable );
			TS_ASSERT_EQUALS( kDbgLvlNoteable, DebugMPI->GetDebugLevel() );
			printf("\nNew debug level kDbgLvlNoteable: %d\n", DebugMPI->GetDebugLevel());
		
			DebugMPI->DebugOut(kDbgLvlValuable, "This is a msg from DebugOut() at level kDbgLvlValuable=%d\n", kDbgLvlValuable);
		
			printf("\n");
	
			DebugMPI->SetDebugLevel( kDbgLvlSilent );
			TS_ASSERT_EQUALS( kDbgLvlSilent, DebugMPI->GetDebugLevel() );
			printf("New debug level kDbgLvlSilent: %d\n", DebugMPI->GetDebugLevel());
	
			DebugMPI->DebugOut(kDbgLvlCritical, "This msg should not be displayed !, since DebugOut() at level kDbgLvlCritical=%d\n", kDbgLvlCritical);
	
			printf("\n");
	
			printf("\nExiting Debug Out Test \n");
		}
#endif
	}
	
		//------------------------------------------------------------------------
	void testTimestamps( )
	{
		PRINT_TEST_NAME();
		
#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT			
		tDebugLevel level;

		if ( DebugMPI->IsValid() ) {
		
			DebugMPI->EnableDebugOutTimestamp();
			
			DebugMPI->DebugOut(kDbgLvlCritical, "This is a timestamped msg from DebugOut() at level %d\n", kDbgLvlCritical);
	
			DebugMPI->DisableDebugOutTimestamp();
			
			DebugMPI->DebugOut(kDbgLvlCritical, "This is a non-timestamped msg from DebugOut() at level %d\n", kDbgLvlCritical);
	
			DebugMPI->EnableDebugOutTimestamp();
			
			DebugMPI->DebugOut(kDbgLvlCritical, "This is a another timestamped msg from DebugOut() at level %d\n", kDbgLvlCritical);
	
			DebugMPI->DisableDebugOutTimestamp();

			printf("Exiting Timestamp Tests \n");
		}
#endif
	}	

	//------------------------------------------------------------------------
	void testErrors( )
	{
		PRINT_TEST_NAME();
		
#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT			
		if ( DebugMPI->IsValid() ) {
			DebugMPI->DebugOutErr(kDbgLvlCritical, kAudioCreateTaskErr, 
					"in CreateTask()\n");
			DebugMPI->DebugOutErr(kDbgLvlCritical, kDirectoryNotEmptyErr, 
					"in LoadResource\n");
					
		}
#endif
	}
	
	
	void testSetGetDebugLevel()
	{
		PRINT_TEST_NAME();
		
		 const tDebugSignature kMyApp = kFirstCartridge1DebugSig;
		 CDebugMPI dbgMPI1(kMyApp);
		 CDebugMPI dbgMPI2(kMyApp);
		
		printf("YTU SIG: 0x%x, Level = %d, DebugOutIsEnabled: %d\n", kMyApp, kDbgLvlCritical, dbgMPI1.DebugOutIsEnabled(kMyApp, kDbgLvlCritical));

		 dbgMPI1.SetDebugLevel(kDbgLvlCritical);
		 dbgMPI2.SetDebugLevel(kDbgLvlCritical);

		 TS_ASSERT_EQUALS( dbgMPI1.GetDebugLevel(), dbgMPI2.GetDebugLevel() );
		 dbgMPI1.SetDebugLevel(kDbgLvlImportant);
		 
		 TS_ASSERT(dbgMPI1.GetDebugLevel()!=dbgMPI2.GetDebugLevel());
		 dbgMPI1.Assert(dbgMPI1.GetDebugLevel()!=dbgMPI2.GetDebugLevel(), "The debug levels are the same!");
	}


	//------------------------------------------------------------------------
	void testAsserts( )
	{
		PRINT_TEST_NAME();
		
#ifdef LF_BRIO_VERBOSE_TEST_OUTPUT			
		tDebugLevel level;
		CDebugMPI dbg(kTestSuiteDebugSig);

		if ( DebugMPI->IsValid() ) {
		
			DebugMPI->Assert(true, "I'm not supposed to print!\n");
	
			printf("\n");
	
			DebugMPI->Assert(true, "I'm not supposed to print!\n");
			
			dbg.AssertNoErr(kAudioCreateTaskErr, "AssertNoErr(...) test\n");
	
			printf("You should see:  1 green, 2 electric, 3 spoon: \n");
			DebugMPI->Assert(false, "Here they are: %d %s, %d %s, %d %s.\n", 
							1, "green", 2, "electric", 3, "spoon");
	
			printf("Exiting Assert Tests \n");
		}
#endif
	}	

};
