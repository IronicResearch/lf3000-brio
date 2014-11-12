//Emerald Sample Project

#include "CTRLMainState.h"
#include <AppManager.h>
#include <TouchEventQueue.h>
#include <Hardware/HWControllerEventMessage.h>
#include <Hardware/HWController.h>
#include <AccelerometerTypes.h>
#include <KeyboardManager.h>


using LeapFrog::Brio::S16;
const LeapFrog::Brio::tEventType 
kCTRLListenerTypes[] = { LF::Hardware::kHWControllerAnalogStickDataChanged,
		LF::Hardware::kHWControllerButtonStateChanged,
		LF::Hardware::kHWControllerAccelerometerDataChanged };

CTRLMainState* CTRLMainState::Instance(void) {
	// singleton access point
	static CTRLMainState* instance = NULL;
	if (!instance) {
		instance = new CTRLMainState();
	}
	return instance;
}

CTRLMainState::CTRLMainState(void) :
		  IEventListener(kCTRLListenerTypes, ArrayCount(kCTRLListenerTypes)),
		  config_(NULL) {
	// Query screen size
	LeapFrog::Brio::CDisplayMPI display_mpi;
	const LeapFrog::Brio::tDisplayScreenStats* pscreen = display_mpi.GetScreenStats(0);
	screen_width_ = pscreen->width;
	screen_height_ = pscreen->height;
}

CTRLMainState::~CTRLMainState(void) {

	if (config_) {
		delete config_;
		config_ = NULL;
	}
}

void CTRLMainState::Enter(CStateData* userData) {
	Init();
	RegisterEventListeners();
}

void CTRLMainState::Exit(void) {
	UnregisterEventListeners();
}

void CTRLMainState::Suspend(void) {
	UnregisterEventListeners();
}

void CTRLMainState::Resume(void) {
	RegisterEventListeners();
}

void CTRLMainState::Update(CGameStateHandler* sh) {

	//printf("numControllers: %d\n", numControllers);
	for(int i=0; i<numControllers; ++i)
	{
		if(controllerData[i].analog_stick_on_)
		{
			DoAnalogStickLoop(i);
		}
		else
		{
			DoAccEventLoop(i);
			MoveObjectWithGravity(i);
		}
	}
	RenderOGL();
	eglSwapBuffers( config_->eglDisplay, config_->eglSurface );
}

LeapFrog::Brio::tEventStatus CTRLMainState::Notify( const LeapFrog::Brio::IEventMessage &msgIn ) {
	const LF::Hardware::HWControllerEventMessage& msg = reinterpret_cast<const LF::Hardware::HWControllerEventMessage&>(msgIn);
	const LF::Hardware::HWController *controller = msg.GetController();

	U8 ID = controller->GetID();
	//printf("controller ID: %d\n", (int)ID);

	if (msgIn.GetEventType() == LF::Hardware::kHWControllerAnalogStickDataChanged)
	{
		LF::Hardware::tHWAnalogStickData data = controller->GetAnalogStickData();
		// Update current accelerometer x,y,z data
		controllerData[ID].x_ = data.x;
		controllerData[ID].y_ = data.y;
		//printf("%d\t(%f, %f)\n", ID, data.x, data.y);
		return LeapFrog::Brio::kEventStatusOKConsumed;
	}
	else if (msgIn.GetEventType() == LF::Hardware::kHWControllerButtonStateChanged) {
		LeapFrog::Brio::tButtonData2 button_data = controller->GetButtonData();
		static LeapFrog::Brio::U32 button_state = 0;
		static LeapFrog::Brio::U32 button_transition = 0;

		button_state = button_data.buttonState;
		button_transition = button_data.buttonTransition;

		//printf("button_state, button_transition: %d, %d\n", button_state, button_transition);

		if(button_transition == button_state)
		{
			if ((button_state & LeapFrog::Brio::kButtonA))
			{
				controllerData[ID].analog_stick_on_ = !controllerData[ID].analog_stick_on_;
				printf("%d: A button pressed\n", ID);
			}
			else if ((button_state & LeapFrog::Brio::kButtonB))
			{
				//printf("%d: B button pressed\n", ID);
				ResetObject(ID);
			}
			else if ((button_state & LeapFrog::Brio::kButtonSync) || (button_state & LeapFrog::Brio::kButtonHint))
			{
				GetControllers();
			}
			else if ((button_state & LeapFrog::Brio::kButtonMenu))
			{
				CAppManager::Instance()->PopApp();
			}
		}
		return LeapFrog::Brio::kEventStatusOKConsumed;
	}
	else if (msgIn.GetEventType() == LF::Hardware::kHWControllerAccelerometerDataChanged)
	{
		LeapFrog::Brio::tAccelerometerData data = controller->GetAccelerometerData();
		controllerData[ID].ax_ = data.accelX;
		controllerData[ID].ay_ = -data.accelY;
		controllerData[ID].az_ = data.accelZ;

		//printf( "acceleration: %ld, %ld, %ld\n", data.accelX, data.accelY, data.accelZ );
		return LeapFrog::Brio::kEventStatusOKConsumed;
	}
	return LeapFrog::Brio::kEventStatusOK;
}

//Analog Stick
void CTRLMainState::DoAnalogStickLoop(int i)
{
	// Analog Stick x,y data updated in ::Notify() listener (above)
	controllerData[i].x_pos_ = controllerData[i].x_*screen_width_/2;
	controllerData[i].y_pos_ = controllerData[i].y_*screen_height_/2;
	controllerData[i].z_pos_ = -near_plane_ - kLongestSide*kObjectSize;
}
//Accelerometer
int CTRLMainState::DoAccEventLoop(int i) {
	// Accelerometer x,y,z data updated in ::Notify() listener (above)
	controllerData[i].dx_+=((float)controllerData[i].ax_/kAcceleration);
	controllerData[i].dy_+=((float)controllerData[i].ay_/kAcceleration);
	controllerData[i].dz_+=((float)controllerData[i].az_/kAcceleration);

	return 0;
}

void CTRLMainState::MoveObjectWithGravity(int i) {
	float dist = kLongestSide*kObjectSize;
	controllerData[i].x_pos_-=(gravity_*controllerData[i].dx_);
	if(controllerData[i].x_pos_>screen_width_/2-dist) {
		controllerData[i].x_pos_ = screen_width_/2-dist;
		controllerData[i].dx_ = -controllerData[i].dx_/2;
	} else if(controllerData[i].x_pos_<-screen_width_/2+dist) {
		controllerData[i].x_pos_ = -screen_width_/2+dist;
		controllerData[i].dx_ = -controllerData[i].dx_/2;
	}

	controllerData[i].y_pos_+=(gravity_*controllerData[i].dy_);
	if(controllerData[i].y_pos_>screen_height_/2-dist) {
		controllerData[i].y_pos_ = screen_height_/2-dist;
		controllerData[i].dy_ = -controllerData[i].dy_/2;
	} else if(controllerData[i].y_pos_<-screen_height_/2+dist) {
		controllerData[i].y_pos_ = -screen_height_/2+dist;
		controllerData[i].dy_ = -controllerData[i].dy_/2;
	}

	controllerData[i].z_pos_+=(gravity_*controllerData[i].dz_);
	if (controllerData[i].z_pos_<-far_plane_+dist) {
		controllerData[i].z_pos_ = -far_plane_+dist;
		controllerData[i].dz_ = -controllerData[i].dz_/2;
	} else if(controllerData[i].z_pos_>-near_plane_-dist) {
		controllerData[i].z_pos_ = -near_plane_-dist;
		controllerData[i].dz_ = -controllerData[i].dz_/2;
	}
}

void CTRLMainState::RenderOGL(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer

	//***************
	//pyramid
	//***************

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

	//Transformations
	for(int i=0; i<numControllers; ++i)
	{
		GLubyte myColors[] = {
				255*i,	255-(i*127),	0, 255,
				255*i,	255-(i*127),	0, 255,
				255*i,	255-(i*127),	0, 255,
				255*i,	255-(i*127),	0, 255,
				255*i,	255-(i*127),	0, 255,
				255*i,	255-(i*127),	0, 255,
				255*i,	255-(i*127),	0, 255,
				255*i,	255-(i*127),	0, 255,
				255*i,	255-(i*127),	0, 255,
				255*i,	255-(i*127),	0, 255,
				255*i,	255-(i*127),	0, 255,
				255*i,	255-(i*127),	0, 255,
		};

		glLoadIdentity();
		glPushMatrix();
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, myColors);

		glTranslatef(controllerData[i].x_pos_, controllerData[i].y_pos_, controllerData[i].z_pos_);
		++controllerData[i].rotationX_;
		++controllerData[i].rotationY_;
		glRotatef(controllerData[i].rotationX_, 1.0f, 0.0f, 0.0f);
		glRotatef(controllerData[i].rotationY_, 0.0f, 1.0f, 0.0f);

		glPushMatrix();

		//draw it
		glVertexPointer(3, GL_FLOAT, 0, myVertices);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, myColors);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 12);

		glPopMatrix();
	}
}

void CTRLMainState::ResetObject(int i) {
	controllerData[i].x_pos_ = 0.0f;
	controllerData[i].y_pos_ = 0.0f;
	controllerData[i].z_pos_ = -(near_plane_+initial_z_);

	controllerData[i].dx_ = 0.0f;
	controllerData[i].dy_ = 0.0f;

	controllerData[i].rotationX_ = 0;
	controllerData[i].rotationY_ = 0;
}

void CTRLMainState::InitOGL(void) {
	//vars
	initial_z_ = 250;
	near_plane_ = 300;
	far_plane_ = 1000;

	for(int i=0; i<MAX_CONTROLLERS; ++i)
	{
		ResetObject(i);
	}

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

void CTRLMainState::SetClearColor(S16 r, S16 g, S16 b, S16 a) {
	tColor clearColor = tColor(r, g, b, a);

	glClearColorx((GLfixed) clearColor.r * 256,
			(GLfixed) clearColor.g * 256,
			(GLfixed) clearColor.b * 256,
			(GLfixed) clearColor.a * 256);
}

void CTRLMainState::Init(void) {
	//LeapFrog::Brio::CPath myFontPath(CSystemData::Instance()->FindFont("LFCurry.ttf"));

	for(int i=0; i<MAX_CONTROLLERS; ++i)
	{
		controllerData[i].analog_stick_on_ = true;
	}
	gravity_ = 0.1;

	InitOGL();
}

void CTRLMainState::RegisterEventListeners(void) {
	GetControllers();
	controller_mpi_.RegisterEventListener(this);
	// FIXME
	//LF::Hardware::HWController *controller = controller_mpi_.GetControllerByID(LF::Hardware::kHWDefaultControllerID);
	/*if (controller)
		controller->SetAnalogStickMode(LF::Hardware::kHWAnalogStickModeDPad);
*/

}

void CTRLMainState::GetControllers(void)
{
	std::vector< LF::Hardware::HWController * > controllers;
	controller_mpi_.GetAllControllers(controllers);
	numControllers = controller_mpi_.GetNumberOfConnectedControllers();
	printf(" we have %d controllers connected\n", numControllers);
	printf("size: %d\n", controllers.size());
}


void CTRLMainState::UnregisterEventListeners(void) {
	controller_mpi_.UnregisterEventListener(this);
}
