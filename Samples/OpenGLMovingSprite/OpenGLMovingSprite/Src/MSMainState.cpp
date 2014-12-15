#include "MSMainState.h"
#include "OpenGLMovingSpriteApp.h"
#include <AppManager.h>
#include <BrioOpenGLConfig.h>
#include <DisplayMPI.h>
#include <sstream>

// constant used in time conversion for frame rate
static const double kNanoSecondsInASecond = 1000000000;

using LeapFrog::Brio::U8;
using LeapFrog::Brio::U16;

MSMainState::MSMainState() :
    CGameStateBase(),
    brio_opengl_config_(NULL),
    frame_duration_(1.0/15.0)
{
    // seed the time
    clock_gettime(CLOCK_MONOTONIC, &previous_render_time_);
}

MSMainState::~MSMainState()
{
    // memory management
    if (brio_opengl_config_) delete brio_opengl_config_;
}

void MSMainState::Enter(CStateData* userData)
{
    RegisterEventListeners();
    Init();
}

void MSMainState::Exit()
{
    UnregisterEventListeners();
}

void MSMainState::Suspend()
{
    UnregisterEventListeners();
}

void MSMainState::Resume()
{
	RegisterEventListeners();
}

void MSMainState::Update(CGameStateHandler* sh)
{
    //check for physical button events
    if (DoKeyEventLoop() == 1) return;

    if (ShouldRenderNextFrame()) {
        glClear(GL_COLOR_BUFFER_BIT);
        animation_.Render();
        // update the render time with the current time
        clock_gettime(CLOCK_MONOTONIC, &previous_render_time_);
        eglSwapBuffers(brio_opengl_config_->eglDisplay, brio_opengl_config_->eglSurface);
    }
}

int MSMainState::DoKeyEventLoop(void)
{
    std::vector<LeapFrog::Brio::tButtonData2> *button_event_queue = button_event_queue_.GetQueue();
    std::vector<LeapFrog::Brio::tButtonData2>::const_iterator button_reader = button_event_queue->begin();

    // iterate over all events in the button event queue
    while(button_reader != button_event_queue->end())
    {
        static U16 button_state = 0;
        LeapFrog::Brio::tButtonData2 button_data = *(button_reader++);
        button_state = button_data.buttonState;

        /*
         * button states:
         * kButtonMenu, kButtonHint
         * kButtonLeft, kButtonRight, kButtonUp, kButtonDown,
         * kButtonLeftShoulder, kButtonRightShoulder,
         * kButtonA, kButtonB
         */

        if (button_state & LeapFrog::Brio::kButtonMenu)
        {
            CAppManager::Instance()->PopApp();
            return 1;
        }
    }
    return 0;
}


void MSMainState::Init(void)
{
    // instantiate opengl configuration class
    brio_opengl_config_ = new LeapFrog::Brio::BrioOpenGLConfig();

    // obtain screen dimensions
    CDisplayMPI display_mpi;
    // TODO: The default display orientation for the LeapPad2 is Landscape
    // Calling GetScreenStats with kOrientationPortrait on the LeapPad2 gives the
    // landscape width and height.  This is not consistent and needs to be
    // addressed either in the documentation or fixed outright.
    const tDisplayScreenStats* stats = display_mpi.GetScreenStats(kOrientationPortrait);

    // set up opengl remebering that opengl es 1.1 is a state machine
    // In this app we never change the matrix mode, the orthographic projection
    // or the background color
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthox(0, stats->width<<16, 0, stats->height<<16, 10<<16, 0);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(1.0, 1.0, 1.0, 1.0);

    // load all sprites in to the animation
    LeapFrog::Brio::CPath base_path = CSystemData::Instance()->GetCurrentTitlePath();
    LeapFrog::Brio::CPath image_path;
    for (unsigned int i = 1; i <= 20; ++i)
    {
        std::stringstream ss;
        ss << "/images/" << i << ".png";
        image_path = base_path+ss.str();
        animation_.AddSprite(MSSprite(image_path));
    }
    animation_.looping_ = true;

    // move the animation to the center of the screen
    MSSize size = animation_.ImageSize();
    animation_.SetTranslation(MSVector((stats->width-size.width_)/2.0,
                                       (stats->height-size.height_)/2.0));
}

bool MSMainState::ShouldRenderNextFrame(void)
{
    // obtain the current time, in seconds and nanoseconds
    timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    // calculate the elapsed time since the last frame rendering
    double dt = (current_time.tv_sec - previous_render_time_.tv_sec);
    dt += (current_time.tv_nsec - previous_render_time_.tv_nsec)/kNanoSecondsInASecond;

    // based on the change in time, indicate if the next frame should be rendered
    return (dt >= frame_duration_) ? true : false;
}

void MSMainState::RegisterEventListeners(void)
{
    //register keys
    LeapFrog::Brio::CButtonMPI key_event_mpi;
    key_event_mpi.RegisterEventListener(&button_event_queue_);
    key_event_mpi.SetTouchMode(LeapFrog::Brio::kTouchModeDefault);
}

void MSMainState::UnregisterEventListeners(void)
{
    // stop listening for physical button events
    LeapFrog::Brio::CButtonMPI key_event_mpi;
    key_event_mpi.UnregisterEventListener(&button_event_queue_);
}
