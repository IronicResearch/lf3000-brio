// TestCameraMPI.h

#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <CameraMPI.h>
#include <DisplayMPI.h>
#include <KernelMPI.h>
#include <EventMPI.h>
#include <VideoMPI.h>
#include <EventListener.h>
#include <AudioTypes.h>
#include <UnitTestUtils.h>

#include <sstream>

//For strcmp
#include <string.h>

#include "Utility.h"
#include <ImageIO.h>


LF_USING_BRIO_NAMESPACE()

const tDebugSignature kMyApp = kTestSuiteDebugSig;
const tEventType LocalCameraEvents[] = {kAllCameraEvents};

const char* capture_path = "/LF/Bulk/Data/Local/All/";

//============================================================================
// TestCameraMPI functions
//============================================================================
class TestCamera : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CCameraMPI*		pCameraMPI_;
	CDisplayMPI*		pDisplayMPI_;
	CKernelMPI*		pKernelMPI_;
	CVideoMPI*		pVideoMPI_;
	tEventType		reason;

	//============================================================================
	// Local event listener
	//============================================================================
	class CameraListener : public IEventListener
	{
	private:
		tEventType	reason;
		U32			length;
	public:
		CameraListener():
			IEventListener(LocalCameraEvents, ArrayCount(LocalCameraEvents))
			{reason = kUndefinedEventType;}

		tEventStatus Notify(const IEventMessage& Imsg)
		{
			const CCameraEventMessage& msg = dynamic_cast<const CCameraEventMessage&>(Imsg);

			reason = Imsg.GetEventType();

			if(reason == kCaptureTimeOutEvent)
			{
				length = msg.data.timeOut.length;
			}
			if(reason == kCaptureQuotaHitEvent)
			{
				length = msg.data.quotaHit.length;
			}
			if(reason == kCaptureStoppedEvent)
			{
				length = msg.data.stopped.length;
			}
			if(reason == kCameraRemovedEvent)
			{
				length = msg.data.removed.length;
			}

			return kEventStatusOK;
		}

		tEventStatus Reset()
		{
			reason = kUndefinedEventType;
			length = 0;
		}

		tEventType GetReason()
		{
			return reason;
		}

		U32 GetLength()
		{
			return length;
		}
	};

public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		pCameraMPI_ = new CCameraMPI;

		CPath path = capture_path;

		pCameraMPI_->SetCameraVideoPath(path + "Videos/");
		pCameraMPI_->SetCameraStillPath(path + "Photos/");
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete pCameraMPI_;
	}

	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		PRINT_TEST_NAME();
		TS_ASSERT( pCameraMPI_ != NULL );
		TS_ASSERT( pCameraMPI_->IsValid() == true );
	}

	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		PRINT_TEST_NAME();
		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;

		tPlatformCaps test = kCapsCamera;
		TS_ASSERT(HasPlatformCapability(test));

		if ( pCameraMPI_->IsValid() ) {
			pName = pCameraMPI_->GetMPIName();
			TS_ASSERT_EQUALS( *pName, "CameraMPI" );
			version = pCameraMPI_->GetModuleVersion();
			pName = pCameraMPI_->GetModuleName();
			printf("Module: %s, version: %d\n", pName->c_str(), version);
			pURI = pCameraMPI_->GetModuleOrigin();
			TS_ASSERT_EQUALS( *pURI, "/LF/System/Camera" );
		}

	}

	//------------------------------------------------------------------------
	void testOverlay()
	{
		PRINT_TEST_NAME();

		tVidCapHndl					capture;
		Boolean						bRet;
		tErrType					err;

		// For displaying captured data
		tVideoSurf				surf;
		tDisplayHandle			disp;

		pKernelMPI_ = new CKernelMPI;
		pDisplayMPI_ = new CDisplayMPI;
		pVideoMPI_ = new CVideoMPI;

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
			capture = pCameraMPI_->StartVideoCapture(&surf, NULL, "", 0, false);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(5000);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
		}
		else
			TS_FAIL("MPI was deemed invalid");

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, true);
		delete pVideoMPI_;
		delete pDisplayMPI_;
		delete pKernelMPI_;
	}

	//------------------------------------------------------------------------
	void testCameraSelect()
	{
		PRINT_TEST_NAME();

		tVidCapHndl					capture;
		Boolean						bRet;
		tErrType					err;

		// For displaying captured data
		tVideoSurf				surf;
		tDisplayHandle			disp;
		tCameraDevice			camera;

		pKernelMPI_ = new CKernelMPI;
		pDisplayMPI_ = new CDisplayMPI;

		disp = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatYUV420);
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
			camera = pCameraMPI_->GetCurrentCamera();
			TS_ASSERT_EQUALS( camera, kCameraDefault );

			err = pCameraMPI_->SetCurrentCamera(kCameraFront);
			TS_ASSERT_EQUALS( err, kNoErr );

			camera = pCameraMPI_->GetCurrentCamera();
			TS_ASSERT_EQUALS( camera, kCameraFront );

			capture = pCameraMPI_->StartVideoCapture(&surf);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(5000);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
		}
		else
			TS_FAIL("MPI was deemed invalid");

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, true);
		delete pDisplayMPI_;
		delete pKernelMPI_;
	}

	//------------------------------------------------------------------------
	void testCapture()
	{
		PRINT_TEST_NAME();

		tVidCapHndl					capture;
		Boolean						bRet;
		tErrType					err;

		// For displaying captured data
		tVideoSurf				surf;
		tDisplayHandle			disp;

		pKernelMPI_ = new CKernelMPI;
		pDisplayMPI_ = new CDisplayMPI;
		pVideoMPI_ = new CVideoMPI;

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
			capture = pCameraMPI_->StartVideoCapture(&surf, NULL, "test_Capture.avi", 0, true);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(5000);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
		}
		else
			TS_FAIL("MPI was deemed invalid");

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, true);
		delete pVideoMPI_;
		delete pDisplayMPI_;
		delete pKernelMPI_;
	}

	//------------------------------------------------------------------------
	void testCaptureMuted()
	{
		PRINT_TEST_NAME();

		tVidCapHndl					capture;
		Boolean						bRet;
		tErrType					err;

		// For displaying captured data
		tVideoSurf				surf;
		tDisplayHandle			disp;

		pKernelMPI_ = new CKernelMPI;
		pDisplayMPI_ = new CDisplayMPI;
		pVideoMPI_ = new CVideoMPI;

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
			capture = pCameraMPI_->StartVideoCapture(&surf, NULL, "test_CaptureMuted.avi", 0, false);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(5000);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
		}
		else
			TS_FAIL("MPI was deemed invalid");

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, true);
		delete pVideoMPI_;
		delete pDisplayMPI_;
		delete pKernelMPI_;
	}

	//------------------------------------------------------------------------
	void testCaptureNoVF()
	{
		PRINT_TEST_NAME();

		tVidCapHndl					capture;
		Boolean						bRet;
		tErrType					err;

		pKernelMPI_ = new CKernelMPI;

		if ( pCameraMPI_->IsValid() )
		{
			capture = pCameraMPI_->StartVideoCapture(NULL, NULL, "test_CaptureNoVF.avi", 0, true);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(5000);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
		}
		else
			TS_FAIL("MPI was deemed invalid");

		delete pKernelMPI_;
	}

	//------------------------------------------------------------------------
	void testCaptureNoVFMuted()
	{
		PRINT_TEST_NAME();

		tVidCapHndl					capture;
		Boolean						bRet;
		tErrType					err;

		pKernelMPI_ = new CKernelMPI;

		if ( pCameraMPI_->IsValid() )
		{
			capture = pCameraMPI_->StartVideoCapture(NULL, NULL, "test_CaptureNoVFMuted.avi", 0, false);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(5000);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
		}
		else
			TS_FAIL("MPI was deemed invalid");

		delete pKernelMPI_;
	}

	//------------------------------------------------------------------------
	void testEnumFormats()
	{
		PRINT_TEST_NAME();

		tErrType					err;
		tCaptureMode*				mode;
		tCaptureModes				list;
		tCaptureModes::iterator		it;

		err = pCameraMPI_->EnumFormats(list);
		TS_ASSERT_EQUALS(err, kNoErr);

		for (it = list.begin(); it != list.end(); it++)
		{
			mode = *it;
			printf("size=%dx%d, fps=%d:%d, format=%d\n", mode->width, mode->height, (unsigned)mode->fps_numerator, (unsigned)mode->fps_denominator, (int)mode->pixelformat);
		}

		mode = pCameraMPI_->GetCurrentFormat();
		TS_ASSERT(mode != kNull);
		if (mode)
			printf("size=%dx%d, fps=%d:%d, format=%d\n", mode->width, mode->height, (unsigned)mode->fps_numerator, (unsigned)mode->fps_denominator, (int)mode->pixelformat);
	}
	
	//------------------------------------------------------------------------
	void XXXXtestSnapFrame()
	{
		PRINT_TEST_NAME();

		tVidCapHndl hndl = pCameraMPI_->StartVideoCapture(NULL);
		pCameraMPI_->SnapFrame(hndl, "snap.jpg");
		pCameraMPI_->StopVideoCapture(hndl);
	}

	//------------------------------------------------------------------------
	void testGetFrame()
	{
		PRINT_TEST_NAME();

		tErrType					err;
		tCaptureMode*				mode, entry;
		tCaptureModes				list, modes;
		tCaptureModes::iterator		it;
		tVidCapHndl 				hndl;
		tVideoSurf					surf;
		Boolean						ret;
		CImageIO					image;
		CPath						path, root = pCameraMPI_->GetCameraStillPath()->c_str();
		char						label[20];

		err = pCameraMPI_->EnumFormats(list);
		TS_ASSERT_EQUALS(err, kNoErr);

		for (it = list.begin(); it != list.end(); it++)
		{
			mode = *it;
			entry = *mode;
			modes.push_back(new tCaptureMode(entry));
		}

		for (it = modes.begin(); it != modes.end(); it++)
		{
			mode = *it;
			sprintf(label, "%dx%d", mode->width, mode->height);

			err = pCameraMPI_->SetCurrentFormat(mode);
			TS_ASSERT_EQUALS(err, kNoErr);

			hndl = pCameraMPI_->StartVideoCapture(NULL);
			TS_ASSERT_DIFFERS(hndl, kInvalidVidCapHndl);

			surf.width 	= mode->width;
			surf.height = mode->height;
			surf.pitch 	= 3 * mode->width;
			surf.format = kPixelFormatRGB888;
			surf.buffer = new U8[surf.pitch * surf.height];

			ret = pCameraMPI_->GetFrame(hndl, &surf);
			TS_ASSERT_EQUALS(ret, true);

			path = root + "GetFrame" + label + ".png";
			ret = image.Save(path, surf);
			TS_ASSERT_EQUALS(ret, true);

			delete[] surf.buffer;

			pCameraMPI_->StopVideoCapture(hndl);
//			break;
		}
	}

	//------------------------------------------------------------------------
	void testFlipFrame()
	{
		PRINT_TEST_NAME();

		Boolean						ret;
		tCaptureMode*				mode;
		tVidCapHndl 				hndl;
		tVideoSurf					surf, surf90;
		tControlInfo				ctrl;
		CImageIO					image;
		CPath						path, root = pCameraMPI_->GetCameraStillPath()->c_str();

		hndl = pCameraMPI_->StartVideoCapture(NULL);
		TS_ASSERT_DIFFERS(hndl, kInvalidVidCapHndl);

		mode = pCameraMPI_->GetCurrentFormat();
		TS_ASSERT(mode != kNull);

		surf.width 	= mode->width;
		surf.height = mode->height;
		surf.pitch 	= 3 * mode->width;
		surf.format = kPixelFormatRGB888;
		surf.buffer = new U8[surf.pitch * surf.height];

		ret = pCameraMPI_->GetFrame(hndl, &surf);
		TS_ASSERT_EQUALS(ret, true);

		ctrl.type = kControlTypeHorizontalFlip;
		ret = pCameraMPI_->SetCameraControl(&ctrl, 1);
		TS_ASSERT_EQUALS(ret, true);

		ret = pCameraMPI_->GetFrame(hndl, &surf);
		TS_ASSERT_EQUALS(ret, true);

		path = root + "FlippedHorizontal.png";
		ret = image.Save(path, surf);
		TS_ASSERT_EQUALS(ret, true);

		ctrl.type = kControlTypeVerticalFlip;
		ret = pCameraMPI_->SetCameraControl(&ctrl, 1);
		TS_ASSERT_EQUALS(ret, true);

		ret = pCameraMPI_->GetFrame(hndl, &surf);
		TS_ASSERT_EQUALS(ret, true);

		path = root + "FlippedVertical.png";
		ret = image.Save(path, surf);
		TS_ASSERT_EQUALS(ret, true);

		ctrl.type = kControlTypeRotate;
		ret = pCameraMPI_->SetCameraControl(&ctrl, 90);
		TS_ASSERT_EQUALS(ret, true);

		surf90 = surf;
		surf90.width = surf.height;
		surf90.pitch = 3 * surf90.width;
		surf90.height = surf.width;
		ret = pCameraMPI_->GetFrame(hndl, &surf90);
		TS_ASSERT_EQUALS(ret, true);

		path = root + "FlippedRotate.png";
		ret = image.Save(path, surf90);
		TS_ASSERT_EQUALS(ret, true);

		ctrl.type = kControlTypeRotate;
		ret = pCameraMPI_->SetCameraControl(&ctrl, 0);
		TS_ASSERT_EQUALS(ret, true);

		ctrl.type = kControlTypeVerticalFlip;
		ret = pCameraMPI_->SetCameraControl(&ctrl, 0);
		TS_ASSERT_EQUALS(ret, true);

		ctrl.type = kControlTypeHorizontalFlip;
		ret = pCameraMPI_->SetCameraControl(&ctrl, 0);
		TS_ASSERT_EQUALS(ret, true);

		ret = pCameraMPI_->GetFrame(hndl, &surf);
		TS_ASSERT_EQUALS(ret, true);

		path = root + "FlippedNormal.png";
		ret = image.Save(path, surf);
		TS_ASSERT_EQUALS(ret, true);

		delete[] surf.buffer;

		pCameraMPI_->StopVideoCapture(hndl);
	}

};

// EOF
