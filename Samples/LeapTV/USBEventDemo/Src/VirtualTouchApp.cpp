#include <iostream>
#include <AppManager.h>
#include "VirtualTouchApp.h"
#include "VTCHMainState.h"

using namespace LeapFrog::Brio;
using namespace LogData::Lightning;
using namespace LTM;

bool VirtualTouchApp::newState_                     = false;
CGameStateBase* VirtualTouchApp::newStateRequested_ = NULL;
CGameStateBase* VirtualTouchApp::currentState_      = NULL;

bool isCameraConnectedAtLaunch;

/*
 * All initialization and memory allocations should be handled in the Enter state
 * Constructor should only be used for getting arguments that may be passed during launch.
 */
VirtualTouchApp::VirtualTouchApp(void * userData=NULL) : CAppInterface()
{
	bool myTest = _cameraMPI_.IsCameraPresent();
	//printf("myTest: %d\n", myTest);
	if (myTest)
	{
		//printf("yes cam is present\n\n\n\n");
		isCameraConnectedAtLaunch = true;
	}
	else
	{
		//printf("no cam is not present\n\n\n\n");
		isCameraConnectedAtLaunch = false;
	}
}

VirtualTouchApp::~VirtualTouchApp(void)
{

}

void VirtualTouchApp::Suspend(void)
{
	CGameStateBase* topState;
	topState = stateHandler_->GetTopState();
	topState->Suspend();
}

void VirtualTouchApp::Resume(void)
{
	CGameStateBase* topState;
	topState = stateHandler_->GetTopState();
	topState->Resume();
}

void VirtualTouchApp::Update(void)
{
	// switch states if necessary
	bool switched_states = false;
	if (newState_ && newStateRequested_ != NULL) {
	stateHandler_->Request(newStateRequested_);
	newState_ = false;
	switched_states = true;
	}
  
	// this will call update on the current state
	stateHandler_->Run();

	if (switched_states)
	{
		if(currentState_)
			delete currentState_;
		currentState_ = newStateRequested_;
		newStateRequested_ = NULL;
	}
}

void VirtualTouchApp::Enter(void)
{
	stateHandler_ = new CGameStateHandler();

	// start with this state
	stateHandler_->Request(new VTCHMainState());
}

void VirtualTouchApp::Exit(void)
{
	//kill the state handler, so they can cleanup before we clean up here
	delete stateHandler_;

	if(currentState_)
		delete currentState_;
}

void VirtualTouchApp::RequestNewState(CGameStateBase* state)
{
	newState_ = true;
	newStateRequested_ = state;
}


