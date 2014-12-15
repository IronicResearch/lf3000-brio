#include "AccelerometerApp.h"
#include <AppManager.h>
#include "ACCMainState.h"

bool AccelerometerApp::newState_		     = false;
CGameStateBase* AccelerometerApp::newStateRequested_ = NULL;

/*
 * All initialization and memory allocations should be handled in the Enter state
 * Constructor should only be used for getting arguments that may be passed during launch.
 */
AccelerometerApp::AccelerometerApp(void * userData=NULL)
  : CAppInterface() {
}

AccelerometerApp::~AccelerometerApp(void) {
}

void AccelerometerApp::Suspend(void) {
  CGameStateBase* topState;
  topState = stateHandler_->GetTopState();
  topState->Suspend();
}

void AccelerometerApp::Resume(void) {
  CGameStateBase* topState;
  topState = stateHandler_->GetTopState();
  topState->Resume();
}

void AccelerometerApp::Update(void) {
  // switch states if necessary
  if (newState_ && newStateRequested_ != NULL) {
    stateHandler_->Request(newStateRequested_);
    newState_ = false;
    newStateRequested_ = NULL;
  }
  
  // this will call update on the current state
  stateHandler_->Run();
}

void AccelerometerApp::Enter(void) {
  stateHandler_ = new CGameStateHandler();
  
  // start with this state
  stateHandler_->Request(ACCMainState::Instance());
}

void AccelerometerApp::Exit(void) {
  //kill the state handler, so they can cleanup before we clean up here
  delete stateHandler_;
}

void AccelerometerApp::RequestNewState(CGameStateBase* state) {
  newState_ = true;
  newStateRequested_ = state;
}

