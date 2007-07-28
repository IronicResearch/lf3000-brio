// TestFontMPI.h

#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <FontMPI.h>
#include <ResourceMPI.h>
#include <UnitTestUtils.h>

LF_USING_BRIO_NAMESPACE()

const tDebugSignature kMyApp = kFirstCartridge1DebugSig;

//============================================================================
// TestFontMPI functions
//============================================================================
class TestFont : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CFontMPI*		pFontMPI_;
	CResourceMPI*	pResourceMPI_;
public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		pFontMPI_ = new CFontMPI;
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete pFontMPI_; 
	}

	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		TS_ASSERT( pFontMPI_ != NULL );
		TS_ASSERT( pFontMPI_->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;
		
		if ( pFontMPI_->IsValid() ) {
			pName = pFontMPI_->GetMPIName();
			TS_ASSERT_EQUALS( *pName, "FontMPI" );
			version = pFontMPI_->GetModuleVersion();
			TS_ASSERT_EQUALS( version, 2 );
			pName = pFontMPI_->GetModuleName();
			TS_ASSERT_EQUALS( *pName, "Font" );
			pURI = pFontMPI_->GetModuleOrigin();
			TS_ASSERT_EQUALS( *pURI, "/LF/System/Font" );
		}
	}
	
	//------------------------------------------------------------------------
	void testFontResources()
	{
		tRsrcHndl	pkg;
		tRsrcHndl	handle1;
		tRsrcHndl	handle2;
		tFontHndl	font1;
		tFontHndl	font2;
		tFontProp	prop1 = {1, 12, 0, 0};
		tFontProp	prop2 = {1, 24, 0, 0};

		pResourceMPI_ = new CResourceMPI;
		pResourceMPI_->OpenAllDevices();
		pResourceMPI_->SetDefaultURIPath("LF/Brio/UnitTest/Font");

		pkg = pResourceMPI_->FindPackage("FontTest");
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg );
		TS_ASSERT_EQUALS( kNoErr, pResourceMPI_->OpenPackage(pkg) );

		handle1 = pResourceMPI_->FindRsrc("Verdana");
		TS_ASSERT( handle1 != kInvalidRsrcHndl );
		handle2 = pResourceMPI_->FindRsrc("Avatar");
		TS_ASSERT( handle2 != kInvalidRsrcHndl );

		font1 = pFontMPI_->LoadFont(handle1, prop1);
		TS_ASSERT( font1 != kInvalidFontHndl );
		font2 = pFontMPI_->LoadFont(handle2, prop2);
		TS_ASSERT( font2 != kInvalidFontHndl );

		pFontMPI_->UnloadFont(font1);
		pFontMPI_->UnloadFont(font2);

		pResourceMPI_->ClosePackage(pkg);
		pResourceMPI_->CloseAllDevices();
		delete pResourceMPI_;
	}

};

// EOF
