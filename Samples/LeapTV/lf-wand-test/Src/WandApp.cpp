#include <iostream>
#include <AppManager.h>
#include "WandApp.h"
#include "WNDMainState.h"

using namespace LeapFrog::Brio;
using namespace LogData::Lightning;
using namespace LTM;

bool WandApp::newState_                     = false;
CGameStateBase* WandApp::newStateRequested_ = NULL;
CGameStateBase* WandApp::currentState_      = NULL;

/*
 * All initialization and memory allocations should be handled in the Enter state
 * Constructor should only be used for getting arguments that may be passed during launch.
 */
WandApp::WandApp(void * userData=NULL)
  : CAppInterface() {
}

WandApp::~WandApp(void) {
	printf("WandApp destructor\n");
}

void WandApp::Suspend(void) {
  CGameStateBase* topState;
  topState = stateHandler_->GetTopState();
  topState->Suspend();
}

void WandApp::Resume(void) {
  CGameStateBase* topState;
  topState = stateHandler_->GetTopState();
  topState->Resume();
}

void WandApp::Update(void) {
  // switch states if necessary
  bool switched_states = false;
  if (newState_ && newStateRequested_ != NULL) {
	  printf("new state\n\n");
    stateHandler_->Request(newStateRequested_);
    newState_ = false;
    switched_states = true;
  }
  
  // this will call update on the current state
  stateHandler_->Run();
  
  if (switched_states) {
    if(currentState_)
      delete currentState_;
    currentState_ = newStateRequested_;
    newStateRequested_ = NULL;
  }
}

void WandApp::Enter(void) {
  stateHandler_ = new CGameStateHandler();
  
  // start with this state
  currentState_ = new WNDMainState();
  stateHandler_->Request(currentState_);
}

void WandApp::Exit(void) {
  //kill the state handler, so they can cleanup before we clean up here
	printf("\n\nExiting WandApp\n");
  delete stateHandler_;
  
  printf("delete currentState_?\n");
  if(currentState_)
  {
    delete currentState_;
	printf("delete currentState_!\n");
  }

	printf("Exiting WandApp done\n");
}

void WandApp::RequestNewState(CGameStateBase* state) {
  newState_ = true;
  newStateRequested_ = state;
}


