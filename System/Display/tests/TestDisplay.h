// TestDisplayMPI.h

#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <DisplayMPI.h>
#include <KernelMPI.h>
#include <UnitTestUtils.h>
#include <BrioOpenGLConfig.h>

LF_USING_BRIO_NAMESPACE()

//============================================================================
// TestDisplayMPI functions
//============================================================================
class TestDisplay : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CDisplayMPI*	pDisplayMPI_;
public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		pDisplayMPI_ = new CDisplayMPI;
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete pDisplayMPI_; 
	}

	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		TS_ASSERT( pDisplayMPI_ != NULL );
		TS_ASSERT( pDisplayMPI_->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;
		
		if ( pDisplayMPI_->IsValid() ) {
			pName = pDisplayMPI_->GetMPIName();
			TS_ASSERT_EQUALS( *pName, "DisplayMPI" );
			version = pDisplayMPI_->GetModuleVersion();
			TS_ASSERT_EQUALS( version, 2 );
			pName = pDisplayMPI_->GetModuleName();
			TS_ASSERT_EQUALS( *pName, "Display" );
			pURI = pDisplayMPI_->GetModuleOrigin();
			TS_ASSERT_EQUALS( *pURI, "/LF/System/Display" );
		}
	}

	//------------------------------------------------------------------------
	void testDisplayContext( )
	{
		tDisplayHandle 	handle;
		tPixelFormat	format;
		U8* 			buffer;
		U16				width;
		U16				height;
		U16				pitch;
		U16				depth;
		const U16		WIDTH = 320;
		const U16		HEIGHT = 240;

		const tDisplayScreenStats* pstats = pDisplayMPI_->GetScreenStats(0);
		TS_ASSERT( pstats != NULL );
		TS_ASSERT( pstats->width == WIDTH );
		TS_ASSERT( pstats->height == HEIGHT );
		
		handle = pDisplayMPI_->CreateHandle(HEIGHT, WIDTH, kPixelFormatARGB8888, NULL);
		TS_ASSERT( handle != kInvalidDisplayHandle );
		pDisplayMPI_->Register(handle, 0, 0, 0, 0);
		TS_ASSERT( handle == pDisplayMPI_->GetCurrentDisplayHandle() );

		buffer = pDisplayMPI_->GetBuffer(handle);
		TS_ASSERT( buffer != kNull );
		format = pDisplayMPI_->GetPixelFormat(handle);
		TS_ASSERT( format == kPixelFormatARGB8888 );
		width = pDisplayMPI_->GetWidth(handle);
		TS_ASSERT( width == WIDTH );
		pitch = pDisplayMPI_->GetPitch(handle);
		TS_ASSERT( pitch == 4 * WIDTH );
		depth = pDisplayMPI_->GetDepth(handle);
		TS_ASSERT( depth == 32 );
		height = pDisplayMPI_->GetHeight(handle);
		TS_ASSERT( height == HEIGHT );

		for (int i = 0; i < height; i++) 
		{
			bool blu = (i < HEIGHT/2);
			bool grn = (i < HEIGHT/4) || (i > HEIGHT/2 && i < 3*HEIGHT/4);
			bool red = (i < HEIGHT/4) || (i > 3*HEIGHT/4);
			U8   val = (i * 0xFF / (HEIGHT/4));
			for (int j = 0, m = i*pitch; j < width; j++, m+=4)
			{
				buffer[m+0] = (blu) ? val : 0;
				buffer[m+1] = (grn) ? val : 0;
				buffer[m+2] = (red) ? val : 0;
				buffer[m+3] = 0xFF;
			}
		}
		pDisplayMPI_->Invalidate(0, NULL);

		sleep(1);

		pDisplayMPI_->UnRegister(handle, 0);
		pDisplayMPI_->DestroyHandle(handle, false);
	}

	//------------------------------------------------------------------------
	void testOpenGLContext( )
	{
		BrioOpenGLConfig*	oglctx = new BrioOpenGLConfig();
		
		TS_ASSERT( oglctx != NULL );
		TS_ASSERT( oglctx->eglContext );
		TS_ASSERT( oglctx->eglDisplay );
		TS_ASSERT( oglctx->eglSurface );

		sleep(1);
		eglWaitGL();

		glClearColorx(0x10000, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		eglSwapBuffers(oglctx->eglDisplay, oglctx->eglSurface);
		sleep(1);
		
		glClearColorx(0, 0x10000, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		eglSwapBuffers(oglctx->eglDisplay, oglctx->eglSurface);
		sleep(1);

		glClearColorx(0, 0, 0x10000, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		eglSwapBuffers(oglctx->eglDisplay, oglctx->eglSurface);
		sleep(1);

		for (int i = 0; i < 100; i++)
		{
			(i % 2) 
				? glClearColorx(0x10000, 0, 0, 0)
				: glClearColorx(0, 0, 0x10000, 0);
			glClear(GL_COLOR_BUFFER_BIT);
			eglSwapBuffers(oglctx->eglDisplay, oglctx->eglSurface);
		}
		eglWaitGL();

		delete oglctx;
	}

	//------------------------------------------------------------------------
	void testBrightnessContrast( )
	{
		tDisplayHandle 	handle;
		U8* 			buffer;
		U16				width;
		U16				height;
		U16				pitch;
		S8				brightness;
		S8				contrast;
		const U16		WIDTH = 320;
		const U16		HEIGHT = 240;
		// TODO: fix min/max bounds logic
		const S8		BRIGHT_INC = 4;
		const S8		BRIGHT_MIN = -128+BRIGHT_INC; //-128; //underflow
		const S8		BRIGHT_MAX = 127-BRIGHT_INC; //127; //overflow
		const S8		CONTRAST_INC = 16;
		const S8		CONTRAST_MIN = -128+CONTRAST_INC;
		const S8		CONTRAST_MAX = 127-CONTRAST_INC;

		handle = pDisplayMPI_->CreateHandle(HEIGHT, WIDTH, kPixelFormatARGB8888, NULL);
		TS_ASSERT( handle != kInvalidDisplayHandle );
		pDisplayMPI_->Register(handle, 0, 0, 0, 0);

		buffer = pDisplayMPI_->GetBuffer(handle);
		TS_ASSERT( buffer != kNull );
		width = pDisplayMPI_->GetWidth(handle);
		TS_ASSERT( width == WIDTH );
		pitch = pDisplayMPI_->GetPitch(handle);
		TS_ASSERT( pitch == 4 * WIDTH );
		height = pDisplayMPI_->GetHeight(handle);
		TS_ASSERT( height == HEIGHT );

		for (int i = 0; i < height; i++) 
		{
			for (int j = 0, m = i*pitch; j < width; j++, m+=4)
			{
				U8 val = m % 0xFF;
				buffer[m+0] = val;
				buffer[m+1] = val;
				buffer[m+2] = val;
				buffer[m+3] = 0xFF;
			}
		}
		pDisplayMPI_->Invalidate(0, NULL);

		for (brightness = 0; brightness < BRIGHT_MAX; brightness+=BRIGHT_INC)
		{
			pDisplayMPI_->SetBrightness(0, brightness);
			usleep(1000);
			TS_ASSERT( brightness == pDisplayMPI_->GetBrightness(0) );
			usleep(1000);
		}
		for (; brightness > BRIGHT_MIN; brightness-=BRIGHT_INC)
		{
			pDisplayMPI_->SetBrightness(0, brightness);
			usleep(1000);
			TS_ASSERT( brightness == pDisplayMPI_->GetBrightness(0) );
			usleep(1000);
		}
		for (; brightness <= 0; brightness+=BRIGHT_INC)
		{
			pDisplayMPI_->SetBrightness(0, brightness);
			usleep(1000);
			TS_ASSERT( brightness == pDisplayMPI_->GetBrightness(0) );
			usleep(1000);
		}
		for (contrast = 0; contrast < CONTRAST_MAX; contrast+=CONTRAST_INC)
		{
			pDisplayMPI_->SetContrast(0, contrast);
			usleep(1000);
			TS_ASSERT( contrast == pDisplayMPI_->GetContrast(0) );
			usleep(1000);
		}
		for (; contrast > CONTRAST_MIN; contrast-=CONTRAST_INC)
		{
			pDisplayMPI_->SetContrast(0, contrast);
			usleep(1000);
			TS_ASSERT( contrast == pDisplayMPI_->GetContrast(0) );
			usleep(1000);
		}
		for (; contrast <= 0; contrast+=CONTRAST_INC)
		{
			pDisplayMPI_->SetContrast(0, contrast);
			usleep(1000);
			TS_ASSERT( contrast == pDisplayMPI_->GetContrast(0) );
			usleep(1000);
		}

		pDisplayMPI_->UnRegister(handle, 0);
		pDisplayMPI_->DestroyHandle(handle, false);
	}

	//------------------------------------------------------------------------
	void testBacklight( )
	{
		tDisplayHandle 	handle;
		const U16		WIDTH = 320;
		const U16		HEIGHT = 240;
		handle = pDisplayMPI_->CreateHandle(HEIGHT, WIDTH, kPixelFormatARGB8888, NULL);
		pDisplayMPI_->Register(handle, 0, 0, 0, 0);

		pDisplayMPI_->SetBacklight(0, 0);
		usleep(1000);
		TS_ASSERT( pDisplayMPI_->GetBacklight(0) == 0 );
		sleep(1);
		pDisplayMPI_->SetBacklight(0, 100);
		usleep(1000);
		TS_ASSERT( pDisplayMPI_->GetBacklight(0) == 100 );
		sleep(1);
		pDisplayMPI_->SetBacklight(0, 0);
		usleep(1000);
		TS_ASSERT( pDisplayMPI_->GetBacklight(0) == 0 );
		sleep(1);
		
		pDisplayMPI_->UnRegister(handle, 0);
		pDisplayMPI_->DestroyHandle(handle, false);
	}
	
	//------------------------------------------------------------------------
	void testDisplayContext24( )
	{
		tDisplayHandle 	handle;
		tPixelFormat	format;
		U8* 			buffer;
		U16				width;
		U16				height;
		U16				pitch;
		U16				depth;
		const U16		WIDTH = 320;
		const U16		HEIGHT = 240;

		handle = pDisplayMPI_->CreateHandle(HEIGHT, WIDTH, kPixelFormatRGB888, NULL);
		TS_ASSERT( handle != kInvalidDisplayHandle );
		pDisplayMPI_->Register(handle, 0, 0, 0, 0);
		TS_ASSERT( handle == pDisplayMPI_->GetCurrentDisplayHandle() );

		buffer = pDisplayMPI_->GetBuffer(handle);
		TS_ASSERT( buffer != kNull );
		format = pDisplayMPI_->GetPixelFormat(handle);
		TS_ASSERT( format == kPixelFormatRGB888 );
		width = pDisplayMPI_->GetWidth(handle);
		TS_ASSERT( width == WIDTH );
		pitch = pDisplayMPI_->GetPitch(handle);
		TS_ASSERT( pitch == 3 * WIDTH );
		depth = pDisplayMPI_->GetDepth(handle);
		TS_ASSERT( depth == 24 );
		height = pDisplayMPI_->GetHeight(handle);
		TS_ASSERT( height == HEIGHT );

		for (int i = 0; i < height; i++) 
		{
			bool blu = (i < HEIGHT/2);
			bool grn = (i < HEIGHT/4) || (i > HEIGHT/2 && i < 3*HEIGHT/4);
			bool red = (i < HEIGHT/4) || (i > 3*HEIGHT/4);
			for (int j = 0, m = i*pitch; j < width; j++, m+=3)
			{
				U8   val = j % 0xFF;
				buffer[m+0] = (blu) ? val : 0;
				buffer[m+1] = (grn) ? val : 0;
				buffer[m+2] = (red) ? val : 0;
			}
		}
		pDisplayMPI_->Invalidate(0, NULL);

		sleep(1);

		pDisplayMPI_->UnRegister(handle, 0);
		pDisplayMPI_->DestroyHandle(handle, false);
	}

	//------------------------------------------------------------------------
	void testDisplayContext16( )
	{
		tDisplayHandle 	handle;
		tPixelFormat	format;
		U8* 			buffer;
		U16				width;
		U16				height;
		U16				pitch;
		U16				depth;
		const U16		WIDTH = 320;
		const U16		HEIGHT = 240;

		handle = pDisplayMPI_->CreateHandle(HEIGHT, WIDTH, kPixelFormatRGB4444, NULL);
		TS_ASSERT( handle != kInvalidDisplayHandle );
		pDisplayMPI_->Register(handle, 0, 0, 0, 0);
		TS_ASSERT( handle == pDisplayMPI_->GetCurrentDisplayHandle() );

		buffer = pDisplayMPI_->GetBuffer(handle);
		TS_ASSERT( buffer != kNull );
		format = pDisplayMPI_->GetPixelFormat(handle);
		TS_ASSERT( format == kPixelFormatRGB4444 );
		width = pDisplayMPI_->GetWidth(handle);
		TS_ASSERT( width == WIDTH );
		pitch = pDisplayMPI_->GetPitch(handle);
		TS_ASSERT( pitch == 2 * WIDTH );
		depth = pDisplayMPI_->GetDepth(handle);
		TS_ASSERT( depth == 16 );
		height = pDisplayMPI_->GetHeight(handle);
		TS_ASSERT( height == HEIGHT );

		for (int i = 0; i < height; i++) 
		{
			bool blu = (i < HEIGHT/2);
			bool grn = (i < HEIGHT/4) || (i > HEIGHT/2 && i < 3*HEIGHT/4);
			bool red = (i < HEIGHT/4) || (i > 3*HEIGHT/4);
			for (int j = 0, m = i*pitch; j < width; j++, m+=2)
			{
				U8   val = j % 0xF;
				U16  pix = (blu) ? val : 0;
				pix  |= (grn) ? (val << 4) : 0;
				pix  |= (red) ? (val << 8) : 0;
				pix  |= (val << 12);
				U16* ppix = reinterpret_cast<U16*>(&buffer[m]);
				*ppix = pix;
			}
		}
		pDisplayMPI_->Invalidate(0, NULL);

		sleep(1);

		pDisplayMPI_->UnRegister(handle, 0);
		pDisplayMPI_->DestroyHandle(handle, false);
	}

	//------------------------------------------------------------------------
	void testDisplayContext565( )
	{
		tDisplayHandle 	handle;
		tPixelFormat	format;
		U8* 			buffer;
		U16				width;
		U16				height;
		U16				pitch;
		U16				depth;
		const U16		WIDTH = 320;
		const U16		HEIGHT = 240;

		handle = pDisplayMPI_->CreateHandle(HEIGHT, WIDTH, kPixelFormatRGB565, NULL);
		TS_ASSERT( handle != kInvalidDisplayHandle );
		pDisplayMPI_->Register(handle, 0, 0, 0, 0);
		TS_ASSERT( handle == pDisplayMPI_->GetCurrentDisplayHandle() );

		buffer = pDisplayMPI_->GetBuffer(handle);
		TS_ASSERT( buffer != kNull );
		format = pDisplayMPI_->GetPixelFormat(handle);
		TS_ASSERT( format == kPixelFormatRGB565 );
		width = pDisplayMPI_->GetWidth(handle);
		TS_ASSERT( width == WIDTH );
		pitch = pDisplayMPI_->GetPitch(handle);
		TS_ASSERT( pitch == 2 * WIDTH );
		depth = pDisplayMPI_->GetDepth(handle);
		TS_ASSERT( depth == 16 );
		height = pDisplayMPI_->GetHeight(handle);
		TS_ASSERT( height == HEIGHT );

		for (int i = 0; i < height; i++) 
		{
			bool blu = (i < HEIGHT/2);
			bool grn = (i < HEIGHT/4) || (i > HEIGHT/2 && i < 3*HEIGHT/4);
			bool red = (i < HEIGHT/4) || (i > 3*HEIGHT/4);
			for (int j = 0, m = i*pitch; j < width; j++, m+=2)
			{
				U8   val = j % 0x1F;
				U16  pix = (blu) ? val : 0;
				pix  |= (grn) ? (val << 6) : 0; // (0x1F << 1) << 5
				pix  |= (red) ? (val << 11) : 0;
				U16* ppix = reinterpret_cast<U16*>(&buffer[m]);
				*ppix = pix;
			}
		}
		pDisplayMPI_->Invalidate(0, NULL);

		sleep(1);

		pDisplayMPI_->UnRegister(handle, 0);
		pDisplayMPI_->DestroyHandle(handle, false);
	}
	
	//------------------------------------------------------------------------
	void testDisplaySwapBuffers( )
	{
		tDisplayHandle 	handle[2];
		tPixelFormat	format;
		U8* 			buffer[2];
		U16				width;
		U16				height;
		U16				pitch;
		U16				depth;
		const U16		WIDTH = 320;
		const U16		HEIGHT = 240;

		for (int i = 0; i < 2; i++)
		{
			handle[i] = pDisplayMPI_->CreateHandle(HEIGHT, WIDTH, kPixelFormatARGB8888, NULL);
			TS_ASSERT( handle[i] != kInvalidDisplayHandle );
			pDisplayMPI_->Register(handle[i], 0, 0, 0, 0);
			buffer[i] = pDisplayMPI_->GetBuffer(handle[i]);
			TS_ASSERT( buffer[0] != kNull );
			format = pDisplayMPI_->GetPixelFormat(handle[i]);
			TS_ASSERT( format == kPixelFormatARGB8888 );
			width = pDisplayMPI_->GetWidth(handle[i]);
			TS_ASSERT( width == WIDTH );
			pitch = pDisplayMPI_->GetPitch(handle[i]);
			TS_ASSERT( pitch == 4 * WIDTH );
			depth = pDisplayMPI_->GetDepth(handle[i]);
			TS_ASSERT( depth == 32 );
			height = pDisplayMPI_->GetHeight(handle[i]);
			TS_ASSERT( height == HEIGHT );
		}

		memset(buffer[0], 0x0F, pitch * height);
		memset(buffer[1], 0xF0, pitch * height);

		for (int i = 0; i < 10; i++)
		{
			tErrType	rc;
			Boolean 	bc;
			rc = pDisplayMPI_->SwapBuffers(handle[i%2], true);
			TS_ASSERT( rc == kNoErr );
			TS_ASSERT( handle[i%2] == pDisplayMPI_->GetCurrentDisplayHandle() );
			bc = pDisplayMPI_->IsBufferSwapped(handle[i%2]);
			TS_ASSERT( bc == true );
		}

		for (int i = 0; i < 10; i++)
		{
			tErrType	rc;
			Boolean 	bc = false;
			int		counter = 0;
			rc = pDisplayMPI_->SwapBuffers(handle[i%2], false);
			TS_ASSERT( rc == kNoErr );
			TS_ASSERT( handle[i%2] == pDisplayMPI_->GetCurrentDisplayHandle() );
			while (bc != true) 
			{
				bc = pDisplayMPI_->IsBufferSwapped(handle[i%2]);
				if (++counter > 16)
					break; // 16.7 msec max for 60 Hz
				usleep(1000);
			}
			TS_ASSERT( bc == true );
		}

		for (int i = 0; i < 2; i++)
		{
			pDisplayMPI_->UnRegister(handle[i], 0);
			pDisplayMPI_->DestroyHandle(handle[i], false);
		}
	}

	//------------------------------------------------------------------------
	void testDisplayTripleBuffers( )
	{
		tDisplayHandle 	handle[3];
		tPixelFormat	format;
		U8* 			buffer[3];
		U16				width;
		U16				height;
		U16				pitch;
		U16				depth;
		const U16		WIDTH = 320;
		const U16		HEIGHT = 240;
		const U16		DEPTHS[4] = {16, 16, 24, 32};
		const tPixelFormat	FORMATS[4] = {kPixelFormatRGB4444, kPixelFormatRGB565, kPixelFormatRGB888, kPixelFormatARGB8888};
		
		for (int j = 0; j < 4; j++)
		{
			for (int i = 0; i < 3; i++)
			{
				handle[i] = pDisplayMPI_->CreateHandle(HEIGHT, WIDTH, FORMATS[j], NULL);
				TS_ASSERT( handle[i] != kInvalidDisplayHandle );
				buffer[i] = pDisplayMPI_->GetBuffer(handle[i]);
				TS_ASSERT( buffer[0] != kNull );
				format = pDisplayMPI_->GetPixelFormat(handle[i]);
				TS_ASSERT( format == FORMATS[j] );
				width = pDisplayMPI_->GetWidth(handle[i]);
				TS_ASSERT( width == WIDTH );
				pitch = pDisplayMPI_->GetPitch(handle[i]);
				TS_ASSERT( pitch == (DEPTHS[j]/8) * WIDTH )
				depth = pDisplayMPI_->GetDepth(handle[i]);
				TS_ASSERT( depth == DEPTHS[j] );
				height = pDisplayMPI_->GetHeight(handle[i]);
				TS_ASSERT( height == HEIGHT );
				memset(buffer[i], 0x55, pitch * height);
				pDisplayMPI_->Register(handle[i], 0, 0, 0, 0);
			}
			for (int i = 0; i < 3; i++)
			{
				pDisplayMPI_->UnRegister(handle[i], 0);
				pDisplayMPI_->DestroyHandle(handle[i], false);
			}
		}
	}

	//------------------------------------------------------------------------
	void testDisplayAllocations( )
	{
		tDisplayHandle 		handle[3];
		BrioOpenGLConfig*	oglctx;
		U32					totalmem;
		U32					freemem;
		U32					usedmem;
		
		totalmem = pDisplayMPI_->GetDisplayMem(kDisplayMemTotal);
		freemem = pDisplayMPI_->GetDisplayMem(kDisplayMemFree);
		usedmem = pDisplayMPI_->GetDisplayMem(kDisplayMemUsed);
		TS_ASSERT( freemem == totalmem );
		TS_ASSERT( usedmem == 0 );

#ifndef EMULATION
		oglctx = new BrioOpenGLConfig();
		freemem = pDisplayMPI_->GetDisplayMem(kDisplayMemFree);
		usedmem = pDisplayMPI_->GetDisplayMem(kDisplayMemUsed);
		TS_ASSERT( freemem + usedmem == totalmem );
		TS_ASSERT( usedmem != 0 );
		delete oglctx;

		freemem = pDisplayMPI_->GetDisplayMem(kDisplayMemFree);
		usedmem = pDisplayMPI_->GetDisplayMem(kDisplayMemUsed);
		TS_ASSERT( freemem == totalmem );
		TS_ASSERT( usedmem == 0 );
		
		for (int i = 0; i < 3; i++)
			handle[i] = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatARGB8888, NULL);
		freemem = pDisplayMPI_->GetDisplayMem(kDisplayMemFree);
		usedmem = pDisplayMPI_->GetDisplayMem(kDisplayMemUsed);
		TS_ASSERT( freemem + usedmem == totalmem );
		TS_ASSERT( usedmem != 0 );
		for (int i = 0; i < 3; i++)
			pDisplayMPI_->DestroyHandle(handle[i], false);

		freemem = pDisplayMPI_->GetDisplayMem(kDisplayMemFree);
		usedmem = pDisplayMPI_->GetDisplayMem(kDisplayMemUsed);
		TS_ASSERT( freemem == totalmem );
		TS_ASSERT( usedmem == 0 );

		oglctx = new BrioOpenGLConfig();
		handle[0] = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatARGB8888, NULL);
		handle[1] = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatARGB8888, NULL);
		handle[2] = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatYUV420, NULL);
		freemem = pDisplayMPI_->GetDisplayMem(kDisplayMemFree);
		usedmem = pDisplayMPI_->GetDisplayMem(kDisplayMemUsed);
		TS_ASSERT( freemem + usedmem == totalmem );
		TS_ASSERT( usedmem != 0 );
		delete oglctx;
		for (int i = 0; i < 3; i++)
			pDisplayMPI_->DestroyHandle(handle[i], false);
		
		freemem = pDisplayMPI_->GetDisplayMem(kDisplayMemFree);
		usedmem = pDisplayMPI_->GetDisplayMem(kDisplayMemUsed);
		TS_ASSERT( freemem == totalmem );
		TS_ASSERT( usedmem == 0 );
#endif	// !EMULATION
	}

	//------------------------------------------------------------------------
	void testDisplayContextYUV( )
	{
		tDisplayHandle 	handle;
		tDisplayHandle 	offscreen;
		tPixelFormat	format;
		U8* 			buffer;
		U16				width;
		U16				height;
		U16				pitch;
		U16				depth;
		const U16		WIDTH = 320;
		const U16		HEIGHT = 240;

		handle = pDisplayMPI_->CreateHandle(HEIGHT, WIDTH, kPixelFormatYUV420, NULL);
		TS_ASSERT( handle != kInvalidDisplayHandle );
		pDisplayMPI_->Register(handle, 0, 0, 0, 0);
		TS_ASSERT( handle == pDisplayMPI_->GetCurrentDisplayHandle() );

		buffer = pDisplayMPI_->GetBuffer(handle);
		TS_ASSERT( buffer != kNull );
		format = pDisplayMPI_->GetPixelFormat(handle);
		TS_ASSERT( format == kPixelFormatYUV420 );
		width = pDisplayMPI_->GetWidth(handle);
		TS_ASSERT( width == WIDTH );
		pitch = pDisplayMPI_->GetPitch(handle);
		TS_ASSERT( pitch >= 2*WIDTH );
		depth = pDisplayMPI_->GetDepth(handle);
		TS_ASSERT( depth == 8 );
		height = pDisplayMPI_->GetHeight(handle);
		TS_ASSERT( height == HEIGHT );

		memset(buffer, 0xFF, pitch * height); // 0 = dim green, 255 = bright magenta
		pDisplayMPI_->Invalidate(0, NULL);
		sleep(1);

		U8* membuffer = new U8[HEIGHT*WIDTH*4];
		TS_ASSERT( membuffer != NULL );
		offscreen = pDisplayMPI_->CreateHandle(HEIGHT, WIDTH, kPixelFormatARGB8888, membuffer);
		TS_ASSERT( offscreen != NULL );
		for (int i = 0; i < HEIGHT; i++) 
		{
			bool blu = (i < HEIGHT/2);
			bool grn = (i < HEIGHT/4) || (i > HEIGHT/2 && i < 3*HEIGHT/4);
			bool red = (i < HEIGHT/4) || (i > 3*HEIGHT/4);
			for (int j = 0, m = i*WIDTH*4; j < WIDTH; j++, m+=4)
			{
				U8   val = j % 0xFF;
				membuffer[m+0] = (blu) ? val : 0;
				membuffer[m+1] = (grn) ? val : 0;
				membuffer[m+2] = (red) ? val : 0;
				membuffer[m+3] = 0xFF;
			}
		}
		pDisplayMPI_->Register(offscreen, 0, 0, kDisplayOnTop, 0);
		pDisplayMPI_->Invalidate(0, NULL);
		sleep(1);

		pDisplayMPI_->UnRegister(offscreen, 0);
		pDisplayMPI_->DestroyHandle(offscreen, false);

		pDisplayMPI_->UnRegister(handle, 0);
		pDisplayMPI_->DestroyHandle(handle, false);
	}

	//------------------------------------------------------------------------

};

// EOF
