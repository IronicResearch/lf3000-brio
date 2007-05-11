// TestDisplayMPI.h

#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <DisplayMPI.h>
#include <UnitTestUtils.h>

LF_USING_BRIO_NAMESPACE()

//============================================================================
// TestDisplayMPI functions
//============================================================================
class TestDisplay : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CDisplayMPI*	DisplayMPI;
public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		DisplayMPI = new CDisplayMPI;
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete DisplayMPI; 
	}

	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		TS_ASSERT( DisplayMPI != NULL );
		TS_ASSERT( DisplayMPI->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;
		
		if ( DisplayMPI->IsValid() ) {
			pName = DisplayMPI->GetMPIName();
			TS_ASSERT_EQUALS( *pName, "DisplayMPI" );
			version = DisplayMPI->GetModuleVersion();
			TS_ASSERT_EQUALS( version, 2 );
			pName = DisplayMPI->GetModuleName();
			TS_ASSERT_EQUALS( *pName, "Display" );
			pURI = DisplayMPI->GetModuleOrigin();
			TS_ASSERT_EQUALS( *pURI, "/LF/System/Display" );
		}
	}
};

// EOF
