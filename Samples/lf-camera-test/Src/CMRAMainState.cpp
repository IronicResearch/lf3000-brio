//Emerald Sample Project
#include "CMRAMainState.h"
#include "CameraApp.h"
#include <AppManager.h>
#include <CGraphics2D.h>

using namespace LeapFrog::Brio;

CMRAMainState::CMRAMainState(void) {
}

CMRAMainState::~CMRAMainState(void) {
}

void CMRAMainState::Enter(CStateData* userData) {
  RegisterEventListeners();
  InitSample();
}

void 
CMRAMainState::Exit(void) {
  cameraMPI_.StopVideoCapture(videoCapture_);
  CleanDisplay();
  UnRegisterEventListeners();
}

void CMRAMainState::Suspend(void) {
  cameraMPI_.PauseVideoCapture(videoCapture_);
  CleanDisplay();
  UnRegisterEventListeners();
}

void CMRAMainState::Resume(void) {
  cameraMPI_.ResumeVideoCapture(videoCapture_);
  RegisterEventListeners();
}

void CMRAMainState::Update(CGameStateHandler* sh) {
  //check for buttons
  DoKeyEventLoop();
}

int CMRAMainState::DoKeyEventLoop(void) {
  std::vector<LeapFrog::Brio::tButtonData2> *buttonEventQueue = buttonEventQueue_.GetQueue();
  std::vector<LeapFrog::Brio::tButtonData2>::const_iterator buttonReader = buttonEventQueue->begin();
  
  while (buttonReader != buttonEventQueue->end()) {
    static U32 buttonState = 0;
    LeapFrog::Brio::tButtonData2 buttonData = *(buttonReader++);
    buttonState = buttonData.buttonState;
    
    /*
     * button states:
     * kButtonMenu, kButtonHint
     * kButtonLeft, kButtonRight, kButtonUp, kButtonDown,
     * kButtonLeftShoulder, kButtonRightShoulder,
     * kButtonA, kButtonB
     */
    
    if (buttonState & (kButtonMenu | kButtonEscape | kButtonSync)) {
      CAppManager::Instance()->PopApp();
      return 1;
    }
  }
  return 0;
}

void 
CMRAMainState::CleanDisplay(void) {
  displayManager_.DestroyHandle(displayHandle_, true);
}


const tEventType 
gCameraEventKey[] = {kCaptureFrameEvent};
//kCaptureTimeOutEvent, 
//kCaptureQuotaHitEvent, 
//kCaptureStoppedEvent, 
//kCameraRemovedEvent, 
//kAudioTriggeredEvent};

CMRAMainState::CameraListener::CameraListener() : 
  IEventListener(gCameraEventKey, 5),
  frameCount_(0),
  frameTime_(time(0)) {
}

LeapFrog::Brio::tEventStatus
CMRAMainState::CameraListener::Notify(const LeapFrog::Brio::IEventMessage& msg) {
  const LeapFrog::Brio::CCameraEventMessage *cemsg =
    dynamic_cast<const LeapFrog::Brio::CCameraEventMessage*>(&msg);
  if (cemsg) {
    if (cemsg->GetEventType() == LeapFrog::Brio::kCaptureFrameEvent) {
      
      LeapFrog::Brio::tCaptureFrameMsg frameMsg = cemsg->data.framed;
      LeapFrog::Brio::tVideoSurf *surf = cameraMPI_.GetCaptureVideoSurface(frameMsg.vhndl);

      ++frameCount_;
      if (frameCount_ % 30 == 0) {
	std::cout << "FPS = " << frameCount_ / ((float)(time(0) - frameTime_)) << std::endl;
      }
    }
  }
  
  return kEventStatusOK;
}

void 
CMRAMainState::InitSample(void) {
  SetupDisplaySurface(); 
}

void
CMRAMainState::SetupDisplaySurface(void) {  
	//get device stats
	const LeapFrog::Brio::tDisplayScreenStats* myStats;
	myStats = displayManager_.GetScreenStats(0);

	screenWidth_ = myStats->width;
	screenHeight_ = myStats->height;

	//set display properties
	//scaling will only work on embedded builds. Emulation does not support scaling.
	int targetWidth = 512;
	int targetHeight = 512;

	int offset_x = (screenWidth_-targetWidth)/2;
	int offset_y = (screenHeight_-targetHeight)/2;

	//set up display
	displayHandle_ = displayManager_.CreateHandle(targetHeight,
			targetWidth,
			kPixelFormatYUV420,
			NULL);
	displayManager_.Register(displayHandle_, 0, 0, kDisplayOnTop, 0);
	displayManager_.SetWindowPosition(displayHandle_, offset_x, offset_y, targetWidth, targetHeight, false);

	//set up camera
	tCaptureMode* mode = cameraMPI_.GetCurrentFormat();
	mode->width = 320;
	mode->height = 240;
	cameraMPI_.SetCurrentFormat(mode);

	videoDisplaySurface_.width = mode->width;
	videoDisplaySurface_.height = mode->height;
	videoDisplaySurface_.pitch = displayManager_.GetPitch(displayHandle_);
	videoDisplaySurface_.buffer = displayManager_.GetBuffer(displayHandle_);
	videoDisplaySurface_.format = displayManager_.GetPixelFormat(displayHandle_);

	displayManager_.SetVideoScaler(displayHandle_, mode->width, mode->height, false);
	videoCapture_ = cameraMPI_.StartVideoCapture(&videoDisplaySurface_, &cameraListener_, "");
}

void
CMRAMainState::RegisterEventListeners(void) {
  //register keys
  LeapFrog::Brio::CButtonMPI keyEventMPI;
  keyEventMPI.RegisterEventListener(&buttonEventQueue_);

  // register for camera events
  LeapFrog::Brio::CEventMPI eventMPI;
  eventMPI.RegisterEventListener(&cameraListener_);
}

void
CMRAMainState::UnRegisterEventListeners(void) {
  //clean keys
  LeapFrog::Brio::CButtonMPI keyEventMPI;
  keyEventMPI.UnregisterEventListener(&buttonEventQueue_);

  // clean camera listener
  LeapFrog::Brio::CEventMPI eventMPI;
  eventMPI.UnregisterEventListener(&cameraListener_);
}
