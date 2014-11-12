#include "ALMainState.h"
#include "AudioListenerApp.h"
#include "ALAudioListener.h"
#include <AppManager.h>

static const size_t kNumSampleAudioFiles = 5; ///< number of sample audio files
static const U16 kButtonHeight = 41; ///< height of the touch button
static const U16 kButtonWidth = 97;  ///< width of the touch button

ALMainState::ALMainState(LeapFrog::Brio::tDisplayHandle handle) :
    audio_id_(kNoAudioID),
    first_audio_playing_(false)
{
}

ALMainState::~ALMainState()
{
}

void ALMainState::Enter(CStateData* userData)
{
    RegisterEventListeners();
    Init();
}

void ALMainState::Exit()
{
    if (audio_id_ != kNoAudioID)
    {
        audio_mpi_.StopAudio(audio_id_, true);
    }

    UnregisterEventListeners();
    display_renderer_.TearDownDisplay();
}

void ALMainState::Suspend()
{
	audio_mpi_.PauseAudio(audio_id_);
    UnregisterEventListeners();
    display_renderer_.TearDownDisplay();
}

void ALMainState::Resume()
{
    audio_mpi_.ResumeAudio(audio_id_);
    RegisterEventListeners();
    display_renderer_.InitDisplay();
}

void ALMainState::Update(CGameStateHandler* sh)
{
    //check for screen touch events
    DoTouchEventLoop();

    //check for physical button events
    if (DoKeyEventLoop() == 1) return;

    display_renderer_.SetupFrameRender();

    touch_button_.render(display_renderer_.CurrentBuffer());

    display_renderer_.SwapBuffers();
}

int ALMainState::DoKeyEventLoop(void)
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

        if (button_state & kButtonMenu)
        {
            CAppManager::Instance()->PopApp();
            return 1;
        }
    }
    return 0;
}

int ALMainState::DoTouchEventLoop(void)
{
    std::vector<LeapFrog::Brio::tTouchData> *touch_event_queue = touch_event_queue_.GetQueue();

    std::vector<LeapFrog::Brio::tTouchData>::const_iterator touch_reader = touch_event_queue->begin();

    // iterate through all touch events in the touch event queue
    while(touch_reader != touch_event_queue->end())
    {
        static U16 touch_state = 0;
        LeapFrog::Brio::tTouchData touch_data = *(touch_reader++);

        //get touch_state:
        //0:	up
        //1:	down
        touch_state = touch_data.touchState;

        //check for button press
        int pen_x = touch_data.touchX;
        int pen_y = touch_data.touchY;

        touch_button_.checkForButtonTouchEvent(touch_state, pen_x, pen_y);
    }
    return 0;
}

void ALMainState::HandleTouchEvent(unsigned int id)
{
    // if there is already an audio id, try to stop it before we play the
	// next sample
    if (audio_id_ == kNoAudioID)
    {
        first_audio_playing_ = true;
        PlayRandomAudio();
    }
}

void ALMainState::HandleAudioDonePlaying(tAudioID audio_id)
{
    if (audio_id == audio_id_)
    {
        if (first_audio_playing_)
        {
            first_audio_playing_ = false;
            PlayRandomAudio();
        }
        else
        {
            audio_id_ = kNoAudioID;
        }
    }
}

void ALMainState::PlayRandomAudio(void)
{
    // play one of the audio files randomly
    // it is the same call regardless if the audio file is
    // in wav format or ogg format
	// ADD DETAIL ON WHAT EACH PARAMETER DOES
    unsigned int sample_index = rand() % kNumSampleAudioFiles;
    audio_id_ = audio_mpi_.StartAudio(sample_audio_[sample_index],
                                     kAudioVolumeMax,
                                     0,
                                     kAudioPanDefault,
                                     audio_listener_,
                                     0,
                                     kAudioOptionsDoneMsgAfterComplete );

}

void ALMainState::FillSampleAudioVector(void)
{
    // notice there are both wav and ogg files
	LeapFrog::Brio::CPath titlePath = CSystemData::Instance()->GetCurrentTitlePath();
	sample_audio_.push_back(titlePath+"audio/SampleWAVaudio1.wav");
	sample_audio_.push_back(titlePath+"audio/SampleWAVaudio2.wav");
	sample_audio_.push_back(titlePath+"audio/SampleOGGAudio1.ogg");
	sample_audio_.push_back(titlePath+"audio/SampleOGGAudio2.ogg");
	sample_audio_.push_back(titlePath+"audio/SampleOGGAudio3.ogg");
}

void ALMainState::AddTouchButton(void)
{
    // create the touch button
    touch_button_.createButton(
        CSystemData::Instance()->GetCurrentTitlePath()+"images/button.png",
        CSystemData::Instance()->GetCurrentTitlePath()+"images/buttonHigh.png",
        (display_renderer_.Width()-kButtonWidth)/2,
        ((display_renderer_.Height()-kButtonHeight)/2),
        kButtonWidth,
        kButtonHeight,
        true,
        this);
}

void ALMainState::CreateAudioListener(void)
{
    audio_listener_ = new ALAudioListener(this);

}

void ALMainState::Init(void)
{
    display_renderer_.InitDisplay();
    AddTouchButton();
    FillSampleAudioVector();
    CreateAudioListener();
}

void ALMainState::RegisterEventListeners(void)
{
    //register touch screen
    LeapFrog::Brio::CEventMPI event_mpi;
    event_mpi.RegisterEventListener(&touch_event_queue_);

    //register keys
    LeapFrog::Brio::CButtonMPI key_event_mpi;
    key_event_mpi.RegisterEventListener(&button_event_queue_);
    key_event_mpi.SetTouchMode( kTouchModeDefault );
}

void ALMainState::UnregisterEventListeners(void)
{
    // stop listening for screen touch events
    LeapFrog::Brio::CEventMPI event_mpi;
    event_mpi.UnregisterEventListener(&touch_event_queue_);

    // stop listening for physical button events
    LeapFrog::Brio::CButtonMPI key_event_mpi;
    key_event_mpi.UnregisterEventListener(&button_event_queue_);
}
