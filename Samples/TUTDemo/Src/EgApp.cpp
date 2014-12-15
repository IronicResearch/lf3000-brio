#include <iostream>
#include <AppManager.h>
#include "BaseUtils.h"
#include "Alloc.h"
#include <LTM.h>

#include "EgApp.h"
#include "MainState.h"


//LF_USING_BRIO_NAMESPACE()
using namespace LeapFrog::Brio;
using namespace LogData::Lightning;
using namespace LTM;

bool EgApp::newState_						= false;
CGameStateBase* EgApp::newStateRequested_	= NULL;
CGameStateBase* EgApp::currentState_		= NULL;

/*
 * All initialization and memory allocations should be handled in the Enter state
 * Constructor should only be used for getting arguments that may be passed during launch.
 */
EgApp::EgApp(void * userData=NULL)
	: CAppInterface(),
	debugMPI_(kFirstCartridge1DebugSig),
	productID_(0), partNumber_("000-000000"), pkgVersion_("0.0.0.0"), logDataVersion_(0)
{
}

EgApp::~EgApp()
{
}

void EgApp::Suspend()
{
	debugMPI_.DebugOut(kDbgLvlNoteable, "[App] Suspend\n");

	CGameStateBase* topState;
	topState = stateHandler_->GetTopState();
	topState->Suspend();
}

void EgApp::Resume()
{
	debugMPI_.DebugOut(kDbgLvlNoteable, "[App] Resume\n");

	CGameStateBase* topState;
	topState = stateHandler_->GetTopState();
	topState->Resume();
}

void EgApp::Update()
{
	debugMPI_.DebugOut(kDbgLvlVerbose, "[App] Update()\n");

	// switch states if necessary
	bool switched_states = false;
	if (newState_ && newStateRequested_ != NULL) {
		stateHandler_->Request(newStateRequested_);
		newState_ = false;
		switched_states = true;
	}
	debugMPI_.DebugOut(kDbgLvlVerbose, "[App] Update() mid\n");

	// this will call update on the current state
	stateHandler_->Run();

	if (switched_states) {
		if(currentState_)
			delete currentState_;
		currentState_ = newStateRequested_;
		newStateRequested_ = NULL;
	}

	debugMPI_.DebugOut(kDbgLvlVerbose, "[App] Update() End\n");
}

void EgApp::Enter()
{
	debugMPI_.SetDebugLevel(DEBUG_LEVEL);
	debugMPI_.DebugOut(kDbgLvlNoteable, "[App] Enter()\n");

	stateHandler_ = new CGameStateHandler();

	// ideally this would go in the constructor, but the current title is not set yet.
	// initialize data used for logging
	LTM::CMetaInfo metaInfo(CSystemData::Instance()->GetCurrentTitlePath());
	if (metaInfo.IsValid())
	{
		productID_ =	metaInfo.GetProductId();
		partNumber_ =	metaInfo.GetPartNumber();
		pkgVersion_ =	metaInfo.GetPackageVersion();
		logDataVersion_ =	1;
	}
	debugMPI_.DebugOut(kDbgLvlNoteable, "\tProductID: %lu\n", getProductID());
	debugMPI_.DebugOut(kDbgLvlNoteable, "\tPartNumber: %s\n", getPartNumber());
	debugMPI_.DebugOut(kDbgLvlNoteable, "\tproductVersion: %s\n", getProductVersion());
	debugMPI_.DebugOut(kDbgLvlNoteable, "\tlog data version: %d\n", getLogDataVersion());

	// Log TitleStart event.
	U32 productId = getProductID();
//	LogData::Lightning::CTitleStart titleStart(getProductID(), getPartNumber(),
//			getProductVersion(), getLogDataVersion());
//	CLogFile::Instance()->Append(titleStart);

	// start with this state
	stateHandler_->Request(new MainState());
}

void EgApp::Exit()
{
	debugMPI_.DebugOut(kDbgLvlNoteable, "[App] Exit()\n");

	//kill the state handler, so they can cleanup before we clean up here
	delete stateHandler_;

	if(currentState_)
		delete currentState_;

	// Log the title exit event
//	LogData::Lightning::CTitleExit titleExit(LogData::Lightning::CTitleExit::kReasonUnknown);
//	CLogFile::Instance()->Append(titleExit);
}

void EgApp::RequestNewState(CGameStateBase* state)
{
	//debugMPI_.DebugOut(kDbgLvlNoteable, "[EgGameState] RequestNewState\n");
	newState_ = true;
	newStateRequested_ = state;
}


