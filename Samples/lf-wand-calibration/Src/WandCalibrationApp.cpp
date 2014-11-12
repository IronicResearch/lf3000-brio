#include <iostream>
#include <AppManager.h>
#include "WandCalibrationApp.h"
#include "CALBMainState.h"

using namespace LeapFrog::Brio;
using namespace LogData::Lightning;
using namespace LTM;

bool WandCalibrationApp::newState_                     = false;
CGameStateBase* WandCalibrationApp::newStateRequested_ = NULL;
CGameStateBase* WandCalibrationApp::currentState_      = NULL;

/*
 * All initialization and memory allocations should be handled in the Enter state
 * Constructor should only be used for getting arguments that may be passed during launch.
 */
WandCalibrationApp::WandCalibrationApp(void * userData=NULL)
  : CAppInterface() {
}

WandCalibrationApp::~WandCalibrationApp(void) {
	printf("WandCalibrationApp destructor\n");
}

void WandCalibrationApp::Suspend(void) {
  CGameStateBase* topState;
  topState = stateHandler_->GetTopState();
  topState->Suspend();
}

void WandCalibrationApp::Resume(void) {
  CGameStateBase* topState;
  topState = stateHandler_->GetTopState();
  topState->Resume();
}

void WandCalibrationApp::Update(void) {
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

void WandCalibrationApp::Enter(void) {
  stateHandler_ = new CGameStateHandler();

  // start with this state
  currentState_ = new CALBMainState();
  stateHandler_->Request(currentState_);
}

void WandCalibrationApp::Exit(void) {
  //kill the state handler, so they can cleanup before we clean up here
	printf("\n\nExiting WandCalibrationApp\n");
  delete stateHandler_;

  printf("delete currentState_?\n");
  if(currentState_)
  {
    delete currentState_;
	printf("delete currentState_!\n");
  }

	printf("Exiting WandCalibrationApp done\n");
}

void WandCalibrationApp::RequestNewState(CGameStateBase* state) {
  newState_ = true;
  newStateRequested_ = state;
}
