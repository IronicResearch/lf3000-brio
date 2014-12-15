#include "ACCMainState.h"
#include <AppManager.h>
#include <TouchEventQueue.h>

using LeapFrog::Brio::S16;
const LeapFrog::Brio::tEventType kACCListenerTypes[] = { LeapFrog::Brio::kAllAccelerometerEvents };

ACCMainState* ACCMainState::Instance(void) {
  // singleton access point
  static ACCMainState* instance = NULL;
  if (!instance) {
    instance = new ACCMainState();
  }
  return instance;
}

ACCMainState::ACCMainState(void) :
  IEventListener(kACCListenerTypes, ArrayCount(kACCListenerTypes)),
  accelerometerMPI_(NULL),
  config_(NULL) {
  // Query screen size
  LeapFrog::Brio::CDisplayMPI display_mpi;
  const LeapFrog::Brio::tDisplayScreenStats* pscreen = display_mpi.GetScreenStats(0);
  screenWidth_ = pscreen->width;
  screenHeight_ = pscreen->height;
}

ACCMainState::~ACCMainState(void) {
  if (accelerometerMPI_) {
    delete accelerometerMPI_;
    accelerometerMPI_ = NULL;
  }
  
  if (config_) {
    delete config_;
    config_ = NULL;
  } 
}

void ACCMainState::Enter(CStateData* userData) {
  Init();
  RegisterEventListeners();
}

void ACCMainState::Exit(void) {
  UnregisterEventListeners();
}

void ACCMainState::Suspend(void) {
  CAppManager::Instance()->EnableInactivity(true);
  UnregisterEventListeners();
}

void ACCMainState::Resume(void) {
  RegisterEventListeners();
}

void ACCMainState::Update(CGameStateHandler* sh) {
  //check for input
  DoKeyEventLoop();
  DoTouchEventLoop();
  
  if (accelerometerOn_ && !touching_) {
    DoAccEventLoop();
    MoveObjectWithGravity();
  } else if (touching_) {
    MoveObjectIncrementally();
  }
  
  RenderOGL();
  eglSwapBuffers(config_->eglDisplay, 
		 config_->eglSurface);
}

int ACCMainState::DoKeyEventLoop(void) {
  std::vector<LeapFrog::Brio::tButtonData2> *buttonEventQueue = buttonEventQueue_.GetQueue();
  std::vector<LeapFrog::Brio::tButtonData2>::const_iterator buttonReader = buttonEventQueue->begin();
  
  while (buttonReader != buttonEventQueue->end()) {
    static LeapFrog::Brio::U16 buttonState = 0;
    LeapFrog::Brio::tButtonData2 button_data = *(buttonReader++);
    buttonState = button_data.buttonState;
    
    if ((buttonState & LeapFrog::Brio::kButtonLeft)) {
      gravity_*=-1;
    } else if ((buttonState & LeapFrog::Brio::kButtonA)) {
      accelerometerOn_ = !accelerometerOn_;
    } else if ((buttonState & LeapFrog::Brio::kButtonB)) {
      ResetObject();
    } else if((buttonState & LeapFrog::Brio::kButtonLeftShoulder)) {
      gravity_*=-1;
    }
    if (buttonState & LeapFrog::Brio::kButtonMenu) {
      CAppManager::Instance()->PopApp();
    }
  }
  return 0;
}

int ACCMainState::DoTouchEventLoop(void) {
  std::vector<LeapFrog::Brio::tTouchData> *touchEventQueue = touchEventQueue_.GetQueue();
  std::vector<LeapFrog::Brio::tTouchData>::const_iterator touchReader = touchEventQueue->begin();
  
  while (touchReader != touchEventQueue->end()) {
    static LeapFrog::Brio::U16 touchState = 0;
    LeapFrog::Brio::tTouchData touchData = *(touchReader++);
    
    //get touchState:
    //0:	up
    //1:	down
    touchState = touchData.touchState;
    
    //check for button press
    int pen_x = touchData.touchX-screenWidth_/2;
    int pen_y = screenHeight_ - touchData.touchY-screenHeight_/2;	//need to reverse the values
    
    if (touchState == 1) {
      touching_ = true;
      dx_ = ((float)pen_x-xPos_)/10;
      dy_ = ((float)pen_y-yPos_)/10;
      dz_ = (-300-zPos_)/10;
    } else {
      touching_ = false;
      dx_ = 0.0f;
      dy_ = 0.0f;
      dz_ = 0.0f;
    }
  }
  return 0;
}

LeapFrog::Brio::tEventStatus ACCMainState::Notify( const LeapFrog::Brio::IEventMessage &msgIn ) {
  if (msgIn.GetEventType() == LeapFrog::Brio::kAccelerometerDataChanged) {
    const LeapFrog::Brio::CAccelerometerMessage& aclmsg = reinterpret_cast<const LeapFrog::Brio::CAccelerometerMessage&>(msgIn);
    LeapFrog::Brio::tAccelerometerData acldata = aclmsg.GetAccelerometerData();
    // Update current accelerometer x,y,z data
    x_ = acldata.accelX;
    y_ = acldata.accelY;
    z_ = acldata.accelZ;
    printf( "accel: %d, %d, %d\n", x_, y_, z_ );
    return LeapFrog::Brio::kEventStatusOKConsumed;
  }
  return LeapFrog::Brio::kEventStatusOK;
}

int ACCMainState::DoAccEventLoop(void) {
  // Accelerometer x,y,z data updated in ::Notify() listener (above)
  
  dx_+=((float)x_/kAcceleration);
  dy_+=((float)y_/kAcceleration);
  dz_+=((float)z_/kAcceleration);
  
  return 0;
}

void ACCMainState::MoveObjectWithGravity(void) {
  float dist = kLongestSide*kObjectSize;
  xPos_-=(gravity_*dx_);
  if (xPos_>screenWidth_/2-dist) {
    xPos_ = screenWidth_/2-dist;
    dx_ = -dx_/2;
  } else if (xPos_<-screenWidth_/2+dist) {
    xPos_ = -screenWidth_/2+dist;
    dx_ = -dx_/2;
  }

  yPos_+=(gravity_*dy_);
  if (yPos_>screenHeight_/2-dist) {
    yPos_ = screenHeight_/2-dist;
    dy_ = -dy_/2;
  } else if (yPos_<-screenHeight_/2+dist) {
    yPos_ = -screenHeight_/2+dist;
    dy_ = -dy_/2;
  }

  zPos_+=(gravity_*dz_);
  if (zPos_<-farPlane_+dist) {
    zPos_ = -farPlane_+dist;
    dz_ = -dz_/2;
  } else if(zPos_>-nearPlane_-dist) {
    zPos_ = -nearPlane_-dist;
    dz_ = -dz_/2;
  }
}

void ACCMainState::MoveObjectIncrementally(void) {
  xPos_ += dx_;
  yPos_ += dy_;
  zPos_ += dz_;
}

void ACCMainState::RenderOGL(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear The Screen And The Depth Buffer
  
  //***************
  //pyramid
  //***************
  glLoadIdentity();
  
  static const GLfloat myVertices[] = {
    -kObjectSize, -kObjectSize, kObjectSize,
    kObjectSize, -kObjectSize, kObjectSize,
    0, kObjectSize, 0,
    -kObjectSize, -kObjectSize, kObjectSize,
    0, -kObjectSize, -kObjectSize,
    0, kObjectSize, 0,
    0, kObjectSize, 0,
    kObjectSize, -kObjectSize, kObjectSize,
    0, -kObjectSize, -kObjectSize,
    0, -kObjectSize, -kObjectSize,
    -kObjectSize, -kObjectSize, kObjectSize,
    kObjectSize, -kObjectSize, kObjectSize,
  };
  
  static const GLubyte myColors[] = {
    255,	255,	0,		255,
    0,		255,	0,		255,
    0,		255,	0,		255,
    255,	255,	0,		255,
    255,	0,		255,	255,
    0,		0,		255,	255,
    0,		0,		255,	255,
    255,	0,		255,	255,
    255,	0,		255,	255,
    255,	255,	0,		255,
    0,		0,		255,	255,
    0,		0,		255,	255,
  };
  
  //Transformations
  glTranslatef(xPos_, yPos_, zPos_);
  glRotatef(rotation_, 0.0f, 1.0f, 0.0f);
  rotation_ += 5;
  
  glPushMatrix();
  
  //draw it
  glVertexPointer(3, GL_FLOAT, 0, myVertices);
  glColorPointer(4, GL_UNSIGNED_BYTE, 0, myColors);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 12);
  
  glPopMatrix();
}

void ACCMainState::ResetObject(void) {
  xPos_ = 0.0f;
  yPos_ = 0.0f;
  zPos_ = -(nearPlane_ + initialZ_);
  
  dx_ = 0.0f;
  dy_ = 0.0f;
  dz_ = 0.0f;
  
  rotation_ = 0;
}

void ACCMainState::InitOGL(void) {
  //vars
  initialZ_  = 250;
  nearPlane_ = 300;
  farPlane_  = 1000;
  
  ResetObject();
  
  //colors
  bgColor_.r_ = 100;
  bgColor_.g_ = 100;
  bgColor_.b_ = 120;
  
  config_ = new LeapFrog::Brio::BrioOpenGLConfig(8, 8);
  
  //figure out what the rest of this stuff does
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_ALPHA_TEST);
  glAlphaFuncx(GL_GREATER, 0);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glCullFace(GL_FRONT);
  
  glDisable(GL_TEXTURE_2D);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  
  //reset perspective
  nearPlane_ = 300;
  farPlane_  = 1000;
  
  SetClearColor(bgColor_.r_, bgColor_.g_, bgColor_.b_, 255);
  
  glViewport(0, 0, screenWidth_, screenHeight_);
  
  //set on here (always on)
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glDisable(GL_BLEND);
  
  tVect2 screensize = tVect2(screenWidth_, screenHeight_);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  glFrustumx(
	     -screensize.x.g/2,    //left
	     screensize.x.g/2,     //right
	     -screensize.y.g/2,    //bottom
	     screensize.y.g/2,     //top
	     tFixed(nearPlane_).g, //near
	     tFixed(farPlane_).g   //far
	     );
  
  glMatrixMode(GL_MODELVIEW);
}

void ACCMainState::SetClearColor(S16 r, S16 g, S16 b, S16 a) {
  tColor clearColor = tColor(r, g, b, a);
  
  glClearColorx((GLfixed) clearColor.r * 256,
		(GLfixed) clearColor.g * 256,
		(GLfixed) clearColor.b * 256,
		(GLfixed) clearColor.a * 256);
}

void ACCMainState::Init(void) {
  accelerometerOn_ = true;
  gravity_ = 1;
  touching_ = false;
  
  InitOGL();
}

void ACCMainState::RegisterEventListeners(void) {
  //register touch screen
  LeapFrog::Brio::CEventMPI eventMPI;
  eventMPI.RegisterEventListener(&touchEventQueue_);
  
  //register keys
  LeapFrog::Brio::CButtonMPI key_eventMPI;
  key_eventMPI.RegisterEventListener(&buttonEventQueue_);
  key_eventMPI.SetTouchMode( LeapFrog::Brio::kTouchModeDefault );
  
  // register for accelerometer notifications
  accelerometerMPI_ = new LeapFrog::Brio::CAccelerometerMPI;
  accelerometerMPI_->RegisterEventListener(this);
}

void ACCMainState::UnregisterEventListeners(void) {
  // stop listening for screen touch events
  LeapFrog::Brio::CEventMPI eventMPI;
  eventMPI.UnregisterEventListener(&touchEventQueue_);
  
  // stop listening for physical button events
  LeapFrog::Brio::CButtonMPI key_eventMPI;
  key_eventMPI.UnregisterEventListener(&buttonEventQueue_);

  // stop listening for accelerometer events
  accelerometerMPI_->UnregisterEventListener(this);
}
