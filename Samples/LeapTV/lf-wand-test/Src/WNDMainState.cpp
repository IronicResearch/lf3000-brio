//Emerald Sample Project
#ifndef KHRONOS
#include <GLES/egl.h>
#endif

#include "WNDMainState.h"
#include "WandApp.h"
#include <AppManager.h>
#include <CGraphics2D.h>
#include <Vision/VNRectHotSpot.h>
#include <Vision/VNHotSpotEventMessage.h>
#include <Vision/VNWandTracker.h>
#include <Vision/VNWand.h>
#include <Vision/VNPointTrigger.h>
#include <Vision/VNArbitraryShapeHotSpot.h>
#include <Hardware/HWControllerEventMessage.h>
#include <Vision/VNOcclusionTrigger.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace LeapFrog::Brio;

inline bool IS_QVGA()
{
	struct stat s;
	return (0 == stat("/flags/qvga_vision_mode", &s));
}

// NOTE: for VGA set kCaptureScale to 1.0 and 
//       for qVGA set kCAptureScale to 0.5
#define kCaptureScale (IS_QVGA() ? 0.5f : 1.0f)
static const int kCaptureWidth = 640 * kCaptureScale;
static const int kCaptureHeight = 480 * kCaptureScale;
const LeapFrog::Brio::tEventType 
kWNDListenerTypes[] = { LF::Hardware::kHWControllerConnected,
		LF::Hardware::kHWControllerDisconnected,
		LF::Hardware::kHWControllerModeChanged,
		LF::Hardware::kHWControllerButtonStateChanged };

WNDMainState::WNDMainState(void) :
		  LeapFrog::Brio::IEventListener(kWNDListenerTypes, ArrayCount(kWNDListenerTypes)),
		  controller_(NULL),
		  color_(0) {
}

WNDMainState::~WNDMainState(void) {
  printf("WNDMainState destructor\n");
  if (visionAlgorithm_) {
    delete visionAlgorithm_;
  }
  for (std::vector<const LF::Vision::VNHotSpot*>::iterator it = hotSpots_.begin();
       it != hotSpots_.end(); ++it) {
    delete *it;
  }
  hotSpots_.clear();
}

LeapFrog::Brio::tEventStatus 
WNDMainState::Notify( const LeapFrog::Brio::IEventMessage &msgIn ) {
	const LF::Hardware::HWControllerEventMessage& msg = reinterpret_cast<const LF::Hardware::HWControllerEventMessage&>(msgIn);
	const LF::Hardware::HWController *controller = msg.GetController();

	int id = controller->GetID();
	printf("Notify for controller: %d\n\t", id);

	if (msgIn.GetEventType() == LF::Hardware::kHWControllerConnected) {
		printf("\nController has connected\n");
		TurnOffControllerLED();
		if (!controller_) {
			controller_ = const_cast<LF::Hardware::HWController*>(controller);
			TurnOnControllerLEDAndTrack();
		}
		printf("\n");
		return LeapFrog::Brio::kEventStatusOKConsumed;
	} else if (msgIn.GetEventType() == LF::Hardware::kHWControllerModeChanged) {
		printf("\nController Mode Changed\n");
		//std::cout << "Controller Mode Changed\n";
		if (controller->GetCurrentMode() == LF::Hardware::kHWControllerWandMode) {
			TurnOffControllerLED();
			if (!controller_) {
				controller_ = const_cast<LF::Hardware::HWController*>(controller);
			}
			TurnOnControllerLEDAndTrack();
			printf("turning on LED\n\n");
		} else {
			TurnOffControllerLED();
			controller_ = NULL;
		}
		printf("\n");
		return LeapFrog::Brio::kEventStatusOKConsumed;
	} else if (msgIn.GetEventType() == LF::Hardware::kHWControllerButtonStateChanged) {
		LeapFrog::Brio::tButtonData2 button_data = controller->GetButtonData();
		static LeapFrog::Brio::U32 button_state = 0;
		static LeapFrog::Brio::U32 button_transition = 0;

		button_state = button_data.buttonState;
		button_transition = button_data.buttonTransition;

		printf("button_state, button_transition: %d, %d\n", (int)button_state, (int)button_transition);

		//if (button_transition == button_state) {
		if ((button_state & LeapFrog::Brio::kButtonA)) {
		  printf("A button pressed\n");
		  if (controller != controller_)
		    {
		      //turn off other controller
		      TurnOffControllerLED();
		      controller_ = NULL;
		    }
		  
		  //take control (on/off)
		  controller_ = const_cast<LF::Hardware::HWController*>(controller);
		  if (controller_ && controller_->GetLEDColor() == LF::Hardware::kHWControllerLEDOff) {
		    TurnOnControllerLEDAndTrack();
		  } else
		    {
		      TurnOffControllerLED();
		      controller_ = NULL;
		    }
		}
		else if ((button_state & LeapFrog::Brio::kButtonB)) {
		  printf("B button pressed\n");
		  if (controller == controller_) {
		    printf("\tchanging colors\n");
		    color_++;
		    if (color_ > 5)
		      color_ = 0;
		    
		    //only change colors if you are in control
		    //TurnOffControllerLED();
		    TurnOnControllerLED();
		  }
		}
		else if ((button_state & LeapFrog::Brio::kButtonMenu)) {
		  CAppManager::Instance()->PopApp();
		}
		//}
		return LeapFrog::Brio::kEventStatusOKConsumed;

	} else if (msgIn.GetEventType() == LF::Hardware::kHWControllerDisconnected) {

		printf( "a controller was disconnected\n");
		return LeapFrog::Brio::kEventStatusOKConsumed;
	}
	return LeapFrog::Brio::kEventStatusOK;
}

void 
WNDMainState::Enter(CStateData* userData) {
	RegisterEventListeners();
	InitSample();
}

void 
WNDMainState::Exit(void) {
	printf("Exitting\n");
	CleanDisplay();
	printf("Display Cleaned\n");
	UnRegisterEventListeners();
	printf("Listeners unregistered\n");
	TurnOffControllerLED();
	printf("LED off\n");
	visionMPI_.Stop();
	printf("vision stopped\n");
}

void WNDMainState::Suspend(void) {
	CleanDisplay();
	UnRegisterEventListeners();
	visionMPI_.Pause();
	TurnOffControllerLED();
}

void WNDMainState::Resume(void) {
	InitDisplay();
	RegisterEventListeners();
	visionMPI_.Resume();
	TurnOnControllerLED();
}

void WNDMainState::Update(CGameStateHandler* sh) {
	UpdateVisionAlgorithm();
	DoKeyEventLoop();
	DrawSample();
}

void
WNDMainState::UpdateVisionAlgorithm(void) {
	if (!visionMPI_.IsRunning()) {
		printf("starting vision\n\n\n\n");
		LeapFrog::Brio::tErrType error =  visionMPI_.Start(&videoDisplaySurface_,
				//NULL,
				false);
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
		// turn on controller LED
		if (!controller_) {
		  controller_ = controllerMPI_.GetControllerByID();
		}
		TurnOnControllerLEDAndTrack();
	} //else
	//visionMPI_.Update();
}

int WNDMainState::DoKeyEventLoop(void) {
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
WNDMainState::InitDisplay(void) {
}

void 
WNDMainState::CleanDisplay(void) {
	displayManager_.DestroyHandle(displayHandle_, true);
}

void
WNDMainState::TurnOffControllerLED(void) {
	if (controller_ && controller_->IsConnected()) {
		controller_->SetLEDColor(LF::Hardware::kHWControllerLEDOff);
	}
}

void
WNDMainState::TurnOnControllerLEDAndTrack(void) {
	if (controller_ && controller_->IsConnected()) {
		LF::Hardware::HWControllerLEDColorMask colors = controller_->GetAvailableLEDColors();
		LF::Hardware::HWControllerRGBLEDColor color = U8(1<<color_);
		if (colors & color) {
			if (controller_->StartTracking(color) != kNoErr) {
				controller_ = NULL;
				std::cout << "Failed to start tracking color" << std::endl;
			}
		}
	}
}

void
WNDMainState::TurnOnControllerLED(void) {
	if (controller_ && controller_->IsConnected()) {
		printf("Changing color for controller: %d\n", controller_->GetID());
		LF::Hardware::HWControllerLEDColorMask colors = controller_->GetAvailableLEDColors();
		LF::Hardware::HWControllerRGBLEDColor color = U8(1<<color_);
		if (colors & color) {
			controller_->SetLEDColor(color);
		}
	}
}

const tEventType 
gHotSpotEventKey[] = {LF::Vision::kVNHotSpotTriggeredEvent};

WNDMainState::HotSpotListener::HotSpotListener() : 
		  IEventListener(gHotSpotEventKey, 5) {
}

LeapFrog::Brio::tEventStatus
WNDMainState::HotSpotListener::Notify(const LeapFrog::Brio::IEventMessage& msg) {
	const LF::Vision::VNHotSpotEventMessage& hsMessage = dynamic_cast<const LF::Vision::VNHotSpotEventMessage&>(msg);
	const LF::Vision::VNHotSpot* hs = hsMessage.GetHotSpot();
	//std::cout << "HotSpot:" << hs->GetTag();
	if (hs->IsTriggered()) {
		//std::cout << " is triggered";
	} else {
		//std::cout << " is NOT triggered";
	}
	//std::cout << std::endl;

	return kEventStatusOK;
}

void 
WNDMainState::InitSample(void) {

	SetupDisplaySurface();
	SetupOGL();
	SetupVisionAlgorithm();
}

void
WNDMainState::SetupDisplaySurface(void) {  
	//get device stats
	const LeapFrog::Brio::tDisplayScreenStats* myStats;
	myStats = displayManager_.GetScreenStats(0);
	// FIXME: for now we need to set the resolution to match the vision resolution
	// this will change soon
	screenWidth_ = myStats->width;
	screenHeight_ = myStats->height;
	displayHandle_ = displayManager_.CreateHandle(kCaptureHeight,
			kCaptureWidth,
			kPixelFormatYUYV422,
			//kPixelFormatYUV420,
			//kPixelFormatRGB888,
			NULL);

	videoDisplaySurface_.width = displayManager_.GetWidth(displayHandle_);
	videoDisplaySurface_.pitch = displayManager_.GetPitch(displayHandle_);
	videoDisplaySurface_.height = displayManager_.GetHeight(displayHandle_);
	videoDisplaySurface_.buffer = displayManager_.GetBuffer(displayHandle_);
	videoDisplaySurface_.format = displayManager_.GetPixelFormat(displayHandle_);
}

void
WNDMainState::SetupOGL(void) {  
	CGraphics2D::Instance()->init(tVect2(screenWidth_,
			screenHeight_),
			tColor(0, 0, 0, 0));
	BrioOpenGLConfig* glconfig = CGraphics2D::Instance()->config;
	LeapFrog::Brio::tDisplayHandle
	oglDisplay = CGraphics2D::Instance()->config->hndlDisplay;
	displayManager_.SetAlpha(oglDisplay,70,true);

	// setup ogl camera display texture
	glEnable( GL_TEXTURE_2D );
	glGenTextures(1, &oglCameraTexture_);
	glBindTexture(GL_TEXTURE_2D, oglCameraTexture_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	int size = videoDisplaySurface_.width*videoDisplaySurface_.height;
	rgbBuffer_ = (unsigned char*)malloc(size * 3 );
	for (int i = 0; i < size; ++i) {
		rgbBuffer_[3*i  ] = static_cast<unsigned char>(23);
		rgbBuffer_[3*i+1] = static_cast<unsigned char>(217);
		rgbBuffer_[3*i+2] = static_cast<unsigned char>(255);
	}

#if defined(EMULATION)
	int textureWidth = 2;
	while (textureWidth < kCaptureWidth) {
		textureWidth *= 2;
	}
	int textureHeight = 2;
	while (textureHeight < kCaptureHeight) {
		textureHeight *= 2;
	}
#else
	int textureWidth = kCaptureWidth;
	int textureHeight = kCaptureHeight;
#endif

	glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGB,
			textureWidth,
			textureHeight,
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			NULL);

	float glw = (float)kCaptureWidth/(float)textureWidth;
	float glh = (float)kCaptureHeight/(float)textureHeight;

	texcoord_[0] = glw; texcoord_[1] = glh;
	texcoord_[2] = 0; texcoord_[3] = glh;
	texcoord_[4] = glw; texcoord_[5] = 0;
	texcoord_[6] = 0; texcoord_[7] = 0;

	// updates the on screen vertices for rendering
	vertices_[0] = 0;
	vertices_[1] = 0;
	vertices_[2] = screenWidth_;
	vertices_[3] = 0;
	vertices_[4] = 0;
	vertices_[5] = screenHeight_;
	vertices_[6] = screenWidth_;
	vertices_[7] = screenHeight_;
}

void
WNDMainState::SetupVisionAlgorithm(void) {
	// setup vision MPI
	LF::Vision::VNInputParameters params;

	// for more information regarding wand scaling please refer to VNWandTracker.h.

	// The VNWTAreaToStartScaling is the area, in number of pixels, at which point the wand
	// tracking algorithm will start to scale down the wand input domain (refer to VNWandTracker.h
	// for a diagram depicting this behavior)
	params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTAreaToStartScaling", 1100.f));

	// The VNWTMinPercentToScale is used to compute the smallest wand pointer input domain.  For
	// example, if the vision domain is 640x480 and the VNWTMinPercentToScale value is 0.25, the
	// smallest wand input domain will be 160x120.  This smallest input domain size will occur
	// when the wand area reaches the VNWTMinWandArea
	params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTMinPercentToScale", 0.25f));

	// The VNWTMinWandArea represents the wand area, in pixels, at which the input domain is
	// smallest.
	params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTMinWandArea", 30.f));

	// Turn on wand smoothing
	// See Include/Vision/VNWandTracker.h for more information
	params.push_back(LF::Vision::VNInputParameter("VNWTUseWandScaling", 1.0f));
	params.push_back(LF::Vision::VNInputParameter("VNWTNumFramesCacheLoc", 10));
	params.push_back(LF::Vision::VNInputParameter("VNWTSmoothingAlpha", 6));

	LF::Vision::VNWandTracker *alg = new LF::Vision::VNWandTracker(&params);
	alg->SetAutomaticWandScaling(true);
	visionAlgorithm_ = alg;
	visionMPI_.SetAlgorithm(visionAlgorithm_);
	controller_ =  controllerMPI_.GetControllerByID();

	LF::Vision::VNPointTrigger* trigger1 = new LF::Vision::VNPointTrigger();
	LeapFrog::Brio::tRect hotSpotRect1;
	hotSpotRect1.left = -10;
	hotSpotRect1.right = 60;
	hotSpotRect1.top = -10;
	hotSpotRect1.bottom = 40;
	LF::Vision::VNHotSpot* hotSpot1 = new LF::Vision::VNRectHotSpot(hotSpotRect1);
	hotSpot1->SetTag(1);
	hotSpot1->SetTrigger(trigger1);
	hotSpots_.push_back(hotSpot1);
	visionMPI_.AddHotSpot(hotSpot1);

	LeapFrog::Brio::tRect hotSpotRect2;
	hotSpotRect2.left = 200;
	hotSpotRect2.right = 300;
	hotSpotRect2.top = 20;
	hotSpotRect2.bottom = 100;
	LF::Vision::VNHotSpot* hotSpot2 = new LF::Vision::VNRectHotSpot(hotSpotRect2);
	hotSpot2->SetTag(2);
	hotSpot2->SetTrigger(trigger1);
	hotSpots_.push_back(hotSpot2);
	visionMPI_.AddHotSpot(hotSpot2);

	// open the filter image file in to a CBlitBuffer object
	CBlitBuffer filterImage;
	filterImage.createFromPng(CSystemData::Instance()->GetCurrentTitlePath()+"plus.png");

	// insure that the rect size here is the same as the image size you
	// just opneed above
	LeapFrog::Brio::tRect hotSpotRect3;
	int halfWidth = 24;
	int width = 2*halfWidth;
	hotSpotRect3.left = kCaptureWidth-halfWidth;
	hotSpotRect3.right = kCaptureWidth+halfWidth;
	hotSpotRect3.top = 100-halfWidth;
	hotSpotRect3.bottom = 100+halfWidth;

	LF::Vision::VNHotSpot*
	hotSpot3 = new LF::Vision::VNArbitraryShapeHotSpot(hotSpotRect3,
			filterImage.surface);
	hotSpot3->SetTag(3);
	hotSpot3->SetTrigger(trigger1);
	hotSpots_.push_back(hotSpot3);
	visionMPI_.AddHotSpot(hotSpot3);

	// wrong type of hotspot - added to vision mpi but not to state so it does not get rendered
	LeapFrog::Brio::tRect hotSpotRect4;
	hotSpotRect4.left = 50;
	hotSpotRect4.right = 50+width;
	hotSpotRect4.top = 200;
	hotSpotRect4.bottom = 200+width;

	LF::Vision::VNHotSpot*
	  hotSpot4 = new LF::Vision::VNRectHotSpot(hotSpotRect4);
	hotSpot4->SetTag(4);
	LF::Vision::VNOcclusionTrigger *trigger4 = new LF::Vision::VNOcclusionTrigger();
	hotSpot4->SetTrigger(trigger4);
	visionMPI_.AddHotSpot(hotSpot4);
}

void
WNDMainState::SetupOGLViewForDrawing(void) {
	glViewport(0, 0, screenWidth_, screenHeight_);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof(0, screenWidth_, 0, screenHeight_, 1.0f, 0.0f);
	glMatrixMode(GL_MODELVIEW);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void
WNDMainState::DrawCameraBuffer(void) {
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	float vW = videoDisplaySurface_.width;
	float vH = videoDisplaySurface_.height;
	glBindTexture(GL_TEXTURE_2D, oglCameraTexture_);

	//NOTE: For LF3000, do not use glTexSubImage2D
/*	glTexSubImage2D(GL_TEXTURE_2D,
			0,
			0,
			0,
			vW,
			vH,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			rgbBuffer_);*/
	glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGB,
			vW,
			vH,
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			rgbBuffer_);
	glVertexPointer(2, GL_FLOAT, 0, vertices_);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoord_);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);
}

void
WNDMainState::DrawHotSpots(void) {
	glLineWidth(2.0);
	float xscale = float(screenWidth_)/float(kCaptureWidth);
	float yscale = float(screenHeight_)/float(kCaptureHeight);

	// draw hotspots
	std::vector<const LF::Vision::VNHotSpot*>::iterator hotspot_iter;
	for( hotspot_iter = hotSpots_.begin(); hotspot_iter != hotSpots_.end(); hotspot_iter++ ) {
		const LF::Vision::VNRectHotSpot* rect_hotspot = static_cast<const LF::Vision::VNRectHotSpot*>( *hotspot_iter );
		const LeapFrog::Brio::tRect&  rect = rect_hotspot->GetRect();

		GLfloat verts[16];
		verts[0] = screenWidth_ - xscale*rect.left;  verts[1] = screenHeight_ - yscale*rect.top;
		verts[2] = screenWidth_ - xscale*rect.right; verts[3] = screenHeight_ - yscale*rect.top;

		verts[4] = screenWidth_ - xscale*rect.right; verts[5] = screenHeight_ - yscale*rect.top;
		verts[6] = screenWidth_ - xscale*rect.right; verts[7] = screenHeight_ - yscale*rect.bottom;

		verts[8] = screenWidth_ - xscale*rect.right; verts[9] = screenHeight_ - yscale*rect.bottom;
		verts[10] = screenWidth_ - xscale*rect.left; verts[11] = screenHeight_ - yscale*rect.bottom;

		verts[12] = screenWidth_ - xscale*rect.left; verts[13] = screenHeight_ - yscale*rect.bottom;
		verts[14] = screenWidth_ - xscale*rect.left; verts[15] = screenHeight_ - yscale*rect.top;

		if( rect_hotspot->IsTriggered() ) {
			glColor4f( 1, 0, 0, 1 );
		} else {
			glColor4f( 0, 1, 0, 1 );
		}

		glPushMatrix();
		glLoadIdentity();

		glTranslatef( screenWidth_, 0, 0 );
		glScalef(-1, 1, 1);
		
		glVertexPointer( 2, GL_FLOAT, 0, verts );
		glDrawArrays( GL_LINES, 0, 8 );
		
		glPopMatrix();
	}
}

void
WNDMainState::DrawPointer(void) {
	//printf("controller_: %d\n", (int)controller_->GetID());
	if (controller_) {
		LF::Vision::VNPoint p = controller_->GetLocation();
		float xscale = float(screenWidth_)/float(kCaptureWidth);
		float yscale = float(screenHeight_)/float(kCaptureHeight);

		p.x = screenWidth_ - xscale*p.x;
		p.y = screenHeight_ - yscale*p.y;

		float rad = 20.0f;

		GLfloat verts[] = {
				p.x - rad, p.y, p.x + rad, p.y,

				p.x, p.y - rad,
				p.x, p.y + rad
		};

		glVertexPointer(2, GL_FLOAT, 0, verts);
		glColor4f(0, 0, 0, 1);
		glLineWidth(5.0);
		glPushMatrix();
		glLoadIdentity();

		glTranslatef( screenWidth_, 0, 0 );
		glScalef(-1, 1, 1);

		glDrawArrays(GL_LINES, 0, 4);
		glPopMatrix();
	}
}

void 
WNDMainState::DrawSample(void) {
	CGraphics2D::Instance()->clearBuffer();

	SetupOGLViewForDrawing();
	DrawCameraBuffer();
	DrawHotSpots();
	DrawPointer();

	glColor4f( 1, 1, 1, 1 );
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_BLEND);

	CGraphics2D::Instance()->swapBuffer();
}

void
WNDMainState::RegisterEventListeners(void) {
	//register keys
	LeapFrog::Brio::CButtonMPI keyEventMPI;
	keyEventMPI.RegisterEventListener(&buttonEventQueue_);

	// register for hot spot events
	LeapFrog::Brio::CEventMPI eventMPI;
	eventMPI.RegisterEventListener(&hotspotListener_);

	// register for controller events
	controllerMPI_.RegisterEventListener(this);
}

void
WNDMainState::UnRegisterEventListeners(void) {
	//clean keys
	LeapFrog::Brio::CButtonMPI keyEventMPI;
	keyEventMPI.UnregisterEventListener(&buttonEventQueue_);

	// clean hot spot listener
	LeapFrog::Brio::CEventMPI eventMPI;
	eventMPI.UnregisterEventListener(&hotspotListener_);

	// clean controller listener
	controllerMPI_.UnregisterEventListener(this);
}
