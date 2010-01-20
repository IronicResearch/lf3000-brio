// TestCameraMPI.h

#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <CameraMPI.h>
#include <DisplayMPI.h>
#include <KernelMPI.h>
#include <EventMPI.h>
#include <EventListener.h>
#include <AudioTypes.h>
#include <UnitTestUtils.h>

LF_USING_BRIO_NAMESPACE()

const tDebugSignature kMyApp = kTestSuiteDebugSig;

inline void YCbCr2YUV(U8 * pd, U8 * ps, int width, int height )
{
    int         dPitch = 4096;
    int         dHeight = height;
	U8*			du = pd + dPitch/2; // U,V in double-width buffer
	U8*			dv = pd + dPitch/2 + dPitch * dHeight/2;
	U8			y,cb,cr;
	int			i,j,m,n;

	for (i = 0; i < height; i++)
	{
		for (j = m = n = 0; n < width; m+=3, n++)
		{
			y		= ps[m+0];
			cb		= ps[m+1];
			cr		= ps[m+2];

			pd[n]	= y;
			if(!(i % 2) && !(n % 2))
			{
				du[j]	= cb;
				dv[j++]	= cr;
			}
		}
		ps += m;      //sdc->pitch;
		pd += dPitch; //ddc->pitch;
		if (i % 2)
		{
			du += dPitch; //ddc->pitch;
			dv += dPitch; //ddc->pitch;
		}
	}
}

//============================================================================
// TestCameraMPI functions
//============================================================================
class TestCamera : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CCameraMPI*		pCameraMPI_;
	CDisplayMPI*	pDisplayMPI_;
public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		pCameraMPI_ = new CCameraMPI;
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete pCameraMPI_;
	}

	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		TS_ASSERT( pCameraMPI_ != NULL );
		TS_ASSERT( pCameraMPI_->IsValid() == true );
	}

	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;

		if ( pCameraMPI_->IsValid() ) {
			pName = pCameraMPI_->GetMPIName();
			TS_ASSERT_EQUALS( *pName, "CameraMPI" );
			version = pCameraMPI_->GetModuleVersion();
			TS_ASSERT_EQUALS( version, 2 );
			pName = pCameraMPI_->GetModuleName();
			TS_ASSERT_EQUALS( *pName, "Camera" );
			pURI = pCameraMPI_->GetModuleOrigin();
			TS_ASSERT_EQUALS( *pURI, "/LF/System/Camera" );
		}
	}

	//------------------------------------------------------------------------
	void testBasicCapture()
	{
		tVidCapHndl				capture;
		Boolean					bRet;

		// For camera control
		tCaptureModes			modes;
		tCaptureModes::iterator	it;
		tCaptureMode			qqvga = {kCaptureFormatMJPEG, 160, 120, 1, 30};

		// For working with captured data
		tFrameInfo				frame = { kCaptureFormatMJPEG, 160, 120 };
		tBitmapInfo				bitmap = { kBitmapFormatYCbCr888, 160, 120, NULL, 0 };

		// For displaying captured data
		tVideoSurf				surf;
		tDisplayHandle			disp;
		int						count = 120;

		pDisplayMPI_ = new CDisplayMPI;
		disp = pDisplayMPI_->CreateHandle(120, 160, kPixelFormatYUV420, NULL);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 80, 60, kDisplayOnTop, 0);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);
		TS_ASSERT( surf.format == kPixelFormatYUV420 );

		if ( pCameraMPI_->IsValid() ) {

			CKernelMPI*	kernel = new CKernelMPI();

			bRet = pCameraMPI_->GetCameraModes(modes);
			TS_ASSERT_EQUALS( bRet, true );

			for(it = modes.begin(); it < modes.end(); it++)
			{
				bRet = pCameraMPI_->SetCameraMode(*it);
				TS_ASSERT_EQUALS( bRet, true );
			}

			bRet = pCameraMPI_->SetCameraMode(&qqvga);
			TS_ASSERT_EQUALS( bRet, true );


			bRet = pCameraMPI_->SetBuffers(4);
			TS_ASSERT_EQUALS( bRet, true );

			capture = pCameraMPI_->StartVideoCapture();
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			bitmap.data = static_cast<U8*>(kernel->Malloc( 120 * 160 * 3 * sizeof(U8) ));

			do {
				bRet = pCameraMPI_->GetFrame(capture, &frame);
				TS_ASSERT_EQUALS( bRet, true );

				bRet = pCameraMPI_->RenderFrame(&frame, &bitmap);
				TS_ASSERT_EQUALS( bRet, true );

				YCbCr2YUV(surf.buffer, bitmap.data, 160, 120);

				pDisplayMPI_->Invalidate(0);

				bRet = pCameraMPI_->PutFrame(capture, &frame);
				TS_ASSERT_EQUALS( bRet, true );

			} while (count--);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			kernel->Free(bitmap.data);
			bitmap.data = NULL;

			delete kernel;
		}
	}

	//------------------------------------------------------------------------
	void testCaptureThread()
	{
//			capture = pCameraMPI_->StartVideoCapture("", false, &surf, &rect);
//			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

//			bRet = pCameraMPI_->StopVideoCapture(capture);
//			TS_ASSERT_EQUALS( bRet, true );
	}
};

// EOF
