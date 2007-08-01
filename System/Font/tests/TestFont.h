// TestFontMPI.h

#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <FontMPI.h>
#include <ResourceMPI.h>
#include <DisplayMPI.h>
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
	CDisplayMPI*	pDisplayMPI_;
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

	//------------------------------------------------------------------------
	void testFontDisplay()
	{
		tRsrcHndl	pkg;
		tRsrcHndl	handle1;
		tRsrcHndl	handle2;
		tFontHndl	font1;
		tFontHndl	font2;
		tFontProp	prop1 = {1, 18, 0, 0};
		tFontProp	prop2 = {1, 24, 0, 0};
		tFontAttr	attr;
		tFontSurf	surf;
		tFontMetrics	mtrx;
		tDisplayHandle 	disp;
		CString			text1 = CString("The Quick Brown Fox");
		CString			text2 = CString("Jumps Over the Lazy Dog");

		pDisplayMPI_ = new CDisplayMPI;
		disp = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatARGB8888, NULL);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, 0, 0);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = kPixelFormatARGB8888;
		memset(surf.buffer, 0, surf.height * surf.pitch);
		
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
		pFontMPI_->GetFontMetrics(&mtrx);
		TS_ASSERT( mtrx.height != 0 );

		attr.version = 1;
		attr.color = 0x0000FFFF;
		attr.antialias = true;
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text1, 0, 0, &surf);
		pFontMPI_->DrawString(&text2, 0, mtrx.height, &surf);

		font2 = pFontMPI_->LoadFont(handle2, prop2);
		TS_ASSERT( font2 != kInvalidFontHndl );
		pFontMPI_->GetFontMetrics(&mtrx);
		TS_ASSERT( mtrx.height != 0 );

		attr.color = 0x00FF0000;
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text1, 0, 2*mtrx.height, &surf);
		pFontMPI_->DrawString(&text2, 0, 3*mtrx.height, &surf);
		
		pDisplayMPI_->Invalidate(0, NULL);
		sleep(1);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		
		pFontMPI_->UnloadFont(font1);
		pFontMPI_->UnloadFont(font2);

		pResourceMPI_->ClosePackage(pkg);
		pResourceMPI_->CloseAllDevices();
		delete pResourceMPI_;
		delete pDisplayMPI_;
	}

};

// EOF
