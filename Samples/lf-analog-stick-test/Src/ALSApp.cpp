#include <iostream>
#include <AppManager.h>

#include "ALSApp.h"
#include "ALSMainState.h"

#include "BaseUtils.h"
#include "Alloc.h"

bool ALSApp::new_state_						 = false;
CGameStateBase* ALSApp::new_state_requested_ = NULL;

/*
 * All initialization and memory allocations should be handled in the Enter state
 * Constructor should only be used for getting arguments that may be passed during launch.
 */
ALSApp::ALSApp(void * userData=NULL)
	: CAppInterface()
{
}

ALSApp::~ALSApp()
{
}

void ALSApp::Suspend()
{
	CGameStateBase* top_state_;
	top_state_ = state_handler_->GetTopState();
	top_state_->Suspend();
}

void ALSApp::Resume()
{
	CGameStateBase* top_state_;
	top_state_ = state_handler_->GetTopState();
	top_state_->Resume();
}

void ALSApp::Update()
{
	// switch states if necessary
	if (new_state_ && new_state_requested_ != NULL) {
		state_handler_->Request(new_state_requested_);
		new_state_ = false;
		new_state_requested_ = NULL;
	}

	// this will call update on the current state
	state_handler_->Run();
}

void ALSApp::Enter()
{
	state_handler_ = new CGameStateHandler();

	// start with this state
	state_handler_->Request(ALSMainState::Instance());
}

void ALSApp::Exit()
{
	//kill the state handler, so they can cleanup before we clean up here
	delete state_handler_;
}

void ALSApp::RequestNewState(CGameStateBase* state)
{
	new_state_ = true;
	new_state_requested_ = state;
}

