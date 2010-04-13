// TestFontMPI.h

#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <FontMPI.h>
#include <DisplayMPI.h>
#include <BrioOpenGLConfig.h>
#include <UnitTestUtils.h>

#include <ft2build.h>		// FreeType auto-conf settings
#include <freetype.h>

LF_USING_BRIO_NAMESPACE()

const tDebugSignature kMyApp = kFirstCartridge1DebugSig;

//----------------------------------------------------------------------------
inline CPath GetTestRsrcFolder( )
{
#ifdef EMULATION
	CPath dir = EmulationConfig::Instance().GetCartResourceSearchPath();
	return dir + "Font/";
#else	// EMULATION
	return "/LF/Base/Brio/rsrc/Font/";
#endif	// EMULATION
}

//============================================================================
// TestFontMPI functions
//============================================================================
class TestFont : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CFontMPI*		pFontMPI_;
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
		PRINT_TEST_NAME();
		
		TS_ASSERT( pFontMPI_ != NULL );
		TS_ASSERT( pFontMPI_->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		PRINT_TEST_NAME();
		
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
		PRINT_TEST_NAME();
		
		tFontHndl	font1;
		tFontHndl	font2;
		tFontHndl	font3;
		tFontHndl	font4;
		tFontHndl   fontbogus;
		tFontProp	prop1 = {1, 12, 0, 0};
		tFontProp	prop2 = {1, 24, 0, 0};
		tFontProp	prop3 = {2, 24, kUTF8CharEncoding, true};
		tFontProp	prop4 = {2, 24, kUTF16CharEncoding, true};
		
		CPath dir = GetTestRsrcFolder();
		pFontMPI_->SetFontResourcePath(dir);
		CPath* path = pFontMPI_->GetFontResourcePath();
		TS_ASSERT( dir == *path );
		
		font1 = pFontMPI_->LoadFont("DidjPropBold.ttf", prop1);
		TS_ASSERT( font1 != kInvalidFontHndl );
		font2 = pFontMPI_->LoadFont("DidjPropBold.ttf", prop2);
		TS_ASSERT( font2 != kInvalidFontHndl );
		font3 = pFontMPI_->LoadFont("FreeSans.ttf", prop3);
		TS_ASSERT( font3 != kInvalidFontHndl );
		font4 = pFontMPI_->LoadFont("FreeSerif.ttf", prop4);
		TS_ASSERT( font4 != kInvalidFontHndl );
		fontbogus = pFontMPI_->LoadFont("NonExistantFont.ttf", prop1);
		TS_ASSERT( fontbogus  == kInvalidFontHndl );
		fontbogus = pFontMPI_->LoadFont("", prop1);
		TS_ASSERT( fontbogus  == kInvalidFontHndl );

		Boolean r;
		r = pFontMPI_->SelectFont(font1);
		TS_ASSERT( r != false );
		r = pFontMPI_->SelectFont(font2);
		TS_ASSERT( r != false );
		r = pFontMPI_->SelectFont(font3);
		TS_ASSERT( r != false );
		r = pFontMPI_->SelectFont(font4);
		TS_ASSERT( r != false );
		r = pFontMPI_->SelectFont(fontbogus);
		TS_ASSERT( r == false );

		pFontMPI_->UnloadFont(font1);
		pFontMPI_->UnloadFont(font2);
		pFontMPI_->UnloadFont(font3);
		pFontMPI_->UnloadFont(font4);
	}

	//------------------------------------------------------------------------
	void testFontDisplay()
	{
		PRINT_TEST_NAME();
		
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
		tRect			rect1,rect2;

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
		
		pFontMPI_->SetFontResourcePath(GetTestRsrcFolder());
		font1 = pFontMPI_->LoadFont("DidjPropBold.ttf", prop1);
		TS_ASSERT( font1 != kInvalidFontHndl );
		pFontMPI_->GetFontMetrics(&mtrx);
		TS_ASSERT( mtrx.height != 0 );
		TS_ASSERT( mtrx.ascent != 0 );
		TS_ASSERT( mtrx.descent != 0 );
		
		pFontMPI_->GetStringRect(&text1, &rect1);
		TS_ASSERT( rect1.top <= mtrx.ascent );
		TS_ASSERT( rect1.bottom >= mtrx.descent );
		pFontMPI_->GetStringRect(&text2, &rect2);
		TS_ASSERT( rect2.top <= mtrx.ascent );
		TS_ASSERT( rect2.bottom >= mtrx.descent );
		
		attr.version = 1;
		attr.color = 0x000000FF; // blue
		attr.antialias = true;
		pFontMPI_->SetFontAttr(attr);
		TS_ASSERT_EQUALS( pFontMPI_->GetFontColor(), attr.color );
		TS_ASSERT_EQUALS( pFontMPI_->GetFontAntiAliasing(), attr.antialias );
		pFontMPI_->DrawString(&text1, 0, 0, &surf);
		TS_ASSERT_DELTA( (rect1.right - rect1.left), pFontMPI_->GetX(), 1 );
		pFontMPI_->DrawString(&text2, 0, mtrx.height, &surf);
		TS_ASSERT_DELTA( (rect2.right - rect2.left), pFontMPI_->GetX(), 1 );
		
		font2 = pFontMPI_->LoadFont("DidjPropBold.ttf", prop2);
		TS_ASSERT( font2 != kInvalidFontHndl );
		pFontMPI_->GetFontMetrics(&mtrx);
		TS_ASSERT( mtrx.height != 0 );

		attr.color = 0x00FF0000; // red
		pFontMPI_->SetFontAttr(attr);
		TS_ASSERT_EQUALS( pFontMPI_->GetFontColor(), attr.color );
		pFontMPI_->DrawString(&text1, 0, 2*mtrx.height, &surf);
		pFontMPI_->DrawString(&text2, 0, 3*mtrx.height, &surf);
		
		attr.color = 0x0000FF00; // green
		pFontMPI_->SelectFont(font1);
		pFontMPI_->SetFontAttr(attr);
		TS_ASSERT_EQUALS( pFontMPI_->GetFontColor(), attr.color );
		pFontMPI_->DrawString(&text1, 0, 4*mtrx.height, &surf);
		pFontMPI_->DrawString(&text2, 0, 5*mtrx.height, &surf);
		
		pDisplayMPI_->Invalidate(0, NULL);
		sleep(1);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		pFontMPI_->UnloadFont(font1);
		pFontMPI_->UnloadFont(font2);
		delete pDisplayMPI_;
	}

	//------------------------------------------------------------------------
	void testFontMetrics()
	{
		PRINT_TEST_NAME();
		
		tFontHndl	font1;
		tFontHndl	font2;
		tFontSurf	surf;
		tFontMetrics	mtrx;
		tDisplayHandle 	disp;
		CString			text[9] = {CString("The "),CString("Quick "),CString("Brown "),
							CString("Fox "),CString("Jumps "),CString("Over "), 
							CString("the "),CString("Lazy "),CString("Dog.\n")};

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
		
		pFontMPI_->SetFontResourcePath(GetTestRsrcFolder());
		font1 = pFontMPI_->LoadFont("DidjPropBold.ttf", 24);
		TS_ASSERT( font1 != kInvalidFontHndl );
		pFontMPI_->GetFontMetrics(&mtrx);
		TS_ASSERT( mtrx.height != 0 );
		pFontMPI_->SetFontColor(0x0000FFFF); // cyan
		
		S32 x = 0;
		S32 y = 0;
		for (int n = 0; n < 9; n++)
		{
			tRect* prect = pFontMPI_->GetStringRect(text[n]);
			if (x + prect->right > surf.width)
			{
				x = 0;
				y += mtrx.height;
			}
			pFontMPI_->DrawString(text[n], x, y, surf);
			TS_ASSERT_EQUALS(x, pFontMPI_->GetX());
			TS_ASSERT_EQUALS(y, pFontMPI_->GetY());
		}
		
		font2 = pFontMPI_->LoadFont("DidjPropBold.ttf", 36);
		TS_ASSERT( font2 != kInvalidFontHndl );
		pFontMPI_->GetFontMetrics(&mtrx);
		TS_ASSERT( mtrx.height != 0 );
		pFontMPI_->SetFontColor(0x00FF00FF); // magenta

		for (int n = 0; n < 9; n++)
		{
			tRect* prect = pFontMPI_->GetStringRect(text[n]);
			if (x + prect->right > surf.width)
			{
				x = 0;
				y += mtrx.height;
			}
			pFontMPI_->DrawString(text[n], x, y, surf);
			TS_ASSERT_EQUALS(x, pFontMPI_->GetX());
			TS_ASSERT_EQUALS(y, pFontMPI_->GetY());
		}
		
		pDisplayMPI_->Invalidate(0, NULL);
		sleep(1);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		pFontMPI_->UnloadFont(font1);
		pFontMPI_->UnloadFont(font2);
		delete pDisplayMPI_;
	}
	
	//------------------------------------------------------------------------
	void testFontTextWrapping()
	{
		PRINT_TEST_NAME();
		
		tFontHndl	font1;
		tFontHndl	font2;
		tFontSurf	surf;
		tDisplayHandle 	disp;
		CString			text = CString("The Quick Brown Fox Jumps Over the Lazy Dog.\n");

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
		
		pFontMPI_->SetFontResourcePath(GetTestRsrcFolder());
		font1 = pFontMPI_->LoadFont("DidjPropBold.ttf", 24);
		TS_ASSERT( font1 != kInvalidFontHndl );
		pFontMPI_->SetFontColor(0x00FFFF00); // yellow
		
		S32 x = 0;
		S32 y = 0;
		pFontMPI_->DrawString(text, x, y, surf, true);
		TS_ASSERT_EQUALS(x, pFontMPI_->GetX());
		TS_ASSERT_EQUALS(y, pFontMPI_->GetY());
		
		font2 = pFontMPI_->LoadFont("DidjPropBold.ttf", 36);
		TS_ASSERT( font2 != kInvalidFontHndl );
		pFontMPI_->SetFontColor(0x0000FFFF); // cyan

		pFontMPI_->DrawString(text, x, y, surf, true);
		TS_ASSERT_EQUALS(x, pFontMPI_->GetX());
		TS_ASSERT_EQUALS(y, pFontMPI_->GetY());
		
		pDisplayMPI_->Invalidate(0, NULL);
		sleep(1);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		pFontMPI_->UnloadFont(font1);
		pFontMPI_->UnloadFont(font2);
		delete pDisplayMPI_;
	}
	
	//------------------------------------------------------------------------
	void testFontClipping()
	{
		PRINT_TEST_NAME();
		
		tFontHndl	font1;
		tFontHndl	font2;
		tFontSurf	surf;
		tDisplayHandle 	disp;
		CString			text = CString("Clipped String Test");
		U8*				buffer = new U8[320 * 240 * 4];

		pDisplayMPI_ = new CDisplayMPI;
		disp = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatARGB8888, buffer);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, 0, 0);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);
		memset(surf.buffer, 0, surf.height * surf.pitch);
		
		pFontMPI_->SetFontResourcePath(GetTestRsrcFolder());
		font1 = pFontMPI_->LoadFont("DidjPropBold.ttf", 24);
		TS_ASSERT( font1 != kInvalidFontHndl );
		pFontMPI_->SetFontColor(0x00FFFFFF); // white
		
		for (int x = 0; x < 320; x++)
		{
			memset(surf.buffer, 0, surf.height * surf.pitch);
			pFontMPI_->DrawString(&text, x, 0, &surf);
			pDisplayMPI_->Invalidate(0, NULL);
		}
		
		for (int x = 320; x > -320; x--)
		{
			memset(surf.buffer, 0, surf.height * surf.pitch);
			pFontMPI_->DrawString(&text, x, 0, &surf);
			pDisplayMPI_->Invalidate(0, NULL);
		}
		
		font2 = pFontMPI_->LoadFont("DidjPropBold.ttf", 36);
		TS_ASSERT( font2 != kInvalidFontHndl );
		
		for (int y = 0; y < 240; y++)
		{
			memset(surf.buffer, 0, surf.height * surf.pitch);
			pFontMPI_->DrawString(&text, 0, y, &surf);
			pDisplayMPI_->Invalidate(0, NULL);
		}
		
		for (int y = 240; y > -240; y--)
		{
			memset(surf.buffer, 0, surf.height * surf.pitch);
			pFontMPI_->DrawString(&text, 0, y, &surf);
			pDisplayMPI_->Invalidate(0, NULL);
		}
		
		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		pFontMPI_->UnloadFont(font1);
		pFontMPI_->UnloadFont(font2);
		delete pDisplayMPI_;
		delete buffer;
	}
	
	//------------------------------------------------------------------------
	void testFontKerning()
	{
		PRINT_TEST_NAME();
		
		tFontHndl	font1;
		tFontHndl	font2;
		tFontSurf	surf;
		tFontMetrics	*pmtrx;
		tDisplayHandle 	disp;
		CString			text = CString("WAVE overall");
		S32				dy,y = 0;
		
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
		
		pFontMPI_->SetFontResourcePath(GetTestRsrcFolder());
		font1 = pFontMPI_->LoadFont("DidjPropBold.ttf", 36);
		TS_ASSERT( font1 != kInvalidFontHndl );
		pmtrx = pFontMPI_->GetFontMetrics();
		TS_ASSERT( pmtrx != kNull );
		dy = pmtrx->height;
		
		pFontMPI_->SetFontColor(0xFFFFFFFF);
		pFontMPI_->SetFontAntiAliasing(true);
		pFontMPI_->SetFontKerning(false);
		TS_ASSERT( pFontMPI_->GetFontKerning() == false );
		pFontMPI_->DrawString(&text, 0, y, &surf); y+=dy;
		pFontMPI_->SetFontKerning(true);
		TS_ASSERT( pFontMPI_->GetFontKerning() == true );
		pFontMPI_->DrawString(&text, 0, y, &surf); y+=dy;
		
		font2 = pFontMPI_->LoadFont("DidjPropBold.ttf", 48);
		TS_ASSERT( font2 != kInvalidFontHndl );
		pmtrx = pFontMPI_->GetFontMetrics();
		TS_ASSERT( pmtrx != kNull );
		dy = pmtrx->height;
		
		pFontMPI_->SetFontKerning(false);
		TS_ASSERT( pFontMPI_->GetFontKerning() == false );
		pFontMPI_->DrawString(&text, 0, y, &surf); y+=dy;
		pFontMPI_->SetFontKerning(true);
		TS_ASSERT( pFontMPI_->GetFontKerning() == true );
		pFontMPI_->DrawString(&text, 0, y, &surf); y+=dy;
		pDisplayMPI_->Invalidate(0, NULL);
		sleep(1);
		
		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		pFontMPI_->UnloadFont(font1);
		pFontMPI_->UnloadFont(font2);
		delete pDisplayMPI_;
	}
	
	//------------------------------------------------------------------------
	void testFontUnderlining()
	{
		PRINT_TEST_NAME();
		
		tFontHndl	font1;
		tFontHndl	font2;
		tFontSurf	surf;
		tFontMetrics	*pmtrx;
		tDisplayHandle 	disp;
		CString			text = CString("Underline yes/no question.");
		S32				dy,y = 0;
		
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
		
		pFontMPI_->SetFontResourcePath(GetTestRsrcFolder());
		font1 = pFontMPI_->LoadFont("DidjPropBold.ttf", 24);
		TS_ASSERT( font1 != kInvalidFontHndl );
		pmtrx = pFontMPI_->GetFontMetrics();
		TS_ASSERT( pmtrx != kNull );
		dy = pmtrx->height;
		
		pFontMPI_->SetFontColor(0xFFFFFFFF);
		pFontMPI_->SetFontUnderlining(true);
		TS_ASSERT( pFontMPI_->GetFontUnderlining() == true );
		pFontMPI_->DrawString(&text, 0, y, &surf); y+=dy;
		pFontMPI_->DrawString(&text, 0, y, &surf); y+=dy;
		pFontMPI_->SetFontUnderlining(false);
		TS_ASSERT( pFontMPI_->GetFontUnderlining() == false );
		pFontMPI_->DrawString(&text, 0, y, &surf); y+=dy;
		
		font2 = pFontMPI_->LoadFont("DidjPropBold.ttf", 36);
		TS_ASSERT( font2 != kInvalidFontHndl );
		pmtrx = pFontMPI_->GetFontMetrics();
		TS_ASSERT( pmtrx != kNull );
		dy = pmtrx->height;
		
		pFontMPI_->SetFontUnderlining(true);
		TS_ASSERT( pFontMPI_->GetFontUnderlining() == true );
		pFontMPI_->DrawString(&text, 0, y, &surf); y+=dy;
		pFontMPI_->DrawString(&text, 0, y, &surf); y+=dy;
		pFontMPI_->SetFontUnderlining(false);
		TS_ASSERT( pFontMPI_->GetFontUnderlining() == false );
		pFontMPI_->DrawString(&text, 0, y, &surf); y+=dy;
		pDisplayMPI_->Invalidate(0, NULL);
		sleep(1);
		
		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		pFontMPI_->UnloadFont(font1);
		pFontMPI_->UnloadFont(font2);
		delete pDisplayMPI_;
	}
	
	//------------------------------------------------------------------------
	void XXXXtestFontUnicode()	// FIXME/dm: find a font which supports these glyphs
	{
		PRINT_TEST_NAME();
		
		tFontHndl	font1;
		tFontHndl	font2;
		tFontSurf	surf;
		tFontMetrics	mtrx;
		tDisplayHandle 	disp;
		S32				x,y,dy;
		
		// UTF8 text copied from email
		CString text1 = CString("Primary Test를 통해 현재");
		CString text2 = CString("LF1000의 Major한 Feature들(3D 제외)");
		CString text3 = CString("* Audio : 진행중 - PCM In/Out 확인.");
		CString text4 = CString("* 3D Engine : 진행중");

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
		
		// Extra glyphs not supported by FreeSans/Serif or DejaVuSans/Serif
		pFontMPI_->SetFontResourcePath(GetTestRsrcFolder());
		font1 = pFontMPI_->LoadFont("FreeSans.ttf", 24, kUTF8CharEncoding);
		TS_ASSERT( font1 != kInvalidFontHndl );
		pFontMPI_->GetFontMetrics(&mtrx);
		TS_ASSERT( mtrx.height != 0 );
		pFontMPI_->SetFontColor(0x00FFFFFF); // white
		
		x = y = 0; dy = mtrx.height;
		pFontMPI_->DrawString(&text1, 0, y, &surf); y+=dy;
		pFontMPI_->DrawString(&text2, 0, y, &surf); y+=dy;
		pFontMPI_->DrawString(&text3, 0, y, &surf); y+=dy;
		pFontMPI_->DrawString(&text4, 0, y, &surf); y+=dy;
		
		font2 = pFontMPI_->LoadFont("FreeSerif.ttf", 24, kUTF8CharEncoding);
		TS_ASSERT( font2 != kInvalidFontHndl );
		
		pFontMPI_->DrawString(&text1, 0, y, &surf); y+=dy;
		pFontMPI_->DrawString(&text2, 0, y, &surf); y+=dy;
		pFontMPI_->DrawString(&text3, 0, y, &surf); y+=dy;
		pFontMPI_->DrawString(&text4, 0, y, &surf); y+=dy;
		
		pDisplayMPI_->Invalidate(0, NULL);
		sleep(1);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		pFontMPI_->UnloadFont(font1);
		pFontMPI_->UnloadFont(font2);
		delete pDisplayMPI_;
	}
	
	//------------------------------------------------------------------------
	void testFontUnicodeUTF8()
	{
		PRINT_TEST_NAME();
		
		tFontHndl	font1;
		tFontHndl	font2;
		tFontSurf	surf;
		tFontMetrics	mtrx;
		tDisplayHandle 	disp;
		S32				x,y,dy;
		
		// UTF8 text copied from http://www.columbia.edu/~kermit/utf8.html
		CString text1 = CString("Spanish: El pingüino Wenceslao hizo kilómetros bajo exhaustiva lluvia y frío, añoraba a su querido cachorro. ");
		CString text2 = CString("French: Les naïfs ægithales hâtifs pondant à Noël où il gèle sont sûrs d'être déçus et de voir leurs drôles d'œufs abîmés. ");
		CString text3 = CString("German: Falsches Üben von Xylophonmusik quält jeden größeren Zwerg. ");
		CString text4 = CString("Swedish: Flygande bäckasiner söka strax hwila på mjuka tuvor.\n");

		pDisplayMPI_ = new CDisplayMPI;
		disp = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatARGB8888, NULL);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, 0, 0);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);
		memset(surf.buffer, 0, surf.height * surf.pitch);
		
		pFontMPI_->SetFontResourcePath(GetTestRsrcFolder());
		font1 = pFontMPI_->LoadFont("FreeSans.ttf", 18, kUTF8CharEncoding);
		TS_ASSERT( font1 != kInvalidFontHndl );
		pFontMPI_->GetFontMetrics(&mtrx);
		TS_ASSERT( mtrx.height != 0 );
		pFontMPI_->SetFontColor(0xFFFFFFFF); // white
		
		x = y = 0; dy = mtrx.height;
		pFontMPI_->DrawString(text1, x, y, surf, true);
		pFontMPI_->DrawString(text2, x, y, surf, true);
		pFontMPI_->DrawString(text3, x, y, surf, true);
		pFontMPI_->DrawString(text4, x, y, surf, true);
		
		font2 = pFontMPI_->LoadFont("FreeSerif.ttf", 18, kUTF8CharEncoding);
		TS_ASSERT( font2 != kInvalidFontHndl );
		
		pFontMPI_->DrawString(text1, x, y, surf, true);
		pFontMPI_->DrawString(text2, x, y, surf, true);
		pFontMPI_->DrawString(text3, x, y, surf, true);
		pFontMPI_->DrawString(text4, x, y, surf, true);
		
		pDisplayMPI_->Invalidate(0, NULL);
		sleep(1);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		pFontMPI_->UnloadFont(font1);
		pFontMPI_->UnloadFont(font2);
		delete pDisplayMPI_;
	}
	
	//------------------------------------------------------------------------
	void testFontUnicode16()
	{
		PRINT_TEST_NAME();
		
		tFontHndl	font1;
		tFontHndl	font2;
		tFontSurf	surf;
		tFontMetrics	mtrx;
		tDisplayHandle 	disp;
		S32				x,y,dy;
		const gunichar	kLatinExt = 0x0100;
		const gunichar	kGreek = 0x03B1;
		const gunichar	kCyrillic = 0x0400;
//		const gunichar	kHebrew = 0x05D0;
//		const gunichar	kArabic = 0x0621;
//		const gunichar 	kBengali = 0x0994;
		const gunichar	kKatakana = 0x30A1;
//		const gunichar 	kChineseUni =0x4E00;
		
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
		
		pFontMPI_->SetFontResourcePath(GetTestRsrcFolder());
		font1 = pFontMPI_->LoadFont("FreeSans.ttf", 24, kUTF16CharEncoding);
		TS_ASSERT( font1 != kInvalidFontHndl );
		pFontMPI_->GetFontMetrics(&mtrx);
		TS_ASSERT( mtrx.height != 0 );
		pFontMPI_->SetFontColor(0x00FFFFFF); // white
		
		x = y = 0; dy = mtrx.height;
		for (gunichar c = kLatinExt; c<kLatinExt+0x10; c++)
		{	
			CString code = CString(1,c);
			pFontMPI_->DrawString(code, x, y, surf);
		}
		x = 0; y += dy;
		for (gunichar c = kGreek; c<kGreek+0x10; c++)
		{	
			CString code = CString(1,c);
			pFontMPI_->DrawString(code, x, y, surf);
		}
		x = 0; y += dy;
		for (gunichar c = kCyrillic; c<kCyrillic+0x10; c++)
		{	
			CString code = CString(1,c);
			pFontMPI_->DrawString(code, x, y, surf);
		}
		x = 0; y += dy;
		for (gunichar c = kKatakana; c<kKatakana+0x10; c++)
		{	
			CString code = CString(1,c);
			pFontMPI_->DrawString(code, x, y, surf);
		}
		
		font2 = pFontMPI_->LoadFont("FreeSerif.ttf", 24, kUTF16CharEncoding);
		TS_ASSERT( font2 != kInvalidFontHndl );
		pFontMPI_->GetFontMetrics(&mtrx);
		TS_ASSERT( mtrx.height != 0 );
		dy = mtrx.height;
		
		x = 0; y += dy;
		for (gunichar c = kLatinExt; c<kLatinExt+0x10; c++)
		{	
			CString code = CString(1,c);
			pFontMPI_->DrawString(code, x, y, surf);
		}
		x = 0; y += dy;
		for (gunichar c = kGreek; c<kGreek+0x10; c++)
		{	
			CString code = CString(1,c);
			pFontMPI_->DrawString(code, x, y, surf);
		}
		x = 0; y += dy;
		for (gunichar c = kCyrillic; c<kCyrillic+0x10; c++)
		{	
			CString code = CString(1,c);
			pFontMPI_->DrawString(code, x, y, surf);
		}
		x = 0; y += dy;
		for (gunichar c = kKatakana; c<kKatakana+0x10; c++)
		{	
			CString code = CString(1,c);
			pFontMPI_->DrawString(code, x, y, surf);
		}
		
		pDisplayMPI_->Invalidate(0, NULL);
		sleep(1);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		pFontMPI_->UnloadFont(font1);
		pFontMPI_->UnloadFont(font2);
		delete pDisplayMPI_;
	}
	
	//------------------------------------------------------------------------
	void testFontMono()
	{
		PRINT_TEST_NAME();
		
		tFontHndl	font1;
		tFontSurf	surf;
		tFontMetrics	mtrx;
		tDisplayHandle 	disp;
		S32				x,y,dx,dy;
		U8*				buffer = new U8[(16*8) * (6*16) * 3];
		
		pDisplayMPI_ = new CDisplayMPI;
		disp = pDisplayMPI_->CreateHandle(6*16, 16*8, kPixelFormatRGB888, buffer);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, (320-16*8)/2, (240-6*16)/2, 0, 0);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);
		memset(surf.buffer, 0, surf.height * surf.pitch);
		
		pFontMPI_->SetFontResourcePath(GetTestRsrcFolder());
		font1 = pFontMPI_->LoadFont("FreeMono.ttf", 14);
		TS_ASSERT( font1 != kInvalidFontHndl );
		pFontMPI_->GetFontMetrics(&mtrx);
		TS_ASSERT( mtrx.height != 0 );
		pFontMPI_->SetFontColor(0xFFFFFFFF); // white
		
		x = y = 0; 
		dx = mtrx.advance;
		dy = mtrx.height;
		for (gunichar c = 0x20; c < 0x80; c++)
		{	
			CString code = CString(1,c);
			pFontMPI_->DrawString(code, x, y, surf);
//			x += dx;
			if (c % 0x10 == 0x0F) 
			{
				x = 0;
				y += dy;
			}
		}
		pDisplayMPI_->Invalidate(0, NULL);
		
		sleep(1);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		pFontMPI_->UnloadFont(font1);
		delete pDisplayMPI_;
		delete buffer;
	}
	
	//------------------------------------------------------------------------
	void testFontOpenGL()
	{
		PRINT_TEST_NAME();
		
		tFontHndl	font1;
		tFontProp	prop1 = {1, 24, 0, 0};
		tFontAttr	attr;
		tFontSurf	surf;
		tFontMetrics mtrx;
		CString		text1 = CString("Red");
		CString		text2 = CString("Green");
		CString		text3 = CString("Blue");
		CString		text4 = CString("ARGB8888");
		
		BrioOpenGLConfig* ctx = new BrioOpenGLConfig();

		pFontMPI_->SetFontResourcePath(GetTestRsrcFolder());
		font1 = pFontMPI_->LoadFont("DidjPropBold.ttf", prop1);
		TS_ASSERT( font1 != kInvalidFontHndl );
		pFontMPI_->GetFontMetrics(&mtrx);
		
		// Create local buffer for texture
		surf.width = surf.height = 128;
		surf.pitch = surf.width * 4;
		surf.format = kPixelFormatARGB8888;
		surf.buffer = static_cast<U8*>(malloc(surf.pitch * surf.height));
		TS_ASSERT( surf.buffer != NULL );
		memset(surf.buffer, 0, surf.pitch * surf.height);
		
		// Draw text to buffer
		attr.version = 1;
		attr.color = 0xFF0000FF; // GL red
		attr.antialias = true;
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text1, 0, 0, &surf);
		attr.color = 0xFF00FF00; // GL green
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text2, 0, mtrx.height, &surf);
		attr.color = 0xFFFF0000; // GL blue
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text3, 0, 2*mtrx.height, &surf);
		attr.color = 0xFFFFFFFF; // GL white
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text4, 0, 3*mtrx.height, &surf);
		
		GLuint 		texture;
		GLshort 	quad[] = { -1,-1,0,  1,-1,0,  1,1,0,  -1,1,0 }; 
		GLshort 	texmap[] = { 0,1,  1,1,  1,0,  0,0 };  
		// Download buffer as texture and render textured quad
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf.width, surf.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf.buffer);
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(3, GL_SHORT, 0, quad);
		glTexCoordPointer(2, GL_SHORT, 0, texmap);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		eglSwapBuffers(ctx->eglDisplay, ctx->eglSurface);

		sleep(1);

		glDeleteTextures(1, &texture);
		free(surf.buffer);
		pFontMPI_->UnloadFont(font1);
		delete ctx;
	}

	//------------------------------------------------------------------------
	void testFontOpenGL24bpp()
	{
		PRINT_TEST_NAME();
		
		tFontHndl	font1;
		tFontProp	prop1 = {1, 24, 0, 0};
		tFontSurf	surf;
		tFontMetrics mtrx;
		CString		text1 = CString("Red");
		CString		text2 = CString("Green");
		CString		text3 = CString("Blue");
		CString		text4 = CString("RGB888");

		BrioOpenGLConfig* ctx = new BrioOpenGLConfig();

		pFontMPI_->SetFontResourcePath(GetTestRsrcFolder());
		font1 = pFontMPI_->LoadFont("DidjPropBold.ttf", prop1);
		TS_ASSERT( font1 != kInvalidFontHndl );
		pFontMPI_->GetFontMetrics(&mtrx);
		
		// Create local buffer for texture = 24bpp RGB
		surf.width = surf.height = 128;
		surf.pitch = surf.width * 3;
		surf.format = kPixelFormatRGB888;
		surf.buffer = static_cast<U8*>(malloc(surf.pitch * surf.height));
		TS_ASSERT( surf.buffer != NULL );
		memset(surf.buffer, 0, surf.pitch * surf.height);
		
		// Draw text to buffer
		pFontMPI_->SetFontColor(0x0000FF); // GL red
		pFontMPI_->DrawString(&text1, 0, 0, &surf);
		pFontMPI_->SetFontColor(0x00FF00); // GL green
		pFontMPI_->DrawString(&text2, 0, mtrx.height, &surf);
		pFontMPI_->SetFontColor(0xFF0000); // GL blue
		pFontMPI_->DrawString(&text3, 0, 2*mtrx.height, &surf);
		pFontMPI_->SetFontColor(0xFFFFFF); // GL white
		pFontMPI_->DrawString(&text4, 0, 3*mtrx.height, &surf);
		
		GLuint 		texture;
		GLshort 	quad[] = { -1,-1,0,  1,-1,0,  1,1,0,  -1,1,0 }; 
		GLshort 	texmap[] = { 0,1,  1,1,  1,0,  0,0 };  
		// Download buffer as texture and render textured quad
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surf.width, surf.height, 0, GL_RGB, GL_UNSIGNED_BYTE, surf.buffer);
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(3, GL_SHORT, 0, quad);
		glTexCoordPointer(2, GL_SHORT, 0, texmap);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		eglSwapBuffers(ctx->eglDisplay, ctx->eglSurface);

		sleep(1);

		glDeleteTextures(1, &texture);
		free(surf.buffer);
		pFontMPI_->UnloadFont(font1);
		delete ctx;
	}

	//------------------------------------------------------------------------
	void testFontOpenGL16bppARGB()
	{
		PRINT_TEST_NAME();
		
		tFontHndl	font1;
		tFontSurf	surf;
		tFontMetrics mtrx;
		CString		text1 = CString("Red");
		CString		text2 = CString("Green");
		CString		text3 = CString("Blue");
		CString		text4 = CString("ARGB4444");
		
		BrioOpenGLConfig* ctx = new BrioOpenGLConfig();

		pFontMPI_->SetFontResourcePath(GetTestRsrcFolder());
		font1 = pFontMPI_->LoadFont("DidjPropBold.ttf", 24);
		TS_ASSERT( font1 != kInvalidFontHndl );
		pFontMPI_->GetFontMetrics(&mtrx);
		
		// Create local buffer for texture = 16bpp ARGB4444 format
		surf.width = surf.height = 128;
		surf.pitch = surf.width * 2;
		surf.format = kPixelFormatRGB4444;
		surf.buffer = static_cast<U8*>(malloc(surf.pitch * surf.height));
		TS_ASSERT( surf.buffer != NULL );
		memset(surf.buffer, 0, surf.pitch * surf.height);
		
		// Draw text to buffer
		pFontMPI_->SetFontColor(0xF00F); // 16bpp packed GL red
		pFontMPI_->DrawString(&text1, 0, 0, &surf);
		pFontMPI_->SetFontColor(0x0F0F); // 16bpp packed GL green
		pFontMPI_->DrawString(&text2, 0, mtrx.height, &surf);
		pFontMPI_->SetFontColor(0x00FF); // 16bpp packed GL blue
		pFontMPI_->DrawString(&text3, 0, 2*mtrx.height, &surf);
		pFontMPI_->SetFontColor(0xFFFF); // 16bpp packed GL white
		pFontMPI_->DrawString(&text4, 0, 3*mtrx.height, &surf);
		
		GLuint 		texture;
		GLshort 	quad[] = { -1,-1,0,  1,-1,0,  1,1,0,  -1,1,0 }; 
		GLshort 	texmap[] = { 0,1,  1,1,  1,0,  0,0 };  
		// Download buffer as texture and render textured quad
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf.width, surf.height, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, surf.buffer);
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(3, GL_SHORT, 0, quad);
		glTexCoordPointer(2, GL_SHORT, 0, texmap);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		eglSwapBuffers(ctx->eglDisplay, ctx->eglSurface);

		sleep(1);

		glDeleteTextures(1, &texture);
		free(surf.buffer);
		pFontMPI_->UnloadFont(font1);
		delete ctx;
	}

	//------------------------------------------------------------------------
	void testFontOpenGL16bppRGB()
	{
		PRINT_TEST_NAME();
		
		tFontHndl	font1;
		tFontProp	prop1 = {1, 24, 0, 0};
		tFontAttr	attr;
		tFontSurf	surf;
		tFontMetrics mtrx;
		CString		text1 = CString("Red");
		CString		text2 = CString("Green");
		CString		text3 = CString("Blue");
		CString		text4 = CString("RGB565");
		
		BrioOpenGLConfig* ctx = new BrioOpenGLConfig();

		pFontMPI_->SetFontResourcePath(GetTestRsrcFolder());
		font1 = pFontMPI_->LoadFont("DidjPropBold.ttf", prop1);
		TS_ASSERT( font1 != kInvalidFontHndl );
		pFontMPI_->GetFontMetrics(&mtrx);
		
		// Create local buffer for texture = 16bpp RGB565 format
		surf.width = surf.height = 128;
		surf.pitch = surf.width * 2;
		surf.format = kPixelFormatRGB565;
		surf.buffer = static_cast<U8*>(malloc(surf.pitch * surf.height));
		TS_ASSERT( surf.buffer != NULL );
		memset(surf.buffer, 0, surf.pitch * surf.height);
		
		// Draw text to buffer
		attr.version = 1;
		attr.color = 0xF800; // 16bpp packed GL red
		attr.antialias = true;
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text1, 0, 0, &surf);
		attr.color = 0x07E0; // 16bpp packed GL green
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text2, 0, mtrx.height, &surf);
		attr.color = 0x001F; // 16bpp packed GL blue
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text3, 0, 2*mtrx.height, &surf);
		attr.color = 0xFFFF; // 16bpp packed GL white
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text4, 0, 3*mtrx.height, &surf);
		
		GLuint 		texture;
		GLshort 	quad[] = { -1,-1,0,  1,-1,0,  1,1,0,  -1,1,0 }; 
		GLshort 	texmap[] = { 0,1,  1,1,  1,0,  0,0 };  
		// Download buffer as texture and render textured quad
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surf.width, surf.height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, surf.buffer);
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(3, GL_SHORT, 0, quad);
		glTexCoordPointer(2, GL_SHORT, 0, texmap);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		eglSwapBuffers(ctx->eglDisplay, ctx->eglSurface);

		sleep(1);

		glDeleteTextures(1, &texture);
		free(surf.buffer);
		pFontMPI_->UnloadFont(font1);
		delete ctx;
	}

	//------------------------------------------------------------------------
	void XXXXtestFontLoadFlags()
	{
		PRINT_TEST_NAME();
		
		tFontHndl	font1;
		tFontHndl	font2;
		tFontHndl	font3;
		tFontHndl	font4;
		tFontProp	prop1 = {3, 24, 0, 0 /*, FT_LOAD_NO_HINTING | FT_LOAD_TARGET_NORMAL */ };
		tFontProp	prop2 = {3, 24, 0, 0 /*, FT_LOAD_NO_HINTING | FT_LOAD_TARGET_LIGHT */ };
		tFontProp	prop3 = {3, 24, 0, 0 /*, FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_NORMAL */ };
		tFontProp	prop4 = {3, 24, 0, 0 /*, FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT */ };
		tFontSurf	surf;
		CString		text1 = CString("The Quick Brown Fox\n");
		CString		text2 = CString("Jumps Over the Lazy Dog\n");
		S32			x,y;
		
		pDisplayMPI_ = new CDisplayMPI;
		tDisplayHandle disp = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatARGB8888, NULL);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, 0, 0);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);
		memset(surf.buffer, 0, surf.height * surf.pitch);
		
		CPath dir = GetTestRsrcFolder();
		pFontMPI_->SetFontResourcePath(dir);
		CPath* path = pFontMPI_->GetFontResourcePath();
		TS_ASSERT( dir == *path );
		
		font1 = pFontMPI_->LoadFont("DidjPropBold.ttf", prop1);
		TS_ASSERT( font1 != kInvalidFontHndl );
		pFontMPI_->SetFontColor(0xFFFFFFFF); // white
		pFontMPI_->DrawString(text1, x = 0, y = 0, surf);
		font2 = pFontMPI_->LoadFont("DidjPropBold.ttf", prop2);
		TS_ASSERT( font2 != kInvalidFontHndl );
		pFontMPI_->DrawString(text1, x = 0, y, surf);
		font3 = pFontMPI_->LoadFont("DidjPropBold.ttf", prop3);
		TS_ASSERT( font3 != kInvalidFontHndl );
		pFontMPI_->DrawString(text1, x = 0, y, surf);
		font4 = pFontMPI_->LoadFont("DidjPropBold.ttf", prop4);
		TS_ASSERT( font4 != kInvalidFontHndl );
		pFontMPI_->DrawString(text1, x = 0, y, surf);

		sleep(1);

		pFontMPI_->UnloadFont(font1);
		pFontMPI_->UnloadFont(font2);
		pFontMPI_->UnloadFont(font3);
		pFontMPI_->UnloadFont(font4);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		delete pDisplayMPI_;

	}

};

// EOF
