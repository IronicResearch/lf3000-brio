// TestVideoMPI.h

#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <VideoMPI.h>
#include <ResourceMPI.h>
#include <UnitTestUtils.h>

LF_USING_BRIO_NAMESPACE()

const tDebugSignature kMyApp = kFirstCartridge1DebugSig;

//============================================================================
// TestVideoMPI functions
//============================================================================
class TestVideo : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CVideoMPI*		pVideoMPI_;
	CResourceMPI*	pResourceMPI_;
public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		pVideoMPI_ = new CVideoMPI;
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete pVideoMPI_; 
	}

	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		TS_ASSERT( pVideoMPI_ != NULL );
		TS_ASSERT( pVideoMPI_->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;
		
		if ( pVideoMPI_->IsValid() ) {
			pName = pVideoMPI_->GetMPIName();
			TS_ASSERT_EQUALS( *pName, "VideoMPI" );
			version = pVideoMPI_->GetModuleVersion();
			TS_ASSERT_EQUALS( version, 2 );
			pName = pVideoMPI_->GetModuleName();
			TS_ASSERT_EQUALS( *pName, "Video" );
			pURI = pVideoMPI_->GetModuleOrigin();
			TS_ASSERT_EQUALS( *pURI, "/LF/System/Video" );
		}
	}
	
	//------------------------------------------------------------------------
	void testVideoResources()
	{
		tRsrcHndl	pkg;
		tRsrcHndl	handle1;
		tRsrcHndl	handle2;
		tVideoHndl	video;

		pResourceMPI_ = new CResourceMPI;
		pResourceMPI_->OpenAllDevices();
		pResourceMPI_->SetDefaultURIPath("LF/Brio/UnitTest/Video");

		pkg = pResourceMPI_->FindPackage("ClipTest");
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg );
		TS_ASSERT_EQUALS( kNoErr, pResourceMPI_->OpenPackage(pkg) );

		handle1 = pResourceMPI_->FindRsrc("Theora10Vorbis0");
		TS_ASSERT( handle1 != kInvalidRsrcHndl );

		video = pVideoMPI_->StartVideo(handle1);
		TS_ASSERT( video != kInvalidVideoHndl );

		pVideoMPI_->StopVideo(video);

		pResourceMPI_->ClosePackage(pkg);
		pResourceMPI_->CloseAllDevices();
		delete pResourceMPI_;
	}

};

// EOF
