// TestVideoMPI.h

#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <VideoMPI.h>
#include <DisplayMPI.h>
#include <KernelMPI.h>
#include <EventMPI.h>
#include <EventListener.h>
#include <AudioTypes.h>
#include <DisplayTypes.h>
#include <UnitTestUtils.h>

LF_USING_BRIO_NAMESPACE()

const tDebugSignature kMyApp = kFirstCartridge1DebugSig;

//----------------------------------------------------------------------------
inline CPath GetTestRsrcFolder( )
{
#ifdef EMULATION
	CPath dir = EmulationConfig::Instance().GetCartResourceSearchPath();
	return dir + "Video/";
#else	// EMULATION
	return "/LF/Base/Brio/rsrc/Video/";
#endif	// EMULATION
}

//============================================================================
// Video Listener
//============================================================================
const tEventType kMyVideoTypes[] = { kAllVideoEvents };

//----------------------------------------------------------------------------
class VideoListener : public IEventListener
{
public:
	VideoListener( )
		: IEventListener(kMyVideoTypes, ArrayCount(kMyVideoTypes)), dbg_(kMyApp)
	{
	}

	virtual tEventStatus Notify( const IEventMessage &msgIn )
	{
		const CVideoEventMessage& msg = reinterpret_cast<const CVideoEventMessage&>(msgIn);
		if( msg.GetEventType() == kVideoCompletedEvent )
		{
			const tVideoMsgData& data = msg.data_;
			dbg_.DebugOut(kDbgLvlCritical, "Video done: %s, msec %lld, frame %lld, hndl %08X\n", 
					data.isDone ? "true" : "false", 
					data.timeStamp.time,
					data.timeStamp.frame,
					static_cast<unsigned int>(data.hVideo));
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
		PRINT_TEST_NAME();
		
		TS_ASSERT( pVideoMPI_ != NULL );
		TS_ASSERT( pVideoMPI_->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		PRINT_TEST_NAME();
		
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
		PRINT_TEST_NAME();

		tVideoHndl	video;

		CPath dir = GetTestRsrcFolder();
		pVideoMPI_->SetVideoResourcePath(dir);
 		CPath* path = pVideoMPI_->GetVideoResourcePath();
 		TS_ASSERT( *path == dir );

		video = pVideoMPI_->StartVideo("Theora10Vorbis0_mono16kHz.ogg");
		TS_ASSERT( video != kInvalidVideoHndl );
		TS_ASSERT_EQUALS( video, pVideoMPI_->GetCurrentVideoHandle() );

		pVideoMPI_->StopVideo(video);
		TS_ASSERT_EQUALS( kInvalidVideoHndl, pVideoMPI_->GetCurrentVideoHandle() );
	}

	//------------------------------------------------------------------------
	void testVideoDisplay()
	{
		PRINT_TEST_NAME();

#if defined(EMULATION) || !defined(LF1000) // YUYV422 not supported on LF1000

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
		surf.format = pDisplayMPI_->GetPixelFormat(disp);

		pVideoMPI_->SetVideoResourcePath(GetTestRsrcFolder());
		video = pVideoMPI_->StartVideo("Theora10Vorbis0_mono16kHz.ogg");
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
		sleep(1);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		delete pDisplayMPI_;
#endif
	}

	//------------------------------------------------------------------------
	void testVideoDisplayRGB()
	{
		PRINT_TEST_NAME();

		tVideoHndl	video;
		tVideoSurf	surf;
		tDisplayHandle disp;

		pDisplayMPI_ = new CDisplayMPI;
		disp = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatARGB8888, NULL);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, kDisplayOnTop, 0);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);
		TS_ASSERT( surf.format == kPixelFormatARGB8888 );

		pVideoMPI_->SetVideoResourcePath(GetTestRsrcFolder());
		video = pVideoMPI_->StartVideo("Theora10Vorbis0_mono16kHz.ogg");
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
		sleep(1);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		delete pDisplayMPI_;
	}

	//------------------------------------------------------------------------
	void testVideoDisplayYUV()
	{
		PRINT_TEST_NAME();

		tVideoHndl	video;
		tVideoSurf	surf;
		tDisplayHandle disp;

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

		pVideoMPI_->SetVideoResourcePath(GetTestRsrcFolder());
		video = pVideoMPI_->StartVideo("Theora10Vorbis0_mono16kHz.ogg");
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
		sleep(1);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		delete pDisplayMPI_;
	}

	//------------------------------------------------------------------------
	void testVideoAVI()
	{
		PRINT_TEST_NAME();

		tVideoHndl	video;
		tVideoSurf	surf;
		tDisplayHandle disp;

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

		pVideoMPI_->SetVideoResourcePath(GetTestRsrcFolder());
		video = pVideoMPI_->StartVideo("testYUV.avi");
		TS_ASSERT( video != kInvalidVideoHndl );

		for (int i = 0; i < 10; i++)
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
		sleep(1);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		delete pDisplayMPI_;
	}

	//------------------------------------------------------------------------
	void testVideoAVIRGB()
	{
		PRINT_TEST_NAME();

		tVideoHndl	video;
		tVideoSurf	surf;
		tDisplayHandle disp;

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

		pVideoMPI_->SetVideoResourcePath(GetTestRsrcFolder());
		video = pVideoMPI_->StartVideo("testRGB.avi");
		TS_ASSERT( video != kInvalidVideoHndl );

		for (int i = 0; i < 10; i++)
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
		sleep(1);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		delete pDisplayMPI_;
	}

	//------------------------------------------------------------------------
	void testVideoAVIPlay()
	{
		PRINT_TEST_NAME();

		tVideoHndl	video;
		tVideoSurf	surf;
		tDisplayHandle disp;

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

		pVideoMPI_->SetVideoResourcePath(GetTestRsrcFolder());
		video = pVideoMPI_->StartVideo("UncompressedRGB.avi", &surf);
		TS_ASSERT( video != kInvalidVideoHndl );

		CKernelMPI*	kernel = new CKernelMPI();
		for (int i = 0; i < 10; i++)
		{
			Boolean 	r;
			tVideoTime	vt;

			kernel->TaskSleep(500);
			r = pVideoMPI_->GetVideoTime(video, &vt);
			TS_ASSERT( r == true );
			TS_ASSERT( vt.frame >= i );
			TS_ASSERT( vt.time >= 0 );
			kernel->TaskSleep(500);
		}
		delete kernel;

		pVideoMPI_->StopVideo(video);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		delete pDisplayMPI_;
	}

	//------------------------------------------------------------------------
	void testVideoAVISeek()
	{
		PRINT_TEST_NAME();

		tVideoHndl	video;
		tVideoSurf	surf;
		tVideoInfo	info;
		tVideoTime	time;
		tDisplayHandle disp;

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

		pVideoMPI_->SetVideoResourcePath(GetTestRsrcFolder());
		video = pVideoMPI_->StartVideo("UncompressedRGB.avi");
		TS_ASSERT( video != kInvalidVideoHndl );

		pVideoMPI_->GetVideoInfo(video, &info);
		TS_ASSERT( info.fps != 0 );

		for (int i = 0; i < 10; i++)
		{
			time.frame = i * info.fps;
			pVideoMPI_->SeekVideoFrame(video, &time);
			pVideoMPI_->PutVideoFrame(video, &surf);
			pDisplayMPI_->Invalidate(0, NULL);
		}
		for (int i = 10; i >= 0; i--)
		{
			time.frame = i * info.fps;
			pVideoMPI_->SeekVideoFrame(video, &time);
			pVideoMPI_->PutVideoFrame(video, &surf);
			pDisplayMPI_->Invalidate(0, NULL);
		}

		pVideoMPI_->StopVideo(video);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		delete pDisplayMPI_;
	}

	//------------------------------------------------------------------------
	void testVideoAudio()
	{
		PRINT_TEST_NAME();

		tVideoHndl	video;
		tVideoSurf	surf;
		tDisplayHandle disp;

		pDisplayMPI_ = new CDisplayMPI;
		disp = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatYUV420, NULL);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, kDisplayOnTop, 0);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);

		CKernelMPI*	kernel = new CKernelMPI();
		CEventMPI*  evtmgr = new CEventMPI();
		VideoListener  videoListener;
		evtmgr->RegisterEventListener(&videoListener);

		pVideoMPI_->SetVideoResourcePath(GetTestRsrcFolder());
		video = pVideoMPI_->StartVideo("Theora10Vorbis0_mono16kHz.ogg", "Theora10Vorbis0_mono16kHz.ogg", &surf, false, &videoListener);
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

		pVideoMPI_->PauseVideo(video);
		TS_ASSERT( true == pVideoMPI_->IsVideoPaused(video) );
		kernel->TaskSleep(1000);
		pVideoMPI_->ResumeVideo(video);
		TS_ASSERT( false == pVideoMPI_->IsVideoPaused(video) );
		kernel->TaskSleep(1000);

		for (int i = 0; i < 100; i++)
		{
			Boolean 	r;
			tVideoTime	vt;

			kernel->TaskSleep(30);

			r = pVideoMPI_->GetVideoTime(video, &vt);
			TS_ASSERT( r == true );
			TS_ASSERT( vt.time > 0 );
		}

		pVideoMPI_->StopVideo(video);
		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		evtmgr->UnregisterEventListener(&videoListener);
		delete evtmgr;
		delete kernel;
		delete pDisplayMPI_;
	}

	//------------------------------------------------------------------------
	void testVideoState()
	{
		PRINT_TEST_NAME();

		tVideoHndl	video;
		tVideoSurf	surf;
		tDisplayHandle disp;

		pDisplayMPI_ = new CDisplayMPI;
		disp = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatYUV420, NULL);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, kDisplayOnTop, 0);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);

		pVideoMPI_->SetVideoResourcePath(GetTestRsrcFolder());
		video = pVideoMPI_->StartVideo("Theora10Vorbis0_mono16kHz.ogg", "Theora10Vorbis0_mono16kHz.ogg", &surf, false, NULL);
		TS_ASSERT( video != kInvalidVideoHndl );
		TS_ASSERT( true == pVideoMPI_->IsVideoPlaying(video) );

		pVideoMPI_->StopVideo(video);
		TS_ASSERT( false == pVideoMPI_->IsVideoPlaying(video) );

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		delete pDisplayMPI_;
	}

	//------------------------------------------------------------------------
	void testVideoSeek()
	{
		PRINT_TEST_NAME();

		tVideoHndl	video;
		tVideoSurf	surf;
		tVideoInfo	info;
		tVideoTime	time;
		tDisplayHandle disp;

		pDisplayMPI_ = new CDisplayMPI;
		disp = pDisplayMPI_->CreateHandle(240, 320, kPixelFormatYUV420, NULL);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, kDisplayOnTop, 0);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);

		pVideoMPI_->SetVideoResourcePath(GetTestRsrcFolder());
		video = pVideoMPI_->StartVideo("7sec.ogg");
		TS_ASSERT( video != kInvalidVideoHndl );

		pVideoMPI_->GetVideoInfo(video, &info);
		TS_ASSERT( info.fps != 0 );

		for (int i = 0; i < 10; i++)
		{
			time.frame = i * info.fps;
			pVideoMPI_->SeekVideoFrame(video, &time);
			pVideoMPI_->PutVideoFrame(video, &surf);
			pDisplayMPI_->Invalidate(0, NULL);
		}
		for (int i = 10; i >= 0; i--)
		{
			time.frame = i * info.fps;
			pVideoMPI_->SeekVideoFrame(video, &time);
			pVideoMPI_->PutVideoFrame(video, &surf);
			pDisplayMPI_->Invalidate(0, NULL);
		}
		for (int i = 0; i < 30; i++)
		{
			time.frame = i;
			pVideoMPI_->SeekVideoFrame(video, &time);
			pVideoMPI_->PutVideoFrame(video, &surf);
			pDisplayMPI_->Invalidate(0, NULL);
		}
		for (int i = 30; i >= 0; i--)
		{
			time.frame = i;
			pVideoMPI_->SeekVideoFrame(video, &time);
			pVideoMPI_->PutVideoFrame(video, &surf);
			pDisplayMPI_->Invalidate(0, NULL);
		}

		pVideoMPI_->StopVideo(video);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		delete pDisplayMPI_;
	}


	//------------------------------------------------------------------------
	void testVideoLength()
	{
		PRINT_TEST_NAME();

		pVideoMPI_->SetVideoResourcePath(GetTestRsrcFolder());
		S64 length = pVideoMPI_->GetVideoLength("7sec.ogg", 300);
		float lengthSec = (float) length/1000;
		printf("est. length of 7 sec. video (.ogg) = %f\n", lengthSec);
		TS_ASSERT( lengthSec < 8 && lengthSec > 6);

		length = pVideoMPI_->GetVideoLength("testYUV.avi", 300);
		lengthSec = (float) length/1000;
		printf("est. length of 5 sec. video (.avi) = %f\n", lengthSec);
		TS_ASSERT( lengthSec < 6 && lengthSec > 4);
	}

	//------------------------------------------------------------------------
	void testVideoGStreamer()
	{
#if USE_GSTREAMER
		PRINT_TEST_NAME();

		tVideoHndl	video;
		tVideoSurf	surf;
		tVideoInfo	info;
		tVideoTime	time;
		tDisplayHandle disp;
//		CURI		uri = "file:///LF/Bulk/ProgramFiles/WebBrowser/videos/LIVEACTION-2997fps-480x272_h264.mp4";
//		CURI		uri = "http://docs.gstreamer.com/media/sintel_trailer-480p.webm";
//		CURI		uri = "http://s7.leapfrog.com/is/content/LeapFrog/TEST/VIDEOS/360x270-And-Device-Halfrate-Test/360x270-Trailer-TestFile-device.mp4";
//		CURI		uri = "http://s7.leapfrog.com/e2/LeapFrog/Videos/LeapReader/2013-LeapReader-Sizzle/2013-LeapReader-Sizzle-MAUD-2398fps-1920x1080_h264-device.mp4";
//		CURI		uri = "http://192.168.199.19/video/videos/2013-LeapReader-Sizzle-MAUD-2398fps-1920x1080_h264-device.mp4";
		CURI		uri = "http://192.168.199.19/video/videos/LF-AskExpert.flv";

		pDisplayMPI_ = new CDisplayMPI;
		surf.width  = pDisplayMPI_->GetScreenStats(0)->width;
		surf.height = pDisplayMPI_->GetScreenStats(0)->height;
		surf.format = kPixelFormatYUV420;
		disp = pDisplayMPI_->CreateHandle(surf.height, surf.width, surf.format);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		surf.pitch  = pDisplayMPI_->GetPitch(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		pDisplayMPI_->Register(disp, 0, 0, kDisplayOnOverlay);

		video = pVideoMPI_->StartVideo(uri, uri, &surf);
		if (video != kInvalidVideoHndl)
		{
			CKernelMPI kernel;
			while (pVideoMPI_->IsVideoPlaying(video))
				kernel.TaskSleep(1000);
		}
		pVideoMPI_->StopVideo(video);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		delete pDisplayMPI_;
#endif
	}


	//------------------------------------------------------------------------
	void testVideoRotator()
	{
#if USE_ROTATOR
		PRINT_TEST_NAME();

		tVideoHndl	video;
		tVideoSurf	surf;
		tVideoInfo	info;
		tDisplayHandle disp;
//		CPath		path = "Theora10Vorbis0_mono16kHz.ogg";
//		CPath		path = "testYUV.avi";
		CPath		path = "7sec.ogg";

		pVideoMPI_->SetVideoResourcePath(GetTestRsrcFolder());
		video = pVideoMPI_->StartVideo(path);
		TS_ASSERT( video != kInvalidVideoHndl );
		pVideoMPI_->GetVideoInfo(video, &info);
		pVideoMPI_->StopVideo(video);

		pDisplayMPI_ = new CDisplayMPI;
		surf.width  = std::max(info.width, info.height);
		surf.height = std::max(info.width, info.height);
		surf.format = kPixelFormatYUV420;
		disp = pDisplayMPI_->CreateHandle(surf.height, surf.width, surf.format);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		surf.pitch  = pDisplayMPI_->GetPitch(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		pDisplayMPI_->Register(disp, 0, 0, kDisplayOnOverlay);
		pDisplayMPI_->SetOrientation(disp, kOrientationPortrait);

		pVideoMPI_->SetVideoResourcePath(GetTestRsrcFolder());
		video = pVideoMPI_->StartVideo(path, &surf);
		if (video != kInvalidVideoHndl)
		{
			CKernelMPI kernel;
			while (pVideoMPI_->IsVideoPlaying(video))
				kernel.TaskSleep(1000);
		}
		pVideoMPI_->StopVideo(video);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
		delete pDisplayMPI_;
#endif
	}

};

// EOF
