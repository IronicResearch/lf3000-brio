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
		U8* 			buffer;
		U16				width;
		U16				height;
		U16				pitch;
		const U16		WIDTH = 320;
		const U16		HEIGHT = 240;

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

};

// EOF
