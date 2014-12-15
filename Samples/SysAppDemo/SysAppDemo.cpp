/******************************************************************************

 @Description  An example Brio app which monitors system power and USB events.

******************************************************************************/
#include <DisplayMPI.h>
#include <DebugMPI.h>
#include <SystemTypes.h>
#include <EmulationConfig.h>
#include <KernelMPI.h>
#include <VideoMPI.h>
#include <AudioMPI.h>
#include <ButtonMPI.h>
#include <PowerMPI.h>
#include <USBDeviceMPI.h>
#include <stdio.h>

LF_USING_BRIO_NAMESPACE()

#define USE_NFS_LOAD_AHEAD		1	// for avoiding NFS loading delays
#define USE_POWER_RESET			1	// for Power MPI Reset

#define WIDTH	320
#define HEIGHT	240
#define LEFT	0
#define TOP 	0

const U32             kFrameTime = 30;
const U32             kDelayTime = 3000;
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
	return "/LF/Bulk/ProgramFiles/SysAppDemo/rsrc/";
#endif	// EMULATION
}
	
//============================================================================
// Button / USB / Power / Video / Audio Event Listener
//============================================================================

const tEventType kMyHandledTypes[] = { kAllButtonEvents, kAllUSBDeviceEvents, kAllPowerEvents, kAllVideoEvents, kAllAudioEvents };

//----------------------------------------------------------------------------
class MyBtnEventListener : public IEventListener
{
 public:
	MyBtnEventListener( ) : IEventListener(kMyHandledTypes, ArrayCount(kMyHandledTypes)),
		dbg_(kMyApp) 
		{
			isDone_ = false;
			isPaused_ = false;
			isPressed_ = false;
			isTimed_ = false;
			isConnect_ = false;
			isDisconnect_ = false;
			isRestart_ = false;
			isShutdown_ = false;
			time_ = kernel_.GetElapsedTimeAsMSecs();
		}
	
	virtual tEventStatus Notify( const IEventMessage &msg )
		{
			type_ = msg.GetEventType();
			if ( type_ ==  kButtonStateChanged )
			{
				const CButtonMessage& btnmsg = reinterpret_cast<const CButtonMessage&>(msg);
				data_ = btnmsg.GetButtonState();
				isPressed_ = (data_.buttonState & data_.buttonTransition) ? true : false;
				switch (data_.buttonState & data_.buttonTransition)
				{
					case kButtonPause:
						isPaused_ = (isPaused_) ? false : true;
						break;
					case kButtonMenu:
						isDone_ = true;
						isPaused_ = false;
						break;
					case kButtonA:
					case kButtonLeft:
						isConnect_ = true;
						break;
					case kButtonB:
					case kButtonRight:
						isDisconnect_ = true;
						break;
					case kButtonUp:
					case kButtonDown:
						isRestart_ = true;
						break;
					case kButtonHint:
						isTimed_ = (isTimed_) ? false : true;
						time_ = kernel_.GetElapsedTimeAsMSecs();
						break;
				}
				if (data_.buttonTransition & kHeadphoneJackDetect) 
					dbg_.DebugOut(kDbgLvlCritical, "Headphone %s detected\n", data_.buttonState ? "in" : "out");				
				if (data_.buttonTransition & kCartridgeDetect) {
					dbg_.DebugOut(kDbgLvlCritical, "Cartridge %s detected\n", data_.buttonState ? "in" : "out");				
					isPressed_ = isRestart_ = true;
				}
				return kEventStatusOKConsumed;
			}
			else if ( type_ == kUSBDeviceStateChange )
			{
				const CUSBDeviceMessage& usbmsg = reinterpret_cast<const CUSBDeviceMessage&>(msg);
				tUSBDeviceData data = usbmsg.GetUSBDeviceState();
				dbg_.DebugOut(kDbgLvlCritical, "USB Device change: state=%d, driver=%d, support=%d\n", 
						static_cast<unsigned int>(data.USBDeviceState),
						static_cast<unsigned int>(data.USBDeviceDriver),
						static_cast<unsigned int>(data.USBDeviceSupports));
				if (data.USBDeviceSupports & (kUSBDeviceIsMassStorage | kUSBDeviceIsEthernet))
				{
					if (data.USBDeviceState & kUSBDeviceConnected)
						isConnect_ = true;
					else
						isDisconnect_ = true;
					dbg_.DebugOut(kDbgLvlCritical, "USB Device %s\n", isConnect_ ? "Connected" : "Disconnected");				
				}
				isPressed_ = true;
				return kEventStatusOKConsumed;
			}
			else if ( type_ == kPowerStateChanged )
			{
				const CPowerMessage& pwrmsg = reinterpret_cast<const CPowerMessage&>(msg);
				tPowerState state = pwrmsg.GetPowerState();
				dbg_.DebugOut(kDbgLvlCritical, "Power state change: %d\n", 
						static_cast<unsigned int>(state));
				if (state == kPowerShutdown) {
					isShutdown_ = true;
					dbg_.DebugOut(kDbgLvlCritical, "Power shutdown pending\n");				
				}
				isPressed_ = true;
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

	bool IsTimedOut()
		{
			U32 time = kernel_.GetElapsedTimeAsMSecs();
			U32 diff = time - time_;
			if (diff > kDelayTime) 
			{
				time_ = time;
				return true;
			}
			return false;
		}

	bool IsPressed()
		{
		if (isTimed_)
			return IsTimedOut();
		if (!isPressed_)
			return false;
		isPressed_ = false;
		return true;
		}

	bool IsConnect()
		{
			if (!isConnect_)
				return false;
			isConnect_ = false;
			return true;
		}

	bool IsDisconnect()
		{
		if (!isDisconnect_)
			return false;
		isDisconnect_ = false;
		return true;
		}

	bool IsRestart()
		{
		if (!isRestart_)
			return false;
		isRestart_ = false;
		return true;
		}

	bool IsShutdown()
		{
		if (!isShutdown_)
			return false;
		isShutdown_ = false;
		return true;
		}

private:
	tEventType	type_;
	tButtonData	data_;
	bool		isDone_;
	bool		isPaused_;
	bool		isPressed_;
	bool		isTimed_;
	bool		isConnect_;
	bool		isDisconnect_;
	bool		isRestart_;
	bool		isShutdown_;
	U32			time_;
	CDebugMPI	dbg_;
	CKernelMPI	kernel_;
};

//============================================================================
// Display RGB billboard screen
//============================================================================
void DisplayRGBScreen(const char* name)
{
	CDisplayMPI		dispmgr;
	tDisplayHandle	dispctx;
	U8*				buffer;
	FILE*			fp;
	CPath			path = GetAppRsrcFolder() + name;

	// Billboard screens are already packed in 24bpp RGB format
	dispctx = dispmgr.CreateHandle(HEIGHT, WIDTH, kPixelFormatRGB888); 
	buffer = dispmgr.GetBuffer(dispctx);
	fp = fopen(path.c_str(), "r");
	if (fp) {
		fread(buffer, HEIGHT, 3*WIDTH, fp);
		fclose(fp);
	}
	dispmgr.Register(dispctx, 0, 0, kDisplayOnTop);
}

//----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	unsigned char*	buffer;
	tDisplayHandle	dispctx;
	tPixelFormat	format = kPixelFormatYUV420; // LF1000 only format
	int 			left = LEFT, top = TOP, width = WIDTH, height = HEIGHT;
	int 			pitch;

	// Instantiate Brio MPIs
	CDebugMPI 		dbg(kMyApp);
	dbg.SetDebugLevel(kDbgLvlImportant);
	CKernelMPI		kernel;
	CDisplayMPI 	dispmgr;
	CButtonMPI		button;
	CPowerMPI		powermgr;
	CUSBDeviceMPI	usbmgr;
	
	// Set Power button shutdown delay
	powermgr.SetShutdownTimeMS(2 * kDelayTime);
	
	// Setup multi-event handler after MPIs
	MyBtnEventListener	handler;
	
	if (button.RegisterEventListener(&handler) != kNoErr) {
		dbg.DebugOut(kDbgLvlCritical, "FALIURE to load button manager!\n");
		return kKernelExitError;
	}
	
	// Create display surface for video rendering
	dispctx = dispmgr.CreateHandle(height, width, format, NULL);
	dispmgr.Register(dispctx, left, top, kDisplayOnTop, 0);
	buffer = dispmgr.GetBuffer(dispctx);
	pitch = dispmgr.GetPitch(dispctx);

	// Setup video manager to play video with button handler as event listener
	CVideoMPI			vidmgr;
	CPath				vidfile = "DidjLogo.ogg";
	CPath				audfile = "DidjLogo.ogg";
	tVideoSurf			mySurf = {width, height, pitch, buffer, format};
	tVideoHndl			myVideo;

	vidmgr.SetVideoResourcePath(GetAppRsrcFolder());
	
	// Run app demo until button press or usb connection
	while (!handler.IsDone())
	{
		myVideo = vidmgr.StartVideo(vidfile, audfile, &mySurf, false, &handler);
		while (vidmgr.IsVideoPlaying(myVideo))
		{	
			while (!handler.IsPressed() && vidmgr.IsVideoPlaying(myVideo))
				kernel.TaskSleep(100);
			if (handler.IsDone())
				break;
			if (handler.IsConnect())
			{
				vidmgr.StopVideo(myVideo);
				dispmgr.UnRegister(dispctx, 0);
				DisplayRGBScreen("Connect.rgb");
				return kKernelExitSuccess;
			}
			if (handler.IsDisconnect())
			{
				vidmgr.StopVideo(myVideo);
				dispmgr.UnRegister(dispctx, 0);
				DisplayRGBScreen("Disconnect.rgb");
				return kKernelExitSuccess;
			}
			if (handler.IsRestart())
			{
				vidmgr.StopVideo(myVideo);
				dispmgr.UnRegister(dispctx, 0);
				DisplayRGBScreen("Restart.rgb");
				powermgr.Reset();
				return kKernelExitReset;
			}
			if (handler.IsShutdown())
			{
				vidmgr.StopVideo(myVideo);
				dispmgr.UnRegister(dispctx, 0);
				DisplayRGBScreen("Shutdown.rgb");
				powermgr.Shutdown();
				return kKernelExitShutdown;
			}
		}
		vidmgr.StopVideo(myVideo);
	}

	button.UnregisterEventListener(&handler);
	dispmgr.UnRegister(dispctx, 0);
	dispmgr.DestroyHandle(dispctx, false);
	return kKernelExitSuccess;
}
