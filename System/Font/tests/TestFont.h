// TestFontMPI.h

#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <FontMPI.h>
#include <ResourceMPI.h>
#include <DisplayMPI.h>
#include <BrioOpenGLConfig.h>
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
		pFontMPI_->DrawString(&text1, 0, 0, &surf);
		TS_ASSERT( static_cast<unsigned>(rect1.right - rect1.left) == pFontMPI_->GetX() );
		pFontMPI_->DrawString(&text2, 0, mtrx.height, &surf);
//		TS_ASSERT( static_cast<unsigned>(rect2.right - rect2.left) == pFontMPI_->GetX() );
		
		font2 = pFontMPI_->LoadFont(handle2, prop2);
		TS_ASSERT( font2 != kInvalidFontHndl );
		pFontMPI_->GetFontMetrics(&mtrx);
		TS_ASSERT( mtrx.height != 0 );

		attr.color = 0x00FF0000; // red
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text1, 0, 2*mtrx.height, &surf);
		pFontMPI_->DrawString(&text2, 0, 3*mtrx.height, &surf);
		
		attr.color = 0x0000FF00; // green
		pFontMPI_->SelectFont(font1);
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text1, 0, 4*mtrx.height, &surf);
		pFontMPI_->DrawString(&text2, 0, 5*mtrx.height, &surf);
		
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

	//------------------------------------------------------------------------
	void testFontOpenGL()
	{
		tRsrcHndl	pkg;
		tRsrcHndl	handle1;
		tFontHndl	font1;
		tFontProp	prop1 = {1, 24, 0, 0};
		tFontAttr	attr;
		tFontSurf	surf;
		tFontMetrics mtrx;
		CString		text1 = CString("Red");
		CString		text2 = CString("Green");
		CString		text3 = CString("Blue");
		CString		text4 = CString("ARGB8888");
		GLuint 		texture;
		GLshort 	quad[] = { -1,-1,0,  1,-1,0,  1,1,0,  -1,1,0 }; 
		GLshort 	texmap[] = { 0,1,  1,1,  1,0,  0,0 };  
		
		BrioOpenGLConfig* ctx = new BrioOpenGLConfig();

		pResourceMPI_ = new CResourceMPI;
		pResourceMPI_->OpenAllDevices();
		pResourceMPI_->SetDefaultURIPath("LF/Brio/UnitTest/Font");
		pkg = pResourceMPI_->FindPackage("FontTest");
		TS_ASSERT( pkg != kInvalidPackageHndl );
		pResourceMPI_->OpenPackage(pkg);
		handle1 = pResourceMPI_->FindRsrc("Verdana");
		TS_ASSERT( handle1 != kInvalidRsrcHndl );
		font1 = pFontMPI_->LoadFont(handle1, prop1);
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
		pResourceMPI_->ClosePackage(pkg);
		pResourceMPI_->CloseAllDevices();
		delete pResourceMPI_;
		delete ctx;
	}

	//------------------------------------------------------------------------
	void testFontOpenGL24bpp()
	{
		tRsrcHndl	pkg;
		tRsrcHndl	handle1;
		tFontHndl	font1;
		tFontProp	prop1 = {1, 24, 0, 0};
		tFontAttr	attr;
		tFontSurf	surf;
		tFontMetrics mtrx;
		CString		text1 = CString("Red");
		CString		text2 = CString("Green");
		CString		text3 = CString("Blue");
		CString		text4 = CString("RGB888");
		GLuint 		texture;
		GLshort 	quad[] = { -1,-1,0,  1,-1,0,  1,1,0,  -1,1,0 }; 
		GLshort 	texmap[] = { 0,1,  1,1,  1,0,  0,0 };  

		BrioOpenGLConfig* ctx = new BrioOpenGLConfig();

		pResourceMPI_ = new CResourceMPI;
		pResourceMPI_->OpenAllDevices();
		pResourceMPI_->SetDefaultURIPath("LF/Brio/UnitTest/Font");
		pkg = pResourceMPI_->FindPackage("FontTest");
		TS_ASSERT( pkg != kInvalidPackageHndl );
		pResourceMPI_->OpenPackage(pkg);
		handle1 = pResourceMPI_->FindRsrc("Verdana");
		TS_ASSERT( handle1 != kInvalidRsrcHndl );
		font1 = pFontMPI_->LoadFont(handle1, prop1);
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
		attr.version = 1;
		attr.color = 0x0000FF; // GL red
		attr.antialias = true;
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text1, 0, 0, &surf);
		attr.color = 0x00FF00; // GL green
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text2, 0, mtrx.height, &surf);
		attr.color = 0xFF0000; // GL blue
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text3, 0, 2*mtrx.height, &surf);
		attr.color = 0xFFFFFF; // GL white
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text4, 0, 3*mtrx.height, &surf);
		
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
		pResourceMPI_->ClosePackage(pkg);
		pResourceMPI_->CloseAllDevices();
		delete pResourceMPI_;
		delete ctx;
	}

	//------------------------------------------------------------------------
	void testFontOpenGL16bppARGB()
	{
		tRsrcHndl	pkg;
		tRsrcHndl	handle1;
		tFontHndl	font1;
		tFontProp	prop1 = {1, 24, 0, 0};
		tFontAttr	attr;
		tFontSurf	surf;
		tFontMetrics mtrx;
		CString		text1 = CString("Red");
		CString		text2 = CString("Green");
		CString		text3 = CString("Blue");
		CString		text4 = CString("ARGB4444");
		GLuint 		texture;
		GLshort 	quad[] = { -1,-1,0,  1,-1,0,  1,1,0,  -1,1,0 }; 
		GLshort 	texmap[] = { 0,1,  1,1,  1,0,  0,0 };  
		
		BrioOpenGLConfig* ctx = new BrioOpenGLConfig();

		pResourceMPI_ = new CResourceMPI;
		pResourceMPI_->OpenAllDevices();
		pResourceMPI_->SetDefaultURIPath("LF/Brio/UnitTest/Font");
		pkg = pResourceMPI_->FindPackage("FontTest");
		TS_ASSERT( pkg != kInvalidPackageHndl );
		pResourceMPI_->OpenPackage(pkg);
		handle1 = pResourceMPI_->FindRsrc("Verdana");
		TS_ASSERT( handle1 != kInvalidRsrcHndl );
		font1 = pFontMPI_->LoadFont(handle1, prop1);
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
		attr.version = 1;
		attr.color = 0xF00F; // 16bpp packed GL red 
		attr.antialias = true;
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text1, 0, 0, &surf);
		attr.color = 0x0F0F; // 16bpp packed GL green
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text2, 0, mtrx.height, &surf);
		attr.color = 0x00FF; // 16bpp packed GL blue
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text3, 0, 2*mtrx.height, &surf);
		attr.color = 0xFFFF; // 16bpp packed GL white
		pFontMPI_->SetFontAttr(attr);
		pFontMPI_->DrawString(&text4, 0, 3*mtrx.height, &surf);
		
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
		pResourceMPI_->ClosePackage(pkg);
		pResourceMPI_->CloseAllDevices();
		delete pResourceMPI_;
		delete ctx;
	}

	//------------------------------------------------------------------------
	void testFontOpenGL16bppRGB()
	{
		tRsrcHndl	pkg;
		tRsrcHndl	handle1;
		tFontHndl	font1;
		tFontProp	prop1 = {1, 24, 0, 0};
		tFontAttr	attr;
		tFontSurf	surf;
		tFontMetrics mtrx;
		CString		text1 = CString("Red");
		CString		text2 = CString("Green");
		CString		text3 = CString("Blue");
		CString		text4 = CString("RGB565");
		GLuint 		texture;
		GLshort 	quad[] = { -1,-1,0,  1,-1,0,  1,1,0,  -1,1,0 }; 
		GLshort 	texmap[] = { 0,1,  1,1,  1,0,  0,0 };  
		
		BrioOpenGLConfig* ctx = new BrioOpenGLConfig();

		pResourceMPI_ = new CResourceMPI;
		pResourceMPI_->OpenAllDevices();
		pResourceMPI_->SetDefaultURIPath("LF/Brio/UnitTest/Font");
		pkg = pResourceMPI_->FindPackage("FontTest");
		TS_ASSERT( pkg != kInvalidPackageHndl );
		pResourceMPI_->OpenPackage(pkg);
		handle1 = pResourceMPI_->FindRsrc("Verdana");
		TS_ASSERT( handle1 != kInvalidRsrcHndl );
		font1 = pFontMPI_->LoadFont(handle1, prop1);
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
		pResourceMPI_->ClosePackage(pkg);
		pResourceMPI_->CloseAllDevices();
		delete pResourceMPI_;
		delete ctx;
	}

};

// EOF
