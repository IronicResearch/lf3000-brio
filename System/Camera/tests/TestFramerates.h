/*****************************
 * Test the framerates associated with different combinations of captures
 *
 */


#include <cxxtest/TestSuite.h>
#include <UnitTestUtils.h>

#include <CameraMPI.h>
#include <DisplayMPI.h>
#include <KernelMPI.h>

using namespace LeapFrog::Brio;

const char* capture_path = "/LF/Bulk/Data/Local/All/";

class TestCamera : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CCameraMPI*		pCameraMPI_;
	CDisplayMPI*		pDisplayMPI_;
	CKernelMPI*		pKernelMPI_;

	tVidCapHndl					capture;
	Boolean						bRet;
	tErrType					err;

	// For viewfinder
	tVideoSurf				surf;
	tDisplayHandle			disp;

public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		pCameraMPI_ = new CCameraMPI;
		pDisplayMPI_ = new CDisplayMPI;
		pKernelMPI_ = new CKernelMPI;
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete pCameraMPI_;
		delete pDisplayMPI_;
		delete pKernelMPI_;
	}
	
	void testYUV_WithAudio()
	{
		PRINT_TEST_NAME();
		
		//Setup viewfinder surface
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

			capture = pCameraMPI_->StartVideoCapture(&surf, NULL, "YUV_WithAudio.avi", 0, true);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(60000);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
			
		}
		else
			TS_FAIL("MPI was deemed invalid");

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, true);
	}
	
	void testYUV_WithoutAudio()
	{
		PRINT_TEST_NAME();
		
		//Setup viewfinder surface
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

			capture = pCameraMPI_->StartVideoCapture(&surf, NULL, "YUV_WithoutAudio.avi", 0, false);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(60000);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
			
		}
		else
			TS_FAIL("MPI was deemed invalid");

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, true);
	}
	
	void testRGB_WithAudio()
	{
		PRINT_TEST_NAME();
		
		//Setup viewfinder surface
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

			capture = pCameraMPI_->StartVideoCapture(&surf, NULL, "RGB_WithAudio.avi", 0, true);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(60000);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
			
		}
		else
			TS_FAIL("MPI was deemed invalid");

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, true);
	}

	void testRGB_WithoutAudio()
	{
		PRINT_TEST_NAME();
		
		//Setup viewfinder surface
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

			capture = pCameraMPI_->StartVideoCapture(&surf, NULL, "RGB_WithoutAudio.avi", 0, false);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(60000);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
			
		}
		else
			TS_FAIL("MPI was deemed invalid");

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, true);
	}
	
	void testNone_WithAudio()
	{
		PRINT_TEST_NAME();
		
		if ( pCameraMPI_->IsValid() )
		{
			err = pCameraMPI_->SetCameraVideoPath(capture_path);
			TS_ASSERT_EQUALS( err, kNoErr );

			capture = pCameraMPI_->StartVideoCapture(NULL, NULL, "None_WithAudio.avi", 0, true);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(60000);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
			
		}
		else
			TS_FAIL("MPI was deemed invalid");
	}
	
	void testNone_WithoutAudio()
	{
		PRINT_TEST_NAME();
		
		if ( pCameraMPI_->IsValid() )
		{
			err = pCameraMPI_->SetCameraVideoPath(capture_path);
			TS_ASSERT_EQUALS( err, kNoErr );

			capture = pCameraMPI_->StartVideoCapture(NULL, NULL, "None_WithoutAudio.avi", 0, false);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			pKernelMPI_->TaskSleep(60000);

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
			
		}
		else
			TS_FAIL("MPI was deemed invalid");
	}
};
