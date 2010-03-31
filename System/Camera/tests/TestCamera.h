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
const tEventType LocalCameraEvents[] = {kAllCameraEvents};

//============================================================================
// TestCameraMPI functions
//============================================================================
class TestCamera : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CCameraMPI*		pCameraMPI_;
	CDisplayMPI*	pDisplayMPI_;
	CKernelMPI*		pKernelMPI_;
	tEventType		reason;
#if 0
	//============================================================================
	// Local event listener
	//============================================================================
	class CameraListener : public IEventListener
	{
	private:
		tEventType	reason;
	public:
		CameraListener():
			IEventListener(LocalCameraEvents, ArrayCount(LocalCameraEvents))
			{reason = kUndefinedEventType;}

		tEventStatus Notify(const IEventMessage& msg)
		{
			reason = msg.GetEventType();
			if(reason == kCaptureTimeOutEvent)
			{
printf("timeout\n");
			}
			else if(reason == kCaptureQuotaHitEvent)
			{
printf("quota\n");
			}
			else if(reason == kCaptureStoppedEvent)
			{
printf("fstop\n");
			}

			return kEventStatusOK;
		}

		tEventStatus Reset()
		{
			reason = kUndefinedEventType;
		}

		tEventType GetReason()
		{
			return reason;
		}
	};
#endif
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
	void testCaptureYUV()
	{
		tVidCapHndl					capture;
		Boolean						bRet;
		tErrType					err;

		// For displaying captured data
		tVideoSurf				surf;
		tDisplayHandle			disp;

		pKernelMPI_ = new CKernelMPI;
		pDisplayMPI_ = new CDisplayMPI;
		disp = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatYUV420, NULL);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, kDisplayOnTop, 0);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);
		TS_ASSERT( surf.format == kPixelFormatYUV420 );

		if ( pCameraMPI_->IsValid() )
		{
			err = pCameraMPI_->SetCameraVideoPath("/LF/Base/L3B_Video");
			TS_ASSERT_EQUALS( err, kNoErr );

			capture = pCameraMPI_->StartVideoCapture(&surf, NULL, "testYUV.avi");
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
	}

	//------------------------------------------------------------------------
	void testCaptureRGB()
	{
		tVidCapHndl					capture;
		Boolean						bRet;
		tErrType					err;

		// For displaying captured data
		tVideoSurf				surf;
		tDisplayHandle			disp;

		pKernelMPI_ = new CKernelMPI;
		pDisplayMPI_ = new CDisplayMPI;
		disp = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatRGB888, NULL);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, kDisplayOnTop, 0);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);
		TS_ASSERT( surf.format == kPixelFormatRGB888 );

		if ( pCameraMPI_->IsValid() )
		{
			err = pCameraMPI_->SetCameraVideoPath("/LF/Base/L3B_Video");
			TS_ASSERT_EQUALS( err, kNoErr );

			capture = pCameraMPI_->StartVideoCapture(&surf, NULL, "testRGB.avi");
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
	}

	//------------------------------------------------------------------------
	void testCaptureEvent()
	{
#if 0
		tVidCapHndl					capture;
		Boolean						bRet;
		tErrType					err;

		// For displaying captured data
		tVideoSurf				surf;
		tDisplayHandle			disp;

		CameraListener			listener;

		pKernelMPI_ = new CKernelMPI;
		pDisplayMPI_ = new CDisplayMPI;
		disp = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatYUV420, NULL);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, kDisplayOnTop, 0);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);
		TS_ASSERT( surf.format == kPixelFormatYUV420 );

		if ( pCameraMPI_->IsValid() )
		{
			err = pCameraMPI_->SetCameraVideoPath("/LF/Base/L3B_Video");
			TS_ASSERT_EQUALS( err, kNoErr );

			capture = pCameraMPI_->StartVideoCapture(&surf, &listener, "", 2);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			//Capture will stop after 2 seconds
			pKernelMPI_->TaskSleep(2500);
			TS_ASSERT_EQUALS( listener.GetReason(), kCaptureTimeOutEvent );

			//Reset listener
			listener.Reset();

			capture = pCameraMPI_->StartVideoCapture(&surf, &listener, "");
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			//Unlimited recording
			pKernelMPI_->TaskSleep(2500);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
			TS_ASSERT_EQUALS( listener.GetReason(), kCaptureStoppedEvent );
		}

		pDisplayMPI_->UnRegister(disp, 0);
		delete pDisplayMPI_;
		delete pKernelMPI_;
#endif
	}

	//------------------------------------------------------------------------
	void testControls()
	{
		tVidCapHndl					capture;
		Boolean						bRet;

		tCameraControls				controls;
		tCameraControls::iterator	it;

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
			bRet = pCameraMPI_->GetCameraControls(controls);
			TS_ASSERT_EQUALS( bRet, true );

			capture = pCameraMPI_->StartVideoCapture(&surf);
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
	}

	//------------------------------------------------------------------------
	void testPause()
	{
		tVidCapHndl					capture;
		Boolean						bRet;

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
			capture = pCameraMPI_->StartVideoCapture(&surf);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			int step = 0;
			while(step++ < 3)
			{
				pKernelMPI_->TaskSleep(2000);

				bRet = pCameraMPI_->PauseVideoCapture(capture);
				TS_ASSERT_EQUALS( bRet, true );

				bRet = pCameraMPI_->IsVideoCapturePaused(capture);
				TS_ASSERT_EQUALS( bRet, true );

				pKernelMPI_->TaskSleep(2000);

				bRet = pCameraMPI_->ResumeVideoCapture(capture);
				TS_ASSERT_EQUALS( bRet, true );

				bRet = pCameraMPI_->IsVideoCapturePaused(capture);
				TS_ASSERT_EQUALS( bRet, false );
			}

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
		}

		pDisplayMPI_->UnRegister(disp, 0);
		delete pDisplayMPI_;
		delete pKernelMPI_;
	}

	//------------------------------------------------------------------------
	void testSnap()
	{
		tVidCapHndl				capture;
		Boolean					bRet;
		tErrType				err;

		// For displaying captured data
		tVideoSurf				surf;
		tDisplayHandle			disp;

		pKernelMPI_ = new CKernelMPI;
		pDisplayMPI_ = new CDisplayMPI;
		disp = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatYUV420, NULL);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, kDisplayOnTop, 0);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);
		TS_ASSERT( surf.format == kPixelFormatYUV420 );

		if ( pCameraMPI_->IsValid() )
		{
			err = pCameraMPI_->SetCameraStillPath("/LF/Base/L3B_Video");
			TS_ASSERT_EQUALS( err, kNoErr );

			capture = pCameraMPI_->StartVideoCapture(&surf);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			int step = 0;
			while(step++ < 50)
			{
				pKernelMPI_->TaskSleep(100);

				if(step == 10)
				{
					bRet = pCameraMPI_->SnapFrame(capture, "test.jpg");
					TS_ASSERT_EQUALS( bRet, true );
				}

			}

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			bRet = pCameraMPI_->RenderFrame("test.jpg", &surf);
			TS_ASSERT_EQUALS( bRet, true );

			step = 0;
			while(step++ < 50)
			{
				pKernelMPI_->TaskSleep(100);
			}
		}

		pDisplayMPI_->UnRegister(disp, 0);
		delete pDisplayMPI_;
		delete pKernelMPI_;
	}

};

// EOF
