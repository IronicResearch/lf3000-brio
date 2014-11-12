#include "VIDMainState.h"
#include <AppManager.h>
#include <KernelMPI.h>
#include "VIDVideoListener.h"

using LeapFrog::Brio::tRect;
using LeapFrog::Brio::U16;
using LeapFrog::Brio::U32;
using namespace LeapFrog::Brio;

static const float kVID4_3AspectRatioMin = 1.32;
static const float kVID4_3AspectRatioMax = 1.34;

// do not initialize anything that accesses the hardware in the constructor do
// it in the Enter method.  Rule of thumb is to not initialize member variables
// in States or App classes.
VIDMainState::VIDMainState(void) :
    video_handle_(LeapFrog::Brio::kInvalidVideoHndl),
    video_listener_(NULL)
{
}

VIDMainState::~VIDMainState()
{
	if (video_listener_)
	{
		delete video_listener_;
	}
}

void VIDMainState::Enter(CStateData* userData)
{
    RegisterEventListeners();
    Init();
}

void VIDMainState::Exit()
{
    UnregisterEventListeners();
    KillVideo();
    renderer_.TearDownDisplay();
}

void VIDMainState::Suspend()
{
    UnregisterEventListeners();
    PauseVideo();
    renderer_.TearDownDisplay();
}

void VIDMainState::Resume()
{
    RegisterEventListeners();
    ResumeVideo();
    renderer_.InitDisplay();
}

void VIDMainState::Update(CGameStateHandler* sh)
{
    //check for physical button events
    if (DoKeyEventLoop() == 1) return;
}

int VIDMainState::DoKeyEventLoop(void)
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
        // check for which button is pressed and respond accordingly
        // for all Video, highlight the indicator on the screen
        if (button_state & LeapFrog::Brio::kButtonMenu)
        {
            CAppManager::Instance()->PopApp();
            return 1;
        }
    }
    return 0;
}

void VIDMainState::PauseVideo()
{
	if (video_handle_ != LeapFrog::Brio::kInvalidVideoHndl &&
		video_mpi_.IsVideoPlaying(video_handle_))
	{
		video_mpi_.PauseVideo(video_handle_);
	}
}

void VIDMainState::ResumeVideo()
{
	if (video_handle_ != LeapFrog::Brio::kInvalidVideoHndl &&
		video_mpi_.IsVideoPaused(video_handle_))
	{
		video_mpi_.ResumeVideo(video_handle_);
	}
}

void VIDMainState::StopVideo()
{
	if (video_handle_ != LeapFrog::Brio::kInvalidVideoHndl)
	{
		video_mpi_.StopVideo(video_handle_);
		video_handle_ = LeapFrog::Brio::kInvalidVideoHndl;
	}
}

void VIDMainState::KillVideo(void)
{
	StopVideo();
	if (video_display_handle_ != NULL)
	{
		CDisplayMPI display_mpi;
		display_mpi.UnRegister(video_display_handle_, 0);
		display_mpi.DestroyHandle(video_display_handle_, false);

		video_display_handle_ = NULL;
	}

}

void VIDMainState::HandleVideoDonePlaying(LeapFrog::Brio::tVideoHndl video_handle)
{
	StopVideo();
}

VIDInitializationError VIDMainState::SetupVideo(void)
{
    const LeapFrog::Brio::tDisplayScreenStats* screen_stats = renderer_.GetScreenStats();
	const LeapFrog::Brio::tPixelFormat format = LeapFrog::Brio::kPixelFormatYUV420;
	// Create display surface for video renderingdisplayHandle
	LeapFrog::Brio::CDisplayMPI* display_mpi = renderer_.GetDisplayManager();
	video_display_handle_ = display_mpi->CreateHandle(screen_stats->height, screen_stats->width, format, NULL);

	// set up surface for later use by video manager
	unsigned char* buffer = display_mpi->GetBuffer(video_display_handle_);
	int pitch = display_mpi->GetPitch(video_display_handle_);

	// full screen video size for LeapsterGS
	// change variable name to target_width and target_height
	// set these to the actual display dimensions
	U16 target_width = screen_stats->width;
	U16 target_height = screen_stats->height;

	float aspect_ratio = static_cast<float>(target_width)/static_cast<float>(target_height);

	// Check aspect ratio, pick the video that closely matches the ratio stretch to match screen size.
    if ((aspect_ratio > kVID4_3AspectRatioMin) && (aspect_ratio < kVID4_3AspectRatioMax))
    {
    	video_path_ = CSystemData::Instance()->GetCurrentTitlePath()+"videos/320x240_Example-12fps_a2v6.ogg";
    }
    else
    {
    	// The standard for LeapPad platform delivery is half the native frame rate video set
    	// to video compression quality 6 with either 364x272 anamorphic (default) or
    	// 320x240 anamorphic for all full screen content. As an exception, live-action pieces
    	// may require further compression to 320x240 anamorphic.
    	video_path_ = CSystemData::Instance()->GetCurrentTitlePath()+"videos/480x272-Example-12fps-364x272-Anamorphic_a2v6.ogg";
    }

    // perform basic error checking
	if (!BaseUtils::Instance()->FileExists(video_path_))
	{
        return kVIDError;
	}

    // create the video surface to play the video on
	LeapFrog::Brio::tVideoSurf temp = { target_width, target_height, pitch, buffer, format };
	surface_ = temp;
    // set the on screen position of the video
    // in this case it is centered vertically and horizontally
    display_mpi->SetWindowPosition(video_display_handle_, (renderer_.Width()-target_width)/2, (renderer_.Height()-target_height)/2, target_width, target_height, true);

	// register video display with the display manager
    // Be sure to use kDisplayOnTop if you don't need to overlay any graphics on top of the video.  If you are not
    // stretching the video to full screen and want to have curtains to fill the letterbox, use kDisplayOnBottom
	if (kNoErr != display_mpi->Register(video_display_handle_, 0, 0, LeapFrog::Brio::kDisplayOnTop, 0))
	{
        return kVIDError;
	}

	return kVIDNoError;
}

void VIDMainState::Init(void)
{
	renderer_.InitDisplay();

	// exit if video setup is unsuccessful
	if (SetupVideo() != kVIDNoError)
	{
		// error, kill app and exit
        CAppManager::Instance()->PopApp();
        return;
	}

	// as long as there is a valid video handle, play the video
    if (video_handle_ == LeapFrog::Brio::kInvalidVideoHndl)
    {
    	// the audio is interleaved with the video so pass the path to the video file
    	// for the audio file path (parameter 2)
    	// add event listener to detect end of video play
    	bool isLooping = false;
    	video_listener_ = new VIDVideoListener(this);
    	video_handle_ = video_mpi_.StartVideo(video_path_, video_path_, &surface_, isLooping, video_listener_);
    }
}

void VIDMainState::RegisterEventListeners(void)
{
    //register keys
    LeapFrog::Brio::CButtonMPI key_event_mpi;
    key_event_mpi.RegisterEventListener(&button_event_queue_);
    key_event_mpi.SetTouchMode( LeapFrog::Brio::kTouchModeDefault );
}

void VIDMainState::UnregisterEventListeners(void)
{
    // stop listening for physical button events
    LeapFrog::Brio::CButtonMPI key_event_mpi;
    key_event_mpi.UnregisterEventListener(&button_event_queue_);
}
