/******************************************************************************

 @Description  Simple CameraMPI SDK example

******************************************************************************/
#include <DisplayMPI.h>
#include <DebugMPI.h>
#include <SystemTypes.h>
#include <EmulationConfig.h>
#include <KernelMPI.h>
#include <CameraMPI.h>
#include <VideoMPI.h>
#include <AudioMPI.h>
#include <ButtonMPI.h>
#include <USBDeviceMPI.h>
LF_USING_BRIO_NAMESPACE()

#define WIDTH	320
#define HEIGHT	240
#define LEFT	0
#define TOP 	0

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
	return "/LF/Bulk/ProgramFiles/CameraDemo/rsrc/";
#endif	// EMULATION
}
	
//============================================================================
// Button / Video / Audio Event Listener
//============================================================================

const tEventType kMyHandledTypes[] = { kAllButtonEvents, kAllCameraEvents, kAllUSBDeviceEvents };

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
					case kButtonSync:
						isDone_ = true;
						isPaused_ = false;
						break;
				}
				return kEventStatusOKConsumed;
			}
			else if ( type_ ==  kCaptureTimeOutEvent )
			{
				dbg_.DebugOut(kDbgLvlImportant, "kCaptureTimeOutEvent posted\n");
				isDone_ = true;
				isPaused_ = false;
				return kEventStatusOKConsumed;
			}
			else if ( type_ ==  kCaptureQuotaHitEvent )
			{
				dbg_.DebugOut(kDbgLvlImportant, "kCaptureQuotaHitEvent posted\n");
				isDone_ = true;
				isPaused_ = false;
				return kEventStatusOKConsumed;
			}
			else if ( type_ ==  kCaptureStoppedEvent )
			{
				dbg_.DebugOut(kDbgLvlImportant, "kCaptureStoppedEvent posted\n");
				isDone_ = true;
				isPaused_ = false;
				return kEventStatusOKConsumed;
			}
#if 0
			else if ( type_ ==  kUSBDeviceStateChange )
			{
				dbg_.DebugOut(kDbgLvlImportant, "kUSBDeviceStateChange posted\n");
				isDone_ = true;
				isPaused_ = false;
				return kEventStatusOKConsumed;
			}
#endif
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
	CDisplayMPI 	dispmgr;
	dbg.SetDebugLevel(kDbgLvlImportant);
	
	// Setup button handler
	CButtonMPI		button;
	MyBtnEventListener	handler;
	
	if (button.RegisterEventListener(&handler) != kNoErr) {
		dbg.DebugOut(kDbgLvlCritical, "FALIURE to load button manager!\n");
		return -1;
	}
	
	// Create display surface for video rendering
	U8*				buffer;
	tDisplayHandle	handle;
	tPixelFormat	format = kPixelFormatYUV420; // LF1000 only format
	int 			left = LEFT, top = TOP, width = WIDTH, height = HEIGHT;
	int 			pitch;

	// Command line options for camera mode size
	if (argc > 3) {
		width  = atoi(argv[2]);
		height = atoi(argv[3]);
	}

	handle = dispmgr.CreateHandle(height, width, format, NULL);
	dispmgr.Register(handle, left, top, kDisplayOnTop, 0);
	buffer = dispmgr.GetBuffer(handle);
	pitch = dispmgr.GetPitch(handle);

	// Setup camera MPI to capture video with button handler as event listener
	CCameraMPI			camera;
	CPath				vidfile = ""; //"test.avi";
	tVideoSurf			mySurf = {width, height, pitch, buffer, format};
	tVidCapHndl			myVideo;

	// Command line overide option for video capture file
	if (argc > 1)
		vidfile = argv[1];
	else
		camera.SetCameraVideoPath(GetAppRsrcFolder());

	// Option for capture input without saving to file = viewfinder mode
	if (vidfile == "none")
		vidfile = "";

	// Option for camera mode
	tCaptureMode* 	mode = camera.GetCurrentFormat();
	mode->width  = width;
	mode->height = height;
	camera.SetCurrentFormat(mode);

	// Start video capture 
	myVideo = camera.StartVideoCapture(&mySurf, &handler, vidfile);
	
	// Continue capturing video until button press
	while (myVideo != kInvalidHndl)
	{
		kernel.TaskSleep(100);
		
		if (handler.IsDone())
			break;
		if (handler.IsPaused())
		{
			camera.PauseVideoCapture(myVideo);
			while (handler.IsPaused())
				kernel.TaskSleep(100);
			camera.ResumeVideoCapture(myVideo);
		}
	}

	camera.StopVideoCapture(myVideo);

	dispmgr.UnRegister(handle, 0);
	dispmgr.DestroyHandle(handle, false);
	
	button.UnregisterEventListener(&handler);

	return 0;
}
