// TestAudioMgr.h 

#include <string>
#include <iostream>
#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <AudioMPI.h>
#include <KernelMPI.h>
#include <UnitTestUtils.h> 

using namespace std;

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
		CString			empty;
		ConstPtrCString	pName = &empty;
		ConstPtrCURI	pURI = &empty;
		
		if (AudioMPI->IsValid()) {
			TS_ASSERT_EQUALS( kNoErr, AudioMPI->GetMPIVersion(version) );
			TS_ASSERT_EQUALS( kNoErr, AudioMPI->GetMPIName(pName) );
			TS_ASSERT_EQUALS( version, MakeVersion(0, 1) );
			TS_ASSERT_EQUALS( *pName, "AudioMPI" );
	
			TS_ASSERT_EQUALS( kNoErr, AudioMPI->GetModuleVersion(version) );
			TS_ASSERT_EQUALS( version, MakeVersion(0, 1) );
			TS_ASSERT_EQUALS( kNoErr, AudioMPI->GetModuleName(pName) );
			TS_ASSERT_EQUALS( *pName, "Audio" );
			TS_ASSERT_EQUALS( kNoErr, AudioMPI->GetModuleOrigin(pURI) );
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
		
	    std::cout << "\nHello, world dump Audio MPI!\n";
		
		fValid = AudioMPI->IsValid();
		std::cout << "MPI IsValid = " << fValid << endl;
		
		if (AudioMPI->IsValid()) {
			err = AudioMPI->GetMPIName( pName );
			std::cout << "MPI name is: " << *pName << endl;
		
			err = AudioMPI->GetMPIVersion(version);
			std::cout << "MPI Version is: " << version << endl;
			
			err = AudioMPI->GetModuleVersion( version );
			std::cout << "Module version is: " << version << endl;
			
			err = AudioMPI->GetModuleName( pName );
			std::cout << "Module name is: " << *pName << endl;
			
			err = AudioMPI->GetModuleOrigin( pURI );
			std::cout << "Module Origin name is: " << *pURI << endl;
		}		
	}

	//------------------------------------------------------------------------
	void testAudioasdfadfOutput( )
	{
		tErrType err;
		
		err = AudioMPI->StartAudio();
		TS_ASSERT_EQUALS( kNoErr, err );

		err = KernelMPI->TaskSleep( 5000 );
		TS_ASSERT_EQUALS( kNoErr, err );

		err = AudioMPI->StopAudio();
		TS_ASSERT_EQUALS( kNoErr, err );
	}
	
};
