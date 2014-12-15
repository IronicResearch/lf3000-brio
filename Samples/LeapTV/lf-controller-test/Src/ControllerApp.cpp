#include <iostream>
#include <AppManager.h>
#include "ControllerApp.h"
#include "CTRLMainState.h"
#include "BaseUtils.h"
#include "Alloc.h"

bool ControllerApp::new_state_ = false;
CGameStateBase* ControllerApp::new_state_requested_ = NULL;

/*
 * All initialization and memory allocations should be handled in the 
 * Enter state.  Constructor should only be used for getting arguments 
 * that may be passed during launch.
 */
ControllerApp::ControllerApp(void * userData=NULL)
  : CAppInterface() {
}

ControllerApp::~ControllerApp(void) {
}

void ControllerApp::Suspend(void) {
  CGameStateBase* top_state_;
  top_state_ = state_handler_->GetTopState();
  top_state_->Suspend();
}

void ControllerApp::Resume(void) {
  CGameStateBase* top_state_;
  top_state_ = state_handler_->GetTopState();
  top_state_->Resume();
}

void ControllerApp::Update(void) {
  // switch states if necessary
  if (new_state_ && new_state_requested_ != NULL) {
    state_handler_->Request(new_state_requested_);
    new_state_ = false;
    new_state_requested_ = NULL;
  }
  
  // this will call update on the current state
  state_handler_->Run();
}

void ControllerApp::Enter(void) {
  state_handler_ = new CGameStateHandler();
  
  // start with this state
  state_handler_->Request(CTRLMainState::Instance());
}

void ControllerApp::Exit(void) {
  //kill the state handler, so they can cleanup before we clean up here
  delete state_handler_;
  CTRLMainState* inst = CTRLMainState::Instance();
  delete inst;
}

void ControllerApp::RequestNewState(CGameStateBase* state) {
  new_state_ = true;
  new_state_requested_ = state;
}

