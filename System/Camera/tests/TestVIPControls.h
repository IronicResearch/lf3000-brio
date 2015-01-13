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

#define WIDTH 	400
#define HEIGHT	300
#define DELAY	200

inline U32 GetFrameCount()
{
	unsigned int framecount = 0;
	FILE* fp = fopen("/sys/devices/platform/vip.0/frame_count", "r");
	if (!fp)
		fp = fopen("/sys/devices/platform/nxp-v4l2.0/clipper_count", "r");
	if (fp) {
		fscanf(fp, "%u", &framecount);
		fclose(fp);
	}
	return framecount;
}

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

			if (reason == kCaptureFrameEvent)
			{
				printf(".");
				length = msg.data.framed.frame; // overloaded meaning as frame count
				return kEventStatusOKConsumed;
			}
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
	void testEnumControls()
	{
		PRINT_TEST_NAME();

		Boolean						bRet;
		tCameraControls				controls;
		tCameraControls::iterator	it;
		tControlInfo				*ctrl;

		bRet = pCameraMPI_->GetCameraControls(controls);
		TS_ASSERT_EQUALS( bRet, true );

		for (it = controls.begin(); it != controls.end(); it++)
		{
			ctrl = *it;
			printf("ctrl=%d, min=%d, max=%d, preset=%d, current=%d\n", (int)ctrl->type, (int)ctrl->min, (int)ctrl->max, (int)ctrl->preset, (int)ctrl->current);
		}
	}

	//------------------------------------------------------------------------
	void XXXXtestBrightness()
	{
		PRINT_TEST_NAME();

		tVidCapHndl					capture;
		Boolean						bRet;
		tErrType					err;
		tVideoSurf					surf;
		tDisplayHandle				disp;
		tControlInfo				ctrl;
		tControlType				types[3] = { kControlTypeBrightness, kControlTypeContrast, kControlTypeSaturation };
		tCaptureMode*				mode;
		tCameraControls				controls;
		tCameraControls::iterator	it;

		pKernelMPI_ = new CKernelMPI;
		pDisplayMPI_ = new CDisplayMPI;
		CameraListener* pListener = new CameraListener;

		disp = pDisplayMPI_->CreateHandle(HEIGHT, WIDTH, kPixelFormatYUV420);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, kDisplayOnTop);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);
		TS_ASSERT( surf.format == kPixelFormatYUV420 );

		mode = pCameraMPI_->GetCurrentFormat();
		mode->width  = WIDTH;
		mode->height = HEIGHT;
		err = pCameraMPI_->SetCurrentFormat(mode);
		TS_ASSERT_EQUALS( err, kNoErr );

		bRet = pCameraMPI_->GetCameraControls(controls);
		TS_ASSERT_EQUALS( bRet, true );

		if ( pCameraMPI_->IsValid() )
		{
			capture = pCameraMPI_->StartVideoCapture(&surf);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			for (int j = 0; j < 3; j++)
			{
				for (it = controls.begin(); it != controls.end(); it++)
				{
					tControlInfo* ctrlx = *it;
					if (ctrlx->type == types[j])
					{
						ctrl = *ctrlx;
						break;
					}
				}
				if (ctrl.type != types[j])
					continue;
				for (S32 i = ctrl.preset; i <= ctrl.max; i++)
				{
					bRet = pCameraMPI_->SetCameraControl(&ctrl, i);
					TS_ASSERT_EQUALS( bRet, true );
					pKernelMPI_->TaskSleep(DELAY);
				}
				for (S32 i = ctrl.max; i >= ctrl.min; i--)
				{
					bRet = pCameraMPI_->SetCameraControl(&ctrl, i);
					TS_ASSERT_EQUALS( bRet, true );
					pKernelMPI_->TaskSleep(DELAY);
				}
				for (S32 i = ctrl.min; i <= ctrl.preset; i++)
				{
					bRet = pCameraMPI_->SetCameraControl(&ctrl, i);
					TS_ASSERT_EQUALS( bRet, true );
					pKernelMPI_->TaskSleep(DELAY);
				}
			}

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
		}
		else
			TS_FAIL("MPI was deemed invalid");

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, true);
		delete pListener;
		delete pDisplayMPI_;
		delete pKernelMPI_;
	}

	//------------------------------------------------------------------------
	void XXXXtestAutoWhiteBalance()
	{
		PRINT_TEST_NAME();

		tVidCapHndl					capture;
		Boolean						bRet;
		tErrType					err;
		tVideoSurf					surf;
		tDisplayHandle				disp;
		tControlInfo				ctrl;
		tCaptureMode*				mode;

		pKernelMPI_ = new CKernelMPI;
		pDisplayMPI_ = new CDisplayMPI;
		CameraListener* pListener = new CameraListener;

		disp = pDisplayMPI_->CreateHandle(HEIGHT, WIDTH, kPixelFormatYUV420);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, kDisplayOnTop);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);
		TS_ASSERT( surf.format == kPixelFormatYUV420 );

		mode = pCameraMPI_->GetCurrentFormat();
		mode->width  = WIDTH;
		mode->height = HEIGHT;
		err = pCameraMPI_->SetCurrentFormat(mode);
		TS_ASSERT_EQUALS( err, kNoErr );

		if ( pCameraMPI_->IsValid() )
		{
			capture = pCameraMPI_->StartVideoCapture(&surf);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			ctrl.type = kControlTypeAutoWhiteBalance;
			bRet = pCameraMPI_->SetCameraControl(&ctrl, 1);
			TS_ASSERT_EQUALS( bRet, true );
			pKernelMPI_->TaskSleep(5000);

			bRet = pCameraMPI_->SetCameraControl(&ctrl, 0);
			TS_ASSERT_EQUALS( bRet, true );
			pKernelMPI_->TaskSleep(5000);

			bRet = pCameraMPI_->SetCameraControl(&ctrl, 1);
			TS_ASSERT_EQUALS( bRet, true );
			pKernelMPI_->TaskSleep(5000);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
		}
		else
			TS_FAIL("MPI was deemed invalid");

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, true);
		delete pListener;
		delete pDisplayMPI_;
		delete pKernelMPI_;
	}

	//------------------------------------------------------------------------
	void testOverlayEvents()
	{
		PRINT_TEST_NAME();

		tVidCapHndl					capture;
		Boolean						bRet;
		tErrType					err;
		tVideoSurf					surf;
		tDisplayHandle				disp;
		tCaptureMode*				mode;
		U32							marktime;
		U32							framecount;

		pKernelMPI_ = new CKernelMPI;
		pDisplayMPI_ = new CDisplayMPI;
		CameraListener* pListener = new CameraListener;

		disp = pDisplayMPI_->CreateHandle(HEIGHT, WIDTH, kPixelFormatYUV420);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, kDisplayOnTop, 0);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);
		TS_ASSERT( surf.format == kPixelFormatYUV420 );

		mode = pCameraMPI_->GetCurrentFormat();
		mode->width  = WIDTH;
		mode->height = HEIGHT;
		mode->fps_denominator = 30;
		err = pCameraMPI_->SetCurrentFormat(mode);
		TS_ASSERT_EQUALS( err, kNoErr );

		if ( pCameraMPI_->IsValid() )
		{
			capture = pCameraMPI_->StartVideoCapture(&surf, pListener, "", 0, false);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			framecount = GetFrameCount();
			marktime = pKernelMPI_->GetElapsedTimeAsMSecs();
			pKernelMPI_->TaskSleep(5000);
			marktime = pKernelMPI_->GetElapsedTimeAsMSecs() - marktime;

			framecount = GetFrameCount() - framecount;
			printf("\nframe count = %d, frames per sec = %d.%d\n", (int)framecount, (int)(1000 * framecount / marktime), (int)(1000 * framecount % marktime));

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			capture = pCameraMPI_->StartVideoCapture(&surf, NULL, "", 0, false);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			framecount = GetFrameCount();
			marktime = pKernelMPI_->GetElapsedTimeAsMSecs();
			pKernelMPI_->TaskSleep(5000);
			marktime = pKernelMPI_->GetElapsedTimeAsMSecs() - marktime;

			framecount = GetFrameCount() - framecount;
			printf("\nframe count = %d, frames per sec = %d.%d\n", (int)framecount, (int)(1000 * framecount / marktime), (int)(1000 * framecount % marktime));

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
		}
		else
			TS_FAIL("MPI was deemed invalid");

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, true);
		delete pListener;
		delete pDisplayMPI_;
		delete pKernelMPI_;
	}

};

// EOF
