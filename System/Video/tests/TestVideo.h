// TestVideoMPI.h

#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <VideoMPI.h>
#include <DisplayMPI.h>
#include <KernelMPI.h>
#include <ResourceMPI.h>
#include <EventMPI.h>
#include <EventListener.h>
#include <AudioTypes.h>
#include <UnitTestUtils.h>

LF_USING_BRIO_NAMESPACE()

const tDebugSignature kMyApp = kFirstCartridge1DebugSig;

//============================================================================
// Audio Listener
//============================================================================
const tEventType kMyAudioTypes[] = { kAllAudioEvents };

//----------------------------------------------------------------------------
class AudioListener : public IEventListener
{
public:
	AudioListener( )
		: IEventListener(kMyAudioTypes, ArrayCount(kMyAudioTypes)), dbg_(kMyApp)
	{
	}

	virtual tEventStatus Notify( const IEventMessage &msgIn )
	{
		const CAudioEventMessage& msg = dynamic_cast<const CAudioEventMessage&>(msgIn);
		if( msg.GetEventType() == kAudioCompletedEvent )
		{
			const tAudioMsgDataCompleted& data = msg.audioMsgData.audioCompleted;
			dbg_.DebugOut(kDbgLvlVerbose, "Audio done: id=%d, payload=%d\n", 
					static_cast<int>(data.audioID), static_cast<int>(data.payload));
			return kEventStatusOKConsumed;
		}
		return kEventStatusOK;
	}
private:
	CDebugMPI	dbg_;
};

//============================================================================
// TestVideoMPI functions
//============================================================================
class TestVideo : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CVideoMPI*		pVideoMPI_;
	CResourceMPI*	pResourceMPI_;
	CDisplayMPI*	pDisplayMPI_;
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

	//------------------------------------------------------------------------
	void xxxxtestVideoDisplay()
	{
		tRsrcHndl	pkg;
		tRsrcHndl	handle1;
		tVideoHndl	video;
		tVideoSurf	surf;
		tDisplayHandle disp;
		
		pDisplayMPI_ = new CDisplayMPI;
		disp = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatYUYV422, NULL);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, kDisplayOnTop, 0);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		
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

		for (int i = 0; i < 100; i++)
		{
			Boolean 	r;
			tVideoTime	vt;

			r = pVideoMPI_->GetVideoFrame(video, NULL);
			TS_ASSERT( r == true );
			r = pVideoMPI_->PutVideoFrame(video, &surf);
			TS_ASSERT( r == true );
			pDisplayMPI_->Invalidate(0, NULL);
			
			r = pVideoMPI_->GetVideoTime(video, &vt);
			TS_ASSERT( r == true );
			TS_ASSERT( vt.frame == i );
			TS_ASSERT( vt.time >= 0 );
		}
		
		pVideoMPI_->StopVideo(video);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		pResourceMPI_->ClosePackage(pkg);
		pResourceMPI_->CloseAllDevices();
		delete pResourceMPI_;
		delete pDisplayMPI_;
	}

	//------------------------------------------------------------------------
	void testVideoAudio()
	{
		tRsrcHndl	pkg;
		tRsrcHndl	handle1;
		tRsrcHndl	handle2;
		tVideoHndl	video;
		tVideoSurf	surf;
		tDisplayHandle disp;

		pDisplayMPI_ = new CDisplayMPI;
		disp = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatYUYV422, NULL);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, kDisplayOnTop, 0);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		
		pResourceMPI_ = new CResourceMPI;
		pResourceMPI_->OpenAllDevices();
		pResourceMPI_->SetDefaultURIPath("LF/Brio/UnitTest/Video");

		pkg = pResourceMPI_->FindPackage("ClipTest");
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg );
		TS_ASSERT_EQUALS( kNoErr, pResourceMPI_->OpenPackage(pkg) );

		handle1 = pResourceMPI_->FindRsrc("Theora10Vorbis0");
		TS_ASSERT( handle1 != kInvalidRsrcHndl );
		handle2 = pResourceMPI_->FindRsrc("Vorbis0");
		TS_ASSERT( handle2 != kInvalidRsrcHndl );

		CKernelMPI*	kernel = new CKernelMPI();
		CEventMPI*  evtmgr = new CEventMPI();
		AudioListener  audioListener;
		evtmgr->RegisterEventListener(&audioListener);

		video = pVideoMPI_->StartVideo(handle1, handle2, &surf, false, &audioListener);
		TS_ASSERT( video != kInvalidVideoHndl );
		
		for (int i = 0; i < 100; i++)
		{
			Boolean 	r;
			tVideoTime	vt;
			
			kernel->TaskSleep(30);

			r = pVideoMPI_->GetVideoTime(video, &vt);
			TS_ASSERT( r == true );
			TS_ASSERT( vt.time >= 0 );
		}
//		pVideoMPI_->PauseVideo(video);
//		sleep(1);
//		pVideoMPI_->ResumeVideo(video);
//		sleep(1);
		
		pVideoMPI_->StopVideo(video);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		pResourceMPI_->ClosePackage(pkg);
		pResourceMPI_->CloseAllDevices();
		delete pResourceMPI_;
		delete pDisplayMPI_;
	}

};

// EOF
