//Emerald Sample Project
#include "STUBMainState.h"
#include "STUBApp.h"

#include <AppManager.h>
#include <LogFile.h>
#include <math.h>

#include <Vision/VNRectHotSpot.h>
#include <Vision/VNHotSpotEventMessage.h>
#include <Vision/VNVirtualTouch.h>
#include <Vision/VNOcclusionTrigger.h>

#include <Vect2.h>

using namespace LeapFrog::Brio;

STUBMainState* STUBMainState::mState = NULL;

STUBMainState::STUBMainState() : currentSelection_(0), debugMPI_(kFirstCartridge1DebugSig)
{
}

STUBMainState::~STUBMainState()
{
}
/*
 * Enter is called when the state first loads.
 * All initial setup should take place here
 * Enter is only called once per instance
 */
void STUBMainState::Enter(CStateData* userData)
{
	debugMPI_.SetDebugLevel(DEBUG_LEVEL);
	debugMPI_.DebugOut(kDbgLvlNoteable, "[STUBMainState] Main state STARTED\n\n");

	//register touch screen
	LeapFrog::Brio::CEventMPI event_mpi;
	event_mpi.RegisterEventListener(&m_touch_event_queue);

	//register keys
	LeapFrog::Brio::CButtonMPI key_event_mpi;
	key_event_mpi.RegisterEventListener(&m_button_event_queue);

	//SAMPLE CODE
	initSample();
	//
}

void STUBMainState::Exit()
{
	debugMPI_.DebugOut(kDbgLvlNoteable, "[STUBMainState] Exiting\n");

	//clean touch screen listener
	LeapFrog::Brio::CEventMPI event_mpi;
	event_mpi.UnregisterEventListener(&m_touch_event_queue);

	//clean keys
	LeapFrog::Brio::CButtonMPI key_event_mpi;
	key_event_mpi.UnregisterEventListener(&m_button_event_queue);

	mVisionMPI.Stop();
	for( int i=0; i<hot_spots_.size(); ++i )
	{
		mVisionMPI.RemoveHotSpot(hot_spots_[i]);
		//delete hot spots...
	}
	delete m_visionAlgorithm;
	mDisplayMPI.DestroyHandle(mDisplayHandle, true);

	delete mBrioOpenGLConfig;
	delete rgbBuffer_;
}

void STUBMainState::Suspend()
{
	debugMPI_.DebugOut(kDbgLvlNoteable, "[STUBMainState] Suspend\n");

	//clean touch screen listener
	LeapFrog::Brio::CEventMPI event_mpi;
	event_mpi.UnregisterEventListener(&m_touch_event_queue);

	//clean keys
	LeapFrog::Brio::CButtonMPI key_event_mpi;
	key_event_mpi.UnregisterEventListener(&m_button_event_queue);

	mVisionMPI.Pause();
}

void STUBMainState::Resume()
{
	debugMPI_.DebugOut(kDbgLvlNoteable, "[STUBMainState] Resume\n");

	//register touch screen
	LeapFrog::Brio::CEventMPI event_mpi;
	event_mpi.RegisterEventListener(&m_touch_event_queue);

	//register keys
	LeapFrog::Brio::CButtonMPI key_event_mpi;
	key_event_mpi.RegisterEventListener(&m_button_event_queue);

	//SAMPLE CODE

	//Invalidate the display
	mDisplayMPI.Invalidate(0, NULL);

	//
	mVisionMPI.Start(&mVideoSurf);
}


void STUBMainState::Update(CGameStateHandler* sh)
{
	debugMPI_.DebugOut(kDbgLvlVerbose, "[STUBMainState] Update()\n");

	if (!mVisionMPI.IsRunning())
	{
		mVisionMPI.Start(&mVideoSurf);
	}

	DoTouchEventLoop();
	DoKeyEventLoop();

	//SAMPLE CODE
	drawSample();
}

/*
 * Sample input checks
 */
int STUBMainState::DoKeyEventLoop()
{
	std::vector<LeapFrog::Brio::tButtonData2> *button_event_queue = m_button_event_queue.GetQueue();
	std::vector<LeapFrog::Brio::tButtonData2>::const_iterator button_reader = button_event_queue->begin();

	while(button_reader != button_event_queue->end())
	{
		static U16 button_state = 0;
		LeapFrog::Brio::tButtonData2 button_data = *(button_reader++);
		button_state = button_data.buttonState;
		debugMPI_.DebugOut(kDbgLvlNoteable, "[STUBMainState] button: %d\n", button_state);
		mVisionMPI.Start(  &mVideoSurf );

		/*
		 * button states:
		 * kButtonMenu, kButtonHint
		 * kButtonLeft, kButtonRight, kButtonUp, kButtonDown,
		 * kButtonLeftShoulder, kButtonRightShoulder,
		 * kButtonA, kButtonB
		 */

		if (button_state & kButtonMenu)
		{
			debugMPI_.DebugOut(kDbgLvlNoteable, "\tHOME\n");
			CAppManager::Instance()->PopApp();
			return 1;
		}
		else if(button_state & kButtonUp)
		{
			bRotate = !bRotate;
		}
		else if(!bRotate)
		{
			if(button_state & kButtonLeft)
			{
				rotation_ -= 10;
			}
			else if(button_state & kButtonRight)
			{
				rotation_ += 10;
			}
		}
	}
	return 0;
}

int STUBMainState::DoTouchEventLoop()
{
	std::vector<LeapFrog::Brio::tTouchData> *touch_event_queue = m_touch_event_queue.GetQueue();
	std::vector<LeapFrog::Brio::tTouchData>::const_iterator touch_reader = touch_event_queue->begin();

	while(touch_reader != touch_event_queue->end())
	{
		static U16 touch_state = 0;
		LeapFrog::Brio::tTouchData touch_data = *(touch_reader++);

		//get touch_state:
		//0:	up
		//1:	down*
		touch_state = touch_data.touchState;

		//check for button press
		int pen_x = touch_data.touchX;
		int pen_y = touch_data.touchY;
	}
	return 0;
}

void STUBMainState::initSample()
{
	//Sample Code
	//get device stats
	const LeapFrog::Brio::tDisplayScreenStats* myStats;
	myStats = mDisplayMPI.GetScreenStats(0);
	screenWidth = myStats->width;
	screenHeight = myStats->height;

	bRotate = true;
	rotation_ = 0;

	//NOTE: texture width and height need to be power of 2.
	//set each a little larger than camera rez
	texWidth = 1024;
	texHeight = 512;

	initCamera(640, 480);
	initOGL();
	initVision();
}

void STUBMainState::initCamera(int cw, int ch)
{
	//need to use kPixelFormatRGB888 for emulation
	mDisplayHandle = mDisplayMPI.CreateHandle(ch, cw, kPixelFormatRGB888, NULL);

	mVideoSurf.width = mDisplayMPI.GetWidth(mDisplayHandle);
	mVideoSurf.height = mDisplayMPI.GetHeight(mDisplayHandle);
	mVideoSurf.pitch = mDisplayMPI.GetPitch(mDisplayHandle);
	mVideoSurf.format = mDisplayMPI.GetPixelFormat(mDisplayHandle);

	mVideoSurf.buffer = mDisplayMPI.GetBuffer(mDisplayHandle);

	tCaptureMode *cameraMode;
	cameraMode = mCameraMPI.GetCurrentFormat();
	cameraMode->width = cw;
	cameraMode->height = ch;

	mCameraMPI.SetCurrentFormat(cameraMode);
	rgbBuffer_ = (unsigned char*)malloc(mVideoSurf.pitch * mVideoSurf.height  );

}

void STUBMainState::initVision()
{
	//Set the Algorithm to useSTUBMainState
	m_visionAlgorithm =	new LF::Vision::VNVirtualTouch();
	mVisionMPI.SetAlgorithm( m_visionAlgorithm );

	//hot spots
	LeapFrog::Brio::tRect rect;
	rect.left = 0;
	rect.right = 60;
	rect.top = 0;
	rect.bottom = 60;
	LF::Vision::VNHotSpot* hotSpot1 = new LF::Vision::VNRectHotSpot(rect);
	hotSpot1->SetTag(1);
	LF::Vision::VNOcclusionTrigger* trigger1 = new LF::Vision::VNOcclusionTrigger();
	hotSpot1->SetTrigger(trigger1);
	hot_spots_.push_back(hotSpot1);
	mVisionMPI.AddHotSpot(hotSpot1);

	LeapFrog::Brio::tRect rect2;
	rect2.left = 580;
	rect2.right = 640;
	rect2.top = 0;
	rect2.bottom = 60;
	LF::Vision::VNHotSpot* hotSpot2 = new LF::Vision::VNRectHotSpot(rect2);
	hotSpot2->SetTag(2);
	hotSpot2->SetTrigger(trigger1);
	hot_spots_.push_back(hotSpot2);
	mVisionMPI.AddHotSpot(hotSpot2);

	LeapFrog::Brio::CEventMPI event_mpi;
	event_mpi.RegisterEventListener(&m_hotspot_listener);
}

void STUBMainState::initOGL()
{
	mBrioOpenGLConfig = new BrioOpenGLConfig(kBrioOpenGL11);

	//set up texture for camera
	////////////////////////////////////////////
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &mRGBTexture);
	glBindTexture(GL_TEXTURE_2D, mRGBTexture);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, mVideoSurf.buffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	//set up 3d world
	////////////////////////////////////////////
	int near_plane_ = 300;
	int far_plane_ = 1000;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_ALPHA_TEST);
	glAlphaFuncx(GL_GREATER, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_FRONT);

	glClearColorx((GLfixed) 100 * 256,
				  (GLfixed) 100 * 256,
				  (GLfixed) 200 * 256,
				  (GLfixed) 255 * 256);

	glViewport(0, 0, screenWidth, screenHeight);

	glEnableClientState(GL_VERTEX_ARRAY);

	tVect2 screensize = tVect2(screenWidth, screenHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glFrustumx(
		-screensize.x.g/2,		//left
		screensize.x.g/2,		//right
		-screensize.y.g/2,		//bottom
		screensize.y.g/2,		//top
		tFixed(near_plane_).g,	//near
		tFixed(far_plane_).g		//far
	);

	glMatrixMode(GL_MODELVIEW);
	//checkGLError();
}

void STUBMainState::drawSample()
{

	/////
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer

	glLoadIdentity();

	//Transformations
	glTranslatef(0.0, 0.0, -500);
	glRotatef(rotation_, 0.0f, 1.0f, 0.0f);
	if(bRotate) rotation_ += 2;

	glEnableClientState(GL_VERTEX_ARRAY);
	GLfloat vertices[] = {
			-320.0, 240.0, 0.0,
			320.0, 240.0, 0.0,
			-320.0, -240.0, 0.0,
			320.0, -240.0, 0.0
	};
	glVertexPointer(3, GL_FLOAT, 0, vertices);

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	float glw, glh;
	glw = (float)mVideoSurf.width/(float)texWidth;
	glh = (float)mVideoSurf.height/(float)texHeight;
	GLfloat textures[] = {
			0.0, 0.0,
			glw, 0.0,
			0.0, glh,
			glw, glh
	};

	glTexCoordPointer(2, GL_FLOAT, 0, textures);

	//draw it
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, mRGBTexture);

	swizzleColors();

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mVideoSurf.width, mVideoSurf.height, GL_RGB, GL_UNSIGNED_BYTE, rgbBuffer_);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glPopMatrix();

	glFinish();

	eglSwapBuffers(mBrioOpenGLConfig->eglDisplay, mBrioOpenGLConfig->eglSurface);
}

void STUBMainState::swizzleColors()
{
	for(int i=0; i<mVideoSurf.height ; ++i)
	{
		for(int j=0; j<mVideoSurf.pitch; j+=mVideoSurf.pitch/mVideoSurf.width)
		{
			rgbBuffer_[i*mVideoSurf.pitch + j] = mVideoSurf.buffer[i*mVideoSurf.pitch + j+2];;
			rgbBuffer_[i*mVideoSurf.pitch + j+1] = mVideoSurf.buffer[i*mVideoSurf.pitch + j+1];;
			rgbBuffer_[i*mVideoSurf.pitch + j+2] = mVideoSurf.buffer[i*mVideoSurf.pitch + j];;
		}
	}
}

void STUBMainState::checkGLError()
{
	GLenum err = glGetError();
	printf("\n");
	switch(err)
	{
	case GL_NO_ERROR:
		printf("no error\n");
		break;
	case GL_INVALID_ENUM:
		printf("GL_INVALID_ENUM\n");
		break;
	case GL_INVALID_VALUE:
		printf("GL_INVALID_VALUE\n");
		break;
	case GL_INVALID_OPERATION:
		printf("GL_INVALID_OPERATION\n");
		break;
	case GL_OUT_OF_MEMORY:
		printf("GL_OUT_OF_MEMORY\n");
		break;
	case GL_STACK_UNDERFLOW:
		printf("GL_STACK_UNDERFLOW\n");
		break;
	case GL_STACK_OVERFLOW:
		printf("GL_STACK_OVERFLOW\n");
		break;
	default:
		printf("error %d\n", (int)err);
		break;
	}
	printf("\n");
}
//Hot Spot Listener
const tEventType gHotSpotEventKey[] = {LF::Vision::kVNHotSpotTriggeredEvent};

STUBMainState::HotSpotListener::HotSpotListener() : IEventListener(gHotSpotEventKey, 5)
{}

LeapFrog::Brio::tEventStatus
STUBMainState::HotSpotListener::Notify(const LeapFrog::Brio::IEventMessage& msg)
{
	const LF::Vision::VNHotSpotEventMessage& hsMessage = dynamic_cast<const LF::Vision::VNHotSpotEventMessage&>(msg);
	const LF::Vision::VNHotSpot* hs = hsMessage.GetHotSpot();
	if (hs->IsTriggered())
	{
		if(hs->GetTag()==1)
			printf("Change region one to true \n");
		else if(hs->GetTag()==2)
			printf("Change region two to true \n");
	}

	return kEventStatusOK;
}
//

