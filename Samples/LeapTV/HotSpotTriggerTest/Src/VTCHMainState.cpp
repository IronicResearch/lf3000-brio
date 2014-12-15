#define RENDER_WITH_OGL 1
#define RENDER_WITH_DISPLAY_MPI 1
#define ALLOCATE_VN_VISION_MPI 1
#define CYCLE_START_STOP_IN_VISION 0
#define ALLOCATE_VISION_ALGORITHM 1
#define ALLOCATE_HOT_SPOTS 1
#define NUM_START_STOP_CYCLES 100
#define FRAME_COUNT_PER_CYCLE 300
#define NUM_INITIAL_FRAMES_TO_PROCESS 1000

#if RENDER_WITH_OGL
#include <GLES/gl.h>
#ifndef KHRONOS
#include <GLES/egl.h>
#endif
#endif

#include "VTCHMainState.h"
#include "VirtualTouchApp.h"
#include <AppManager.h>
#include <BrioOpenGLConfig.h>
#include <Vision/VNRectHotSpot.h>
#include <Vision/VNHotSpotEventMessage.h>
#include <Vision/VNVirtualTouch.h>
#include <Vision/VNOcclusionTrigger.h>
#include <Vision/VNCompoundTrigger.h>
#include <Vision/VNDurationTrigger.h>
#include <Vision/VNIntervalTrigger.h>
#include <Vision/VNCircleHotSpot.h>
#include <Vision/VNArbitraryShapeHotSpot.h>
#include <cmath>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define LESS_THAN_FIVE_PERCENT_COVERAGE 1
#define FIFTY_PERCENT_COVERAGE 2
#define FULL_COVERAGE 3

//#define HOT_SPOT_FLAG LESS_THAN_FIVE_PERCENT_COVERAGE
//#define HOT_SPOT_FLAG FIFTY_PERCENT_COVERAGE
#define HOT_SPOT_FLAG FULL_COVERAGE

using namespace LeapFrog::Brio;
// NOTE: for VGA set kCaptureScale to 1.0 and 
//       for qVGA set kCAptureScale to 0.5
// this must be set to qVGA if you have the qvga_vision_mode flag set
inline bool IS_QVGA()
{
	struct stat s;
	return (0 == stat("/flags/qvga_vision_mode", &s));
}
static const float kCaptureScale = IS_QVGA() ? 0.5f : 1.0f;
static const int kCaptureWidth = kCaptureScale * 640;
static const int kCaptureHeight = kCaptureScale * 480;

VTCHMainState::VTCHMainState(void) :
  visionMPI_(NULL) {
#if ALLOCATE_VN_VISION_MPI
  visionMPI_ = new LF::Vision::VNVisionMPI();
#endif
}

VTCHMainState::~VTCHMainState(void) {
  CleanUpHotSpots();
  if (visionMPI_) {
    visionMPI_->Stop();
    delete visionMPI_;
  }
  CleanUpAlgorithm();
}

void
VTCHMainState::CleanUpHotSpots(void) {
  std::vector<const LF::Vision::VNHotSpot*>::iterator it = hotSpots_.begin();
  for ( ;it != hotSpots_.end(); ++it) {
    const LF::Vision::VNHotSpot *hs = *it;
    if (hs) {
      if (visionMPI_) {
	visionMPI_->RemoveHotSpot(hs);
      }
      delete hs;
      hs = NULL;
    }
  }
  hotSpots_.clear();
}

void
VTCHMainState::CleanUpAlgorithm(void) {
  if (visionAlgorithm_) {
    delete visionAlgorithm_;
  }
  visionAlgorithm_ = NULL;
}

void VTCHMainState::Enter(CStateData* userData) {
	RegisterEventListeners();
	InitSample();
}

void 
VTCHMainState::Exit(void) {
  CleanDisplay();
  UnRegisterEventListeners();
  CleanUpHotSpots();
  if (visionMPI_) {
    visionMPI_->Stop();
  }
  CleanUpAlgorithm();
}

void VTCHMainState::Suspend(void) {
  CleanDisplay();
  UnRegisterEventListeners();
  if (visionMPI_) {
    visionMPI_->Pause();
  }
}

void VTCHMainState::Resume(void) {
  InitDisplay();
  RegisterEventListeners();
}

void VTCHMainState::Update(CGameStateHandler* sh) {
#if CYCLE_START_STOP_IN_VISION
  static int count = 0;

  if (visionMPI_) {
    if (count > NUM_INITIAL_FRAMES_TO_PROCESS && 
	count < 1000+NUM_START_STOP_CYCLES*FRAME_COUNT_PER_CYCLE) {
      
      if ((count % FRAME_COUNT_PER_CYCLE) == 0) {
	visionMPI_->Stop();
      }
    }
    count++;
  }
#endif

  UpdateVisionAlgorithm();
 
  //check for buttons
  DoKeyEventLoop();
  DrawSample();
}

void 
VTCHMainState::SetWindowPosition(void) {
#if RENDER_WITH_DISPLAY_MPI
    displayManager_.SetWindowPosition(displayHandle_, 
				      displayFrame_.left,
				      displayFrame_.top,
				      displayFrame_.right-displayFrame_.left,
				      displayFrame_.bottom - displayFrame_.top,
				      true);
#if RENDER_WITH_OGL
    displayManager_.SetAlpha(brioOGLConfig_->hndlDisplay, 100, true);
#endif
#endif
}

void
VTCHMainState::UpdateVisionAlgorithm(void) {
  if (visionMPI_ && !visionMPI_->IsRunning()) {
    // the true indicates I want to have the vision processing run
    // in a separate thread and not manually call Upate
    LeapFrog::Brio::tErrType error = visionMPI_->Start(&videoDisplaySurface_, 
						      //NULL,
						      false, 
						      &displayFrame_);
    if (error != kNoErr) {
      std::cout << "Error starting visionMPI: " << error << std::endl;
      if (error == LF::Vision::kVNVideoCaptureFailed)
	std::cout << "  Failed to start video capture\n";
      else if (error == LF::Vision::kVNCameraDoesNotSupportRequiredVisionFormat)
	std::cout << "  Camera does not support required vision format\n";
      else if (error = LF::Vision::kVNVideoSurfaceNotOfCorrectSizeForVisionCapture)
	std::cout << "  Display surface is not of the correct size for vision capture\n";
      
      CAppManager::Instance()->PopApp();
    }
    SetWindowPosition();
  } //else {
    //visionMPI_->Update();
  //}
}

int VTCHMainState::DoKeyEventLoop(void) {
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
    
    if (buttonState & kButtonMenu
	|| buttonState & kButtonSync) {
      CAppManager::Instance()->PopApp();
      return 1;
    }
  }
  return 0;
}

void 
VTCHMainState::InitDisplay(void) {
}

void 
VTCHMainState::CleanDisplay(void) {
  displayManager_.DestroyHandle(displayHandle_, true);
}


const tEventType 
gHotSpotEventKey[] = {LF::Vision::kVNHotSpotGroupTriggeredEvent};

VTCHMainState::HotSpotListener::HotSpotListener() : 
  IEventListener(gHotSpotEventKey, 5) {
}

LeapFrog::Brio::tEventStatus
VTCHMainState::HotSpotListener::Notify(const LeapFrog::Brio::IEventMessage& msg) {
  const LF::Vision::VNHotSpotEventMessage& hsMessage = dynamic_cast<const LF::Vision::VNHotSpotEventMessage&>(msg);
  std::vector<const LF::Vision::VNHotSpot*> hs = hsMessage.GetHotSpots();
  
  std::cout << hs.size() << ".";

  return kEventStatusOKConsumed;
}

void 
VTCHMainState::InitSample(void) {
  SetupDisplaySurface();
  SetupOGL();
  SetupVisionAlgorithm();
#if RENDER_WITH_OGL
  displayManager_.SetAlpha(brioOGLConfig_->hndlDisplay, 100, true);
#endif
}

void
VTCHMainState::SetupDisplaySurface(void) {  
  //get device stats
  const LeapFrog::Brio::tDisplayScreenStats* myStats;
  myStats = displayManager_.GetScreenStats(0);
  //NOTE: for now if you decide to use qvga you should change the size of
  // these values to be 320 and 240, otherwise use at least 640 x 480
  screenWidth_ = myStats->width;
  screenHeight_ = myStats->height;
  float screenAspectRatio = (float)screenWidth_/(float)screenHeight_;
  float captureAspectRatio = (float)kCaptureWidth/(float)kCaptureHeight;
  
  if (captureAspectRatio > screenAspectRatio) {
    // fit hieght
    targetWidth_ = screenHeight_*captureAspectRatio;
    targetHeight_ = screenHeight_;
  } else {
    // fit width
    targetWidth_ = screenWidth_;
    targetHeight_ = screenWidth_/captureAspectRatio;
  }
  
  displayFrame_.left = (screenWidth_-targetWidth_)/2;
  displayFrame_.top = (screenHeight_-targetHeight_)/2;
  displayFrame_.right = displayFrame_.left + targetWidth_;
  displayFrame_.bottom = displayFrame_.top + targetHeight_;
  
  printf("\nScreen dimensions (%d, %d)\n\n", screenWidth_, screenHeight_);
  printf("\nTarget dimensions (%d, %d)\n\n", targetWidth_, targetHeight_);
  
  
  displayHandle_ = displayManager_.CreateHandle(targetHeight_,
						targetWidth_,
						//kPixelFormatYUYV422,
						kPixelFormatYUV420,
						//kPixelFormatRGB888,
						NULL);
  
  videoDisplaySurface_.width = kCaptureWidth;
  videoDisplaySurface_.height = kCaptureHeight;
  videoDisplaySurface_.pitch = displayManager_.GetPitch(displayHandle_);
  videoDisplaySurface_.buffer = displayManager_.GetBuffer(displayHandle_);
  videoDisplaySurface_.format = displayManager_.GetPixelFormat(displayHandle_);
  
  displayManager_.Invalidate(0, NULL);
#if RENDER_WITH_DISPLAY_MPI
  if (kNoErr != displayManager_.Register(displayHandle_, 0, 0, kDisplayOnBottom, 0)) {
    DidjAssert(0, "Video Display manager registration failed.");
  }
#endif
}

void
VTCHMainState::SetupOGL(void) {  
#if RENDER_WITH_OGL
  brioOGLConfig_ = new LeapFrog::Brio::BrioOpenGLConfig();
  
  // set up opengl remebering that opengl es 1.1 is a state machine
  // In this app we never change the matrix mode, the orthographic projection
  // or the background color
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrthox(0, targetWidth_<<16, 0, targetHeight_<<16, 10<<16, 0);
  glViewport((screenWidth_-targetWidth_)/2, 0, targetWidth_, targetHeight_); //...
  
  glMatrixMode(GL_MODELVIEW);
  glClearColor(0.0, 0.0, 0.0, 0.0);
#endif
}

void
VTCHMainState::SetupVisionAlgorithm(void) {
  if (visionMPI_) {

#if ALLOCATE_VISION_ALGORITHM
    // setup vision MPI
    visionAlgorithm_ = new LF::Vision::VNVirtualTouch(0.15);
    visionMPI_->SetAlgorithm(visionAlgorithm_);
#endif

#if ALLOCATE_HOT_SPOTS
    // we will use a 20x20 grid of hot spots
    int gridSize = 20;
    int dx = screenWidth_/gridSize;
    int dy = screenHeight_/gridSize;
    int numHotSpots = 0;
    LF::Vision::VNOcclusionTrigger* trigger = new LF::Vision::VNOcclusionTrigger();
    
    int low = 0;
    int high = gridSize;
    
#if HOT_SPOT_FLAG == LESS_THAN_FIVE_PERCENT_COVERAGE
    low = 9;
    high = 10;
#elif HOT_SPOT_FLAG == FIFTY_PERCENT_COVERAGE
    low = 5;
    high = 15;
#elif HOT_SPOT_FLAG == FULL_COVERAGE
    low = 0;
    high = gridSize;
#endif
    
    for (int i = 0; i < gridSize; ++i) {
      int x = i*dx;
      if (i >= low && i < high) {
	for (int j = 0; j < gridSize; ++j) {
	  if (j >= low && j < high) { 
	    int y = j*dy;
	    LeapFrog::Brio::tRect rect;
	    rect.left = x;
	    rect.right = x+dx;
	    rect.top = y;
	    rect.bottom = y+dy;
	    LF::Vision::VNHotSpot* hotSpot = new LF::Vision::VNRectHotSpot(rect);
	    hotSpot->SetTag(numHotSpots);
	    hotSpot->SetTrigger(trigger);
	    hotSpots_.push_back(hotSpot);
	    visionMPI_->AddHotSpot(hotSpot);
	    ++numHotSpots;
	  }
	}
      }
    }
#endif
  }
}

void
VTCHMainState::RenderRectHotSpot(const LF::Vision::VNRectHotSpot *rectHS) {
#if RENDER_WITH_OGL
  glEnableClientState(GL_VERTEX_ARRAY);
  
  const LeapFrog::Brio::tRect&  rect = rectHS->GetRect();
  // screen is mirrored, must mirror points here too
  int numVertPairs = 8; //((rectHS->GetTag() == 4) ? 6 : 8);
  GLfloat verts[2*numVertPairs];
  verts[0] = screenWidth_ - rect.left;  verts[1] = screenHeight_ - rect.top;
  verts[2] = screenWidth_ - rect.right; verts[3] = screenHeight_ - rect.top;
  verts[4] = screenWidth_ - rect.right; verts[5] = screenHeight_ - rect.top;
  verts[6] = screenWidth_ - rect.right; verts[7] = screenHeight_ - rect.bottom;
  
  if (numVertPairs == 8) {
    verts[8] = screenWidth_ - rect.right; verts[9] = screenHeight_ - rect.bottom;
    verts[10] = screenWidth_ - rect.left; verts[11] = screenHeight_ - rect.bottom;
    verts[12] = screenWidth_ - rect.left; verts[13] = screenHeight_ - rect.bottom;
    verts[14] = screenWidth_ - rect.left; verts[15] = screenHeight_ - rect.top;
  } else {
    verts[8] = screenWidth_ - rect.right; verts[9] = screenHeight_ - rect.bottom;
    verts[10] = screenWidth_ - rect.left; verts[11] = screenHeight_ - rect.top;
  }
  
  if (rectHS->IsTriggered()) {
    glColor4f(1, 0, 0, 1);
  } else {
    glColor4f(0, 1, 0, 1);
  }
  
  glVertexPointer(2, GL_FLOAT, 0, verts);
  glDrawArrays(GL_LINES, 0, numVertPairs);
  glDisableClientState(GL_VERTEX_ARRAY);
  glColor4f(1,1,1,1);
#endif
}

void
VTCHMainState::RenderCircleHotSpot(const LF::Vision::VNCircleHotSpot *cHS) {
#if RENDER_WITH_OGL
  glEnableClientState(GL_VERTEX_ARRAY);
  
  LF::Vision::VNPoint c = cHS->GetCenter();
  float r = cHS->GetRadius();
  // screen is mirrored, must mirror points here too
  c.x = fabs(screenWidth_ - c.x);
  c.y = fabs(screenHeight_ - c.y);
  
  static const int numLines = 15;
  static GLfloat cVerts[4*numLines];
  cVerts[0] = c.x + r;
  cVerts[1] = c.y;
  for (int i = 1; i < numLines; ++i) {
    float degInRad = i*(360/numLines)*M_PI/180.0f;
    cVerts[4*i-2] = c.x + r*cos(degInRad);
    cVerts[4*i-1] = c.y + r*sin(degInRad);
    cVerts[4*i  ] = c.x + r*cos(degInRad);
    cVerts[4*i+1] = c.y + r*sin(degInRad);
  }
  cVerts[4*numLines-2] = c.x + r;
  cVerts[4*numLines-1] = c.y;
  
  if (cHS->IsTriggered()) {
    glColor4f( 1, 0, 0, 1 );
  } else {
    glColor4f( 0, 1, 0, 1 );
  }
  
  glVertexPointer(2, GL_FLOAT, 0, cVerts);
  glPushMatrix();
  glDrawArrays(GL_LINES, 0, 4*numLines);
  glPopMatrix();
  
  glDisableClientState(GL_VERTEX_ARRAY);
  glColor4f(1,1,1,1);
#endif
}

void
VTCHMainState::DrawHotSpots(void) {
#if RENDER_WITH_OGL
  glPushMatrix();
  glLoadIdentity();
#if !defined(EMULATION)
  glTranslatef( screenWidth_, 0, 0 );
  glScalef(-1, 1, 1);
#endif
  
  std::vector<const LF::Vision::VNHotSpot*>::iterator hotspot_iter;
  for (hotspot_iter = hotSpots_.begin(); hotspot_iter != hotSpots_.end(); hotspot_iter++ ) {
    const LF::Vision::VNRectHotSpot *rectHS = dynamic_cast<const LF::Vision::VNRectHotSpot*>(*hotspot_iter);
    if (rectHS) {
      RenderRectHotSpot(rectHS);
    } else {
      const LF::Vision::VNCircleHotSpot *cHS = dynamic_cast<const LF::Vision::VNCircleHotSpot*>(*hotspot_iter);
      if (cHS) {
	RenderCircleHotSpot(cHS);
      }
    }
  }
  glPopMatrix();
#endif
}

void 
VTCHMainState::DrawSample(void) {
#if RENDER_WITH_OGL
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  DrawHotSpots();
  
  eglSwapBuffers(brioOGLConfig_->eglDisplay, brioOGLConfig_->eglSurface);
#endif
}

void
VTCHMainState::RegisterEventListeners(void) {
  //register keys
  LeapFrog::Brio::CButtonMPI keyEventMPI;
  keyEventMPI.RegisterEventListener(&buttonEventQueue_);
  
  // register for hot spot events
  LeapFrog::Brio::CEventMPI eventMPI;
  eventMPI.RegisterEventListener(&hotspotListener_);
}

void
VTCHMainState::UnRegisterEventListeners(void) {
  //clean keys
  LeapFrog::Brio::CButtonMPI keyEventMPI;
  keyEventMPI.UnregisterEventListener(&buttonEventQueue_);
  
  // clean hot spot listener
  LeapFrog::Brio::CEventMPI eventMPI;
  eventMPI.UnregisterEventListener(&hotspotListener_);
}
