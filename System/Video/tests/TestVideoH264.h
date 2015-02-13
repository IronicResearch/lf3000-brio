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
	U16				SCREEN_WIDTH;
	U16 			SCREEN_HEIGHT;

public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		pVideoMPI_ = new CVideoMPI;
		pDisplayMPI_ = new CDisplayMPI;
		SCREEN_WIDTH = pDisplayMPI_->GetScreenStats(0)->width;
		SCREEN_HEIGHT = pDisplayMPI_->GetScreenStats(0)->height;
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete pDisplayMPI_;
		delete pVideoMPI_; 
	}

	//------------------------------------------------------------------------
	void testVideoGetFrame()
	{
		PRINT_TEST_NAME();

		tVideoHndl	video;
		tVideoSurf	surf;
		tDisplayHandle disp;
		tVideoInfo	info;

		disp = pDisplayMPI_->CreateHandle(SCREEN_HEIGHT, SCREEN_WIDTH, kPixelFormatYUV420);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, kDisplayOnTop);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);
		TS_ASSERT( surf.format == kPixelFormatYUV420 );

		pVideoMPI_->SetVideoResourcePath(GetTestRsrcFolder());
		video = pVideoMPI_->StartVideo("LIVEACTION-2997fps-480x272_h264.mp4");
		TS_ASSERT( video != kInvalidVideoHndl );

		pVideoMPI_->GetVideoInfo(video, &info);
		TS_ASSERT( info.fps != 0 );
		pDisplayMPI_->SetVideoScaler(disp, info.width, info.height, false);
		pDisplayMPI_->SetWindowPosition(disp, 0, 0, info.width, info.height, false);

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
	}

	//------------------------------------------------------------------------
	void testVideoPlayback()
	{
		PRINT_TEST_NAME();

		tVideoHndl	video;
		tVideoSurf	surf;
		tDisplayHandle disp;
		tVideoInfo	info;

		disp = pDisplayMPI_->CreateHandle(SCREEN_HEIGHT, SCREEN_WIDTH, kPixelFormatYUV420);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, kDisplayOnTop);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);
		TS_ASSERT( surf.format == kPixelFormatYUV420 );

		pVideoMPI_->SetVideoResourcePath(GetTestRsrcFolder());
		video = pVideoMPI_->StartVideo("LIVEACTION-2997fps-480x272_h264.mp4", "", &surf);
		TS_ASSERT( video != kInvalidVideoHndl );

		pVideoMPI_->GetVideoInfo(video, &info);
		TS_ASSERT( info.fps != 0 );

		for (int i = 0; i < 100; i++)
		{
			Boolean 	r;
			tVideoTime	vt;
			CKernelMPI  kernel;
			int         delta = 1000 / info.fps;

			kernel.TaskSleep(delta);

			r = pVideoMPI_->GetVideoTime(video, &vt);
			TS_ASSERT( r == true );
			TS_ASSERT( vt.frame >= i );
			TS_ASSERT( vt.time >= 0 );
		}

		pVideoMPI_->StopVideo(video);
		sleep(1);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
	}

	//------------------------------------------------------------------------
	void testVideoPlaybackAudio()
	{
		PRINT_TEST_NAME();

		tVideoHndl	video;
		tVideoSurf	surf;
		tDisplayHandle disp;
		tVideoInfo	info;

		disp = pDisplayMPI_->CreateHandle(SCREEN_HEIGHT, SCREEN_WIDTH, kPixelFormatYUV420);
		TS_ASSERT( disp != kInvalidDisplayHandle );
		pDisplayMPI_->Register(disp, 0, 0, kDisplayOnTop);

		surf.width = pDisplayMPI_->GetWidth(disp);
		surf.pitch = pDisplayMPI_->GetPitch(disp);
		surf.height = pDisplayMPI_->GetHeight(disp);
		surf.buffer = pDisplayMPI_->GetBuffer(disp);
		surf.format = pDisplayMPI_->GetPixelFormat(disp);
		TS_ASSERT( surf.format == kPixelFormatYUV420 );

		pVideoMPI_->SetVideoResourcePath(GetTestRsrcFolder());
		video = pVideoMPI_->StartVideo("LIVEACTION-2997fps-480x272_h264.mp4", "LIVEACTION-2997fps-480x272_h264.mp4", &surf);
		TS_ASSERT( video != kInvalidVideoHndl );

		pVideoMPI_->GetVideoInfo(video, &info);
		TS_ASSERT( info.fps != 0 );

		for (int i = 0; i < 1000; i++)
		{
			Boolean 	r;
			tVideoTime	vt;
			CKernelMPI  kernel;
			int         delta = 1000 / info.fps;

			kernel.TaskSleep(delta);

			r = pVideoMPI_->GetVideoTime(video, &vt);
			TS_ASSERT( r == true );
			TS_ASSERT( vt.frame >= i );
			TS_ASSERT( vt.time >= 0 );
		}

		pVideoMPI_->StopVideo(video);
		sleep(1);

		pDisplayMPI_->UnRegister(disp, 0);
		pDisplayMPI_->DestroyHandle(disp, false);
	}

};

// EOF
