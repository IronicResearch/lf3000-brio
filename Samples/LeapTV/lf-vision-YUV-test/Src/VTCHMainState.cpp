// NOTE: Turning draw hotspots off speeds things up quite a bit
#define DRAW_HOTSPOTS 1

#if DRAW_HOTSPOTS
#include <GLES/gl.h>
#ifndef KHRONOS
#include <GLES/egl.h>
#endif
#include <BrioOpenGLConfig.h>
#endif

#include "VTCHMainState.h"
#include "VirtualTouchApp.h"
#include <AppManager.h>
#include <Vision/VNRectHotSpot.h>
#include <Vision/VNHotSpotEventMessage.h>
#include <Vision/VNVirtualTouch.h>
#include <Vision/VNOcclusionTrigger.h>
#include <Vision/VNCompoundTrigger.h>
#include <Vision/VNDurationTrigger.h>
#include <Vision/VNIntervalTrigger.h>
#include <Vision/VNCircleHotSpot.h>
#include <Vision/VNArbitraryShapeHotSpot.h>
#include <Vision/VNPointTrigger.h>
#include <cmath>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace LeapFrog::Brio;
// NOTE: for VGA set kCaptureScale to 1.0 and
//       for qVGA set kCAptureScale to 0.5
// this must be set to qVGA if you have the qvga_vision_mode flag set
inline bool IS_QVGA()
{
	struct stat s;
	return (0 == stat("/flags/qvga_vision_mode", &s));
}
#define kCaptureScale (IS_QVGA() ? 0.5f : 1.0f)
static const int kCaptureWidth = 640 * kCaptureScale;
static const int kCaptureHeight = 480 * kCaptureScale;

VTCHMainState::VTCHMainState(void) {
}

VTCHMainState::~VTCHMainState(void) {
}

void VTCHMainState::Enter(CStateData* userData) {
  RegisterEventListeners();
  InitSample();
}

void
VTCHMainState::Exit(void) {
  CleanDisplay();
  UnRegisterEventListeners();
  visionMPI_.Stop();
}

void VTCHMainState::Suspend(void) {
  CleanDisplay();
  UnRegisterEventListeners();
  visionMPI_.Pause();
}

void VTCHMainState::Resume(void) {
  InitDisplay();
  RegisterEventListeners();
}

void VTCHMainState::Update(CGameStateHandler* sh) {
  UpdateVisionAlgorithm();
  //check for buttons
  DoKeyEventLoop();
#if DRAW_HOTSPOTS
  DrawSample();
#endif // DRAW_HOTSPOTS
}

void
VTCHMainState::SetWindowPosition(void) {
  displayManager_.SetWindowPosition(displayHandle_,
				    displayFrame_.left,
				    displayFrame_.top,
				    displayFrame_.right-displayFrame_.left,
				    displayFrame_.bottom - displayFrame_.top,
				    true);
#if DRAW_HOTSPOTS
  displayManager_.SetAlpha(brioOGLConfig_->hndlDisplay, 100, true);
#endif
}

void
VTCHMainState::UpdateVisionAlgorithm(void) {
  if (!visionMPI_.IsRunning()) {
    // the true indicates I want to have the vision processing run
    // in a separate thread and not manually call Upate
    LeapFrog::Brio::tErrType error = visionMPI_.Start(&videoDisplaySurface_, false, &displayFrame_);
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
    //visionMPI_.Update();
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
gHotSpotEventKey[] = {LF::Vision::kVNHotSpotTriggeredEvent};

VTCHMainState::HotSpotListener::HotSpotListener() :
  IEventListener(gHotSpotEventKey, 5) {
}

LeapFrog::Brio::tEventStatus
VTCHMainState::HotSpotListener::Notify(const LeapFrog::Brio::IEventMessage& msg) {
  const LF::Vision::VNHotSpotEventMessage& hsMessage = dynamic_cast<const LF::Vision::VNHotSpotEventMessage&>(msg);
  const LF::Vision::VNHotSpot* hs = hsMessage.GetHotSpot();
  
  if (hs->IsTriggered()) {
    // is triggered
    //std::cout << "triggered" << std::endl;
    
    LF::Vision::VNOcclusionTrigger *trigger = static_cast<LF::Vision::VNOcclusionTrigger*>(hs->GetTrigger());
    // float po = trigger->GetPercentOccluded();
    // std::cout << "The percent occluded is: " << po << std::endl;
    
  }
  
  if (hs->GetTag() == 3) {
    // query the percent occluded
    // LF::Vision::VNOcclusionTrigger *trigger = static_cast<LF::Vision::VNOcclusionTrigger*>(hs->GetTrigger());
    // float po = trigger->GetPercentOccluded();
    // std::cout << "The percent occluded is: " << po << std::endl;
  }
  return kEventStatusOK;
}

void
VTCHMainState::InitSample(void) {
  SetupDisplaySurface();
  SetupOGL();
  SetupVisionAlgorithm();
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
  displayFrame_.bottom =  displayFrame_.top + targetHeight_;
  
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
  //videoDisplaySurface_.buffer += 10*videoDisplaySurface_.pitch;
  
  displayManager_.Invalidate(0, NULL);
  if (kNoErr != displayManager_.Register(displayHandle_, 0, 0, kDisplayOnBottom, 0))
    {
      DidjAssert(0, "Video Display manager registration failed.");
    }
}

void
VTCHMainState::SetupOGL(void) {
#if DRAW_HOTSPOTS  
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
  // setup vision MPI
  visionAlgorithm_ = new LF::Vision::VNVirtualTouch(0.15);
  visionMPI_.SetAlgorithm(visionAlgorithm_);
  
  LeapFrog::Brio::tRect hotSpotRect1;
  hotSpotRect1.left = 10;
  hotSpotRect1.right = 60;
  hotSpotRect1.top = 10;
  hotSpotRect1.bottom = 40;
  LF::Vision::VNHotSpot* hotSpot1 = new LF::Vision::VNRectHotSpot(hotSpotRect1);
  hotSpot1->SetTag(1);
  
  // This hot spot will trigger at 20% occlusion but not more frequently than 4 seconds
  // between trigger events
  LF::Vision::VNOcclusionTrigger* trigger1 = new LF::Vision::VNOcclusionTrigger();
  LF::Vision::VNIntervalTrigger* intervalTrigger = new LF::Vision::VNIntervalTrigger(1.0f);
  LF::Vision::VNCompoundTrigger*
    cmpTrigger = new LF::Vision::VNCompoundTrigger(trigger1,
						   intervalTrigger);
  hotSpot1->SetTrigger(cmpTrigger);
  hotSpots_.push_back(hotSpot1);
  visionMPI_.AddHotSpot(hotSpot1);
  
  
  // this hot spot is clipped as it is partially offscreen
  LeapFrog::Brio::tRect hotSpotRect2;
  hotSpotRect2.left = 260;
  hotSpotRect2.right = 310;
  hotSpotRect2.top = 10;
  hotSpotRect2.bottom = 40;
  // This hot spot will trigger once it has over 20% occlusio for more than 1.0 second
  LF::Vision::VNHotSpot* hotSpot2 = new LF::Vision::VNRectHotSpot(hotSpotRect2);
  hotSpot2->SetTag(2);
  LF::Vision::VNDurationTrigger *temporalTrigger = new LF::Vision::VNDurationTrigger(1.0);
  LF::Vision::VNCompoundTrigger *compoundTrigger = new LF::Vision::VNCompoundTrigger(trigger1,
										     temporalTrigger);
  hotSpot2->SetTrigger(compoundTrigger);
  hotSpots_.push_back(hotSpot2);
  visionMPI_.AddHotSpot(hotSpot2);
  
  LF::Vision::VNCircleHotSpot *hotSpot3 = new LF::Vision::VNCircleHotSpot(LF::Vision::VNPoint(0.7*screenWidth_,0), 24.0f);
  hotSpot3->SetTag(3);
  LF::Vision::VNOcclusionTrigger *occTrigger = new LF::Vision::VNOcclusionTrigger();
  hotSpot3->SetTrigger(occTrigger);
  hotSpots_.push_back(hotSpot3);
  visionMPI_.AddHotSpot(hotSpot3);
  
  // add an arbitray shaped hot spot, programmatically created
  // that is also clipped by being partially off screen
  LeapFrog::Brio::tRect hotSpotRect4;
  int halfWidth = 24;
  int width = 2*halfWidth;
  hotSpotRect4.left = 0-halfWidth;
  hotSpotRect4.right = 0+halfWidth;
  hotSpotRect4.top = 100-halfWidth;
  hotSpotRect4.bottom = 100+halfWidth;
  unsigned char *data = (unsigned char*)(malloc(width*width*sizeof(unsigned char)));
  
  // create an upper triangular mask
  // set all pixels that are in the upper right triangular protion of the square to
  // a pixel value of 0, part of the mask.  All other pixels will be set to 255, or
  // not part of the mask.
  for (int i = 0; i < width; ++i) {
    for (int j = 0; j < width; ++j) {
      if (j >= i)
	data[i*width+j] = LF::Vision::kVNMaxPixelValue;
      else
	data[i*width+j] = LF::Vision::kVNMinPixelValue;
    }
  }
  cv::Mat mask(cv::Size(width,width), CV_8U, data);
  LF::Vision::VNHotSpot*
    hotSpot4 = new LF::Vision::VNArbitraryShapeHotSpot(hotSpotRect4,
						       mask);
  hotSpot4->SetTag(4);
  hotSpot4->SetTrigger(trigger1);
  hotSpots_.push_back(hotSpot4);
  visionMPI_.AddHotSpot(hotSpot4);


  // add a point trigger hot spot to insure the system can handle having 
  // an incorrect trigger type based on the algorithm it's using
  LeapFrog::Brio::tRect hotSpotRect5;
  hotSpotRect1.left = 210;
  hotSpotRect1.right = 260;
  hotSpotRect1.top = 210;
  hotSpotRect1.bottom = 240;
  LF::Vision::VNHotSpot* hotSpot5 = new LF::Vision::VNRectHotSpot(hotSpotRect5);
  hotSpot5->SetTag(5);
  
  LF::Vision::VNPointTrigger* trigger5 = new LF::Vision::VNPointTrigger();
  hotSpot5->SetTrigger(trigger5);
  visionMPI_.AddHotSpot(hotSpot5);

}

void
VTCHMainState::RenderRectHotSpot(const LF::Vision::VNRectHotSpot *rectHS) {
#if DRAW_HOTSPOTS
  glEnableClientState(GL_VERTEX_ARRAY);
  
  const LeapFrog::Brio::tRect&  rect = rectHS->GetRect();
  // screen is mirrored, must mirror points here too
  int numVertPairs = ((rectHS->GetTag() == 4) ? 6 : 8);
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
#if DRAW_HOTSPOTS
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
#if DRAW_HOTSPOTS
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
#if DRAW_HOTSPOTS
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
