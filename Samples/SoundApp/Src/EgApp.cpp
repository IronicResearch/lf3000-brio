#include <iostream>
#include <AppManager.h>
#include <LogFile.h>
//#include <CSystemData.h>
#include "EgApp.h"
#include "EgPickerState.h"

LF_USING_BRIO_NAMESPACE()

bool EgApp::newState_						= false;
CGameStateBase* EgApp::newStateRequested_	= NULL;


EgApp::EgApp(void * userData=NULL)
	: CAppInterface(),
	debugMPI_(kFirstCartridge1DebugSig)
//	pGraphicsLib_(NULL)
{
}

EgApp::~EgApp()
{
}

void EgApp::Enter() 
{
	debugMPI_.DebugOut(kDbgLvlNoteable, "EgApp::Enter()\n");
	
	// Log TitleStart event.
	if (CAppManager::Instance()->pLogFile->IsProfileSet())
	{
		// we could do this on the heap - but I prefer the stack
//		LogData::Lightning::CTitleStart * pTitleStart = 
//			new LogData::Lightning::CTitleStart(0, "TBD", "TBD", 0);
//		CAppManager::Instance()->pLogFile->Append(*pTitleStart);
//		delete pTitleStart; pTitleStart = NULL;
		
		// CTitleStart(U32 productId, char * partNumber, char * productVersion, U8 logDataVersion);
		LogData::Lightning::CTitleStart titleStart(EgApp::getProductID(), EgApp::getPartNumber(), 
				EgApp::getProductVersion(), EgApp::getLogDataVersion());
		CAppManager::Instance()->pLogFile->Append(*(dynamic_cast<CLogData*>(&titleStart)));
	}

	// register for button presses
	pKeyManager_ = KeyboardManager::Instance();
	CButtonMPI button;
	CEventMPI	eventmgr;
	if (eventmgr.IsValid()) {	eventmgr.RegisterEventListener(pKeyManager_); }
	else {	debugMPI_.Assert(false, "FAILURE to load Event manager!\n"); }
	
	// initialize the graphics library
	//pGraphicsLib_ = CGraphics2D::Instance();
	CGraphics2D::Instance()->init(tVect2(320, 240), tColor(255, 255, 255, 255));
	//pGraphicsLib->init(tVect2(320, 240), tColor(0, 0, 0, 255));
	// start with this state
	stateHandler_.Request(EgPickerState::Instance());

}

void EgApp::Update() 
{
	//debugMPI_.DebugOut(kDbgLvlVerbose, "EgApp::Update()\n");
	
	// switch states if necessary
	if (newState_ && newStateRequested_ != NULL) {
		stateHandler_.Request(newStateRequested_);
		newState_ = false;
		newStateRequested_ = NULL;
	}
	
	// this will call update on the current state
	stateHandler_.Run();
}

void EgApp::Exit()
{
	debugMPI_.DebugOut(kDbgLvlNoteable, "EgApp::Exit()\n");
	
	// Log the title exit event
	if (CAppManager::Instance()->pLogFile->IsProfileSet())
	{
		// TODO: add the proper reason for leaving the game
		LogData::Lightning::CTitleExit titleExit(LogData::Lightning::CTitleExit::kReasonUnknown);
		CAppManager::Instance()->pLogFile->Append(*(dynamic_cast<CLogData*>(&titleExit)));
	}
	
	// TODO: does this need to be done here?
	CEventMPI	eventmgr;
	if (eventmgr.IsValid()) {	eventmgr.UnregisterEventListener(pKeyManager_); }
	else {	debugMPI_.Assert(false, "FAILURE to load Event manager!\n"); }
	
	CGraphics2D::Instance()->cleanup();
}

void EgApp::RequestNewState(CGameStateBase* state)
{
	newState_ = true;
	newStateRequested_ = state;
}
