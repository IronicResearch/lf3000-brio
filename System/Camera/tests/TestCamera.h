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

#include <sstream>

//For strcmp
#include <string.h>

LF_USING_BRIO_NAMESPACE()

const tDebugSignature kMyApp = kTestSuiteDebugSig;
const tEventType LocalCameraEvents[] = {kAllCameraEvents};

char* GetControlString(tControlType control)
{
	switch(control)
	{
		case kControlTypeBrightness:
			return "Brightness";
		case kControlTypeContrast:
			return "Contrast";
		case kControlTypeSaturation:
			return "Saturation";
		case kControlTypeHue:
			return "Hue";
		case kControlTypeGamma:
			return "Gamma";
		case kControlPowerLineFreq:
			return "PowerLineFreq";
		case kControlTypeSharpness:
			return "Sharpness";
		case kControlTypeBacklightComp:
			return "BacklightComp";
	}
	return "Mising Setting String";
}

const char* capture_path = "/LF/Base/L3B_Video";

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
	void testSetGetPaths()
	{
		PRINT_TEST_NAME();
		//Set and then get each path
		
		//Video Path
		tErrType err = pCameraMPI_->SetCameraVideoPath(capture_path);
		TS_ASSERT_EQUALS(err, kNoErr);
		const char* retrieved_video_path = pCameraMPI_->GetCameraVideoPath()->c_str();
		TSM_ASSERT_EQUALS(retrieved_video_path, strcmp(retrieved_video_path, capture_path), 0);
		
		//Still Image Path
		err = pCameraMPI_->SetCameraStillPath(capture_path);
		TS_ASSERT_EQUALS(err, kNoErr);
		const char* retrieved_still_path = pCameraMPI_->GetCameraStillPath()->c_str();
		TSM_ASSERT_EQUALS(retrieved_still_path, strcmp(retrieved_still_path, capture_path), 0);
		
		//TODO: Audio is scheduled for later, re-enable tests when that happens
		//~ err = pCameraMPI_->SetCameraAudioPath(capture_path);
		//~ TS_ASSERT_EQUALS(err, kNoErr);
		//~ TSM_ASSERT_EQUALS(pCameraMPI_->GetCameraAudioPath()->c_str(), strcmp(pCameraMPI_->GetCameraAudioPath()->c_str(), capture_path), 0);
		
	}
	
	//------------------------------------------------------------------------
	void testCaptureYUV()
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
		
		//Sleep for 5 seconds to allow for hotplug testing if the tester desires
		printf("Sleeping for 5 seconds to give a chance to connect/disconnect camera\n");
		pKernelMPI_->TaskSleep(5000);
		
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
			err = pCameraMPI_->SetCameraVideoPath(capture_path);
			TS_ASSERT_EQUALS( err, kNoErr );

			capture = pCameraMPI_->StartVideoCapture(&surf, NULL, "testYUV.avi");
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(5000);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
			
			//Check to make sure new video file exists
			struct stat file_status;
			TS_ASSERT(! stat("/LF/Base/L3B_Video/testYUV.avi", &file_status) );
		}
		else
			TS_FAIL("MPI was deemed invalid");

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, true);
		delete pDisplayMPI_;
		delete pKernelMPI_;
	}

	//------------------------------------------------------------------------
	void testCaptureRGB()
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
			err = pCameraMPI_->SetCameraVideoPath(capture_path);
			TS_ASSERT_EQUALS( err, kNoErr );

			capture = pCameraMPI_->StartVideoCapture(&surf, NULL, "testRGB.avi");
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(5000);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
			
			//Check to make sure new video file exists
			struct stat file_status;
			TS_ASSERT(! stat("/LF/Base/L3B_Video/testRGB.avi", &file_status) );
		}
		else
			TS_FAIL("MPI was deemed invalid");

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, true);
		delete pDisplayMPI_;
		delete pKernelMPI_;
	}

	//------------------------------------------------------------------------
	void testCaptureEvent()
	{
		PRINT_TEST_NAME();
		
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
			err = pCameraMPI_->SetCameraVideoPath(capture_path);
			TS_ASSERT_EQUALS( err, kNoErr );

			//Capture will automatically stop after 2 seconds
			capture = pCameraMPI_->StartVideoCapture(&surf, &listener, "", 2);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			//Sleeping for 2.5 seconds will result in a timeout
			pKernelMPI_->TaskSleep(2500);
			TS_ASSERT_EQUALS( listener.GetReason(), kCaptureTimeOutEvent );

			listener.Reset();

			//Unlimited recording
			capture = pCameraMPI_->StartVideoCapture(&surf, &listener, "");
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(2500);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
			TS_ASSERT_EQUALS( listener.GetReason(), kCaptureStoppedEvent );
		}
		else
			TS_FAIL("MPI was deemed invalid");

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, true);
		delete pDisplayMPI_;
		delete pKernelMPI_;
	}

	//------------------------------------------------------------------------
	void testSetGetControlValues()
	{
		PRINT_TEST_NAME();
		
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

			//Set/Get each control setting
			for(it = controls.begin(); it < controls.end(); it++)
			{
				//Save current value so we can restore it later
				S32 backup_value = (*it)->current;
				
				for(S32 new_value = (*it)->min; new_value <= (*it)->max; new_value++)
				{
					//Set new value
					bRet = pCameraMPI_->SetCameraControl(*it, new_value);
					TS_ASSERT_EQUALS( bRet, true );
					
					//Check to ensure value was set
					tCameraControls	verify_controls;
					tCameraControls::iterator	verify_it;
					bRet = pCameraMPI_->GetCameraControls(verify_controls);
					TS_ASSERT_EQUALS( bRet, true );
					for(verify_it = verify_controls.begin(); verify_it != verify_controls.end(); verify_it++)
					{
						if( (*verify_it)->type == (*it)->type )
						{
							char msg[32];
							sprintf(msg, "Control Type: %i", (*it)->type);
							TSM_ASSERT_EQUALS(msg, (*verify_it)->current, new_value);
						}
					}
				}
				
				//Restore old value
				bRet = pCameraMPI_->SetCameraControl( *it, backup_value );
				TS_ASSERT_EQUALS( bRet, true );
			}

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
	void testVisualizeControlSettings()
	{
		//This is a test meant to be seen during execution
		//It will change different controls and give time for the tester to make a visual confirmation
		//It will also take snapshots at different control settings for later confirmation
		tVidCapHndl					capture;
		Boolean						bRet;

		tCameraControls				controls;
		tCameraControls::iterator	it;

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
			//Set path for taking snapshots
			tErrType err = pCameraMPI_->SetCameraStillPath(capture_path);
			TS_ASSERT_EQUALS( err, kNoErr );
			
			bRet = pCameraMPI_->GetCameraControls(controls);
			TS_ASSERT_EQUALS( bRet, true );

			capture = pCameraMPI_->StartVideoCapture(&surf);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			//Take a reference image at default settings
			bRet = pCameraMPI_->SnapFrame(capture, "Defaults.jpg" );
			TS_ASSERT_EQUALS( bRet, true );
			
			//Set/Get each control setting
			for(it = controls.begin(); it < controls.end(); it++)
			{
				//Save current value so we can restore it later
				S32 backup_value = (*it)->current;
				S32 step = MAX( ( ( (*it)->max - (*it)->min ) / 20), 1);
				for(S32 new_value = (*it)->min; new_value <= (*it)->max; new_value += step)
				{
					//Set new value
					bRet = pCameraMPI_->SetCameraControl(*it, new_value);
					TS_ASSERT_EQUALS( bRet, true );
					
					//Show for quarter of a second
					pKernelMPI_->TaskSleep(250);
					
					//Take snapshot
					std::stringstream filename;
					filename<<GetControlString( (*it)->type)<<"_"<<(int)new_value<<".jpg";
					printf("%s\n", filename.str().c_str());
					bRet = pCameraMPI_->SnapFrame(capture, filename.str().c_str() );
					TS_ASSERT_EQUALS( bRet, true );
					
					//Show for another quarter of a second
					pKernelMPI_->TaskSleep(250);
				}
				
				//Restore old value
				bRet = pCameraMPI_->SetCameraControl( *it, backup_value );
				TS_ASSERT_EQUALS( bRet, true );
			}
		}

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, true);
		delete pDisplayMPI_;
		delete pKernelMPI_;
	}
	
	//------------------------------------------------------------------------
	void testPause()
	{
		PRINT_TEST_NAME();
		
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
		else
			TS_FAIL("MPI was deemed invalid");

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, true);
		delete pDisplayMPI_;
		delete pKernelMPI_;
	}

	//------------------------------------------------------------------------
	void testSnap()
	{
		PRINT_TEST_NAME();
		
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
			err = pCameraMPI_->SetCameraStillPath(capture_path);
			TS_ASSERT_EQUALS( err, kNoErr );

			capture = pCameraMPI_->StartVideoCapture(&surf);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(1000);

			bRet = pCameraMPI_->SnapFrame(capture, "test.jpg");
			TS_ASSERT_EQUALS( bRet, true );
			
			pKernelMPI_->TaskSleep(4000);
			
			//Check to make sure new picture file exists
			struct stat file_status;
			TS_ASSERT(! stat("/LF/Base/L3B_Video/test.jpg", &file_status) );

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			bRet = pCameraMPI_->RenderFrame("test.jpg", &surf);
			TS_ASSERT_EQUALS( bRet, true );

			pKernelMPI_->TaskSleep(5000);
		}
		else
			TS_FAIL("MPI was deemed invalid");

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, true);
		delete pDisplayMPI_;
		delete pKernelMPI_;
	}

};

// EOF
