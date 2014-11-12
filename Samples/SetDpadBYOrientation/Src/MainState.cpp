//Emerald Sample Project
#include "MainState.h"
#include "EgApp.h"

#include <AppManager.h>
#include <CSystemData.h>

//SAMPLE CODE
#include <BlitBuffer.h>

using namespace std;
using namespace LeapFrog::Brio;
using namespace LTM;


MainState* MainState::mState = NULL;

const tEventType kACCListenerTypes[] = { kAllAccelerometerEvents };

MainState::MainState() : currentSelection_(0), debugMPI_(kFirstCartridge1DebugSig), IEventListener(kACCListenerTypes, ArrayCount(kACCListenerTypes))
{
	//acc
	pAccMPI_ = new LeapFrog::Brio::CAccelerometerMPI;
	pAccMPI_->SetAccelerometerMode(kAccelerometerModeOrientation);
	pAccMPI_->RegisterEventListener(this);
	screenOrientation = 0;

	//checks to see default device orientation
	dpad_orientation_ = mpi_Button_.GetDpadOrientation();

	//if orientation is not landscape, we set the orientation to landscape
	if(dpad_orientation_ != kDpadLandscape)
	{
		orientation_changed_ = mpi_Button_.SetDpadOrientation(kDpadLandscape);
	}
}

MainState::~MainState()
{
	delete pAccMPI_;
}

void MainState::Enter(CStateData* userData)
{
	debugMPI_.SetDebugLevel(DEBUG_LEVEL);
	debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] Main state STARTED\n\n");

	myAppPath = CSystemData::Instance()->GetCurrentTitlePath();
	myArtPath = myAppPath + "images/";
	pngPath = myArtPath + "DPAD.png";

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

	//Sets up screen buffers
	initSample();

	//initializes the DPAD image
	initBlit(pngPath);

	// Checks device orientation
	checkMyOrientation();

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

	//redraw the display
	dispmgr.Invalidate(0, NULL);

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

	//Draws the sample
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
		//Prints out whant dpad button is pressed
		else if (button_state & kButtonUp)
		{
			debugMPI_.DebugOut(kDbgLvlNoteable, "\tDPAD UP\n");
		}
		else if (button_state & kButtonDown)
		{
			debugMPI_.DebugOut(kDbgLvlNoteable, "\tDPAD DOWN\n");
		}
		else if (button_state & kButtonLeft)
		{
			debugMPI_.DebugOut(kDbgLvlNoteable, "\tDPAD LEFT\n");
		}
		else if (button_state & kButtonRight)
		{
			debugMPI_.DebugOut(kDbgLvlNoteable, "\tDPAD RIGHT\n");
		}
		//
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

	return 0;
}

void MainState::initSample()
{
	//initiates the display
	debugMPI_.DebugOut(kDbgLvlNoteable, "[MainState] INITIALIZING \n\n");

	initDisplay();
}

void MainState::drawSample()
{
	//sample code
	if(++iCurrentBuffer > 1) iCurrentBuffer = 0;

	//clean the screen
	tRect rect = {0, screenWidth, 0, screenHeight};
	mScreenBuffer[iCurrentBuffer].blitRect(rect, 255, 255, 255, 0, false);

	//draw blit to screen buffer
	drawBlit(_DPAD_Display);

	//draw it
	dispmgr.SwapBuffers(displayHandle[iCurrentBuffer], true);
}

void MainState::drawBlit(CBlitBuffer &thisBuffer)
{
	//setup image buffer
	LeapFrog::Brio::tRect myRect;
	myRect.top = 0;
	myRect.bottom = thisBuffer.surface.height;
	myRect.left = 0;
	myRect.right = thisBuffer.surface.width;

	S32 xpos = (screenWidth-thisBuffer.surface.width)/2;
	S32 ypos = (screenHeight-thisBuffer.surface.height)/2;

	mScreenBuffer[iCurrentBuffer].blitFromBuffer(thisBuffer, myRect, xpos, ypos, true);

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

S32 MainState::checkOrientation()
{
	//returns the orientation of the device
	return pAccMPI_->GetOrientation();
}

bool MainState::rotate(CBlitBuffer *newImg, CBlitBuffer *srcImg, int angleDeg, int numColors)
{
	//rotates source buffer to correct orientation

	//Make sure our source image exists.
	if(srcImg == NULL || newImg == NULL)
	{
		debugMPI_.DebugOut(kDbgLvlNoteable, "ERROR :: Tried to rotate a non-existent image!\n");
		return false;
	}

	//Setup width, height, and RGBA info.
	int srcWidth 	= srcImg->surface.width;
	int srcHeight 	= srcImg->surface.height;
	int newWidth 	= 0;
	int newHeight 	= 0;

	calcRotateSize(newWidth,newHeight,srcWidth,srcHeight,angleDeg);

	float angle = (2*(M_PI)*angleDeg)/360;

	int mCosine		=(float)cos(angle);
	int mSine		=(float)sin(angle);

	int Point1x = (-srcHeight * mSine);
	int Point1y = (srcHeight  * mCosine);
	int Point2x = ((srcWidth  * mCosine) - (srcHeight * mSine));
	int Point2y = ((srcHeight * mCosine) + (srcWidth  * mSine));
	int Point3x = (srcWidth * mCosine);
	int Point3y = (srcWidth * mSine);

	int mMinX 		= min((int)0,min(Point1x,min(Point2x,Point3x)));
	int mMinY 		= min((int)0,min(Point1y,min(Point2y,Point3y)));
	int mMaxX 		= max(Point1x,max(Point2x,Point3x));
	int mMaxY 		= max(Point1y,max(Point2y,Point3y));

	//Setup our buffers.
	newImg->createNew(newWidth, newHeight);
	tRect myRect;
	myRect.left = 0;
	myRect.right = newWidth;
	myRect.top = 0;
	myRect.bottom = newHeight;
	newImg->blitRect(myRect, 0, 0, 0, 0, false);


	U8* srcBuffer 	= srcImg->surface.buffer;
	U8* newBuffer 	= newImg->surface.buffer;

	int srcPitch 	= srcImg->surface.pitch;
	int newPitch 	= newWidth * numColors;

	for(int y=0; y<newHeight; y++)
	{
		for(int x=0; x < newWidth; x++)
		{
			int srcX 	= (int)((x+mMinX)*mCosine+(y+mMinY)*mSine);
			int srcY 	= (int)((y+mMinY)*mCosine-(x+mMinX)*mSine);
			int newPos 	= (y*newPitch)+(x*numColors);
			int srcPos 	= (srcY*srcPitch)+(srcX*numColors);

			for(int i=0; i < numColors; i++)
			{
				if(srcX >= 0 && srcX < srcWidth && srcY >= 0 && srcY < srcHeight)
				{
					newBuffer[newPos+i] = srcBuffer[srcPos+i];
				}
				else
				{
					newBuffer[newPos+i] = 0;
				}
			}

		}
	}

	return true;
}

void MainState::calcRotateSize(int& newWidth, int& newHeight, int srcWidth, int srcHeight, int angleDeg)
{
	//calcualtes new dimensions based on orientation
	switch(angleDeg)
	{
	case 0:
	case 180:
		newWidth = srcWidth;
		newHeight = srcHeight;
		break;
	case 90:
	case 270:
		newWidth = srcHeight;
		newHeight = srcWidth;
		break;
	}
}

void MainState::checkMyOrientation()
{
	//Get current orientation
	int orientation = checkOrientation();
	debugMPI_.DebugOut(kDbgLvlNoteable, "checkOrientation: orientation: %d, screenOrientation: %d\n",orientation,screenOrientation);

	if(screenOrientation != orientation)
	{
		screenOrientation = orientation;

		//enumerations of possible orientations
		//kDpadLandscape
		//kDpadPortrait
		//kDpadLandscapeUpsideDown
		//kDpadPortraitUpsideDown

		//Set DPAD orietation based on screen orientation
		switch(screenOrientation)
		{
			case 1:
				orientation_changed_ = mpi_Button_.SetDpadOrientation(kDpadPortrait);
				break;
			case 2:
				orientation_changed_ = mpi_Button_.SetDpadOrientation(kDpadLandscapeUpsideDown);
				break;
			case 3:
				orientation_changed_ = mpi_Button_.SetDpadOrientation(kDpadPortraitUpsideDown);
				break;
			case 0:
				orientation_changed_ = mpi_Button_.SetDpadOrientation(kDpadLandscape);
				break;
		}
		rotateScreen(screenOrientation*90);
	}
}

void MainState::rotateScreen(int angle)
{

	rotate(&_DPAD_Display, &_DPAD, angle);
}

void MainState::initBlit(CPath pngPath)
{
	_DPAD.surface.buffer = 0;
	_DPAD_Display.surface.buffer = 0;
	_DPAD.createFromPng(pngPath);
	_DPAD_Display.createFromPng(pngPath);
}

tEventStatus MainState::Notify( const IEventMessage &msgIn )
{
	if (msgIn.GetEventType() == kOrientationChanged)
	{
		const CAccelerometerMessage& aclmsg = reinterpret_cast<const CAccelerometerMessage&>(msgIn);

		//check the device orientation
		checkMyOrientation();

		return kEventStatusOKConsumed;
	}
	return kEventStatusOK;
}
