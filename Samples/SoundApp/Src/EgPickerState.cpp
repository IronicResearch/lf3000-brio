#include "EgPickerState.h"
#include "sound_test.h"
#include "EgApp.h"

#include <CSystemData.h>
#include <Fixed.h>
#include <AppManager.h>
#include <LogFile.h>
#include <CDrawManager.h>


LF_USING_BRIO_NAMESPACE()


CBillboard *background;

EgPickerState* EgPickerState::mState = NULL;

EgPickerState::EgPickerState() : currentSelection_(0)
{
	printf("\n%s: made it as far as the constructor",__FILE__);
}

/**********************************************************/


EgPickerState::~EgPickerState()
{
}

/**********************************************************/


void EgPickerState::Enter(CStateData* userData)
{

    background = new CBillboard();
    background->loadTexture(CSystemData::Instance()->GetCurrentTitlePath()+"images/background.png");

	old_id = cs.GetCurrentPlayerID();

	cp = new LTM::CPlayerProfile(old_id);

	// Log this event
	if (CAppManager::Instance()->pLogFile->IsProfileSet())
	{
		LogData::Lightning::CNavAreaStart navAreaStart("EgPicker");
		CAppManager::Instance()->pLogFile->Append(*(dynamic_cast<CLogData*>(&navAreaStart)));
	}
	printf("\n%s, %s(): Logging succeeded",__FILE__,__FUNCTION__);

	
	KeyboardManager::Instance()->clearButtonPressedMask(kButtonsAll);
	printf("\n%s, %s(): KeyboardManager::Instance()->clearButtonPressedMask(kButtonsAll) succeeded",__FILE__,__FUNCTION__);
	
	// put icons on the stage
	iconDataArray[0].imagePath = CSystemData::Instance()->GetCurrentTitlePath()+"images/menu-item.png";
	iconDataArray[0].textString = "      Quit";
	iconDataArray[0].pGameState = NULL;
	
	iconDataArray[1].imagePath = CSystemData::Instance()->GetCurrentTitlePath()+"images/menu-item.png";
	iconDataArray[1].textString = "     Sound Test";
	iconDataArray[1].pGameState = SOUND_TEST::Instance();

	
	CPath myFontPath(CSystemData::Instance()->FindFont("DidjPropBold.ttf"));


	U16 iIconCount = NUMBER_OF_ICONS;
	tFixed radius(46);
	tFixed theta(360.0/iIconCount);
	tFixed depth(1);
	for (U16 i=0; i<iIconCount; ++i)
	{
		tFixed dx(radius * cos(theta*i));
		tFixed dy(radius * sin(theta*i));
		//iconBillboardArray[i].reset(new CBillboard());
		iconBillboardArray[i].loadTexture(iconDataArray[i].imagePath);
		iconBillboardArray[i].moveRegTo(tVect2(16, 16));
		iconBillboardArray[i].moveTo(tVect2(0+dx, 120+dy));
		iconBillboardArray[i].setVisibility(true);
		//CDrawManager::Instance()->add(iconBillboardArray[i]);

		
		//iconTextArray[i].reset(new CText());
		iconTextArray[i].setText(iconDataArray[i].textString);
		iconTextArray[i].setFont(myFontPath);
		iconTextArray[i].setSize(16);
		iconTextArray[i].setColor(tColor(0, 128,  255, 255));
		iconTextArray[i].moveTo(tVect3(120+dx, 120+dy, depth));

		
		depth += 1;
	}
	
    background->moveTo(tVect3(0,0,11));

	bitz_display.reset(new CText());

	bitz_display->setFont(myFontPath);
	
	bitz	    = (int)cp->GetBitz();

}

/**********************************************************/

void EgPickerState::Exit()
{
	// Log the event
	if (CAppManager::Instance()->pLogFile->IsProfileSet())
	{
		LogData::Lightning::CNavAreaExit navAreaExit;
		CAppManager::Instance()->pLogFile->Append(*(dynamic_cast<CLogData*>(&navAreaExit)));
	}
	
	delete(background);
	
	U16 iIconCount = NUMBER_OF_ICONS;
	for (U16 i=0; i<iIconCount; ++i)
	{
	}
}

/**********************************************************/

void EgPickerState::Update(CGameStateHandler* sh)
{

	


	char buffer[1024];

	static int j;
	static int scroll_up = 0;
	static int scroll_down = 0;
	static int scroll_amount = 360 / NUMBER_OF_ICONS;
	
	U16 iIconCount = NUMBER_OF_ICONS;
	
	tFixed radius(80);
	tFixed theta(360.0/iIconCount);
	tFixed depth(1);	
	
	CGraphics2D::Instance()->clearBuffer();
	
	background->draw();
	
	KeyboardManager* keyManager = KeyboardManager::Instance();
	U32 keyboardKeysPressed = keyManager->getButtonPressedMask();
	
	if ((keyboardKeysPressed & kButtonDown) || (keyboardKeysPressed & kButtonLeft))
	{
		//CDrawManager::Instance()->remove(iconTextArray[currentSelection_]);
		if (currentSelection_==0)
		{
			currentSelection_ = NUMBER_OF_ICONS;
		}
		--currentSelection_;
		
		scroll_down = 1;
		scroll_up   = 0;
		

	}
	else if ((keyboardKeysPressed & kButtonUp) || (keyboardKeysPressed & kButtonRight))
	{
		//CDrawManager::Instance()->remove(iconTextArray[currentSelection_]);
		++currentSelection_;
		if (currentSelection_ >= NUMBER_OF_ICONS)
		{
			currentSelection_ = 0;
		}
		
		scroll_up = 1;
		scroll_down = 0;
				
	}
	
	

		if(j  < (currentSelection_ * scroll_amount) ) j+= abs(j - (currentSelection_ * scroll_amount)) >> 3;
		if(j  > (currentSelection_ * scroll_amount) ) j-= abs(j - (currentSelection_ * scroll_amount)) >> 3;
	

	
	for (U16 i=0; i<iIconCount; ++i)
	{
		tFixed dx(radius * cos(theta*i - j));
		tFixed dy(radius * sin(theta*i - j));
		
		iconBillboardArray[i].moveTo(tVect2(120+dx, 120+dy));
		iconTextArray[i].moveTo(tVect2(120+dx, 120+dy));
	}
	

	// show bitz info
	bitz_display->setSize(10);
	sprintf(buffer,"profile name: %s", cp->GetName().c_str());
	bitz_display->setText(buffer);
	bitz_display->moveTo(tVect2(16, 224));
	bitz_display->setColor(tColor(255, 0, 0, 255));
	bitz_display->draw();


	

	
	if (keyboardKeysPressed & kButtonA)
	{
		EgApp::RequestNewState(iconDataArray[currentSelection_].pGameState);
	}

	if (keyboardKeysPressed & kButtonMenu)
	{
		CAppManager::Instance()->PopApp();
	}
	
	for(int i = 0; i<iIconCount; ++i) 
	{
		iconBillboardArray[i].draw();
		if(i == currentSelection_) iconTextArray[i].draw();
	}

	
	CDrawManager::Instance()->drawAll();

	keyManager->clearButtonPressedMask(keyboardKeysPressed);
	
	CGraphics2D::Instance()->swapBuffer();

}




