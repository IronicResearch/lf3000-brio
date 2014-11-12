//Emerald Sample Project

#include "ALSMainState.h"
#include "ALSApp.h"

#include <AppManager.h>
#include <TouchEventQueue.h>

using LeapFrog::Brio::S16;
const LeapFrog::Brio::tEventType kALSListenerTypes[] = { LF::Hardware::kHWAllAnalogStickEvents };

ALSMainState* ALSMainState::Instance(void)
{
	// singleton access point
	static ALSMainState* instance = NULL;
	if (!instance)
	{
		instance = new ALSMainState();
	}
	return instance;
}

ALSMainState::ALSMainState() :
		IEventListener(kALSListenerTypes, ArrayCount(kALSListenerTypes)),
		analog_stick_mpi_(NULL),
		config_(NULL)
{
	// Query screen size
	LeapFrog::Brio::CDisplayMPI display_mpi;
	const LeapFrog::Brio::tDisplayScreenStats* pscreen = display_mpi.GetScreenStats(0);
	screen_width_ = pscreen->width;
	screen_height_ = pscreen->height;
}

ALSMainState::~ALSMainState()
{
	if (analog_stick_mpi_)
	{
		delete analog_stick_mpi_;
		analog_stick_mpi_ = NULL;
	}

	if (config_)
	{
		delete config_;
		config_ = NULL;
	}

}

void ALSMainState::Enter(CStateData* userData)
{
	Init();
	RegisterEventListeners();
}

void ALSMainState::Exit()
{
	UnregisterEventListeners();
}

void ALSMainState::Suspend()
{
	CAppManager::Instance()->EnableInactivity(true);
	UnregisterEventListeners();
}

void ALSMainState::Resume()
{
	RegisterEventListeners();
}

void ALSMainState::Update(CGameStateHandler* sh)
{
	//check for input
	DoKeyEventLoop();
	DoTouchEventLoop();

	if(analog_stick_on_ && !touching_)
	{
		DoAccEventLoop();
		MoveObjectWithGravity();
	}
	MoveObjectIncrementally();

	RenderOGL();
	eglSwapBuffers( config_->eglDisplay, config_->eglSurface );
}

int ALSMainState::DoKeyEventLoop()
{
	std::vector<LeapFrog::Brio::tButtonData2> *button_event_queue = button_event_queue_.GetQueue();
	std::vector<LeapFrog::Brio::tButtonData2>::const_iterator button_reader = button_event_queue->begin();

	while(button_reader != button_event_queue->end())
	{
		static LeapFrog::Brio::U16 button_state = 0;
		LeapFrog::Brio::tButtonData2 button_data = *(button_reader++);
		button_state = button_data.buttonState;

		if ((button_state & LeapFrog::Brio::kButtonLeft))
		{
			gravity_*=-1;
		}
		else if ((button_state & LeapFrog::Brio::kButtonA))
		{
			analog_stick_on_ = !analog_stick_on_;
		}
		else if ((button_state & LeapFrog::Brio::kButtonB))
		{
			ResetObject();
		}
		else if((button_state & LeapFrog::Brio::kButtonLeftShoulder))
		{
			gravity_*=-1;
		}
		if (button_state & LeapFrog::Brio::kButtonMenu)
		{
			CAppManager::Instance()->PopApp();
		}
	}
	return 0;
}

int ALSMainState::DoTouchEventLoop()
{
	std::vector<LeapFrog::Brio::tTouchData> *touch_event_queue = touch_event_queue_.GetQueue();
	std::vector<LeapFrog::Brio::tTouchData>::const_iterator touch_reader = touch_event_queue->begin();

	while(touch_reader != touch_event_queue->end())
	{
		static LeapFrog::Brio::U16 touch_state = 0;
		LeapFrog::Brio::tTouchData touch_data = *(touch_reader++);

		//get touch_state:
		//0:	up
		//1:	down
		touch_state = touch_data.touchState;

		//check for button press
		int pen_x = touch_data.touchX-screen_width_/2;
		int pen_y = screen_height_ - touch_data.touchY-screen_height_/2;	//need to reverse the values

		if(touch_state == 1)
		{
			touching_ = true;
			dx_ = ((float)pen_x-x_pos_)/10;
			dy_ = ((float)pen_y-y_pos_)/10;
		}
		else
		{
			touching_ = false;
			dx_ = 0.0f;
			dy_ = 0.0f;
		}
	}
	return 0;
}

LeapFrog::Brio::tEventStatus ALSMainState::Notify( const LeapFrog::Brio::IEventMessage &msgIn )
{
	if (msgIn.GetEventType() == LF::Hardware::kHWAnalogStickDataChanged) {
		const LF::Hardware::HWAnalogStickMessage& aclmsg = reinterpret_cast<const LF::Hardware::HWAnalogStickMessage&>(msgIn);
		LF::Hardware::tHWAnalogStickData acldata = aclmsg.GetAnalogStickData();
		// Update current accelerometer x,y,z data
		x_ = acldata.x;
		y_ = acldata.y;
		printf( "accel: %f, %f\n", x_, y_ );
		return LeapFrog::Brio::kEventStatusOKConsumed;
	}
	return LeapFrog::Brio::kEventStatusOK;
}

int ALSMainState::DoAccEventLoop()
{
	// Analog Stick x,y data updated in ::Notify() listener (above)

	dx_+=((float)x_ * kAcceleration);
	dy_+=((float)y_ * kAcceleration);

	return 0;
}

void ALSMainState::MoveObjectWithGravity()
{
	float dist = kLongestSide*kObjectSize;
	x_pos_-=(gravity_*dx_);
	if(x_pos_>screen_width_/2-dist)
	{
		x_pos_ = screen_width_/2-dist;
		dx_ = -dx_/2;
	}
	else if(x_pos_<-screen_width_/2+dist)
	{
		x_pos_ = -screen_width_/2+dist;
		dx_ = -dx_/2;
	}

	y_pos_+=(gravity_*dy_);
	if(y_pos_>screen_height_/2-dist)
	{
		y_pos_ = screen_height_/2-dist;
		dy_ = -dy_/2;
	}
	else if(y_pos_<-screen_height_/2+dist)
	{
		y_pos_ = -screen_height_/2+dist;
		dy_ = -dy_/2;
	}

}

void ALSMainState::MoveObjectIncrementally()
{
	x_pos_+=dx_;
	y_pos_+=dy_;
}

void ALSMainState::RenderOGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer

	//***************
	//pyramid
	//***************
	glLoadIdentity();

	static const GLfloat myVertices[] =
    {
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

    static const GLubyte myColors[] =
    {
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
	glTranslatef(x_pos_, y_pos_, z_pos_);
	glRotatef(rotation_, 0.0f, 1.0f, 0.0f);
	rotation_ += 5;

	glPushMatrix();

	//draw it
	glVertexPointer(3, GL_FLOAT, 0, myVertices);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, myColors);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 12);

	glPopMatrix();
}

void ALSMainState::ResetObject()
{
	x_pos_ = 0.0f;
	y_pos_ = 0.0f;
	z_pos_ = -(near_plane_+initial_z_);

	dx_ = 0.0f;
	dy_ = 0.0f;

	rotation_ = 0;
}

void ALSMainState::InitOGL()
{
	//vars
	initial_z_ = 250;
	near_plane_ = 300;
	far_plane_ = 1000;

	ResetObject();

	//colors
	bg_color_.r_ = 100;
	bg_color_.g_ = 100;
	bg_color_.b_ = 120;

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
	near_plane_ = 300;
	far_plane_ = 1000;

	SetClearColor(bg_color_.r_, bg_color_.g_, bg_color_.b_, 255);

	glViewport(0, 0, screen_width_, screen_height_);

	//set on here (always on)
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glDisable(GL_BLEND);

	tVect2 screensize = tVect2(screen_width_, screen_height_);
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
}

void ALSMainState::SetClearColor(S16 r, S16 g, S16 b, S16 a)
{
	tColor clearColor = tColor(r, g, b, a);

	glClearColorx((GLfixed) clearColor.r * 256,
				  (GLfixed) clearColor.g * 256,
				  (GLfixed) clearColor.b * 256,
				  (GLfixed) clearColor.a * 256);
}

void ALSMainState::Init(void)
{
	LeapFrog::Brio::CPath myFontPath(CSystemData::Instance()->FindFont("LFCurry.ttf"));

	analog_stick_on_ = true;
	touching_ = false;

	InitOGL();
}

void ALSMainState::RegisterEventListeners(void)
{
    //register touch screen
    LeapFrog::Brio::CEventMPI event_mpi;
    event_mpi.RegisterEventListener(&touch_event_queue_);

    //register keys
    LeapFrog::Brio::CButtonMPI key_event_mpi;
    key_event_mpi.RegisterEventListener(&button_event_queue_);
    key_event_mpi.SetTouchMode( LeapFrog::Brio::kTouchModeDefault );

	// register for accelerometer notifications
	analog_stick_mpi_ = new LF::Hardware::HWAnalogStickMPI;
	analog_stick_mpi_->RegisterEventListener(this);
}

void ALSMainState::UnregisterEventListeners(void)
{
    // stop listening for screen touch events
    LeapFrog::Brio::CEventMPI event_mpi;
    event_mpi.UnregisterEventListener(&touch_event_queue_);

    // stop listening for physical button events
    LeapFrog::Brio::CButtonMPI key_event_mpi;
    key_event_mpi.UnregisterEventListener(&button_event_queue_);

	// stop listening for accelerometer events
	analog_stick_mpi_->UnregisterEventListener(this);
}
