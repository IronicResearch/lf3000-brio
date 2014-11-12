#include "BTNMainState.h"
#include <AppManager.h>
#include <Utility.h>

static const U16 kButtonHeight = 75; ///< height of the touch button
static const U16 kButtonWidth = 75;  ///< width of the touch button
static const U16 kIndicatorHeight = 60;
static const U16 kIndicatorWidth = 80;
static const U16 kASize = 34;
static const U16 kBSize = 21;
static const U16 kDeltaX = 5;
static const U16 kDeltaY = 5;

// do not initialize anything that accesses the hardware in the constructor do
// it in the Enter method.  Rule of thumb is to not initialize member variables
// in States or App classes.
BTNMainState::BTNMainState(void)
{
}

BTNMainState::~BTNMainState()
{
}

void BTNMainState::Enter(CStateData* userData)
{
    RegisterEventListeners();
    Init();
}

void BTNMainState::Exit()
{
    UnregisterEventListeners();
    display_renderer_.TearDownDisplay();
    std::vector<LFTouchButton*>::iterator indIt = indicator_buttons_.begin();
    std::vector<LFTouchButton*>::const_iterator indItEnd = indicator_buttons_.end();

    for ( ; indIt != indItEnd; ++indIt)
    {
    	delete (*indIt);
    }
    indicator_buttons_.clear();
}

void BTNMainState::Suspend()
{
    UnregisterEventListeners();
    display_renderer_.TearDownDisplay();
}

void BTNMainState::Resume()
{
    RegisterEventListeners();
    display_renderer_.InitDisplay();
}

void BTNMainState::Update(CGameStateHandler* sh)
{
    //check for screen touch events
    DoTouchEventLoop();

    //check for physical button events
    if (DoKeyEventLoop() == 1) return;

    display_renderer_.SetupFrameRender();

    RenderIndicators(display_renderer_.CurrentBuffer());
    touch_button_.render(display_renderer_.CurrentBuffer());

    // swap the buffers for smooth rendering
    display_renderer_.SwapBuffers();
}

int BTNMainState::DoKeyEventLoop(void)
{
    std::vector<LeapFrog::Brio::tButtonData2> *button_event_queue = button_event_queue_.GetQueue();
    std::vector<LeapFrog::Brio::tButtonData2>::const_iterator button_reader = button_event_queue->begin();

    // CButtonMPI GetButtonState give full state of all buttons, potentially useful on a Resume or an Enter

    // iterate over all events in the button event queue
    while(button_reader != button_event_queue->end())
    {
        static U32 button_state = 0;
        static U32 button_transition = 0;
        LeapFrog::Brio::tButtonData2 button_data = *(button_reader++);
        button_state = button_data.buttonState;
        button_transition = button_data.buttonTransition;

        /*
         * button states:
         * kButtonMenu, kButtonHint
         * kButtonLeft, kButtonRight, kButtonUp, kButtonDown,
         * kButtonLeftShoulder, kButtonRightShoulder,
         * kButtonA, kButtonB
         */
        // check for which button is pressed and respond accordingly
        // for all buttons, highlight the indicator on the screen
        if (button_transition & kButtonMenu)
        {
        	HighlightIndicator(kButtonMenu, button_state & kButtonMenu);
            CAppManager::Instance()->PopApp();
            return 1;
        }
        else if (button_transition & kButtonLeft)
        {
        	U32 key = kButtonLeft;
        	HighlightIndicator(key, button_state & kButtonLeft);
        	indicator_buttons_[indicator_index_[key]]->setHighlightImage(dpad_image_path_[key]);
        	// only move the touch button on button down events
        	if (button_state & kButtonLeft)
        	{
        		MoveTouchButton(button_state);
        	}
        }
        else if (button_transition & kButtonRight)
        {
        	U32 key = kButtonRight;
        	HighlightIndicator(key, button_state & kButtonRight);
        	indicator_buttons_[indicator_index_[key]]->setHighlightImage(dpad_image_path_[key]);
        	if (button_state & kButtonRight)
        	{
        		MoveTouchButton(button_state);
        	}
        }
        else if (button_transition & kButtonUp)
        {
        	U32 key = kButtonUp;
        	HighlightIndicator(key, button_state & kButtonUp);
        	indicator_buttons_[indicator_index_[key]]->setHighlightImage(dpad_image_path_[key]);
        	if (button_state & kButtonUp)
        	{
        		MoveTouchButton(button_state);
        	}
        }
        else if (button_transition & kButtonDown)
        {
        	U32 key = kButtonDown;
        	HighlightIndicator(key, button_state & kButtonDown);
        	indicator_buttons_[indicator_index_[key]]->setHighlightImage(dpad_image_path_[key]);
        	if (button_state & kButtonDown)
        	{
        		MoveTouchButton(button_state);
        	}
        }
        else if (button_transition & kButtonLeftShoulder)
        {
        	HighlightIndicator(kButtonLeftShoulder, button_state & kButtonLeftShoulder);
        }
        else if (button_transition & kButtonRightShoulder)
        {
        	HighlightIndicator(kButtonRightShoulder, button_state & kButtonRightShoulder);
        }
        else if (button_transition & kButtonA)
        {
        	HighlightIndicator(kButtonA, button_state & kButtonA);
        }
        else if (button_transition & kButtonB)
        {
        	HighlightIndicator(kButtonB, button_state & kButtonB);
        }
        else if (button_transition & kButtonPause)
        {
        	HighlightIndicator(kButtonPause, button_state & kButtonPause);
        }
        else if (button_transition & kButtonHint)
        {
        	HighlightIndicator(kButtonHint, button_state & kButtonHint);
        }
    }
    return 0;
}

int BTNMainState::DoTouchEventLoop(void)
{
    std::vector<LeapFrog::Brio::tTouchData> *touch_event_queue = touch_event_queue_.GetQueue();

    std::vector<LeapFrog::Brio::tTouchData>::const_iterator touch_reader = touch_event_queue->begin();

    // iterate through all touch events in the touch event queue
    while(touch_reader != touch_event_queue->end())
    {
        static U16 touch_state = 0;
        LeapFrog::Brio::tTouchData touch_data = *(touch_reader++);

        //get touch_state:
        //0:	up        	{

        //1:	down
        touch_state = touch_data.touchState;

        //check for button press
        int pen_x = touch_data.touchX;
        int pen_y = touch_data.touchY;

        touch_button_.checkForButtonTouchEvent(touch_state, pen_x, pen_y);
    }
    return 0;
}

void BTNMainState::HandleTouchEvent(unsigned int id)
{
    // move button back to center of the screen
	const tDisplayScreenStats* screen_stats = display_renderer_.GetScreenStats();
    touch_button_.x_ = (screen_stats->width-kButtonWidth)/2;
    touch_button_.y_ = ((screen_stats->height-kButtonHeight)/2);
}

void BTNMainState::MoveTouchButton(U16 direction)
{
    // the LeapPad2 screen is in landscape mode, hence the following mapping exists
    // left -> down
    // right -> up
    // down -> left
    // up -> right
    U16 x = touch_button_.x_;
    U16 y = touch_button_.y_;
    U16 width  = display_renderer_.GetScreenStats()->width;
    U16 height = display_renderer_.GetScreenStats()->height;

    if (direction & kButtonLeft)
    {
    	touch_button_.x_ = (x < kDeltaX) ? 0 : x - kDeltaX;
    } else if (direction & kButtonRight)
    {
    	touch_button_.x_ = (x > (width-kButtonWidth-kDeltaX)) ? width-kButtonWidth : x + kDeltaX;
    } else if (direction & kButtonUp)
    {
    	touch_button_.y_ = (y < kDeltaY) ? 0 : y - kDeltaY;
    } else if (direction & kButtonDown)
    {
    	touch_button_.y_ = (y > (height-kButtonHeight-kDeltaY)) ? height-kButtonHeight : y + kDeltaY;
    }
}

void BTNMainState::AddTouchButton(void)
{
	const tDisplayScreenStats* screen_stats = display_renderer_.GetScreenStats();
    // create the touch button
    touch_button_.createButton(
        CSystemData::Instance()->GetCurrentTitlePath()+"images/button.png",
        CSystemData::Instance()->GetCurrentTitlePath()+"images/buttonHigh.png",
        (screen_stats->width-kButtonWidth)/2,
        ((screen_stats->height-kButtonHeight)/2),
        kButtonWidth,
        kButtonHeight,
        true,
        this);
}

void BTNMainState::AddIndicatorButtons(void)
{
	// check for the existence of physical buttons
	// if the device has the button add a touch button to the screen
	// that will allow for displaying when the physical button is pressed.
	// The touch buttons are only used as indicators in this example.
	// Add an entry to the map, indicator_index_, that maps the physical button
	// enumeration value to the index in the touch button array for quicker access
	// to the touch button.
	bool is_active = true;
	const tDisplayScreenStats* screen_stats = display_renderer_.GetScreenStats();
	// The explorer devices are responsible for acting on the home button being pressed.
	// The LeapPad2 the system is responsible for handling the home button being pressed.
	if (HasPlatformCapability(kCapsButtonMenu))
	{
		LFTouchButton* button = new LFTouchButton();
	    button->createButton(
	        CSystemData::Instance()->GetCurrentTitlePath()+"images/home.png",
	        CSystemData::Instance()->GetCurrentTitlePath()+"images/homeHigh.png",
	        (screen_stats->width-kIndicatorWidth)/2,
	        screen_stats->height-kIndicatorHeight,
	        kIndicatorWidth,
	        kIndicatorHeight,
	        is_active);
	    indicator_buttons_.push_back(button);
	    indicator_index_[kButtonMenu] = indicator_buttons_.size()-1;
	}
	// Currently we test for the DPAD existence by just examining one of the directional
	// buttons on the DPAD.  In this case if the Up button is present we assume the
	// entire DPAD (left, right, up down) is present
	if (HasPlatformCapability(kCapsButtonUp))
	{
		LFTouchButton* button = new LFTouchButton();
		button->createButton(
			CSystemData::Instance()->GetCurrentTitlePath()+"images/dpad.png",
			CSystemData::Instance()->GetCurrentTitlePath()+"images/dpad.png",
			0,
			screen_stats->height-kIndicatorHeight,
			kIndicatorWidth,
			kIndicatorHeight,
			is_active);
		indicator_buttons_.push_back(button);
		indicator_index_[kButtonLeft]  = indicator_buttons_.size()-1;
		indicator_index_[kButtonRight] = indicator_buttons_.size()-1;
		indicator_index_[kButtonUp]    = indicator_buttons_.size()-1;
		indicator_index_[kButtonDown]  = indicator_buttons_.size()-1;
		dpad_image_path_[kButtonLeft]  = CSystemData::Instance()->GetCurrentTitlePath()+"images/dpadLeft.png";
		dpad_image_path_[kButtonRight] = CSystemData::Instance()->GetCurrentTitlePath()+"images/dpadRight.png";
		dpad_image_path_[kButtonUp]    = CSystemData::Instance()->GetCurrentTitlePath()+"images/dpadUp.png";
		dpad_image_path_[kButtonDown]  = CSystemData::Instance()->GetCurrentTitlePath()+"images/dpadDown.png";
	}
	if (HasPlatformCapability(kCapsButtonLeftShoulder))
	{
		LFTouchButton* button = new LFTouchButton();
		button->createButton(
			CSystemData::Instance()->GetCurrentTitlePath()+"images/left.png",
			CSystemData::Instance()->GetCurrentTitlePath()+"images/leftHigh.png",
			0,
			0,
			kIndicatorWidth,
			kIndicatorHeight,
			is_active);
		indicator_buttons_.push_back(button);
	    indicator_index_[kButtonLeftShoulder] = indicator_buttons_.size()-1;
	}
	if (HasPlatformCapability(kCapsButtonRightShoulder))
	{
		LFTouchButton* button = new LFTouchButton();
		button->createButton(
			CSystemData::Instance()->GetCurrentTitlePath()+"images/right.png",
			CSystemData::Instance()->GetCurrentTitlePath()+"images/rightHigh.png",
			screen_stats->width-kIndicatorWidth,
			0,
			kIndicatorWidth,
			kIndicatorHeight,
			is_active);
		indicator_buttons_.push_back(button);
	    indicator_index_[kButtonRightShoulder] = indicator_buttons_.size()-1;
	}
	if (HasPlatformCapability(kCapsButtonA))
	{
		LFTouchButton* button = new LFTouchButton();
		button->createButton(
			CSystemData::Instance()->GetCurrentTitlePath()+"images/a.png",
			CSystemData::Instance()->GetCurrentTitlePath()+"images/aHigh.png",
			screen_stats->width-kIndicatorWidth,
	        screen_stats->height-kIndicatorHeight,
			kIndicatorWidth,
			kIndicatorHeight,
			is_active);
		indicator_buttons_.push_back(button);
	    indicator_index_[kButtonA] = indicator_buttons_.size()-1;
	}
	if (HasPlatformCapability(kCapsButtonB))
	{
		LFTouchButton* button = new LFTouchButton();
		button->createButton(
			CSystemData::Instance()->GetCurrentTitlePath()+"images/b.png",
			CSystemData::Instance()->GetCurrentTitlePath()+"images/bHigh.png",
			screen_stats->width-kBSize,
	        screen_stats->height-kIndicatorHeight,
			kIndicatorWidth,
			kIndicatorHeight,
			is_active);
		indicator_buttons_.push_back(button);
	    indicator_index_[kButtonB] = indicator_buttons_.size()-1;
	}
	if (HasPlatformCapability(kCapsButtonPause))
	{
		LFTouchButton* button = new LFTouchButton();
		button->createButton(
			CSystemData::Instance()->GetCurrentTitlePath()+"images/pause.png",
			CSystemData::Instance()->GetCurrentTitlePath()+"images/pauseHigh.png",
			screen_stats->width-kIndicatorWidth,
			(screen_stats->height-kIndicatorHeight)/2,
			kIndicatorWidth,
			kIndicatorHeight,
			is_active);
		indicator_buttons_.push_back(button);
		indicator_index_[kButtonPause] = indicator_buttons_.size()-1;
	}
	if (HasPlatformCapability(kCapsButtonHint))
	{
		LFTouchButton* button = new LFTouchButton();
		button->createButton(
			CSystemData::Instance()->GetCurrentTitlePath()+"images/questionMark.png",
			CSystemData::Instance()->GetCurrentTitlePath()+"images/questionMarkHigh.png",
			0,
			(screen_stats->height-kIndicatorHeight)/2,
			kIndicatorWidth,
			kIndicatorHeight,
			is_active);
		indicator_buttons_.push_back(button);
		indicator_index_[kButtonHint] = indicator_buttons_.size()-1;
	}
}

void BTNMainState::RenderIndicators(CBlitBuffer& buffer)
{
	std::vector<LFTouchButton*>::iterator indIt = indicator_buttons_.begin();
	std::vector<LFTouchButton*>::const_iterator indItEnd = indicator_buttons_.end();

	for ( ; indIt != indItEnd; ++indIt)
	{
		(*indIt)->render(buffer);
	}
}

void BTNMainState::HighlightIndicator(U32 key, bool highlight)
{
	// check to make sure the key's indicator icon was added to the display
	if (indicator_index_.count(key))
	{
		if (highlight)
			indicator_buttons_[indicator_index_[key]]->highlight();
		else
			indicator_buttons_[indicator_index_[key]]->normal();
	}
}

void BTNMainState::Init(void)
{
	//checks to see default device orientation
	dpad_orientation_ = mpi_Button_.GetDpadOrientation();

	//if orientation is not landscape, we set the orientation to landscape
	if(dpad_orientation_ != kDpadLandscape)
	{
		tErrType orientation_changed_ = mpi_Button_.SetDpadOrientation(kDpadLandscape);
	}

    display_renderer_.InitDisplay();
    AddTouchButton();
    AddIndicatorButtons();
}

void BTNMainState::RegisterEventListeners(void)
{
    //register touch screen
    LeapFrog::Brio::CEventMPI event_mpi;
    event_mpi.RegisterEventListener(&touch_event_queue_);

    //register keys
    LeapFrog::Brio::CButtonMPI key_event_mpi;
    key_event_mpi.RegisterEventListener(&button_event_queue_);
    key_event_mpi.SetTouchMode( kTouchModeDefault );
}

void BTNMainState::UnregisterEventListeners(void)
{
    // stop listening for screen touch events
    LeapFrog::Brio::CEventMPI event_mpi;
    event_mpi.UnregisterEventListener(&touch_event_queue_);

    // stop listening for physical button events
    LeapFrog::Brio::CButtonMPI key_event_mpi;
    key_event_mpi.UnregisterEventListener(&button_event_queue_);
}
