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


LF_USING_BRIO_NAMESPACE()

const tDebugSignature kMyApp = kTestSuiteDebugSig;
const tEventType LocalCameraEvents[] = {kAllCameraEvents};

const char* GetControlString(tControlType control)
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
		
		//Audio Path
		err = pCameraMPI_->SetCameraAudioPath(capture_path);
		TS_ASSERT_EQUALS(err, kNoErr);
		const char* retrieved_audio_path = pCameraMPI_->GetCameraAudioPath()->c_str();
		TSM_ASSERT_EQUALS(retrieved_audio_path, strcmp(retrieved_audio_path, capture_path), 0);		
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
		pVideoMPI_ = new CVideoMPI;
		
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

			capture = pCameraMPI_->StartVideoCapture(&surf, NULL, "testYUV.avi", 0, false);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(5000);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
			
			//Try to play back video file using VideoMPI
			//NOTE: This sleep is neccessary between ending video capture and
			//starting playback, but it's not documented
			//pKernelMPI_->TaskSleep(500);
			printf("Playing back video\n");
			pVideoMPI_->SetVideoResourcePath(capture_path);
			tVideoHndl playback = pVideoMPI_->StartVideo("testYUV.avi", &surf);
			TS_ASSERT( playback != kInvalidVideoHndl);
			
			//Wait for video to play for a few seconds
			pKernelMPI_->TaskSleep(3000);
			
			//Stop video playback
			TS_ASSERT(pVideoMPI_->StopVideo(playback) == true);
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
		pVideoMPI_ = new CVideoMPI;
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
			
			//Try to play back video file using VideoMPI
			//NOTE: This sleep is neccessary between ending video capture and
			//starting playback, but it's not documented
			//pKernelMPI_->TaskSleep(100);
			
			pVideoMPI_->SetVideoResourcePath(capture_path);
			tVideoHndl playback = pVideoMPI_->StartVideo("testRGB.avi", "testRGB.avi", &surf);
			TS_ASSERT( playback != kInvalidVideoHndl);
			
			//Wait for video to play for a few seconds
			pKernelMPI_->TaskSleep(3000);
			
			//Stop video playback
			TS_ASSERT(pVideoMPI_->StopVideo(playback) == true);
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
	void testCaptureNoPreview()
	{
		PRINT_TEST_NAME();
		
		tVidCapHndl					capture;
		Boolean						bRet;
		tErrType					err;

		pKernelMPI_ = new CKernelMPI;

		if ( pCameraMPI_->IsValid() )
		{
			err = pCameraMPI_->SetCameraVideoPath(capture_path);
			TS_ASSERT_EQUALS( err, kNoErr );

			capture = pCameraMPI_->StartVideoCapture(NULL, NULL, "testNoPreview.avi");
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(5000);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
			
			//Check to make sure new video file exists
			struct stat file_status;
			TS_ASSERT(! stat("/LF/Bulk/Data/Local/All/testNoPreview.avi", &file_status) );
		}
		else
			TS_FAIL("MPI was deemed invalid");

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

			bRet = pCameraMPI_->StopVideoCapture(capture);
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
		PRINT_TEST_NAME();
		
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

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
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
	void testGetFrame()
	{
		PRINT_TEST_NAME();

		tVidCapHndl					capture;
		Boolean						bRet;
		tErrType					err;
		U8							*pixels, *src, *dest, R, G, B;
		const U16					ROWS = 480, COLS = 640;
		int							row, col;

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
			pixels = (U8*)(pKernelMPI_->Malloc(ROWS*COLS*3));

			capture = pCameraMPI_->StartVideoCapture(&surf);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(3000);

			bRet = pCameraMPI_->PauseVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			pKernelMPI_->TaskSleep(2000);

			bRet = pCameraMPI_->GetFrame(capture, pixels, kDisplayRgb);
			TS_ASSERT_EQUALS( bRet, true );

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			/* plot a + through the bitmap */
			for(row = 0; row < ROWS; row++)
			{
				for(col = 0; col < COLS; col++)
				{
					if((row >= 230 && row <= 250) || (col >= 310 && col <= 330))
					{
						pixels[(row*COLS*3)+(col*3)+0] = 0xFF;	/* Red component */
						pixels[(row*COLS*3)+(col*3)+1] = 0xFF;	/* Green component */
						pixels[(row*COLS*3)+(col*3)+2] = 0xFF;	/* Blue component */
					}
				}
			}

			/* crudely shrink bitmap from VGA to QVGA in place */
			src = dest = pixels;

			for(row = 0; row < ROWS; row+=2)
			{
				for(col = 0; col < COLS; col+=2)
				{
					*dest++ = *src++;	/* Red */
					*dest++ = *src++;	/* Green */
					*dest++ = *src++;	/* Blue */

					/* skip every other pixel */
					src++;
					src++;
					src++;
				}

				/* skip every other row */
				src += COLS*3;
			}

			/* draw shrunken image to screen */
			pDisplayMPI_->UnRegister(disp, 0);
			pDisplayMPI_->DestroyHandle(disp, true);

			disp = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatRGB888, pixels);
			pDisplayMPI_->Register(disp, 0, 0, kDisplayOnTop, 0);

			pDisplayMPI_->Invalidate(0);
			pKernelMPI_->TaskSleep(3000);

			pKernelMPI_->Free(pixels);
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
			TS_ASSERT(! stat("/LF/Bulk/Data/Local/All/test.jpg", &file_status) );

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			bRet = pCameraMPI_->RenderFrame("test.jpg", &surf);
			TS_ASSERT_EQUALS( bRet, true );

			bRet = pCameraMPI_->RenderFrame("nonexistant.jpg", &surf);
			TS_ASSERT_EQUALS( bRet, false );
			
			pKernelMPI_->TaskSleep(5000);

			capture = pCameraMPI_->StartVideoCapture(&surf);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			bRet = pCameraMPI_->SnapFrame(capture, "test.png");
			TS_ASSERT_EQUALS( bRet, true );

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			pKernelMPI_->TaskSleep(1000);
			TS_ASSERT(! stat("/LF/Bulk/Data/Local/All/test.png", &file_status) );
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
