//Emerald Sample Project
#include <GLES/gl.h>
#ifndef KHRONOS
#include <GLES/egl.h>
#endif

#include "STUBMainState.h"
#include "STUBApp.h"

#include <AppManager.h>
#include <LogFile.h>

//SAMPLE CODE
#include <BlitBuffer.h>
//

using namespace LeapFrog::Brio;

STUBMainState* STUBMainState::state_ = NULL;

STUBMainState::STUBMainState() : current_selection_(0), debug_mpi_(kFirstCartridge1DebugSig)
{
}

STUBMainState::~STUBMainState()
{
}

void STUBMainState::Enter(CStateData* userData)
{
	debug_mpi_.SetDebugLevel(DEBUG_LEVEL);
	debug_mpi_.DebugOut(kDbgLvlNoteable, "[STUBMainState] Main state STARTED\n\n");

	//register touch screen
	LeapFrog::Brio::CEventMPI event_mpi;
	event_mpi.RegisterEventListener(&touch_event_queue_);

	//register keys
	LeapFrog::Brio::CButtonMPI key_event_mpi;
	key_event_mpi.RegisterEventListener(&button_event_queue_);

	//SAMPLE CODE
	initSample();
	//
}

void STUBMainState::Exit()
{
	debug_mpi_.DebugOut(kDbgLvlNoteable, "[STUBMainState] Exiting\n");

	cleanDisplay();

	//clean touch screen listener
	LeapFrog::Brio::CEventMPI event_mpi;
	event_mpi.UnregisterEventListener(&touch_event_queue_);

	//clean keys
	LeapFrog::Brio::CButtonMPI key_event_mpi;
	key_event_mpi.UnregisterEventListener(&button_event_queue_);
}

void STUBMainState::Suspend()
{
	debug_mpi_.DebugOut(kDbgLvlNoteable, "[STUBMainState] Suspend\n");

	cleanDisplay();

	//clean touch screen listener
	LeapFrog::Brio::CEventMPI event_mpi;
	event_mpi.UnregisterEventListener(&touch_event_queue_);

	//clean keys
	LeapFrog::Brio::CButtonMPI key_event_mpi;
	key_event_mpi.UnregisterEventListener(&button_event_queue_);
}

void STUBMainState::Resume()
{
	debug_mpi_.DebugOut(kDbgLvlNoteable, "[STUBMainState] Resume\n");

	initDisplay();

	//register touch screen
	LeapFrog::Brio::CEventMPI event_mpi;
	event_mpi.RegisterEventListener(&touch_event_queue_);

	//register keys
	LeapFrog::Brio::CButtonMPI key_event_mpi;
	key_event_mpi.RegisterEventListener(&button_event_queue_);
}

void STUBMainState::Update(CGameStateHandler* sh)
{
	debug_mpi_.DebugOut(kDbgLvlVerbose, "[STUBMainState] Update()\n");

	//check for screen touch
	DoTouchEventLoop();

	//check for buttons
	DoKeyEventLoop();

	//SAMPLE CODE
	drawSample();
	//
}

/*
 * Sample input checks
 */
int STUBMainState::DoKeyEventLoop()
{
	std::vector<LeapFrog::Brio::tButtonData2> *button_event_queue = button_event_queue_.GetQueue();
	std::vector<LeapFrog::Brio::tButtonData2>::const_iterator button_reader = button_event_queue->begin();

	while(button_reader != button_event_queue->end())
	{
		static U32 button_state = 0;
		LeapFrog::Brio::tButtonData2 button_data = *(button_reader++);
		button_state = button_data.buttonState;
		debug_mpi_.DebugOut(kDbgLvlNoteable, "[STUBMainState] button: %08x\n", (unsigned int)button_state);

		/*
		 * button states:
		 * kButtonMenu, kButtonHint
		 * kButtonLeft, kButtonRight, kButtonUp, kButtonDown,
		 * kButtonLeftShoulder, kButtonRightShoulder,
		 * kButtonA, kButtonB
		 */

		if (button_state & (kButtonMenu | kButtonEscape | kButtonSync))
		{
			debug_mpi_.DebugOut(kDbgLvlNoteable, "\tHOME\n");
			CAppManager::Instance()->PopApp();
			return 1;
		}
	}
	return 0;
}

int STUBMainState::DoTouchEventLoop()
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
void STUBMainState::initDisplay()
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

void STUBMainState::cleanDisplay()
{
	display_manager_.DestroyHandle(display_handle_[0], true);
	display_manager_.DestroyHandle(display_handle_[1], true);
}

void STUBMainState::initSample()
{
	//Sample Code
	//get device stats
	const LeapFrog::Brio::tDisplayScreenStats* myStats;
	myStats = display_manager_.GetScreenStats(0);
	screen_width_ = myStats->width;
	screen_height_ = myStats->height;
	initDisplay();

	//init text
	my_text_ = "Hello World!";

	CPath myFontPath(CSystemData::Instance()->FindFont("LFCurry.ttf"));
	tFontHndl fontHandle = my_text_font_.LoadFont(myFontPath, 16);
	my_text_font_.SelectFont(fontHandle);
	my_text_font_.SetFontColor(0xFFFFFF);

}

void STUBMainState::drawSample()
{
	//To launch a new state, call:
	//STUBApp::RequestNewState(new MyNewState());

	//sample code
	if(++current_buffer_ > 1) current_buffer_ = 0;

	//clean the screen
	tRect rect = {0, screen_width_, 0, screen_height_};
	screen_buffer_[current_buffer_].blitRect(rect, 0, 0, 0, 0, false);

	//set text
	my_text_font_.DrawString(&my_text_, 0, 0, &screen_buffer_[current_buffer_].surface);

	//draw it
	display_manager_.SwapBuffers(display_handle_[current_buffer_], true);
}

//

