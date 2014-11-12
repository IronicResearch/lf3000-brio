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

#define VERTEX_ARRAY	0
#define TEXCOORD_ARRAY	1

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

	glDeleteTextures(3, mYUVTexture);

	delete mBrioOpenGLConfig;
	delete camBuffer_;
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

	initCamera(640, 480);
	initOGL();
	initVision();
}

void STUBMainState::initCamera(int cw, int ch)
{
	//need to use kPixelFormatRGB888 for emulation
	mDisplayHandle = mDisplayMPI.CreateHandle(ch, cw, kPixelFormatYUV420, NULL);

	mVideoSurf.width = mDisplayMPI.GetWidth(mDisplayHandle);
	mVideoSurf.height = mDisplayMPI.GetHeight(mDisplayHandle);
	mVideoSurf.pitch = mDisplayMPI.GetPitch(mDisplayHandle);
	mVideoSurf.format = mDisplayMPI.GetPixelFormat(mDisplayHandle);

	mVideoSurf.buffer = mDisplayBuffers[0] = mDisplayMPI.GetBuffer(mDisplayHandle);
	mDisplayBuffers[1] = mDisplayBuffers[0] + mVideoSurf.pitch/2;
	mDisplayBuffers[2] = mDisplayBuffers[1] + mVideoSurf.height/2 * mVideoSurf.pitch;

	tCaptureMode *cameraMode;
	cameraMode = mCameraMPI.GetCurrentFormat();
	cameraMode->width = cw;
	cameraMode->height = ch;

	mCameraMPI.SetCurrentFormat(cameraMode);
	camBuffer_ = (unsigned char*)malloc(mVideoSurf.pitch * mVideoSurf.height  );

	//mVideoCaptureHandle = mCameraMPI.StartVideoCapture(&mVideoSurf);
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
	mBrioOpenGLConfig = new BrioOpenGLConfig(kBrioOpenGL20);

	const char* pszFragShader = "\
			uniform sampler2D samplery;\
			uniform sampler2D sampleru;\
			uniform sampler2D samplerv;\
			/*Precision qualifiers required on LF3000 fragment shader for vec and float*/\
			/*Precision qualifiers break on LF2000*/\
			varying mediump vec2	myTexCoord;\
			void main (void)\
			{\
				mediump float d = texture2D(sampleru, myTexCoord).r - 0.5;\
				mediump float e = texture2D(samplerv, myTexCoord).r - 0.5;\
				\
				/*NVidia post RGB from YUV*/\
				/*float c2 = texture2D(samplery, myTexCoord).r;\
				gl_FragColor.r = c2 + 1.403 * e;\
				gl_FragColor.g = c2 - 0.344 * d - 1.403 * e;\
				gl_FragColor.b = c2 + 1.770 * d;*/\
				\
				/*Brio conversion*/\
				mediump float c2 = 1.1640625 * (texture2D(samplery, myTexCoord).r - 0.0625);\
				gl_FragColor.r = c2 + 1.59765625 * e, 0.0, 1.0;\
				gl_FragColor.g = c2 - 0.390625 * d - 0.8125 * e, 0.0, 1.0;\
				gl_FragColor.b = c2 + 2.015625 * d, 0.0, 1.0;\
				\
				gl_FragColor.a = 1.0;\
			}";
	const char* pszVertShader =  "attribute vec4	myVertex;\
			attribute vec4	myUV;\
			uniform mat4	myPMVMatrix;\
			varying vec2	myTexCoord;\
			void main(void)\
			{\
				gl_Position = myPMVMatrix * myVertex;\
				myTexCoord = myUV.st;\
			}";

	mFragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(mFragShader, 1, (const char**)&pszFragShader, NULL);
	glCompileShader(mFragShader);
	GLint status;
	glGetShaderiv(mFragShader, GL_COMPILE_STATUS, &status);
	if(status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(mFragShader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(mFragShader, infoLogLength, &infoLogLength, strInfoLog);
		printf("Failed to compile fragment shader: %s\n", strInfoLog);
		delete[] strInfoLog;
	}

	mVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(mVertexShader, 1, (const char**)&pszVertShader, NULL);
	glCompileShader(mVertexShader);
	glGetShaderiv(mVertexShader, GL_COMPILE_STATUS, &status);
	if(status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(mVertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(mVertexShader, infoLogLength, &infoLogLength, strInfoLog);
		printf("Failed to compile vertex shader: %s\n", strInfoLog);
		delete[] strInfoLog;
	}

	mProgramObject = glCreateProgram();
	glAttachShader(mProgramObject, mFragShader);
	glAttachShader(mProgramObject, mVertexShader);
	glBindAttribLocation(mProgramObject, VERTEX_ARRAY, "myVertex");
	glBindAttribLocation(mProgramObject, TEXCOORD_ARRAY, "myUV");
	glLinkProgram(mProgramObject);
	GLint bLinked;
	glGetProgramiv(mProgramObject, GL_LINK_STATUS, &bLinked);
	if (!bLinked)
	{
		int ui32InfoLogLength, ui32CharsWritten;
		glGetProgramiv(mProgramObject, GL_INFO_LOG_LENGTH, &ui32InfoLogLength);
		char* pszInfoLog = new char[ui32InfoLogLength];
		glGetProgramInfoLog(mProgramObject, ui32InfoLogLength, &ui32CharsWritten, pszInfoLog);
		printf("Failed to link program: %s\n", pszInfoLog);
		delete [] pszInfoLog;
	}

	glUseProgram(mProgramObject);
	glUniform1i(glGetUniformLocation(mProgramObject, "samplery"), 0);
	glUniform1i(glGetUniformLocation(mProgramObject, "sampleru"), 1);
	glUniform1i(glGetUniformLocation(mProgramObject, "samplerv"), 2);

	GLubyte y[mVideoSurf.width*mVideoSurf.height];
	GLubyte u[mVideoSurf.width/2*mVideoSurf.height/2];
	GLubyte v[mVideoSurf.width/2*mVideoSurf.height/2];

	for(int i = 0; i < mVideoSurf.height; ++i)
	{
		memcpy(&y[i*mVideoSurf.width], &mDisplayBuffers[0][i*mVideoSurf.pitch], mVideoSurf.width);
		if(i%0x1 == 0)
		{
			memcpy(&u[i/2*mVideoSurf.width/2], &mDisplayBuffers[1][i/2*mVideoSurf.pitch], mVideoSurf.width/2);
			memcpy(&v[i/2*mVideoSurf.width/2], &mDisplayBuffers[2][i/2*mVideoSurf.pitch], mVideoSurf.width/2);
		}
	}

	//glEnable(GL_TEXTURE_2D);
	glGenTextures(3, mYUVTexture);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mYUVTexture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mVideoSurf.width, mVideoSurf.height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, y);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mYUVTexture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mVideoSurf.width/2, mVideoSurf.height/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, u);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mYUVTexture[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mVideoSurf.width/2, mVideoSurf.height/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, v);
	checkGLError();
}

void STUBMainState::drawSample()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	static float theta = 0.0;
	theta += 3.14159265/90.0;
	if(theta >= 3.14159265 * 2)
		theta = 0.0;

	float costheta = cos(theta);
	float sintheta = sin(theta);

	static float scale = 1.0;
	static float dScale = 0.99;
	scale *= (dScale);
	if(scale < 0.1)
	{
		dScale = 1.01;
	}
	else if(scale > 2.9)
	{
		dScale = 0.99;
	}

	GLfloat transform_matrix[] = {
		0, scale*costheta,scale*sintheta,0,
		scale*-sintheta,scale*costheta,0,0,
		0,0,1,0,
		0,0,0,1
	};

	GLint mat = glGetUniformLocation(mProgramObject, "myPMVMatrix");
	if(bRotate) glUniformMatrix4fv(mat, 1, GL_FALSE, transform_matrix);

	glEnableVertexAttribArray(VERTEX_ARRAY);
	GLfloat vertices[] = {-1.0, 1.0, 0.0,
			1.0, 1.0, 0.0,
			-1.0, -1.0, 0.0,
			1.0, -1.0, 0.0
	};
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, 0, vertices);

	glEnableVertexAttribArray(TEXCOORD_ARRAY);
	GLfloat texcoords[] = {0.0, 0.0,
			1.0, 0.0,
			0.0, 1.0,
			1.0, 1.0
	};
	glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, 0, texcoords);

	GLubyte y[mVideoSurf.width*mVideoSurf.height];
	GLubyte u[mVideoSurf.width/2*mVideoSurf.height/2];
	GLubyte v[mVideoSurf.width/2*mVideoSurf.height/2];

	//Need to make a copy of the video feed.
	//We run into issues if we try to use the feed directly.
	//TODO: use camera feed event for better perfomance.
	for(int i=0; i<mVideoSurf.height*mVideoSurf.pitch ; ++i)
	{
			camBuffer_[i] = mVideoSurf.buffer[i];
	}
	mDisplayBuffers[0] = camBuffer_;
	mDisplayBuffers[1] = mDisplayBuffers[0] + mVideoSurf.pitch/2;
	mDisplayBuffers[2] = mDisplayBuffers[1] + mVideoSurf.height/2 * mVideoSurf.pitch;

	for(int i = 0; i < mVideoSurf.height; ++i)
	{
		memcpy(&y[i*mVideoSurf.width], &mDisplayBuffers[0][i*mVideoSurf.pitch], mVideoSurf.width);
		if(i%0x1 == 0)
		{
			memcpy(&u[i/2*mVideoSurf.width/2], &mDisplayBuffers[1][i/2*mVideoSurf.pitch], mVideoSurf.width/2);
			memcpy(&v[i/2*mVideoSurf.width/2], &mDisplayBuffers[2][i/2*mVideoSurf.pitch], mVideoSurf.width/2);
		}
	}

	GLenum err = glGetError();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mYUVTexture[0]);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mVideoSurf.width, mVideoSurf.height, GL_LUMINANCE, GL_UNSIGNED_BYTE, y);
	err = glGetError();
	if(err != GL_NO_ERROR)
	{
		printf("%d! %x\n", __LINE__, (int) err);
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mYUVTexture[1]);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mVideoSurf.width/2, mVideoSurf.height/2, GL_LUMINANCE, GL_UNSIGNED_BYTE, u);
	err = glGetError();
	if(err != GL_NO_ERROR)
	{
		printf("%d! %x\n", __LINE__, (int) err);
	}

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mYUVTexture[2]);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mVideoSurf.width/2, mVideoSurf.height/2, GL_LUMINANCE, GL_UNSIGNED_BYTE, v);
	err = glGetError();
	if(err != GL_NO_ERROR)
	{
		printf("%d! %x\n", __LINE__, (int) err);
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	eglSwapBuffers(mBrioOpenGLConfig->eglDisplay, mBrioOpenGLConfig->eglSurface);
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
	default:
		printf("error %d\n", (int)err);
		break;
	}
	printf("\n");
}

void STUBMainState::test()
{
	for(int i=0; i<100; ++i)
	{
		printf("%x", mDisplayBuffers[0][i]);
	}
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

