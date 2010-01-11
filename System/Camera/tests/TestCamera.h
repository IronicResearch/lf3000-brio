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
		tVidCapHndl		capture;
		tVideoSurf		surf;
		tDisplayHandle	disp;
		Boolean			bRet;
		tCaptureModes	modes;
		tRect			rect = {80, 200, 60, 180};

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

			bRet = pCameraMPI_->GetCameraModes(modes);
			TS_ASSERT_EQUALS( bRet, true );

			bRet = pCameraMPI_->SetCameraMode(modes[0]);
			TS_ASSERT_EQUALS( bRet, true );

			bRet = pCameraMPI_->SetBuffers(4);
			TS_ASSERT_EQUALS( bRet, true );

			capture = pCameraMPI_->StartVideoCapture();
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );

			capture = pCameraMPI_->StartVideoCapture("", false, &surf, &rect);
			TS_ASSERT_DIFFERS( capture, kInvalidVidCapHndl );

			bRet = pCameraMPI_->StopVideoCapture(capture);
			TS_ASSERT_EQUALS( bRet, true );
		}
	}
};

// EOF
