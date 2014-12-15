#define RENDER_WITH_OGL 1
#define RENDER_WITH_DISPLAY_MPI 0
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
#include <Hardware/HWControllerEventMessage.h>
#include <Hardware/HWController.h>
#include <USBDeviceMPI.h>
#include <ImageIO.h>
#include <KernelMPI.h>
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

extern bool isCameraConnectedAtLaunch;

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

const LeapFrog::Brio::tEventType
kListenerTypes[] = {LF::Hardware::kHWControllerButtonStateChanged, kAllUSBDeviceEvents};

VTCHMainState::VTCHMainState(void) : visionMPI_(NULL), IEventListener(kListenerTypes, ArrayCount(kListenerTypes))
{
	#if ALLOCATE_VN_VISION_MPI
		visionMPI_ = new LF::Vision::VNVisionMPI();
	#endif
}

VTCHMainState::~VTCHMainState(void)
{
	CleanUpHotSpots();
	if (visionMPI_)
	{
		visionMPI_->Stop();
		delete visionMPI_;
	}
	CleanUpAlgorithm();
}

void VTCHMainState::CleanUpHotSpots(void)
{
	std::vector<const LF::Vision::VNHotSpot*>::iterator it;
	for ( it = hotSpots_.begin(); it != hotSpots_.end(); ++it)
	{
		const LF::Vision::VNHotSpot *hs = *it;
		if (hs)
		{
			if (visionMPI_)
			{
				visionMPI_->RemoveHotSpot(hs);
			}
			delete hs;
			hs = NULL;
		}
	}
	hotSpots_.clear();
}

void VTCHMainState::CleanUpAlgorithm(void)
{
	if (visionAlgorithm_)
	{
		delete visionAlgorithm_;
	}
	visionAlgorithm_ = NULL;
}

void VTCHMainState::Enter(CStateData* userData)
{
	RegisterEventListeners();
	errorDisplayed = false;

	Init();

	GetErrorArt();

	if(isCameraConnectedAtLaunch)
	{
		isCameraConnected = true;

		for(int i=0; i<GRID_SIZE_X*GRID_SIZE_Y; ++i)
		{
			pointData[i].rad = 0;
			pointData[i].dx = 0;
			pointData[i].dy = 0;
		}
		drawState = kNorm;
		switchStateWait = 0;

		InitSample();
	}
	else
	{
		isCameraConnected = false;
		SetupError();
	}
}

void VTCHMainState::Exit(void)
{

	UnRegisterEventListeners();

	if(isCameraConnected)
	{
		CleanDisplay();
		CleanUpHotSpots();
		if (visionMPI_)
		{
			visionMPI_->Stop();
		}
		CleanUpAlgorithm();
	}

	if(errorDisplayed)
	{
		delete rgbBuffer_;
	}
}

void VTCHMainState::Suspend(void)
{
	CleanDisplay();
	UnRegisterEventListeners();
	if(isCameraConnected)
	{
		if (visionMPI_)
		{
			visionMPI_->Pause();
		}
	}
}

void VTCHMainState::Resume(void)
{
	InitDisplay();
	RegisterEventListeners();
}

void VTCHMainState::Update(CGameStateHandler* sh)
{
	#if CYCLE_START_STOP_IN_VISION
		static int count = 0;

		if (visionMPI_)
		{
			if (count > NUM_INITIAL_FRAMES_TO_PROCESS && count < 1000+NUM_START_STOP_CYCLES*FRAME_COUNT_PER_CYCLE)
			{
				if ((count % FRAME_COUNT_PER_CYCLE) == 0)
				{
					visionMPI_->Stop();
				}
			}
			count++;
		}
	#endif

	if(isCameraConnected && (isCameraConnectedAtLaunch || errorDisplayed))
	{
		if(UpdateVisionAlgorithm())
		{
			//check for buttons
			DrawSample();
		}
	}
	else if (!errorDisplayed && !isCameraConnectedAtLaunch)
	{
		Render();
		errorDisplayed = true;
	}
}

void VTCHMainState::SetWindowPosition(void)
{
	#if RENDER_WITH_DISPLAY_MPI
		displayManager_.SetWindowPosition(displayHandle_, displayFrame_.left,
						  displayFrame_.top,
						  displayFrame_.right-displayFrame_.left,
						  displayFrame_.bottom - displayFrame_.top,
						  true);
		#if RENDER_WITH_OGL
			displayManager_.SetAlpha(brioOGLConfig_->hndlDisplay, 100, true);
		#endif
	#endif
}

bool VTCHMainState::UpdateVisionAlgorithm(void)
{
	if (visionMPI_ && !visionMPI_->IsRunning())
	{
		// the true indicates I want to have the vision processing run
		// in a separate thread and not manually call Upate

		CKernelMPI kernel_;
		kernel_.TaskSleep(800);

		LeapFrog::Brio::tErrType error = visionMPI_->Start(&videoDisplaySurface_, false, &displayFrame_);

		if (error != kNoErr)
		{
			std::cout << "Error starting visionMPI: " << error << std::endl;
			if (error == LF::Vision::kVNVideoCaptureFailed)
				std::cout << "  Failed to start video capture\n\n\n\n\n\n";
			else if (error == LF::Vision::kVNCameraDoesNotSupportRequiredVisionFormat)
				std::cout << "  Camera does not support required vision format\n\n\n\n\n\n\n";
			else if (error == LF::Vision::kVNVideoSurfaceNotOfCorrectSizeForVisionCapture)
				std::cout << "  Display surface is not of the correct size for vision capture\n\n\n\n\n\n\n";
			return false;
		}

		SetWindowPosition();
		return true;
	}
}

LeapFrog::Brio::tEventStatus VTCHMainState::Notify( const LeapFrog::Brio::IEventMessage &msgIn )
{
	const CUSBDeviceMessage& msg = reinterpret_cast<const CUSBDeviceMessage&>(msgIn);
	tUSBDeviceData data = msg.GetUSBDeviceState();

//	printf("usb data state: %d\n",(int)data.USBDeviceState);

	if (msgIn.GetEventType() == LF::Hardware::kHWControllerButtonStateChanged)
	{
		CAppManager::Instance()->PopApp();

		return LeapFrog::Brio::kEventStatusOKConsumed;
	}
	else if (data.USBDeviceState & kUSBDeviceConnected)
	{
		isCameraConnected = true;
		InitSample();
	}
	else if (!(data.USBDeviceState & kUSBDeviceConnected) && (isCameraConnectedAtLaunch || isCameraConnected))
	{
		CAppManager::Instance()->PopApp();
	}

	return LeapFrog::Brio::kEventStatusOK;
}

void VTCHMainState::InitDisplay(void)
{

}

void VTCHMainState::CleanDisplay(void)
{
	displayManager_.DestroyHandle(displayHandle_, true);
}


const tEventType gHotSpotEventKey[] = {LF::Vision::kVNHotSpotGroupTriggeredEvent};

VTCHMainState::HotSpotListener::HotSpotListener() : IEventListener(gHotSpotEventKey, 5)
{

}

LeapFrog::Brio::tEventStatus VTCHMainState::HotSpotListener::Notify(const LeapFrog::Brio::IEventMessage& msg)
{
	const LF::Vision::VNHotSpotEventMessage& hsMessage = dynamic_cast<const LF::Vision::VNHotSpotEventMessage&>(msg);
	std::vector<const LF::Vision::VNHotSpot*> hs = hsMessage.GetHotSpots();

	//std::cout << hs.size() << ".";

	return kEventStatusOKConsumed;
}

void VTCHMainState::InitSample(void)
{
	SetupDisplaySurface();
	SetupOGL();
	SetupVisionAlgorithm();
	#if RENDER_WITH_OGL
		displayManager_.SetAlpha(brioOGLConfig_->hndlDisplay, 100, true);
	#endif
	count = -1;
}

void VTCHMainState::SetupDisplaySurface(void)
{
	float screenAspectRatio = (float)screenWidth_/(float)screenHeight_;
	float captureAspectRatio = (float)kCaptureWidth/(float)kCaptureHeight;

	if (captureAspectRatio > screenAspectRatio)
	{
		// fit hieght
		targetWidth_ = screenHeight_*captureAspectRatio;
		targetHeight_ = screenHeight_;
	}
	else
	{
		// fit width
		targetWidth_ = screenWidth_;
		targetHeight_ = screenWidth_/captureAspectRatio;
	}

	displayFrame_.left = (screenWidth_-targetWidth_)/2;
	displayFrame_.top = (screenHeight_-targetHeight_)/2;
	displayFrame_.right = displayFrame_.left + targetWidth_;
	displayFrame_.bottom = displayFrame_.top + targetHeight_;

	//printf("\nScreen dimensions (%d, %d)\n\n", screenWidth_, screenHeight_);
	//printf("\nTarget dimensions (%d, %d)\n\n", targetWidth_, targetHeight_);


	displayHandle_ = displayManager_.CreateHandle(targetHeight_, targetWidth_, kPixelFormatYUV420, NULL);

	videoDisplaySurface_.width = kCaptureWidth;
	videoDisplaySurface_.height = kCaptureHeight;
	videoDisplaySurface_.pitch = displayManager_.GetPitch(displayHandle_);
	videoDisplaySurface_.buffer = displayManager_.GetBuffer(displayHandle_);
	videoDisplaySurface_.format = displayManager_.GetPixelFormat(displayHandle_);

	displayManager_.Invalidate(0, NULL);
	#if RENDER_WITH_DISPLAY_MPI
		if (kNoErr != displayManager_.Register(displayHandle_, 0, 0, kDisplayOnBottom, 0))
		{
			DidjAssert(0, "Video Display manager registration failed.");
		}
	#endif
}

void VTCHMainState::SetupOGL(void)
{
	#if RENDER_WITH_OGL
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

void VTCHMainState::SetupVisionAlgorithm(void)
{
	if (visionMPI_)
	{
		#if ALLOCATE_VISION_ALGORITHM
			// setup vision MPI
			visionAlgorithm_ = new LF::Vision::VNVirtualTouch(0.15);
			visionMPI_->SetAlgorithm(visionAlgorithm_);
		#endif

		#if ALLOCATE_HOT_SPOTS
			// we will use a 20x20 grid of hot spots
			int gridSize_x = GRID_SIZE_X;
			int gridSize_y = GRID_SIZE_Y;
			int dx = screenWidth_/gridSize_x;
			int dy = screenHeight_/gridSize_y;
			int numHotSpots = 0;
			LF::Vision::VNOcclusionTrigger* trigger = new LF::Vision::VNOcclusionTrigger();

			int low = 0;
			int high = gridSize_x;

			#if HOT_SPOT_FLAG == LESS_THAN_FIVE_PERCENT_COVERAGE
				low = 9;
				high = 10;
			#elif HOT_SPOT_FLAG == FIFTY_PERCENT_COVERAGE
				low = 5;
				high = 15;
			#elif HOT_SPOT_FLAG == FULL_COVERAGE
				low = 0;
				high = gridSize_x;
			#endif

			for (int i = 0; i < gridSize_x; ++i)
			{
				int x = i*dx;
				if (i >= low && i < high)
				{
					for (int j = 0; j < gridSize_y; ++j)
					{
						if (j >= low && j < high)
						{
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

void VTCHMainState::RenderRectHotSpot(const LF::Vision::VNRectHotSpot *rectHS)
{
	#if RENDER_WITH_OGL
	GLfloat r;
		int tag = rectHS->GetTag();
		if (rectHS->IsTriggered()) {
			GLfloat maxRad = ((float)screenWidth_/(float)GRID_SIZE_X)/2.0;
			pointData[tag].rad += maxRad/FADE_STEPS;
			if(pointData[tag].rad >= maxRad)
			{
				pointData[tag].rad = maxRad;
				return;
			}
			r = pointData[tag].rad;

			pointData[tag].dy += 10*r/maxRad;
			GLfloat a = 1 - (r/maxRad);
			glColor4f( 1-r/maxRad, r/maxRad, r/maxRad,  a);
			//glColor4f( 0, 1-r/maxRad, 0,  a);
		}
		else
		{
			pointData[tag].dy = 0;
			pointData[tag].rad = screenWidth_/GRID_SIZE_X;
			return;
		}
		DrawRects(rectHS);
	#endif
}

void VTCHMainState::RenderCircleHotSpot(const LF::Vision::VNRectHotSpot *rectHS)
{
	#if RENDER_WITH_OGL
	GLfloat r;
		int tag = rectHS->GetTag();

		if (rectHS->IsTriggered()) {
			GLfloat maxRad = MAX_RAD;
			pointData[tag].rad += maxRad/FADE_STEPS;
			if(pointData[tag].rad >= maxRad)
			{
				pointData[tag].rad = maxRad;
				return;
			}
			r = pointData[tag].rad;

			pointData[tag].dy -= 10*r/maxRad;
			GLfloat a = 1 - (r/maxRad);
			glColor4f( 1-r/maxRad, r/maxRad, r/maxRad,  a);
		}
		else
		{
			pointData[tag].dy = 0;
			pointData[tag].rad = screenWidth_/GRID_SIZE_X;
			return;
		}

		glEnableClientState(GL_VERTEX_ARRAY);

		const LeapFrog::Brio::tRect&  rect = rectHS->GetRect();

		//circles
		LF::Vision::VNPoint c;
		c.x = rect.left + (rect.right-rect.left)/2;
		c.y = rect.top + (rect.bottom-rect.top)/2;
		// screen is mirrored, must mirror points here too
		c.x = fabs(screenWidth_ - c.x);
		c.y = fabs(screenHeight_ - c.y) + pointData[tag].dy;


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

		glVertexPointer(2, GL_FLOAT, 0, cVerts);
		glPushMatrix();
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4*numLines);
		glPopMatrix();

		glDisableClientState(GL_VERTEX_ARRAY);
	#endif
}

void VTCHMainState::RenderExpHotSpot(const LF::Vision::VNRectHotSpot *rectHS)
{
	#if RENDER_WITH_OGL
		GLfloat r;
		int tag = rectHS->GetTag();
		if (rectHS->IsTriggered())
		{
			GLfloat maxRad = ((float)screenWidth_/(float)GRID_SIZE_X)/2.0;
			pointData[tag].rad += maxRad/FADE_STEPS;
			if(pointData[tag].rad >= maxRad)
			{
				pointData[tag].rad = maxRad;
				return;
			}

			r = pointData[tag].rad;
			pointData[tag].dy -= 5*r/maxRad;
			GLfloat a = (r/maxRad);
			glColor4f( 0, r/maxRad, 1-r/maxRad,  a);
		}
		else
		{
			pointData[tag].dx = 0;
			pointData[tag].dy = 0;
			pointData[tag].rad = screenWidth_/GRID_SIZE_X;
			return;
		}

		DrawRects(rectHS);
	#endif
}

void VTCHMainState::RenderNormHotSpot(const LF::Vision::VNRectHotSpot *rectHS)
{
	#if RENDER_WITH_OGL
		GLfloat r;
		int tag = rectHS->GetTag();
		if (rectHS->IsTriggered())
		{
			glColor4f( 1, 1, 1, 1);
			const LeapFrog::Brio::tRect&  rect = rectHS->GetRect();
			pointData[tag].rad = rect.left/screenWidth_;
		}
		else
		{
			pointData[tag].dx = 0;
			pointData[tag].dy = 0;
			pointData[tag].rad = screenWidth_/GRID_SIZE_X;
			return;
		}

		DrawRects(rectHS);
	#endif
}

void VTCHMainState::DrawRects(const LF::Vision::VNRectHotSpot *rectHS)
{
	int tag = rectHS->GetTag();

	glEnableClientState(GL_VERTEX_ARRAY);

	const LeapFrog::Brio::tRect&  rect = rectHS->GetRect();

	// screen is mirrored, must mirror points here too
	int numVertPairs = 8;
	GLfloat verts[2*numVertPairs];
	verts[0] = screenWidth_ - (rect.left + pointData[tag].rad - pointData[tag].dx);  verts[1] = screenHeight_ - (rect.top + pointData[tag].rad + pointData[tag].dy);
	verts[2] = screenWidth_ - (rect.right - pointData[tag].rad + pointData[tag].dx); verts[3] = screenHeight_ - (rect.top + pointData[tag].rad + pointData[tag].dy);
	verts[4] = screenWidth_ - (rect.right - pointData[tag].rad + pointData[tag].dx); verts[5] = screenHeight_ - (rect.top + pointData[tag].rad + pointData[tag].dy);
	verts[6] = screenWidth_ - (rect.right - pointData[tag].rad + pointData[tag].dx); verts[7] = screenHeight_ - (rect.bottom - pointData[tag].rad + pointData[tag].dy);

	verts[8] = screenWidth_ - (rect.right - pointData[tag].rad + pointData[tag].dx); verts[9] = screenHeight_ - (rect.bottom - pointData[tag].rad + pointData[tag].dy);
	verts[10] = screenWidth_ - (rect.left + pointData[tag].rad - pointData[tag].dx); verts[11] = screenHeight_ - (rect.bottom - pointData[tag].rad + pointData[tag].dy);
	verts[12] = screenWidth_ - (rect.left + pointData[tag].rad - pointData[tag].dx); verts[13] = screenHeight_ - (rect.bottom - pointData[tag].rad + pointData[tag].dy);
	verts[14] = screenWidth_ - (rect.left + pointData[tag].rad - pointData[tag].dx); verts[15] = screenHeight_ - (rect.top + pointData[tag].rad + pointData[tag].dy);

	glVertexPointer(2, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLE_FAN, 0, numVertPairs);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void VTCHMainState::DrawHotSpots(void)
{
	#if RENDER_WITH_OGL
		glPushMatrix();
		glLoadIdentity();
		#if !defined(EMULATION)
			glTranslatef( screenWidth_, 0, 0 );
			glScalef(-1, 1, 1);
		#endif

		std::vector<const LF::Vision::VNHotSpot*>::iterator hotspot_iter;
			for (hotspot_iter = hotSpots_.begin(); hotspot_iter != hotSpots_.end(); hotspot_iter++ )
			{
				const LF::Vision::VNRectHotSpot *rectHS = dynamic_cast<const LF::Vision::VNRectHotSpot*>(*hotspot_iter);
				if (rectHS)
				{
					int tag = rectHS->GetTag();
					if( tag == 0 && rectHS->IsTriggered())
					{
						if(++switchStateWait>30)
						{
							SwitchDrawState();
							switchStateWait = 0;
						}
					}
					drawState=kCircles; //forcing drawState to always use kCirlces
					switch(drawState)
					{
						case kRects:
							RenderRectHotSpot(rectHS);
							break;
						case kCircles:
							RenderCircleHotSpot(rectHS);
							break;
						case kExp:
							RenderExpHotSpot(rectHS);
							break;
						case kNorm:
							RenderNormHotSpot(rectHS);
							break;
					}
				}
			}
			glPopMatrix();
	#endif
}

void VTCHMainState::SwitchDrawState()
{
	switch(drawState)
	{
		case kNorm:
			drawState = kRects;
			break;
		case kRects:
			drawState = kExp;
			break;
		case kExp:
			drawState = kCircles;
			break;
		case kCircles:
			drawState = kNorm;
			break;
	}
}

void VTCHMainState::DrawSample(void)
{
	#if RENDER_WITH_OGL
		glClearColor(0.0, 0.0, 0.0, 1.0);
		#if RENDER_WITH_DISPLAY_MPI
			glClearColor(0.0, 0.0, 0.0, 0.0);
		#endif
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		DrawHotSpots();

		eglSwapBuffers(brioOGLConfig_->eglDisplay, brioOGLConfig_->eglSurface);
	#endif
}

void VTCHMainState::RegisterEventListeners(void)
{
//	//register keys
//	LeapFrog::Brio::CButtonMPI keyEventMPI;
//	keyEventMPI.RegisterEventListener(&buttonEventQueue_);

	// register for hot spot events
	LeapFrog::Brio::CEventMPI eventMPI;
	eventMPI.RegisterEventListener(&hotspotListener_);

	GetControllers();
	controller_mpi_.RegisterEventListener(this);
}

void VTCHMainState::UnRegisterEventListeners(void)
{
//	//clean keys
//	LeapFrog::Brio::CButtonMPI keyEventMPI;
//	keyEventMPI.UnregisterEventListener(&buttonEventQueue_);

	// clean hot spot listener
	LeapFrog::Brio::CEventMPI eventMPI;
	eventMPI.UnregisterEventListener(&hotspotListener_);

	controller_mpi_.UnregisterEventListener(this);
}

void VTCHMainState::GetControllers(void)
{
	std::vector< LF::Hardware::HWController * > controllers;
	controller_mpi_.GetAllControllers(controllers);
	numControllers = controller_mpi_.GetNumberOfConnectedControllers();
}

void VTCHMainState::GetErrorArt(void)
{

	path_ = CSystemData::Instance()->GetCurrentTitlePath()+"Art/error-camera.png";
	image_io_.GetInfo(path_, image_surf_);
	image_surf_.buffer = NULL;
	image_io_.Load(path_, image_surf_);
	glGenTextures(1, &texture_);
	glBindTexture(GL_TEXTURE_2D, texture_);

	rgbBuffer_ = (unsigned char*)malloc(image_surf_.pitch * image_surf_.height);

	switch(image_surf_.format)
	{
		case kPixelFormatRGB888:
			//printf("24 bit\n");
			for(int i=0; i<image_surf_.height ; ++i)
			{
				for(int j=0; j<image_surf_.pitch; j+=image_surf_.pitch/image_surf_.width)
				{
					rgbBuffer_[i*image_surf_.pitch + j] = image_surf_.buffer[i*image_surf_.pitch + j+2];;
					rgbBuffer_[i*image_surf_.pitch + j+1] = image_surf_.buffer[i*image_surf_.pitch + j+1];;
					rgbBuffer_[i*image_surf_.pitch + j+2] = image_surf_.buffer[i*image_surf_.pitch + j];;
				}
			}
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_surf_.width, image_surf_.height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbBuffer_);
			break;
		case kPixelFormatARGB8888:
			//printf("32 bit\n");
			for(int i=0; i<image_surf_.height ; ++i)
			{
				for(int j=0; j<image_surf_.pitch; j+=image_surf_.pitch/image_surf_.width)
				{
					rgbBuffer_[i*image_surf_.pitch + j] = image_surf_.buffer[i*image_surf_.pitch + j+2];;
					rgbBuffer_[i*image_surf_.pitch + j+1] = image_surf_.buffer[i*image_surf_.pitch + j+1];;
					rgbBuffer_[i*image_surf_.pitch + j+2] = image_surf_.buffer[i*image_surf_.pitch + j];;
					rgbBuffer_[i*image_surf_.pitch + j+3] = image_surf_.buffer[i*image_surf_.pitch + j+3];;
				}
			}
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_surf_.width, image_surf_.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbBuffer_);
			break;
	}
	delete[]image_surf_.buffer;

	etexcoord_[0] = 0; etexcoord_[1] = 1;
	etexcoord_[2] = 1; etexcoord_[3] = 1;
	etexcoord_[4] = 0; etexcoord_[5] = 0;
	etexcoord_[6] = 1; etexcoord_[7] = 0;
	frame_ = MSRect(MSPoint(0,0), MSSize(image_surf_.width, image_surf_.height));


	MSSize size = ImageSize();
	SetTranslation(MSVector((screenWidth_-size.width_)/2.0,(screenHeight_-size.height_)/2.0));

	UpdateVertices();
}

void VTCHMainState::UpdateVertices(void)
{
    // updates the on screen vertices for rendering
    evertices_[0] = frame_.origin_.x_;
    evertices_[1] = frame_.origin_.y_;

    evertices_[2] = frame_.origin_.x_+frame_.size_.width_;
    evertices_[3] = frame_.origin_.y_;

    evertices_[4] = frame_.origin_.x_;
    evertices_[5] = frame_.origin_.y_+frame_.size_.height_;

    evertices_[6] = frame_.origin_.x_+frame_.size_.width_;
    evertices_[7] = frame_.origin_.y_+frame_.size_.height_;
}

MSSize VTCHMainState::ImageSize(void)
{
    return MSSize(static_cast<float>(image_surf_.width), static_cast<float>(image_surf_.height));
}

void VTCHMainState::Render(void)
{
	glTranslatef(translation_.x_, translation_.y_, 0.0);

	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindTexture(GL_TEXTURE_2D, texture_);
    glVertexPointer(2, GL_FLOAT, 0, evertices_);
    glTexCoordPointer(2, GL_FLOAT, 0, etexcoord_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    glMatrixMode(GL_MODELVIEW);

    eglSwapBuffers(brioOGLConfig_->eglDisplay, brioOGLConfig_->eglSurface);

}

void VTCHMainState::SetFrame(const MSRect& rect)
{
    frame_ = rect;
    UpdateVertices();
}

void VTCHMainState::SetupError(void)
{
	SetupErrorOGL();
}

void VTCHMainState::SetTranslation(const MSVector& vec)
{
    translation_ = vec;
}

void VTCHMainState::Init(void)
{
	//get device stats
	const LeapFrog::Brio::tDisplayScreenStats* myStats;
	myStats = displayManager_.GetScreenStats(0);

	screenWidth_ = myStats->width;
	screenHeight_ = myStats->height;

	#if RENDER_WITH_OGL
		brioOGLConfig_ = new LeapFrog::Brio::BrioOpenGLConfig();
	#endif
}

void VTCHMainState::SetupErrorOGL(void)
{
	#if RENDER_WITH_OGL
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrthox(0, screenWidth_<<16, 0, screenHeight_<<16, 10<<16, 0);
		glMatrixMode(GL_MODELVIEW);
		glClearColor(1.0, 1.0, 1.0, 1.0);
	#endif
}
