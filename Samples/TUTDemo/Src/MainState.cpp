//Emerald Sample Project
#include "MainState.h"
#include "EgApp.h"

#include <AppManager.h>
#include <CSystemData.h>
#include <BaseUtils.h>
#include <Utility.h>
#include <LogFile.h>
#include <iostream>
#include <string>
#include <LTM.h>

//SAMPLE CODE
#include <BlitBuffer.h>
//

using namespace LeapFrog::Brio;
using namespace LTM;


MainState* MainState::mState = NULL;

MainState::MainState() : currentSelection_(0), debugMPI_(kFirstCartridge1DebugSig)
{
}

MainState::~MainState()
{
}

void MainState::Enter(CStateData* userData)
{
	debugMPI_.SetDebugLevel(DEBUG_LEVEL);
	debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] Main state STARTED\n\n");

	CPath myPath = CSystemData::Instance()->GetBaseTutorialPath();
	debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] Legacy Tutorial Path: %s\n",myPath.c_str());


	//store path information
	myAppPath = CSystemData::Instance()->GetCurrentTitlePath();

	_exitButtonText="Exit";

	_MATH_MF_ArtPath=myAppPath+"images/";
	_MATH_MF_ButtonText="MATH MF";

	_MATH_G_ArtPath=myAppPath+"images/";
	_MATH_G_ButtonText="MATH G";

	//initiate my bool variables
	isPressed=false;

	//initiate my font
	myFontPath= CSystemData::Instance()->FindFont("LFCurry.ttf");
	myFontHandle = myTextFont.LoadFont(myFontPath, 16);
	myTextFont.SelectFont(myFontHandle);
	myTextFont.SetFontColor(0xFFFFFFFF);

	//get device stats
	const LeapFrog::Brio::tDisplayScreenStats* myStats;
	myStats = dispmgr.GetScreenStats(0);
	screenWidth = myStats->width;
	screenHeight = myStats->height;

	//register touch screen
	LeapFrog::Brio::CEventMPI event_mpi;
	event_mpi.RegisterEventListener(&m_touch_event_queue);

	//register keys
	LeapFrog::Brio::CButtonMPI key_event_mpi;
	key_event_mpi.RegisterEventListener(&m_button_event_queue);

	initSample();

}

void MainState::Exit()
{
	debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] Exiting\n");

	//clears out the display buffer
	clearDisplay();

	//clean touch screen listener
	LeapFrog::Brio::CEventMPI event_mpi;
	event_mpi.UnregisterEventListener(&m_touch_event_queue);

	//clean keys
	LeapFrog::Brio::CButtonMPI key_event_mpi;
	key_event_mpi.UnregisterEventListener(&m_button_event_queue);
}

void MainState::Suspend()
{

	debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] Suspend\n");

	//clearDisplay();
	//get last key pressed in queue
	DoKeyEventLoop();

	//clean touch screen listener
	LeapFrog::Brio::CEventMPI event_mpi;
	event_mpi.UnregisterEventListener(&m_touch_event_queue);

	//clean keys
	LeapFrog::Brio::CButtonMPI key_event_mpi;
	key_event_mpi.UnregisterEventListener(&m_button_event_queue);
}

void MainState::Resume()
{
	debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] Resume\n");

	isPressed=false;

	//redraw the display
	dispmgr.Invalidate(0, NULL);

	//Restore font
	if(myTextFont.SelectFont(myFontHandle))
		debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] Switching font is successful...\n");
	else
		debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] Switching font was not successful...\n");

	//register touch screen
	LeapFrog::Brio::CEventMPI event_mpi;
	event_mpi.RegisterEventListener(&m_touch_event_queue);

	//register keys
	LeapFrog::Brio::CButtonMPI key_event_mpi;
	key_event_mpi.RegisterEventListener(&m_button_event_queue);
}

void MainState::Update(CGameStateHandler* sh)
{
	debugMPI_.DebugOut(kDbgLvlVerbose, "[MainState] Update()\n");

	//check for screen touch
	DoTouchEventLoop();

	//check for buttons
	DoKeyEventLoop();

	//draw sample
	drawSample();
}

int MainState::DoKeyEventLoop()
{
	std::vector<LeapFrog::Brio::tButtonData2> *button_event_queue = m_button_event_queue.GetQueue();
	std::vector<LeapFrog::Brio::tButtonData2>::const_iterator button_reader = button_event_queue->begin();

	while(button_reader != button_event_queue->end())
	{
		static U16 button_state = 0;
		LeapFrog::Brio::tButtonData2 button_data = *(button_reader++);
		button_state = button_data.buttonState;
		debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] button: %d\n", button_state);

		/*
		 * button states:
		 * kButtonMenu, kButtonHint
		 * kButtonLeft, kButtonRight, kButtonUp, kButtonDown,
		 * kButtonLeftShoulder, kButtonRightShoulder,
		 * kButtonA, kButtonB, kButtonPause
		 */

		if (button_state & kButtonMenu)
		{
			debugMPI_.DebugOut(kDbgLvlNoteable, "\tHOME\n");
			CAppManager::Instance()->PopApp();
			return 1;
		}
		else if(button_state & kButtonPause)
		{
			debugMPI_.DebugOut(kDbgLvlNoteable, "\tPause\n");
			return 1;
		}
	}
	return 0;
}

int MainState::DoTouchEventLoop()
{
	std::vector<LeapFrog::Brio::tTouchData> *touch_event_queue = m_touch_event_queue.GetQueue();

	std::vector<LeapFrog::Brio::tTouchData>::const_iterator touch_reader = touch_event_queue->begin();

	static U16 touch_state = 0;
	int pen_x, pen_y;
	while(touch_reader != touch_event_queue->end())
	{
		LeapFrog::Brio::tTouchData touch_data = *(touch_reader++);

		//get touch_state:
		//0:	up
		//1:	down
		touch_state = touch_data.touchState;

		//check for button press
		pen_x = touch_data.touchX;
		pen_y = touch_data.touchY;	//need to reverse the values

	}

	if(touch_state==1)
	{
		//will check to see of a button on screen is pressed
		checkButtonPress(pen_x, pen_y);
	}
	else
	{
		isPressed=false;
	}
	return 0;
}

void MainState::initSample()
{
	//initiates the widget experience
	debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] INITIALIZE\n\n");

	//initialize the display
	initDisplay();

	//initialize the blitbutton
	initButtons();

	_exit.init(myAppPath + "images/sbutton.png", myAppPath + "images/sbutton_hi.png", _exitButtonText);

	int gap_x;
	int gap_y;

	gap_x = (screenWidth-_MATH_MF.main.surface.width)/2;
	gap_y = (screenHeight-2*_MATH_MF.main.surface.height)/3;

	_MATH_MF.x = gap_x;
	_MATH_MF.y = gap_y;

	_MATH_G.x = gap_x;
	_MATH_G.y = _MATH_MF.main.surface.height + gap_y;

	_exit.x = 10;
	_exit.y = screenHeight-_exit.main.surface.height;

}

void MainState::initButtons()
{
	//debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] INITIALIZING BUTTONS\n\n");

	_MATH_MF.init(_MATH_MF_ArtPath + "UpState.png", _MATH_MF_ArtPath + "DownState.png", _MATH_MF_ButtonText);
	_MATH_G.init(_MATH_G_ArtPath + "UpState.png", _MATH_G_ArtPath + "DownState.png", _MATH_G_ButtonText);
}

void MainState::drawSample()
{
	//Draws elements to screen

	//sample code
	if(++iCurrentBuffer > 1) iCurrentBuffer = 0;

	//clean the screen
	tRect rect = {0, screenWidth, 0, screenHeight};
	mScreenBuffer[iCurrentBuffer].blitRect(rect, 0, 0, 0, 0, false);

	if(myTextFont.SelectFont(myFontHandle))
	{
		//debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] Switching font is successful...\n");
	}
	else
	{
		//debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] Switching font was not successful...\n");
	}

	drawBlit(_MATH_MF, _MATH_MF.bSelected);
	drawBlit(_MATH_G, _MATH_G.bSelected);
	drawBlit(_exit, _exit.bSelected);

	//draw it
	dispmgr.SwapBuffers(displayHandle[iCurrentBuffer], true);
}


void MainState::launchMathMF()
{
	//FUnction to call the tutorial using CCUrricularTools

	printf("\nlaunchMathMF\n");

	//Create virtual json with proper tutorial variables
	mJSONfile.reset(new LTM::CJSonFile());
//	mJSONfile.reset(new LTM::CJSonFile(""));
//	LTM::Value temp(LTM::objectValue);
//	mJSONfile->mRoot=temp;
	mJSONfile->mRoot["TutParams"]["TutorialID"] = "MF.1.1";
	mJSONfile->mRoot["TutParams"]["num0"] = 1;
	mJSONfile->mRoot["TutParams"]["num1"] = 2;
	mJSONfile->mRoot["LegacyTutParams"] = "tutorialId=MF.1.1&operand1=1&operand2=2&result=3";

	//calls the LaunchTutorial() function in CCurricularTools
	tutorialManager.LaunchTutorial(mJSONfile);

}

void MainState::launchMathG()
{
	//FUnction to call the tutorial using CCUrricularTools

	printf("\nlaunchMathG\n");

	//Create virtual json with proper tutorial variables
	mJSONfile.reset(new LTM::CJSonFile());
	mJSONfile->mRoot["TutParams"]["TutorialID"] = "G.1.1";
	mJSONfile->mRoot["LegacyTutParams"] = "tutorialId=G.1.1.2";

	//calls the LaunchTutorial() function in CCurricularTools
	tutorialManager.LaunchTutorial(mJSONfile);

}

void MainState::drawBlit(BlitButton &thisBuffer, bool bHilite)
{
	//draws my buttons
	CBlitBuffer buff;

	if(bHilite)
	{
		buff = thisBuffer.hilite;
	}
	else
	{
		buff = thisBuffer.main;
	}

	LeapFrog::Brio::tRect myRect;
	myRect.top = 0;
	myRect.bottom = buff.surface.height;
	myRect.left = 0;
	myRect.right = buff.surface.width;

	mScreenBuffer[iCurrentBuffer].blitFromBuffer(buff, myRect, thisBuffer.x, thisBuffer.y, true);

	tRect mybox;
	myTextFont.GetStringRect( &thisBuffer.myString, &mybox);
	abw = mybox.left;
	abx = mybox.right;
	aby = mybox.top;
	abz = mybox.bottom;
	myTextFont.DrawString(&thisBuffer.myString, thisBuffer.x+((myRect.right-abx)/2), thisBuffer.y+((myRect.bottom-abz)/2), &mScreenBuffer[iCurrentBuffer].surface);
}

void MainState::checkButtonPress(int x, int y)
{
	//checks to see if my button is pressed

	if(_MATH_MF.hitTest(x,y))
	{
		debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] MATH MF button....\n");
		launchMathMF();
		isPressed=true;
	}
	else if(_MATH_G.hitTest(x,y))
	{
		debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] MATH G button....\n");
		launchMathG();
		isPressed=true;
	}
	else if(_exit.hitTest(x,y))
	{
		debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] exiting....\n");
		CAppManager::Instance()->PopApp();
		isPressed=true;
	}

}

void MainState::initDisplay()
{
	//init Display
	debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] INITIALIZING DISPLAY\n\n");
	iCurrentBuffer = 0;
	displayHandle[0] = dispmgr.CreateHandle(screenHeight, screenWidth, kPixelFormatARGB8888, NULL);
	displayHandle[1] = dispmgr.CreateHandle(screenHeight, screenWidth, kPixelFormatARGB8888, NULL);

	if (kNoErr != dispmgr.Register(displayHandle[0], 0, 0, kDisplayOnTop, 0))
	{
		DidjAssert(0, "UI 0 Display manager registration failed.");
	}
	if (kNoErr != dispmgr.Register(displayHandle[1], 0, 0, kDisplayOnTop, 0))
	{
		DidjAssert(0, "UI 1 Display manager registration failed.");
	}

	mScreenBuffer[0].setFromDisplayHandle(displayHandle[0]);
	mScreenBuffer[1].setFromDisplayHandle(displayHandle[1]);

	tRect rect = {0, screenWidth, 0, screenHeight};
	mScreenBuffer[0].blitRect(rect, 0, 0, 0, 0, false);
	mScreenBuffer[1].blitRect(rect, 0, 0, 0, 0, false);
}

void MainState::clearDisplay()
{
	//cleans the display
	debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] CLEANING DISPLAY\n\n");
	dispmgr.DestroyHandle(displayHandle[0], true);
	dispmgr.DestroyHandle(displayHandle[1], true);
}
