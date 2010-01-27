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

//============================================================================
// TestCameraMPI functions
//============================================================================
class TestCamera : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CCameraMPI*		pCameraMPI_;
	CDisplayMPI*	pDisplayMPI_;
	CKernelMPI*		pKernelMPI_;
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
	void testBasicCaptureYUV()
	{
#if 1
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

				bRet = pCameraMPI_->RenderFrame(&frame, &surf, &bitmap);
				TS_ASSERT_EQUALS( bRet, true );

				pDisplayMPI_->Invalidate(0);

				bRet = pCameraMPI_->ReturnFrame(capture, &frame);
				TS_ASSERT_EQUALS( bRet, true );

			} while (count--);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			kernel->Free(bitmap.data);
			bitmap.data = NULL;

			delete kernel;
		}

		pDisplayMPI_->UnRegister(disp, 0);
		delete pDisplayMPI_;
#endif
	}

	//------------------------------------------------------------------------
	void testBasicCaptureRGB()
	{
#if 1
		tVidCapHndl				capture;
		Boolean					bRet;

		// For camera control
		tCaptureModes			modes;
		tCaptureModes::iterator	it;
		tCaptureMode			qqvga = {kCaptureFormatMJPEG, 160, 120, 1, 30};

		// For working with captured data
		tFrameInfo				frame = { kCaptureFormatMJPEG, 160, 120 };
		tBitmapInfo				bitmap = { kBitmapFormatRGB888, 160, 120, NULL, 0 };

		// For displaying captured data
		tVideoSurf				surf;
		tDisplayHandle			disp;
		int						count = 120;

		pDisplayMPI_ = new CDisplayMPI;
		disp = pDisplayMPI_->CreateHandle(120, 160, kPixelFormatRGB888, NULL);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 80, 60, kDisplayOnTop, 0);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);
		TS_ASSERT( surf.format == kPixelFormatRGB888 );

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

				bRet = pCameraMPI_->RenderFrame(&frame, &surf, &bitmap);
				TS_ASSERT_EQUALS( bRet, true );

				pDisplayMPI_->Invalidate(0);

				bRet = pCameraMPI_->ReturnFrame(capture, &frame);
				TS_ASSERT_EQUALS( bRet, true );

			} while (count--);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			kernel->Free(bitmap.data);
			bitmap.data = NULL;

			delete kernel;
		}

		pDisplayMPI_->UnRegister(disp, 0);
		delete pDisplayMPI_;
#endif
	}

	//------------------------------------------------------------------------
	void testCaptureThread()
	{
#if 1
		tVidCapHndl					capture;
		Boolean						bRet;

		tCaptureMode				qqvga = {kCaptureFormatMJPEG, 160, 120, 1, 30};

		// For displaying captured data
		tVideoSurf				surf;
		tDisplayHandle			disp;

		pKernelMPI_ = new CKernelMPI;
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

		if ( pCameraMPI_->IsValid() )
		{
			bRet = pCameraMPI_->SetCameraMode(&qqvga);
			TS_ASSERT_EQUALS( bRet, true );

			capture = pCameraMPI_->StartVideoCapture("", false, &surf, NULL);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			int step = 0;
			while(step++ < 50)
			{
				pKernelMPI_->TaskSleep(100);
			}

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
		}

		pDisplayMPI_->UnRegister(disp, 0);
		delete pDisplayMPI_;
		delete pKernelMPI_;
#endif
	}

	//------------------------------------------------------------------------
	void testControlsThreaded()
	{
#if 1
		tVidCapHndl					capture;
		Boolean						bRet;

		tCameraControls				controls;
		tCameraControls::iterator	it;

		tCaptureMode				qqvga = {kCaptureFormatMJPEG, 160, 120, 1, 30};

		// For displaying captured data
		tVideoSurf				surf;
		tDisplayHandle			disp;

		pKernelMPI_ = new CKernelMPI;
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

		if ( pCameraMPI_->IsValid() )
		{
			bRet = pCameraMPI_->SetCameraMode(&qqvga);
			TS_ASSERT_EQUALS( bRet, true );

			bRet = pCameraMPI_->GetCameraControls(controls);
			TS_ASSERT_EQUALS( bRet, true );

			capture = pCameraMPI_->StartVideoCapture("", false, &surf, NULL);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			for(it = controls.begin(); it < controls.end(); it++)
			{
				if((*it)->type == kControlTypeBrightness)
				{
					break;
				}
			}

			int bright 	= (*it)->min;
			int step	= MAX( ((*it)->max - (*it)->min) / 50, 1);
			while(bright < (*it)->max)
			{
				bRet = pCameraMPI_->SetCameraControl(*it, bright);
				TS_ASSERT_EQUALS( bRet, true );

				bright += step;
				pKernelMPI_->TaskSleep(100);
			}

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			pCameraMPI_->SetCameraControl(*it, (*it)->preset);
			TS_ASSERT_EQUALS( bRet, true );
		}

		pDisplayMPI_->UnRegister(disp, 0);
		delete pDisplayMPI_;
		delete pKernelMPI_;
#endif
	}
};

// EOF
