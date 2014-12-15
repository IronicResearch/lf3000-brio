//Emerald Sample Project
#ifndef KHRONOS
#include <GLES/egl.h>
#endif

#include "CALBMainState.h"
#include "WandCalibrationApp.h"

#include <AppManager.h>
#include <CGraphics2D.h>
#include <Vision/VNWand.h>
#include <Hardware/HWControllerEventMessage.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <boost/bind.hpp>


using namespace LeapFrog::Brio;

inline bool IS_QVGA()
{
	struct stat s;
	return (0 == stat("/flags/qvga_vision_mode", &s));
}

static const int kWandColorVerifyTimeoutInSeconds = 30;

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

CALBMainState::CALBMainState(void) :
		  LeapFrog::Brio::IEventListener(kWNDListenerTypes, ArrayCount(kWNDListenerTypes)),
		  controller_(NULL),
		  color_(0) {
}

CALBMainState::~CALBMainState(void) {
	printf("CALBMainState destructor\n");
	if (visionAlgorithm_) {
		delete visionAlgorithm_;
	}
}

LeapFrog::Brio::tEventStatus
CALBMainState::Notify( const LeapFrog::Brio::IEventMessage &msgIn ) {
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

		if ((button_state & LeapFrog::Brio::kButtonA)) {
		  printf("A button pressed\n");
		}
		else if ((button_state & LeapFrog::Brio::kButtonB)) {

		  displayCameraBuffer_ = !displayCameraBuffer_;
		  printf("B button pressed\n");
		}
		else if ((button_state & LeapFrog::Brio::kButtonMenu)) {
		  CAppManager::Instance()->PopApp();
		}
		return LeapFrog::Brio::kEventStatusOKConsumed;

	} else if (msgIn.GetEventType() == LF::Hardware::kHWControllerDisconnected) {

		printf( "a controller was disconnected\n");
		return LeapFrog::Brio::kEventStatusOKConsumed;
	}
	return LeapFrog::Brio::kEventStatusOK;
}

void
CALBMainState::Enter(CStateData* userData) {
    displayCameraBuffer_ = false;

    RegisterEventListeners();
    InitSample();

    // init dialogs
    const int dialogWidth = 1024;
    const int dialogHeight = 128;
    animateDialog_ = false;
    currentDialog_ = CALBMainState::kWaitingController;
    // updates the on screen vertices for rendering
    dialogVertices_[0] = 0; 			dialogVertices_[1] = 0;
    dialogVertices_[2] = dialogWidth; 	dialogVertices_[3] = 0;
    dialogVertices_[4] = 0; 			dialogVertices_[5] = dialogHeight;
    dialogVertices_[6] = dialogWidth; 	dialogVertices_[7] = dialogHeight;


    dialogCoords_[0] = 0; dialogCoords_[1] = 1;
    dialogCoords_[2] = 1; dialogCoords_[3] = 1;
    dialogCoords_[4] = 0; dialogCoords_[5] = 0;
    dialogCoords_[6] = 1; dialogCoords_[7] = 0;

    dialogs_[CALBMainState::kMoveWandToGreenSquare] = new CTexturePNG( "/LF/Bulk/ProgramFiles/CALB/dialog-move-wand-to-green-square.png");
    dialogs_[CALBMainState::kVerifyWandCalibration] = new CTexturePNG( "/LF/Bulk/ProgramFiles/CALB/dialog-verify-wand-calibration.png");
    dialogs_[CALBMainState::kCalibrating] = new CTexturePNG( "/LF/Bulk/ProgramFiles/CALB/dialog-calibrating.png");
    dialogs_[CALBMainState::kTouchOtherHotspot] = new CTexturePNG( "/LF/Bulk/ProgramFiles/CALB/dialog-touch-other-hotspot.png");
    dialogs_[CALBMainState::kCalibrationComplete] = new CTexturePNG( "/LF/Bulk/ProgramFiles/CALB/dialog-calibration-complete.png");
    dialogs_[CALBMainState::kWaitingController] = new CTexturePNG( "/LF/Bulk/ProgramFiles/CALB/dialog-waiting-controller.png");
}

void
CALBMainState::Exit(void) {
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

void CALBMainState::Suspend(void) {
	CleanDisplay();
	UnRegisterEventListeners();
	visionMPI_.Pause();
	TurnOffControllerLED();
}

void CALBMainState::Resume(void) {
	InitDisplay();
	RegisterEventListeners();
	visionMPI_.Resume();
	TurnOnControllerLED();
}

void CALBMainState::Update(CGameStateHandler* sh) {
	UpdateVisionAlgorithm();
	DoKeyEventLoop();
	DrawSample();

	if( visionAlgorithm_->GetState() == WandCalibrationAlgorithm::kWandVerifyColor
		|| visionAlgorithm_->GetState() == WandCalibrationAlgorithm::kWandVerifyHotspotTriggered
		|| visionAlgorithm_->GetState() == WandCalibrationAlgorithm::kWaitForWandTrigger ) {
		time_t currentTime;
		time(&currentTime);
		double seconds = difftime( currentTime, calibrationVerificationStartTime_);
		//printf("s: %f", seconds);
		if( seconds > kWandColorVerifyTimeoutInSeconds ) {
			printf("\a\a\a Timeout..\n\n");
			// timed out goto next color
			int newColor = visionAlgorithm_->GetColor() + 1;
			if( newColor < 6 ) {
				color_ = newColor;
				TurnOnControllerLED();
				time( &calibrationVerificationStartTime_ );
				visionAlgorithm_->GotoState( WandCalibrationAlgorithm::kWaitForWandTrigger, (WandCalibrationAlgorithm::CalibrationColorState)newColor );
			} else { // no more colors to calibrate
				TurnOffControllerLED();
				visionAlgorithm_->GotoState( WandCalibrationAlgorithm::kCalibrationComplete, (WandCalibrationAlgorithm::CalibrationColorState)newColor );
				currentDialog_ = kCalibrationComplete;
			}
		}
	}
}

void
CALBMainState::UpdateVisionAlgorithm(void) {

	// wait for controller to connect
	if (!controller_) {
		controller_ = controllerMPI_.GetControllerByID();
		if( controller_ ) {
			visionAlgorithm_->SetController( controller_ );
		}
	}

	if (!visionMPI_.IsRunning() && controller_ ) {
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


		TurnOnControllerLEDAndTrack();

		currentDialog_ = CALBMainState::kMoveWandToGreenSquare;

	} else if( !controller_ ) {
		currentDialog_ = CALBMainState::kWaitingController;
	}
	//visionMPI_.Update();
}

int CALBMainState::DoKeyEventLoop(void) {
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
CALBMainState::InitDisplay(void) {
}

void
CALBMainState::CleanDisplay(void) {
	displayManager_.DestroyHandle(displayHandle_, true);
}

void
CALBMainState::TurnOffControllerLED(void) {
	if (controller_ && controller_->IsConnected()) {
		controller_->SetLEDColor(LF::Hardware::kHWControllerLEDOff);
	}
}

void
CALBMainState::TurnOnControllerLEDAndTrack(void) {
	printf("CALBMainState::TurnOnControllerLEDAndTrack");
	if (controller_ && controller_->IsConnected()) {
		LF::Hardware::HWControllerLEDColorMask colors = controller_->GetAvailableLEDColors();
		LF::Hardware::HWControllerRGBLEDColor color = U8(1<<color_);
		if (colors & color) {
			if (controller_->StartTracking(color) != kNoErr) {
				controller_ = NULL;
				std::cout << "\a\a\aFailed to start tracking color" << std::endl;
			}
		}
	}
}

void
CALBMainState::TurnOnControllerLED(void) {
	printf("CALBMainState::TurnOnControllerLED");
	if (controller_ && controller_->IsConnected()) {
		printf("Changing color for controller: %d\n", controller_->GetID());
		LF::Hardware::HWControllerLEDColorMask colors = controller_->GetAvailableLEDColors();
		LF::Hardware::HWControllerRGBLEDColor color = U8(1<<color_);
		if (colors & color) {
			controller_->SetLEDColor(color);
		}
	}
}


void
CALBMainState::InitSample(void) {

	SetupDisplaySurface();
	SetupOGL();
	SetupVisionAlgorithm();
}

void
CALBMainState::SetupDisplaySurface(void) {
  //get device stats
  const LeapFrog::Brio::tDisplayScreenStats* myStats;
  myStats = displayManager_.GetScreenStats(0);
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
						//kPixelFormatYUV420,
						kPixelFormatRGB888,
						NULL);

  videoDisplaySurface_.width = kCaptureWidth;
  videoDisplaySurface_.height = kCaptureHeight;
  videoDisplaySurface_.pitch = displayManager_.GetPitch(displayHandle_);
  videoDisplaySurface_.buffer = displayManager_.GetBuffer(displayHandle_);
  videoDisplaySurface_.format = displayManager_.GetPixelFormat(displayHandle_);
}

void
CALBMainState::SetupOGL(void) {
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

	// generate mask texture
	glGenTextures(1, &oglAlgorithMaskTexture_);
	glBindTexture(GL_TEXTURE_2D, oglAlgorithMaskTexture_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_LUMINANCE,
			textureWidth,
			textureHeight,
			0,
			GL_LUMINANCE,
			GL_UNSIGNED_BYTE,
			NULL);

	algorithmMask_ = cv::Mat(cv::Size(textureWidth,textureHeight), CV_8U, cv::Scalar(0));
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

	SetupOGLViewForDrawing();
}

void
CALBMainState::SetupVisionAlgorithm(void) {

	// setup vision MPI
	WandCalibrationAlgorithm* alg = new WandCalibrationAlgorithm;
	//alg->SetAutomaticWandScaling(true);
	visionAlgorithm_ = alg;
	visionMPI_.SetAlgorithm((LF::Vision::VNAlgorithm*)visionAlgorithm_);
	controller_ =  controllerMPI_.GetControllerByID();
	visionAlgorithm_->SetController(controller_);

	// setup initial touch hotspot
	std::vector<LeapFrog::Brio::tRect> hotSpots;
	LeapFrog::Brio::tRect hotSpotRect;

	hotSpotRect.left = 200;
	hotSpotRect.right = hotSpotRect.left + 50;
	hotSpotRect.top = (480/2) - 25;
	hotSpotRect.bottom = hotSpotRect.top + 50;
	hotSpots.push_back( hotSpotRect );

	hotSpotRect.left = 20;
	hotSpotRect.right = hotSpotRect.left + 50;
	hotSpotRect.top = (480/2) - 25;
	hotSpotRect.bottom = hotSpotRect.top + 50;
	hotSpots.push_back( hotSpotRect );

	hotSpotRect.left = 400;
	hotSpotRect.right = hotSpotRect.left + 50;
	hotSpotRect.top = (480/2) - 25;
	hotSpotRect.bottom = hotSpotRect.top + 50;
	hotSpots.push_back( hotSpotRect );

	alg->SetHotSpotRects( hotSpots );

	color_ = 0;
	time( &calibrationVerificationStartTime_ );
	visionAlgorithm_->GotoState( WandCalibrationAlgorithm::kWaitForWandTrigger, WandCalibrationAlgorithm::kGreen );


	// setup various notification callbacks
	visionAlgorithm_->SetPostExecuteCallback(&CALBMainState::AlgorithmPostExecuteCallback, (void*)this );
	visionAlgorithm_->SetStateChangedCallback(&CALBMainState::AlgorithmStateChangedCallback, (void*)this );
}


void
CALBMainState::SetupOGLViewForDrawing(void) {

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
CALBMainState::DrawCameraBuffer(void) {
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	if (videoDisplaySurface_.format == kPixelFormatYUYV422) {
		// cv::Mat tmpYUV(cv::Size(videoDisplaySurface_.width,
		// videoDisplaySurface_.height),
		// CV_8UC2,
		// videoDisplaySurface_.buffer,
		// //			tmp_buffer,
		// //cameraBuffer_,
		// videoDisplaySurface_.pitch);
		static cv::Mat tmpRGB(cv::Size(videoDisplaySurface_.width,
		videoDisplaySurface_.height),
		CV_8UC3,
		rgbBuffer_);
		//#ifdef EMULATION
		cv::cvtColor(frame_, tmpRGB, CV_YUV2BGR_YUYV);
	} else if (videoDisplaySurface_.format == kPixelFormatRGB888) {
		unsigned char *data = videoDisplaySurface_.buffer;
		for (int i = 0; i < videoDisplaySurface_.height; ++i) {
			int index = i*videoDisplaySurface_.width*3;
			for (int j = 0; j < videoDisplaySurface_.width; ++j) {
				rgbBuffer_[index+3*j  ] = data[3*j+2];
				rgbBuffer_[index+3*j+1] = data[3*j+1];
				rgbBuffer_[index+3*j+2] = data[3*j  ];
			}
			data += videoDisplaySurface_.pitch;
		}
	}
	glBindTexture(GL_TEXTURE_2D, oglCameraTexture_);

	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGB,
		videoDisplaySurface_.width,
		videoDisplaySurface_.height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		rgbBuffer_);

	// glTexSubImage2D(GL_TEXTURE_2D,
	// 	0,
	// 	0,
	// 	0,
	// 	videoDisplaySurface_.width,
	// 	videoDisplaySurface_.height,
	// 	GL_RGB,
	// 	GL_UNSIGNED_BYTE,
	// 	rgbBuffer_);

	glVertexPointer(2, GL_FLOAT, 0, vertices_);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoord_);
	glPushMatrix();
	glLoadIdentity();

	glTranslatef( screenWidth_, 0, 0 );
	glScalef(-1, 1, 1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glPopMatrix();

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_2D);
}

void
CALBMainState::RenderRectHotSpot(const LF::Vision::VNRectHotSpot *rectHS) {
	glEnableClientState(GL_VERTEX_ARRAY);

	float xscale = float(screenWidth_)/float(kCaptureWidth);
	float yscale = float(screenHeight_)/float(kCaptureHeight);

	LeapFrog::Brio::tRect  rect = rectHS->GetRect();
	rect.left = rect.left * xscale;
	rect.right = rect.right * xscale;
	rect.top = rect.top * yscale;
	rect.bottom = rect.bottom * yscale;
	// screen is mirrored, must mirror points here too
	int numVertPairs = 8;
	GLfloat verts[2*numVertPairs];
	verts[0] = screenWidth_ - rect.left;  verts[1] = screenHeight_ - rect.top;
	verts[2] = screenWidth_ - rect.right; verts[3] = screenHeight_ - rect.top;
	verts[4] = screenWidth_ - rect.right; verts[5] = screenHeight_ - rect.top;
	verts[6] = screenWidth_ - rect.right; verts[7] = screenHeight_ - rect.bottom;

	verts[8] = screenWidth_ - rect.right; verts[9] = screenHeight_ - rect.bottom;
	verts[10] = screenWidth_ - rect.left; verts[11] = screenHeight_ - rect.bottom;
	verts[12] = screenWidth_ - rect.left; verts[13] = screenHeight_ - rect.bottom;
	verts[14] = screenWidth_ - rect.left; verts[15] = screenHeight_ - rect.top;


	if (rectHS->IsTriggered()) {
		glColor4f(1, 0, 0, 1);
	} else {
		glColor4f(0, 1, 0, 1);
	}

	glVertexPointer(2, GL_FLOAT, 0, verts);
	glDrawArrays(GL_LINES, 0, numVertPairs);
	glDisableClientState(GL_VERTEX_ARRAY);
	glColor4f(1,1,1,1);
}


void
CALBMainState::DrawHotSpots(void) {

	glPushMatrix();
	glLoadIdentity();

	glTranslatef( screenWidth_, 0, 0 );
	glScalef(-1, 1, 1);

	std::vector<LF::Vision::VNRectHotSpot*> hotSpots = visionAlgorithm_->GetHotSpots();


	if( visionAlgorithm_->GetState() == WandCalibrationAlgorithm::kWaitForWandTrigger ) {
		RenderRectHotSpot( hotSpots[0] );	// only draw the first hotspot
	} else if ( visionAlgorithm_->GetState() == WandCalibrationAlgorithm::kWandVerifyColor ) {
		std::vector<LF::Vision::VNRectHotSpot*>::iterator it = hotSpots.begin();
		it++; // skip first hot spot
		for( ;it != hotSpots.end(); it++ ) {
			RenderRectHotSpot( *it );
		}

	}


	glPopMatrix();

}

void
CALBMainState::DrawPointer(void) {
	if (controller_) {

		glEnableClientState(GL_VERTEX_ARRAY);

		LF::Vision::VNPoint p = controller_->GetLocation();
		float xscale = float(screenWidth_)/float(kCaptureWidth);
		float yscale = float(screenHeight_)/float(kCaptureHeight);

		p.x *= xscale;
		p.y = screenHeight_ - (p.y * yscale);

		float rad = 20.0f;

		GLfloat verts[] = {
				p.x - rad, p.y, p.x + rad, p.y,
				p.x, p.y - rad, p.x, p.y + rad
		};

		glVertexPointer(2, GL_FLOAT, 0, verts);
		glColor4f(0, 0, 1, 1);
		glLineWidth(5.0);
		glPushMatrix();
		glLoadIdentity();


		glDrawArrays(GL_LINES, 0, 4);
		glPopMatrix();

		glDisableClientState(GL_VERTEX_ARRAY);
	}
}

void
CALBMainState::DrawSample(void) {
	CGraphics2D::Instance()->clearBuffer();

	// SetupOGLViewForDrawing();
	glClearColor(0.0, 0.0, 0.0, 0.0);
  	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  	glMatrixMode(GL_MODELVIEW);

	if( displayCameraBuffer_ ) {
		DrawCameraBuffer();
	} else {
		DrawAlgorithmMask();
	}


	DrawHotSpots();
	DrawPointer();
	DrawDialog( currentDialog_ );


	glColor4f( 1, 1, 1, 1 );
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_BLEND);

	CGraphics2D::Instance()->swapBuffer();
}

void
CALBMainState::DrawDialog( CALBMainState::Dialogs dialog ) {
		// setup the texture environment
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		glBindTexture( GL_TEXTURE_2D, dialogs_[dialog]->GetGLTextureID() );



		glVertexPointer(2, GL_FLOAT, 0, dialogVertices_);
		glTexCoordPointer(2, GL_FLOAT, 0, dialogCoords_);
		glPushMatrix();
		glLoadIdentity();

		if( dialog == CALBMainState::kCalibrating || CALBMainState::kWaitingController) {
			static float ang = 0;
			ang+=0.03f;
			float translate = fabsf(sin(ang) * 200);
			glTranslatef( 0, translate, 0 );
		}


		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glPopMatrix();

		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisable(GL_TEXTURE_2D);


}



void CALBMainState::DrawAlgorithmMask() {
	static int framecnt = 0;
	framecnt++;
	if( framecnt < 20 )
		return;
	if( algorithmMask_.total() < 640*480 )
		return;

	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindTexture(GL_TEXTURE_2D, oglAlgorithMaskTexture_);

	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_LUMINANCE,
		videoDisplaySurface_.width,
		videoDisplaySurface_.height,
		0,
		GL_LUMINANCE,
		GL_UNSIGNED_BYTE,
		algorithmMask_.data);

	// glTexSubImage2D(GL_TEXTURE_2D,
	// 	0,
	// 	0,
	// 	0,
	// 	videoDisplaySurface_.width,
	// 	videoDisplaySurface_.height,
	// 	GL_LUMINANCE,
	// 	GL_UNSIGNED_BYTE,
	// 	algorithmMask_.data);

	glVertexPointer(2, GL_FLOAT, 0, vertices_);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoord_);
	glPushMatrix();
	glLoadIdentity();

	glTranslatef( screenWidth_, 0, 0 );
	glScalef(-1, 1, 1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glPopMatrix();

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_2D);

}

void CALBMainState::AlgorithmPostExecuteCallback( cv::Mat& frame, cv::Mat& output, void* user ) {
	CALBMainState* me = (CALBMainState*)user;
	//printf("here\n");
	//me->DrawSample();
	me->algorithmMask_ = output;
	me->frame_ = frame;
}

void CALBMainState::AlgorithmStateChangedCallback( WandCalibrationAlgorithm::CalibrationState state, WandCalibrationAlgorithm::CalibrationColorState color, void* user ) {
	CALBMainState* me = (CALBMainState*)user;
	printf("Algorithm State Changed: %d %d\n", state, color);

	switch( state ) {
		case WandCalibrationAlgorithm::kWaitForWandTrigger:
		 	time( &me->calibrationVerificationStartTime_ );
			me->currentDialog_ = kMoveWandToGreenSquare;
			me->TurnOffControllerLED();
			me->TurnOnControllerLEDAndTrack();
		break;

		case WandCalibrationAlgorithm::kCalibratingColor:
			me->currentDialog_ = CALBMainState::kCalibrating;
		break;

		case WandCalibrationAlgorithm::kWandColorFinishedCalibration:
			me->currentDialog_ = kVerifyWandCalibration;
			me->TurnOffControllerLED();
			me->TurnOnControllerLEDAndTrack();
			me->verifyHotspotCount_ = 2;
			time( &me->calibrationVerificationStartTime_ );
			me->visionAlgorithm_->GotoState( WandCalibrationAlgorithm::kWandVerifyColor, color );
			me->visionAlgorithm_->SaveCalibration();

		break;

		case WandCalibrationAlgorithm::kWandVerifyHotspotTriggered:
			me->verifyHotspotCount_--;
			if( me->verifyHotspotCount_ > 0 ) {
				me->currentDialog_ = kTouchOtherHotspot;
				me->visionAlgorithm_->GotoState( WandCalibrationAlgorithm::kWandVerifyColor, color );
			} else {
				int newcolor = color + 1;
				if( newcolor < 6 ) {
					me->color_ = newcolor;
					me->TurnOnControllerLED();
					// TODO: check color overflow
					time( &me->calibrationVerificationStartTime_ );
					me->visionAlgorithm_->GotoState( WandCalibrationAlgorithm::kWaitForWandTrigger, (WandCalibrationAlgorithm::CalibrationColorState)newcolor );
				} else {	// no more colors to calibrate
					me->TurnOffControllerLED();
					me->visionAlgorithm_->GotoState( WandCalibrationAlgorithm::kCalibrationComplete, (WandCalibrationAlgorithm::CalibrationColorState)newcolor );
					me->currentDialog_ = kCalibrationComplete;
				}
			}
		break;
	}

}


void
CALBMainState::RegisterEventListeners(void) {
	//register keys
	LeapFrog::Brio::CButtonMPI keyEventMPI;
	keyEventMPI.RegisterEventListener(&buttonEventQueue_);


	// register for controller events
	controllerMPI_.RegisterEventListener(this);
}

void
CALBMainState::UnRegisterEventListeners(void) {
	//clean keys
	LeapFrog::Brio::CButtonMPI keyEventMPI;
	keyEventMPI.UnregisterEventListener(&buttonEventQueue_);


	// clean controller listener
	controllerMPI_.UnregisterEventListener(this);
}
