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
	return "/Base/rsrc/Video/";
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
		tVideoHndl	video;
		
		CPath dir = GetTestRsrcFolder();
		pVideoMPI_->SetVideoResourcePath(dir);
 		CPath* path = pVideoMPI_->GetVideoResourcePath();
 		TS_ASSERT( *path == dir );
		
		video = pVideoMPI_->StartVideo("Theora10Vorbis0_mono16kHz.ogg");
		TS_ASSERT( video != kInvalidVideoHndl );

		pVideoMPI_->StopVideo(video);
	}

	//------------------------------------------------------------------------
	void testVideoDisplay()
	{
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
	void testVideoAudio()
	{
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
		video = pVideoMPI_->StartVideo("Theora10Vorbis0_mono16kHz.ogg", "Vorbis0_mono16kHz.ogg", &surf, false, &videoListener);
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
		delete pDisplayMPI_;
	}

};

// EOF
