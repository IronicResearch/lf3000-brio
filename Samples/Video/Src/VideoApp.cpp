#include <AppManager.h>

#include "VideoApp.h"
#include "VIDMainState.h"

bool VideoApp::new_state_						= false;
CGameStateBase* VideoApp::new_state_requested_	= NULL;
CGameStateBase* VideoApp::current_state_		= NULL;

/*
 * All initialization and memory allocations should be handled in the Enter state
 * Constructor should only be used for getting arguments that may be passed during launch.
 */
VideoApp::VideoApp(void * userData=NULL)
    : CAppInterface()
{
}

VideoApp::~VideoApp()
{
}

void VideoApp::Suspend()
{
    CGameStateBase* topState;
    topState = state_handler_->GetTopState();
    topState->Suspend();
}

void VideoApp::Resume()
{
    CGameStateBase* topState;
    topState = state_handler_->GetTopState();
    topState->Resume();
}

void VideoApp::Update()
{
    // switch states if necessary
    bool switched_states = false;
    if (new_state_ && new_state_requested_ != NULL) {
        state_handler_->Request(new_state_requested_);
        new_state_ = false;
        switched_states = true;
    }

    // this will call update on the current state
    state_handler_->Run();

    if (switched_states) {
        if(current_state_)
            delete current_state_;
        current_state_ = new_state_requested_;
        new_state_requested_ = NULL;
    }

    display_mpi_.Invalidate(LeapFrog::Brio::kOrientationPortrait, NULL);
}

void VideoApp::Enter()
{
    state_handler_ = new CGameStateHandler();

    // start with this state
    state_handler_->Request(new VIDMainState());
}

void VideoApp::Exit()
{
    //kill the state handler, so they can cleanup before we clean up here
    delete state_handler_;

    if(current_state_)
        delete current_state_;
}

void VideoApp::RequestNewState(CGameStateBase* state)
{
	new_state_ = true;
	new_state_requested_ = state;
}
