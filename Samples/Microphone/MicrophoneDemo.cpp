/******************************************************************************

 @Description  Simple CameraMPI SDK example for Microphone capture

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
LF_USING_BRIO_NAMESPACE()

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
	return "/LF/Bulk/ProgramFiles/MicrophoneDemo/rsrc/";
#endif	// EMULATION
}
	
//============================================================================
// Button / Video / Audio Event Listener
//============================================================================

const tEventType kMyHandledTypes[] = { kAllButtonEvents, kAllCameraEvents };

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
			else if ( type_ ==  kAudioTriggeredEvent )
			{
				const CCameraEventMessage& cammsg = reinterpret_cast<const CCameraEventMessage&>(msg);
				dbg_.DebugOut(kDbgLvlImportant, "kAudioTriggeredEvent posted threshold = %d\n", cammsg.data.triggered.threshold);
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
	CDisplayMPI 	dispmgr;
	dbg.SetDebugLevel(kDbgLvlImportant);
	
	// Setup button handler
	CButtonMPI		button;
	MyBtnEventListener	handler;
	
	if (button.RegisterEventListener(&handler) != kNoErr) {
		dbg.DebugOut(kDbgLvlCritical, "FALIURE to load button manager!\n");
		return -1;
	}
	
	// Setup camera MPI to capture audio with button handler as event listener
	CCameraMPI			camera;
	CPath				micfile = "test.wav";
	tAudCapHndl			myAudio;

	// Command line overide option for audio capture file
	if (argc > 1)
		micfile = argv[1];
	else
		camera.SetCameraAudioPath(GetAppRsrcFolder());

	// Option for capture input without saving to file = microphone trigger event
	if (micfile == "none")
		micfile = "";

	// Start microphone audio capture
	myAudio = camera.StartAudioCapture(micfile, &handler);
	
	// Continue capturing audio until button press
	while (myAudio != kInvalidHndl)
	{
		kernel.TaskSleep(100);
		
		if (handler.IsDone())
			break;
		if (handler.IsPaused())
		{
			camera.PauseAudioCapture(myAudio);
			while (handler.IsPaused())
				kernel.TaskSleep(100);
			camera.ResumeAudioCapture(myAudio);
		}
	}

	camera.StopAudioCapture(myAudio);

	button.UnregisterEventListener(&handler);

	return 0;
}
