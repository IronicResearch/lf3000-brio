#include <iostream>
#include <AppManager.h>
#include "CameraApp.h"
#include "CMRAMainState.h"


bool CameraApp::newState_                     = false;
CGameStateBase* CameraApp::newStateRequested_ = NULL;
CGameStateBase* CameraApp::currentState_      = NULL;

/*
 * All initialization and memory allocations should be handled in the Enter state
 * Constructor should only be used for getting arguments that may be passed during launch.
 */
CameraApp::CameraApp(void * userData=NULL)
  : CAppInterface() {
}

CameraApp::~CameraApp(void) {
}

void CameraApp::Suspend(void) {
  CGameStateBase* topState;
  topState = stateHandler_->GetTopState();
  topState->Suspend();
}

void CameraApp::Resume(void) {
  CGameStateBase* topState;
  topState = stateHandler_->GetTopState();
  topState->Resume();
}

void CameraApp::Update(void) {
  // switch states if necessary
  bool switched_states = false;
  if (newState_ && newStateRequested_ != NULL) {
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

void CameraApp::Enter(void) {
  stateHandler_ = new CGameStateHandler();
  
  // start with this state
  stateHandler_->Request(new CMRAMainState());
}

void CameraApp::Exit(void) {
  //kill the state handler, so they can cleanup before we clean up here
  delete stateHandler_;
  
  if(currentState_)
    delete currentState_;
}

void CameraApp::RequestNewState(CGameStateBase* state) {
  newState_ = true;
  newStateRequested_ = state;
}


