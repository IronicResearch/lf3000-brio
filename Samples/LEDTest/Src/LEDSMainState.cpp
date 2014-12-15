//Emerald Sample Project
#include <GLES/gl.h>
#ifndef KHRONOS
#include <GLES/egl.h>
#endif

#include "LEDSMainState.h"
#include "LEDTestApp.h"

#include <AppManager.h>
#include <LogFile.h>

#include <BlitBuffer.h>
#include <Hardware/HWControllerTypes.h>
#include <Hardware/HWController.h>
#include <Hardware/HWControllerEventMessage.h>

using namespace LeapFrog::Brio;

using LeapFrog::Brio::S16;
const LeapFrog::Brio::tEventType 
kLEDSListenerTypes[] = { LF::Hardware::kHWControllerConnected,
			 LF::Hardware::kHWControllerDisconnected,
			 LF::Hardware::kHWControllerButtonStateChanged };

LEDSMainState::LEDSMainState() :
  LeapFrog::Brio::IEventListener(kLEDSListenerTypes, ArrayCount(kLEDSListenerTypes)),
  color_(0) {
}

LEDSMainState::~LEDSMainState() {
}

LeapFrog::Brio::tEventStatus 
LEDSMainState::Notify( const LeapFrog::Brio::IEventMessage &msgIn ) {
  const LF::Hardware::HWControllerEventMessage& msg = reinterpret_cast<const LF::Hardware::HWControllerEventMessage&>(msgIn);
  const LF::Hardware::HWController *controller = msg.GetController();
  if (msgIn.GetEventType() == LF::Hardware::kHWControllerConnected) {
    printf("a controller connected\n");
    return LeapFrog::Brio::kEventStatusOKConsumed;
  } else if (msgIn.GetEventType() == LF::Hardware::kHWControllerButtonStateChanged) {
    LeapFrog::Brio::tButtonData2 button_data = controller->GetButtonData();
    static LeapFrog::Brio::U32 button_state = 0;
    static LeapFrog::Brio::U32 button_transition = 0;
    
    button_state = button_data.buttonState;
    button_transition = button_data.buttonTransition;
    
    //printf("button_state, button_transition: %d, %d\n", button_state, button_transition);
    
    if (button_transition == button_state) {
      if ((button_state & LeapFrog::Brio::kButtonA)) {
	printf("A button pressed\n");
	color_++;
	if (color_ > 5)
	  color_ = 0;

	LF::Hardware::HWController* controller = controllerMPI_.GetControllerByID();
	printf("controller = %p\n", controller);
	U8 colors = controller->GetAvailableLEDColors();
	printf("%s: controller %p = %d, LED caps=%08x\n", __func__, controller, controller->GetID(), (unsigned int)colors);
	
	LF::Hardware::HWControllerRGBLEDColor color = U8(1<<color_);
	if ((color & colors)) {
	  controller->SetLEDColor(color);
	}
	
      } else if ((button_state & LeapFrog::Brio::kButtonB)) {
	printf("B button pressed\n");
	LF::Hardware::HWController* controller = controllerMPI_.GetControllerByID();
	printf("controller = %p\n", controller);
	if (controller) {
	  if (controller->GetLEDColor() == LF::Hardware::kHWControllerLEDOff) {
	    U8 colors = controller->GetAvailableLEDColors();
	    LF::Hardware::HWControllerRGBLEDColor color = U8(1<<color_);
	    if (color & colors) {
	      controller->SetLEDColor(color);
	    }
	  } else
	    controller->SetLEDColor(LF::Hardware::kHWControllerLEDOff);
	}
      } else if ((button_state & LeapFrog::Brio::kButtonSync)) {
	CAppManager::Instance()->PopApp();
      }
    }
    return LeapFrog::Brio::kEventStatusOKConsumed;

  } else if (msgIn.GetEventType() == LF::Hardware::kHWControllerDisconnected) {
    
    printf( "a controller was dicsonnected\n");
    return LeapFrog::Brio::kEventStatusOKConsumed;
  }
  return LeapFrog::Brio::kEventStatusOK;
}

void LEDSMainState::Enter(CStateData* userData)
{
	//register touch screen
	LeapFrog::Brio::CEventMPI event_mpi;
	event_mpi.RegisterEventListener(&touch_event_queue_);

	//register keys
	LeapFrog::Brio::CButtonMPI key_event_mpi;
	key_event_mpi.RegisterEventListener(&button_event_queue_);

	controllerMPI_.RegisterEventListener(this);

	//SAMPLE CODE
	initSample();
	//
}

void LEDSMainState::Exit()
{
	cleanDisplay();

	//clean touch screen listener
	LeapFrog::Brio::CEventMPI event_mpi;
	event_mpi.UnregisterEventListener(&touch_event_queue_);

	//clean keys
	LeapFrog::Brio::CButtonMPI key_event_mpi;
	key_event_mpi.UnregisterEventListener(&button_event_queue_);
}

void LEDSMainState::Suspend()
{
	cleanDisplay();

	//clean touch screen listener
	LeapFrog::Brio::CEventMPI event_mpi;
	event_mpi.UnregisterEventListener(&touch_event_queue_);

	//clean keys
	LeapFrog::Brio::CButtonMPI key_event_mpi;
	key_event_mpi.UnregisterEventListener(&button_event_queue_);
}

void LEDSMainState::Resume()
{
	initDisplay();

	//register touch screen
	LeapFrog::Brio::CEventMPI event_mpi;
	event_mpi.RegisterEventListener(&touch_event_queue_);

	//register keys
	LeapFrog::Brio::CButtonMPI key_event_mpi;
	key_event_mpi.RegisterEventListener(&button_event_queue_);
}

void LEDSMainState::Update(CGameStateHandler* sh)
{
  //check for screen touch
  DoTouchEventLoop();
  
  //check for buttons
  DoKeyEventLoop();
  
  //SAMPLE CODE
  drawSample();
}

/*
 * Sample input checks
 */
int LEDSMainState::DoKeyEventLoop()
{
	std::vector<LeapFrog::Brio::tButtonData2> *button_event_queue = button_event_queue_.GetQueue();
	std::vector<LeapFrog::Brio::tButtonData2>::const_iterator button_reader = button_event_queue->begin();

	while(button_reader != button_event_queue->end())
	{
		static U32 button_state = 0;
		LeapFrog::Brio::tButtonData2 button_data = *(button_reader++);
		button_state = button_data.buttonState;

		/*
		 * button states:
		 * kButtonMenu, kButtonHint
		 * kButtonLeft, kButtonRight, kButtonUp, kButtonDown,
		 * kButtonLeftShoulder, kButtonRightShoulder,
		 * kButtonA, kButtonB
		 */

		if (button_state & (kButtonMenu | kButtonEscape | kButtonSync))
		{
			CAppManager::Instance()->PopApp();
			return 1;
		}
	}
	return 0;
}

int LEDSMainState::DoTouchEventLoop()
{
	std::vector<LeapFrog::Brio::tTouchData> *touch_event_queue = touch_event_queue_.GetQueue();

	std::vector<LeapFrog::Brio::tTouchData>::const_iterator touch_reader = touch_event_queue->begin();

	while(touch_reader != touch_event_queue->end())
	{
		static U16 touch_state = 0;
		LeapFrog::Brio::tTouchData touch_data = *(touch_reader++);

		//get touch_state:
		//0:	up
		//1:	down
		touch_state = touch_data.touchState;

		//check for button press
		int pen_x = touch_data.touchX;
		int pen_y = touch_data.touchY;	//need to reverse the values
	}
	return 0;
}

/*
 * sample "Hello World!" code
 */
void LEDSMainState::initDisplay()
{
	//init Display
	current_buffer_ = 0;
	display_handle_[0] = display_manager_.CreateHandle(screen_height_, screen_width_, kPixelFormatARGB8888, NULL);
	display_handle_[1] = display_manager_.CreateHandle(screen_height_, screen_width_, kPixelFormatARGB8888, NULL);

	if (kNoErr != display_manager_.Register(display_handle_[0], 0, 0, kDisplayOnTop, 0))
	{
		DidjAssert(0, "UI 0 Display manager registration failed.");
	}
	if (kNoErr != display_manager_.Register(display_handle_[1], 0, 0, kDisplayOnTop, 0))
	{
		DidjAssert(0, "UI 1 Display manager registration failed.");
	}

	screen_buffer_[0].setFromDisplayHandle(display_handle_[0]);
	screen_buffer_[1].setFromDisplayHandle(display_handle_[1]);

	tRect rect = {0, screen_width_, 0, screen_height_};
	screen_buffer_[0].blitRect(rect, 0, 0, 0, 0, false);
	screen_buffer_[1].blitRect(rect, 0, 0, 0, 0, false);
}

void LEDSMainState::cleanDisplay()
{
	display_manager_.DestroyHandle(display_handle_[0], true);
	display_manager_.DestroyHandle(display_handle_[1], true);
}

void LEDSMainState::initSample()
{
	//Sample Code
	//get device stats
	const LeapFrog::Brio::tDisplayScreenStats* myStats;
	myStats = display_manager_.GetScreenStats(0);
	screen_width_ = myStats->width;
	screen_height_ = myStats->height;
	initDisplay();
}

void LEDSMainState::drawSample()
{
	//To launch a new state, call:
	//LEDTestApp::RequestNewState(new MyNewState());

	//sample code
	if(++current_buffer_ > 1) current_buffer_ = 0;

	//clean the screen
	tRect rect = {0, screen_width_, 0, screen_height_};
	screen_buffer_[current_buffer_].blitRect(rect, 0, 0, 0, 0, false);

	//draw it
	display_manager_.SwapBuffers(display_handle_[current_buffer_], true);
}

//

