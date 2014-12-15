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
#include "boost/shared_ptr.hpp"
#include <StringTypes.h>

#include "EgApp.h"

//SAMPLE CODE
#include <BlitBuffer.h>
#include <BlitButton.h>
#include <FontMPI.h>
#include <CurricularTools.h>
//

using namespace LeapFrog::Brio;
using namespace LTM;

class MainState : public CGameStateBase
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
	LeapFrog::Brio::CDebugMPI			debugMPI_;
	int DoTouchEventLoop();
	int DoKeyEventLoop();

	void buttonSelected(U16 item);

	static MainState *				mState;

	LeapFrog::Brio::U16 				currentSelection_;

	LeapFrog::Brio::CTouchEventQueue 	m_touch_event_queue;
	LeapFrog::Brio::CButtonEventQueue 	m_button_event_queue;

	//SAMPLE CODE
	void initSample();
	void drawSample();

	void launchMathMF();
	void launchMathG();

	void drawBlit(BlitButton &thisBuffer, bool bHilite = false);
	void checkButtonPress(int x, int y);
	void initDisplay();
	void clearDisplay();
	void initButtons();

	boost::shared_ptr<LTM::CJSonFile>	mJSONfile;

	LeapFrog::Brio::CDisplayMPI 		dispmgr;
	int									screenWidth, screenHeight;

	CBlitBuffer 						mScreenBuffer[2];
	LeapFrog::Brio::tDisplayHandle		displayHandle[2];
	int									iCurrentBuffer;
	int									abw,abx,aby,abz;
	int									bw,bx,by,bz;

	LeapFrog::Brio::CFontMPI			myTextFont;
	CString 							myFontPath;
	tFontHndl 							myFontHandle;

	CString								myAppPath;

	bool								isPressed;

	CString 							_MATH_MF_ArtPath;

	CString 							_MATH_MF_ButtonText;

	BlitButton 							_MATH_MF;

	CString 							_MATH_G_ArtPath;

	CString 							_MATH_G_ButtonText;

	BlitButton 							_MATH_G;


	CCurricularTools tutorialManager;


	BlitButton							_exit;
	CString								_exitButtonText;

	//
};



#endif // MainState_H_

