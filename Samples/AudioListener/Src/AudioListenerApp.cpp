#include "AudioListenerApp.h"
#include <AppManager.h>
#include "ALMainState.h"

bool AudioListenerApp::new_state_						= false;
CGameStateBase* AudioListenerApp::new_state_requested_	= NULL;
CGameStateBase* AudioListenerApp::current_state_		= NULL;

/*
 * All initialization and memory allocations should be handled in the Enter state
 * Constructor should only be used for getting arguments that may be passed during launch.
 */
AudioListenerApp::AudioListenerApp(void * userData=NULL)
    : CAppInterface()
{
}

AudioListenerApp::~AudioListenerApp()
{
}

void AudioListenerApp::Suspend()
{
    CGameStateBase* topState;
    topState = state_handler_->GetTopState();
    topState->Suspend();
}

void AudioListenerApp::Resume()
{
    CGameStateBase* topState;
    topState = state_handler_->GetTopState();
    topState->Resume();
}

void AudioListenerApp::Update()
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

    display_mpi_.Invalidate(0, NULL);
}

void AudioListenerApp::Enter()
{
    state_handler_ = new CGameStateHandler();

    const tDisplayScreenStats* stats = display_mpi_.GetScreenStats(kOrientationPortrait);
    display_handle_ = display_mpi_.CreateHandle(stats->height,stats->width,kPixelFormatARGB8888,NULL);

    // start with this state
    state_handler_->Request(new ALMainState(display_handle_));
    display_mpi_.Register(display_handle_, 0, 0, kDisplayOnTop, kDisplayScreenAllScreens);
}

void AudioListenerApp::Exit()
{
    //kill the state handler, so they can cleanup before we clean up here
    delete state_handler_;

    if(current_state_)
        delete current_state_;

    display_mpi_.UnRegister(display_handle_, kDisplayScreenAllScreens);
    display_mpi_.DestroyHandle(display_handle_, false);
}

void AudioListenerApp::RequestNewState(CGameStateBase* state)
{
	new_state_ = true;
	new_state_requested_ = state;
}
