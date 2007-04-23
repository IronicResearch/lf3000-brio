// TestAudio.h 

#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <AudioMPI.h>
#include <KernelMPI.h>
#include <UnitTestUtils.h> 

// For lots of text output, enable this:
//#define	LF_BRIO_VERBOSE_TEST_OUTPUT

LF_USING_BRIO_NAMESPACE()


//============================================================================
// TestAudioMgr functions
//============================================================================
class TestAudio : public CxxTest::TestSuite, TestSuiteBase 
{
private:
public:
	CAudioMPI*		AudioMPI;
	CKernelMPI*		KernelMPI;
	//------------------------------------------------------------------------
	void setUp( )
	{
		AudioMPI = new CAudioMPI();
		KernelMPI = new CKernelMPI();
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete AudioMPI; 
		delete KernelMPI; 
	}
	


	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		TS_ASSERT( AudioMPI != NULL );
		TS_ASSERT( AudioMPI->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;
		
		if (AudioMPI->IsValid()) {
			pName = AudioMPI->GetMPIName();
			TS_ASSERT_EQUALS( *pName, "AudioMPI" );
			version = AudioMPI->GetModuleVersion();
			TS_ASSERT_EQUALS( version, 2 );
			pName = AudioMPI->GetModuleName();
			TS_ASSERT_EQUALS( *pName, "Audio" );
			pURI = AudioMPI->GetModuleOrigin();
			TS_ASSERT_EQUALS( *pURI, "/Somewhere/AudioModule" );
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
		fValid = AudioMPI->IsValid();
		printf("MPI IsValid = %d\n");
		
		if (AudioMPI->IsValid()) {
			err = AudioMPI->GetMPIName( pName );
			printf("MPI name is: %s\n", *pName);
		
			err = AudioMPI->GetModuleVersion( version );
			printf("Module version is: %d\n", version);
			
			err = AudioMPI->GetModuleName( pName );
			printf("Module name is: %s\n", *pName);
			
			err = AudioMPI->GetModuleOrigin( pURI );
			printf("Module Origin name is: %s\n", *pURI);
		}		
#endif
	}

	//------------------------------------------------------------------------
	void testAudioOutput( )
	{
		tErrType err;
		const int kDuration = 1 * 1000;
		
		// Start up sine output.
		err = AudioMPI->StartAudio();
		TS_ASSERT_EQUALS( kNoErr, err );

		// sleep3 seconds
		err = KernelMPI->TaskSleep( kDuration );
		TS_ASSERT_EQUALS( kNoErr, err );

		// stop the engine.
		err = AudioMPI->StopAudio();
		TS_ASSERT_EQUALS( kNoErr, err );

		// Start up sine output.
		err = AudioMPI->StartAudio();
		TS_ASSERT_EQUALS( kNoErr, err );

		// sleep 3 seconds
		err = KernelMPI->TaskSleep( kDuration );
		TS_ASSERT_EQUALS( kNoErr, err );

		// stop the engine.
		err = AudioMPI->StopAudio();
		TS_ASSERT_EQUALS( kNoErr, err );

		// Start up sine output.
		err = AudioMPI->StartAudio();
		TS_ASSERT_EQUALS( kNoErr, err );

		// sleep 3 seconds
		err = KernelMPI->TaskSleep( kDuration );
		TS_ASSERT_EQUALS( kNoErr, err );

		// stop the engine.
		err = AudioMPI->StopAudio();
		TS_ASSERT_EQUALS( kNoErr, err );
	}
	
};
