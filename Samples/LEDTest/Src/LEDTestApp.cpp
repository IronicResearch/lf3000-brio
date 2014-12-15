#include <iostream>
#include <AppManager.h>
#include "BaseUtils.h"
#include "Alloc.h"
#include <LTM.h>

#include "LEDTestApp.h"
#include "LEDSMainState.h"


//LF_USING_BRIO_NAMESPACE()
using namespace LeapFrog::Brio;
using namespace LogData::Lightning;
using namespace LTM;

bool LEDTestApp::new_state_						= false;
CGameStateBase* LEDTestApp::new_state_requested_	= NULL;
CGameStateBase* LEDTestApp::current_state_			= NULL;

/*
 * All initialization and memory allocations should be handled in the Enter state
 * Constructor should only be used for getting arguments that may be passed during launch.
 */
LEDTestApp::LEDTestApp(void * userData=NULL)
	: CAppInterface(),
	debug_mpi_(kFirstCartridge1DebugSig),
	product_id_(0), part_number_("000-000000"), pkg_version_("0.0.0.0"), log_data_version_(0)
{
}

LEDTestApp::~LEDTestApp()
{
}

void LEDTestApp::Suspend()
{
	debug_mpi_.DebugOut(kDbgLvlNoteable, "[LEDTestApp] Suspend\n");

	CGameStateBase* topState;
	topState = state_handler_->GetTopState();
	topState->Suspend();
}

void LEDTestApp::Resume()
{
	debug_mpi_.DebugOut(kDbgLvlNoteable, "[LEDTestApp] Resume\n");

	CGameStateBase* topState;
	topState = state_handler_->GetTopState();
	topState->Resume();
}

void LEDTestApp::Update()
{
	debug_mpi_.DebugOut(kDbgLvlVerbose, "[LEDTestApp] Update()\n");

	// switch states if necessary
	bool switched_states = false;
	if (new_state_ && new_state_requested_ != NULL) {
		state_handler_->Request(new_state_requested_);
		new_state_ = false;
		switched_states = true;
	}
	debug_mpi_.DebugOut(kDbgLvlVerbose, "[LEDTestApp] Update() mid\n");

	// this will call update on the current state
	state_handler_->Run();

	if (switched_states) {
		if(current_state_)
			delete current_state_;
		current_state_ = new_state_requested_;
		new_state_requested_ = NULL;
	}

	debug_mpi_.DebugOut(kDbgLvlVerbose, "[LEDTestApp] Update() End\n");
}

void LEDTestApp::Enter()
{
	debug_mpi_.SetDebugLevel(DEBUG_LEVEL);
	debug_mpi_.DebugOut(kDbgLvlNoteable, "[LEDTestApp] Enter()\n");

	state_handler_ = new CGameStateHandler();

	// ideally this would go in the constructor, but the current title is not set yet.
	// initialize data used for logging
	LTM::CMetaInfo metaInfo(CSystemData::Instance()->GetCurrentTitlePath());
	if (metaInfo.IsValid())
	{
		product_id_ =	metaInfo.GetProductId();
		part_number_ =	metaInfo.GetPartNumber();
		pkg_version_ =	metaInfo.GetPackageVersion();
		log_data_version_ =	1;
	}
	debug_mpi_.DebugOut(kDbgLvlNoteable, "\tProductID: %lu\n", getProductId());
	debug_mpi_.DebugOut(kDbgLvlNoteable, "\tPartNumber: %s\n", getPartNumber());
	debug_mpi_.DebugOut(kDbgLvlNoteable, "\tproductVersion: %s\n", getProductVersion());
	debug_mpi_.DebugOut(kDbgLvlNoteable, "\tlog data version: %d\n", getLogDataVersion());

	// Log TitleStart event.
	U32 productId = getProductId();
	LogData::Lightning::CTitleStart titleStart(getProductId(), getPartNumber(),
			getProductVersion(), getLogDataVersion());
	CLogFile::Instance()->Append(titleStart);

	// start with this state
	state_handler_->Request(new LEDSMainState());
}

void LEDTestApp::Exit()
{
	debug_mpi_.DebugOut(kDbgLvlNoteable, "[LEDTestApp] Exit()\n");

	//kill the state handler, so they can cleanup before we clean up here
	delete state_handler_;

	if(current_state_)
		delete current_state_;

	// Log the title exit event
	LogData::Lightning::CTitleExit titleExit(LogData::Lightning::CTitleExit::kReasonUnknown);
	CLogFile::Instance()->Append(titleExit);
}

void LEDTestApp::RequestNewState(CGameStateBase* state)
{
	//debug_mpi_.DebugOut(kDbgLvlNoteable, "[EgGameState] RequestNewState\n");
	new_state_ = true;
	new_state_requested_ = state;
}


