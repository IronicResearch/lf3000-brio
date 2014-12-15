#include <AppManager.h>

#include "OpenGLMovingSpriteApp.h"
#include "MSMainState.h"

bool OpenGLMovingSpriteApp::new_state_						= false;
CGameStateBase* OpenGLMovingSpriteApp::new_state_requested_	= NULL;
CGameStateBase* OpenGLMovingSpriteApp::current_state_		= NULL;

/*
 * All initialization and memory allocations should be handled in the Enter state
 * Constructor should only be used for getting arguments that may be passed during launch.
 */
OpenGLMovingSpriteApp::OpenGLMovingSpriteApp(void * userData=NULL)
    : CAppInterface()
{
}

OpenGLMovingSpriteApp::~OpenGLMovingSpriteApp()
{
}

void OpenGLMovingSpriteApp::Suspend()
{
    CGameStateBase* topState;
    topState = state_handler_->GetTopState();
    topState->Suspend();
}

void OpenGLMovingSpriteApp::Resume()
{
    CGameStateBase* topState;
    topState = state_handler_->GetTopState();
    topState->Resume();
}

void OpenGLMovingSpriteApp::Update()
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
}

void OpenGLMovingSpriteApp::Enter()
{
    state_handler_ = new CGameStateHandler();
    state_handler_->Request(new MSMainState());
}

void OpenGLMovingSpriteApp::Exit()
{
    //kill the state handler, so they can cleanup before we clean up here
    delete state_handler_;

    if(current_state_)
        delete current_state_;
}

void OpenGLMovingSpriteApp::RequestNewState(CGameStateBase* state)
{
	new_state_ = true;
	new_state_requested_ = state;
}
