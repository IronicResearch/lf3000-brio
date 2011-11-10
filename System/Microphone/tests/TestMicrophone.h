// TestMicrophoneMPI.h

#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <MicrophoneMPI.h>
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
// TestMicrophoneMPI functions
//============================================================================
class TestMicrophone : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CMicrophoneMPI*		pMicrophoneMPI_;
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
			const CMicrophoneEventMessage& msg = dynamic_cast<const CMicrophoneEventMessage&>(Imsg);

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
			if(reason == kMicrophoneRemovedEvent)
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
		pMicrophoneMPI_ = new CMicrophoneMPI;
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete pMicrophoneMPI_;
	}

	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		PRINT_TEST_NAME();
		TS_ASSERT( pMicrophoneMPI_ != NULL );
		TS_ASSERT( pMicrophoneMPI_->IsValid() == true );
	}

	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		PRINT_TEST_NAME();

		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;

		if ( pMicrophoneMPI_->IsValid() ) {
			pName = pMicrophoneMPI_->GetMPIName();
			TS_ASSERT_EQUALS( *pName, "MicrophoneMPI" );
			version = pMicrophoneMPI_->GetModuleVersion();
			TS_ASSERT_EQUALS( version, 2 );
			pName = pMicrophoneMPI_->GetModuleName();
			TS_ASSERT_EQUALS( *pName, "Microphone" );
			pURI = pMicrophoneMPI_->GetModuleOrigin();
			TS_ASSERT_EQUALS( *pURI, "/LF/System/Microphone" );
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

		if ( pMicrophoneMPI_->IsValid() )
		{
			err = pMicrophoneMPI_->SetAudioPath(capture_path);
			TS_ASSERT_EQUALS( err, kNoErr );

			capture = pMicrophoneMPI_->StartAudioCapture("test.wav", NULL);
			TS_ASSERT_DIFFERS( capture, kInvalidAudCapHndl );

			pKernelMPI_->TaskSleep(5000);

			bRet = pMicrophoneMPI_->StopAudioCapture(capture);
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

		if ( pMicrophoneMPI_->IsValid() )
		{
			err = pMicrophoneMPI_->SetAudioPath(capture_path);
			TS_ASSERT_EQUALS( err, kNoErr );

			capture = pMicrophoneMPI_->StartAudioCapture("testEvent.wav", &listener, 3);
			TS_ASSERT_DIFFERS( capture, kInvalidAudCapHndl );

			pKernelMPI_->TaskSleep(7000);
			TS_ASSERT_EQUALS( listener.GetReason(), kCaptureTimeOutEvent );

			listener.Reset();

			bRet = pMicrophoneMPI_->StopAudioCapture(capture);
			TS_ASSERT_EQUALS( bRet, false );

			capture = pMicrophoneMPI_->StartAudioCapture("testEvent2.wav", &listener, 3);
			TS_ASSERT_DIFFERS( capture, kInvalidAudCapHndl );

			pKernelMPI_->TaskSleep(1000);

			bRet = pMicrophoneMPI_->StopAudioCapture(capture);
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

		if ( pMicrophoneMPI_->IsValid() )
		{
			err = pMicrophoneMPI_->SetAudioPath(capture_path);
			TS_ASSERT_EQUALS( err, kNoErr );

			capture = pMicrophoneMPI_->StartAudioCapture("testPause.wav", NULL);
			TS_ASSERT_DIFFERS( capture, kInvalidAudCapHndl );

			pKernelMPI_->TaskSleep(2000);

			bRet = pMicrophoneMPI_->IsAudioCapturePaused(capture);
			TS_ASSERT_EQUALS( bRet, false );

			bRet = pMicrophoneMPI_->PauseAudioCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			bRet = pMicrophoneMPI_->IsAudioCapturePaused(capture);
			TS_ASSERT_EQUALS( bRet, true );

			pKernelMPI_->TaskSleep(2000);

			bRet = pMicrophoneMPI_->ResumeAudioCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			bRet = pMicrophoneMPI_->IsAudioCapturePaused(capture);
			TS_ASSERT_EQUALS( bRet, false );

			pKernelMPI_->TaskSleep(2000);

			bRet = pMicrophoneMPI_->StopAudioCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			capture = pMicrophoneMPI_->StartAudioCapture("testPause2.wav", NULL, 0, true);
			TS_ASSERT_DIFFERS( capture, kInvalidAudCapHndl );

			pKernelMPI_->TaskSleep(2000);

			bRet = pMicrophoneMPI_->IsAudioCapturePaused(capture);
			TS_ASSERT_EQUALS( bRet, true );

			bRet = pMicrophoneMPI_->ResumeAudioCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			bRet = pMicrophoneMPI_->IsAudioCapturePaused(capture);
			TS_ASSERT_EQUALS( bRet, false );

			pKernelMPI_->TaskSleep(2000);

			bRet = pMicrophoneMPI_->StopAudioCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

		}
		else
			TS_FAIL("MPI was deemed invalid");

		delete pKernelMPI_;
	}

};

// EOF
