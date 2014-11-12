/******************************************************************************

 @Description  A very simple app that plays a video to the display

******************************************************************************/
#include <DisplayMPI.h>
#include <DebugMPI.h>
#include <SystemTypes.h>
#include <EmulationConfig.h>
#include <KernelMPI.h>
#include <VideoMPI.h>
#include <AudioMPI.h>
#include <ButtonMPI.h>
#include <Utility.h>

LF_USING_BRIO_NAMESPACE()

#define WIDTH	320
#define HEIGHT	240
#define LEFT	0
#define TOP 	0

const U32             kFrameTime = 30;
const tDebugSignature kMyApp = kFirstCartridge1DebugSig;

//============================================================================
// Emulation setup
//============================================================================
#ifdef EMULATION
	static bool gInitialized = 
		EmulationConfig::Instance().Initialize(LEAPFROG_CDEVKIT_ROOT);
#endif

//============================================================================
// Application Resource folder setup
//============================================================================
CPath GetAppRsrcFolder( )
{
#ifdef EMULATION
	return EmulationConfig::Instance().GetCartResourceSearchPath();
#else	// EMULATION
	return "/LF/Bulk/ProgramFiles/VideoDemo/rsrc/";
#endif	// EMULATION
}
	
//============================================================================
// Button / Video / Audio Event Listener
//============================================================================

const tEventType kMyHandledTypes[] = { kAllButtonEvents, kAllAudioEvents, kAllVideoEvents };

//----------------------------------------------------------------------------
class MyBtnEventListener : public IEventListener
{
 public:
	MyBtnEventListener( ) : IEventListener(kMyHandledTypes, ArrayCount(kMyHandledTypes)),
		dbg_(kMyApp) 
		{
			isDone_ = false;
			isPaused_ = false;
		}
	
	virtual tEventStatus Notify( const IEventMessage &msg )
		{
			type_ = msg.GetEventType();
			if ( type_ ==  kButtonStateChanged )
			{
				const CButtonMessage& btnmsg = reinterpret_cast<const CButtonMessage&>(msg);
				data_ = btnmsg.GetButtonState();
				switch (data_.buttonState & data_.buttonTransition)
				{
					case kButtonA:
					case kButtonPause:
						isPaused_ = (isPaused_) ? false : true;
						break;
					case kButtonB:
					case kButtonMenu:
					case kButtonEscape:
						isDone_ = true;
						isPaused_ = false;
						break;
				}
				return kEventStatusOKConsumed;
			}
			else if ( type_ == kVideoCompletedEvent )
			{
				const CVideoEventMessage& vidmsg = reinterpret_cast<const CVideoEventMessage&>(msg);
				const tVideoMsgData& data = vidmsg.data_;
				dbg_.DebugOut(kDbgLvlCritical, "Video done: %s, msec=%lld, frame=%lld, hndl=%08X\n", 
						data.isDone ? "true" : "false", 
						data.timeStamp.time,
						data.timeStamp.frame,
						static_cast<unsigned int>(data.hVideo));
				return kEventStatusOKConsumed;
			}
			else if ( type_ == kAudioCompletedEvent )
			{
				const CAudioEventMessage& audmsg = dynamic_cast<const CAudioEventMessage&>(msg);
				const tAudioMsgAudioCompleted& data = audmsg.audioMsgData.audioCompleted;
				dbg_.DebugOut(kDbgLvlCritical, "Audio done: id=%d, payload=%d\n", 
						static_cast<int>(data.audioID), 
						static_cast<int>(data.payload));
				return kEventStatusOKConsumed;
			}
			return kEventStatusOK;
		}

	bool IsDone()
		{
			return isDone_;
		}

	bool IsPaused()
		{
			return isPaused_;
		}

private:
	tEventType	type_;
	tButtonData	data_;
	bool		isDone_;
	bool		isPaused_;
	CDebugMPI	dbg_;
};

//----------------------------------------------------------------------------
int main(int argc, char** argv)
{
	CKernelMPI		kernel;
	CDebugMPI 		dbg(kMyApp);
	dbg.SetDebugLevel(kDbgLvlImportant);
	CDisplayMPI 	dispmgr;

	unsigned char*	buffer;
	tDisplayHandle	handle;
	tPixelFormat	format = kPixelFormatYUV420; // LF1000 only format
	int left = LEFT, top = TOP, width = WIDTH, height = HEIGHT;
	int pitch;

	// Setup button handler
	// NOTE: Order of events is significant on embedded target!
	CButtonMPI	button;				// 2
	MyBtnEventListener	handler;	// 3
	
	if (button.RegisterEventListener(&handler) != kNoErr) {
		dbg.DebugOut(kDbgLvlCritical, "FALIURE to load button manager!\n");
		return -1;
	}
	
	// Create display surface for video rendering
	handle = dispmgr.CreateHandle(height, width, format, NULL);
	dispmgr.Register(handle, left, top, kDisplayOnTop, 0);
	buffer = dispmgr.GetBuffer(handle);

	// Need to know pitch for video surface descriptor
	pitch = dispmgr.GetPitch(handle);
	dbg.DebugOut(kDbgLvlImportant, "the pitch is: %d\n", pitch);

	// Setup video manager to play video with button handler as event listener
	CVideoMPI			vidmgr;
	CPath				vidfile = "Theora10Vorbis0_mono16kHz.ogg";
	CPath				audfile = "Theora10Vorbis0_mono16kHz.ogg";
	tVideoSurf			mySurf = {width, height, pitch, buffer, format};
	tVideoHndl			myVideo;
	Boolean				bLooped = false; // looping suppresses done messages 

	// Command line overide option for video playback file
	if (argc > 1)
		vidfile = audfile = argv[1];
	else
		vidmgr.SetVideoResourcePath(GetAppRsrcFolder());
	
	// Play video continuously until button press
	//while (!handler.IsDone())
	{
		myVideo = vidmgr.StartVideo(vidfile, audfile, &mySurf, bLooped, &handler);

		while (vidmgr.IsVideoPlaying(myVideo))
		{	
			kernel.TaskSleep(100);
			
			if (handler.IsDone())
				break;
			if (handler.IsPaused())
			{
				vidmgr.PauseVideo(myVideo);
				while (handler.IsPaused())
					kernel.TaskSleep(100);
				vidmgr.ResumeVideo(myVideo);
			}
		}

		vidmgr.StopVideo(myVideo);
	}
	button.UnregisterEventListener(&handler);
	dispmgr.UnRegister(handle, 0);
	dispmgr.DestroyHandle(handle, false);
	return 0;
}
