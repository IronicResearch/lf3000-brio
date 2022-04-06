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

LF_USING_BRIO_NAMESPACE()

const tDebugSignature kMyApp = kTestSuiteDebugSig;
const tEventType LocalCameraEvents[] = {kAllCameraEvents};

const char* capture_path = "/LF/Bulk/Data/Local/All/";

//============================================================================
// TestCameraMPI functions
//============================================================================
class TestCameraMicrophone : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CCameraMPI*		pCameraMPI_;
	CKernelMPI*		pKernelMPI_;
	tEventType		reason;

	//============================================================================
	// Local event listener
	//============================================================================
	class CameraListener : public IEventListener
	{
	private:
		tEventType	reason;
		U32 		length;
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
	void testCaptureWAV()
	{
		PRINT_TEST_NAME();

		tAudCapHndl					capture;
		Boolean						bRet;
		tErrType					err;

		pKernelMPI_ = new CKernelMPI;

		if ( pCameraMPI_->IsValid() )
		{
			err = pCameraMPI_->SetCameraAudioPath(capture_path);
			TS_ASSERT_EQUALS( err, kNoErr );

			capture = pCameraMPI_->StartAudioCapture("test.wav", NULL);
			TS_ASSERT_DIFFERS( capture, kInvalidAudCapHndl );

			pKernelMPI_->TaskSleep(5000);

			bRet = pCameraMPI_->StopAudioCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
		}
		else
			TS_FAIL("MPI was deemed invalid");

		delete pKernelMPI_;
	}

	//------------------------------------------------------------------------
	void testCaptureWAVEvent()
	{
		PRINT_TEST_NAME();

		tAudCapHndl					capture;
		Boolean						bRet;
		tErrType					err;

		CameraListener			listener;

		pKernelMPI_ = new CKernelMPI;

		if ( pCameraMPI_->IsValid() )
		{
			err = pCameraMPI_->SetCameraAudioPath(capture_path);
			TS_ASSERT_EQUALS( err, kNoErr );

			capture = pCameraMPI_->StartAudioCapture("testEvent.wav", &listener, 3);
			TS_ASSERT_DIFFERS( capture, kInvalidAudCapHndl );

			pKernelMPI_->TaskSleep(7000);
			TS_ASSERT_EQUALS( listener.GetReason(), kCaptureTimeOutEvent );

			listener.Reset();

			bRet = pCameraMPI_->StopAudioCapture(capture);
			TS_ASSERT_EQUALS( bRet, false );

			capture = pCameraMPI_->StartAudioCapture("testEvent2.wav", &listener, 3);
			TS_ASSERT_DIFFERS( capture, kInvalidAudCapHndl );

			pKernelMPI_->TaskSleep(1000);

			bRet = pCameraMPI_->StopAudioCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
			TS_ASSERT_EQUALS( listener.GetReason(), kCaptureStoppedEvent );
		}
		else
			TS_FAIL("MPI was deemed invalid");

		delete pKernelMPI_;
	}

	//------------------------------------------------------------------------
	void testCaptureWAVPause()
	{
		PRINT_TEST_NAME();

		tAudCapHndl					capture;
		Boolean						bRet;
		tErrType					err;

		CameraListener			listener;

		pKernelMPI_ = new CKernelMPI;

		if ( pCameraMPI_->IsValid() )
		{
			err = pCameraMPI_->SetCameraAudioPath(capture_path);
			TS_ASSERT_EQUALS( err, kNoErr );

			capture = pCameraMPI_->StartAudioCapture("testPause.wav", NULL);
			TS_ASSERT_DIFFERS( capture, kInvalidAudCapHndl );

			pKernelMPI_->TaskSleep(2000);

			bRet = pCameraMPI_->IsAudioCapturePaused(capture);
			TS_ASSERT_EQUALS( bRet, false );

			bRet = pCameraMPI_->PauseAudioCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			bRet = pCameraMPI_->IsAudioCapturePaused(capture);
			TS_ASSERT_EQUALS( bRet, true );

			pKernelMPI_->TaskSleep(2000);

			bRet = pCameraMPI_->ResumeAudioCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			bRet = pCameraMPI_->IsAudioCapturePaused(capture);
			TS_ASSERT_EQUALS( bRet, false );

			pKernelMPI_->TaskSleep(2000);

			bRet = pCameraMPI_->StopAudioCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			capture = pCameraMPI_->StartAudioCapture("testPause2.wav", NULL, 0, true);
			TS_ASSERT_DIFFERS( capture, kInvalidAudCapHndl );

			pKernelMPI_->TaskSleep(2000);

			bRet = pCameraMPI_->IsAudioCapturePaused(capture);
			TS_ASSERT_EQUALS( bRet, true );

			bRet = pCameraMPI_->ResumeAudioCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			bRet = pCameraMPI_->IsAudioCapturePaused(capture);
			TS_ASSERT_EQUALS( bRet, false );

			pKernelMPI_->TaskSleep(2000);

			bRet = pCameraMPI_->StopAudioCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

		}
		else
			TS_FAIL("MPI was deemed invalid");

		delete pKernelMPI_;
	}

};

// EOF
