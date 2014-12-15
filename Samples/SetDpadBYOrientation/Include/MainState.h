#ifndef MainState_H_
#define MainState_H_

#include <DebugMPI.h>
#include <GameStateHandler.h>
#include <GameStateBase.h>
#include <DisplayMPI.h>
#include <ButtonMPI.h>
#include <EventMPI.h>
#include <TouchEventQueue.h>
#include <ButtonEventQueue.h>
//#include "boost/shared_ptr.hpp"
#include <StringTypes.h>
#include <math.h>
#include <CoreTypes.h>
#include <AccelerometerMPI.h>

#include "EgApp.h"

//SAMPLE CODE
#include <BlitBuffer.h>

//
using namespace std;
using namespace LeapFrog::Brio;
using namespace LTM;

class MainState : public CGameStateBase, public IEventListener
{
public:
	MainState();
	~MainState();

	virtual void Suspend();
	virtual void Resume();
	virtual void Enter(CStateData* userData);
	virtual void Update(CGameStateHandler* sh);
	virtual void Exit();

protected:

	virtual tEventStatus Notify( const IEventMessage &msgIn );

	LeapFrog::Brio::CDebugMPI			debugMPI_;
	int DoTouchEventLoop();
	int DoKeyEventLoop();

	//friend class EgAudioListener;
	void buttonSelected(U16 item);

	static MainState *				mState;

	LeapFrog::Brio::U16 				currentSelection_;

	LeapFrog::Brio::CTouchEventQueue 	m_touch_event_queue;
	LeapFrog::Brio::CButtonEventQueue 	m_button_event_queue;

	//SAMPLE CODE
	void initSample();
	void drawSample();

	bool rotate(CBlitBuffer *newImg, CBlitBuffer *srcImg, int angleDeg, int numColors=4);
	S32 checkOrientation();
	void calcRotateSize(int& newWidth, int& newHeight, int srcWidth, int srcHeight, int angle);
	void checkMyOrientation();
	void rotateScreen(int angle);

	void initBlit(CPath pngPath);
	void drawBlit(CBlitBuffer &thisBuffer);
	void initDisplay();
	void clearDisplay();

	LeapFrog::Brio::CDisplayMPI 		dispmgr;
	int									screenWidth, screenHeight;

	LeapFrog::Brio::CAccelerometerMPI*	pAccMPI_;
	int	screenOrientation;

	CBlitBuffer 						mScreenBuffer[2];
	LeapFrog::Brio::tDisplayHandle		displayHandle[2];
	int									iCurrentBuffer;

	CPath								myAppPath;
	CPath								myArtPath;
	CPath 								pngPath;


	CBlitBuffer							_DPAD;
	CBlitBuffer							_DPAD_Display;

    LeapFrog::Brio::CButtonMPI          mpi_Button_;

    tDpadOrientation					dpad_orientation_;
    tErrType 							orientation_changed_;

	//
};



#endif // MainState_H_

