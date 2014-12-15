#ifndef BUTTON_BUTTONS_INCLUDE_BTNMAINSTATE_H_
#define BUTTON_BUTTONS_INCLUDE_BTNMAINSTATE_H_

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file BTNMainState.h
*
* The BTNMainState class is responsible for handling all button and touch events,
* creating the touch button and playing the audio file.  BTNMainState is the
* only state in which the ButtonsApp can take on.
*
**
* author: 	leapfrog
*  			alindblad - 10/22/12 - Created copy from STUB app
*
******************************************************************************/

#include <GameStateBase.h>
#include <TouchEventQueue.h>
#include <ButtonEventQueue.h>
#include "Common/LFTouchButton.h" // helper class for handling touch buttons
#include "Common/LFDisplayRenderer.h"
#include <ButtonMPI.h>

/*
 *  BTNMainState class.
 *
 * The only state of the Buttons app.  This class is responsible
 * for creating the touch button, handling touch and button events.
 */
class BTNMainState : public CGameStateBase, public LFTouchButtonDelegate {
 public:
	/*
	 *  ButtonsApp default constructor
	 *
	 *  Creates an app instance.
	 */
    BTNMainState(void);

	/*
	 *  ButtonsApp destructor
	 *
	 *  performs class cleanup, i.e.frees all memory allocated
	 */
    ~BTNMainState();

	/*
	 * TODO: research Suspend()
	 */
    virtual void Suspend();

	/*
	 * TODO: research Resume()
	 */
    virtual void Resume();

	/*
	 * TODO: research Enter()
	 */
    virtual void Enter(CStateData* userData);

	/*
	 * TODO: research Update()
	 */
    virtual void Update(CGameStateHandler* sh);

	/*
	 * TODO: research Exit()
	 */
    virtual void Exit();

 protected:
	/*
	 * DoTouchEventLoop
	 *
	 * Checks for any touch events and then handles an event appropriately
	 */
    int DoTouchEventLoop(void);

	/*
	 * DoKeyEventLoop
	 *
	 * Checks for any physial button events and then handles an event appropriately
	 * Physical buttons are the native buttons to the device, i.e. the home button
	 */
    int DoKeyEventLoop(void);

    LeapFrog::Brio::CTouchEventQueue    touch_event_queue_;	  ///< TODO: research the touch event queue
    LeapFrog::Brio::CButtonEventQueue   button_event_queue_;  ///< TODO: research the button event queue

    LFTouchButton                       touch_button_;        ///< the touch button used to trigger audio
    std::vector<LFTouchButton*>         indicator_buttons_;   ///< icons to indicate which physical buttons are pressed
    std::map<U32,U16>                   indicator_index_;     ///< index in to indicator button vector
    std::map<U32,LeapFrog::Brio::CPath> dpad_image_path_;

    LFDisplayRenderer                   display_renderer_;    ///< manages display buffers

    LeapFrog::Brio::CButtonMPI          mpi_Button_;

    tDpadOrientation					dpad_orientation_;

 private:
	/*
	 * init
	 *
	 * A helper method that performs all initialization required by
	 * the touch button and audio used in this example.
	 */
    void Init(void);

    /*
	 * addTouchButton
	 *
	 * A helper method that creates the touch button for
	 * rendering on the screen
	 */
    void AddTouchButton(void);

	/*
	 * MoveTouchButton
	 *
	 * A helper method that moves an onscreen virtual button based on
	 * input from the DPAD.
	 */
    void MoveTouchButton(U16 direction);

	/*
	 * AddIndicatorButtons
	 *
	 * A helper method that adds onscreen indicators for each
	 * physical button that is present on the device.
	 */
    void AddIndicatorButtons(void);

	/*
	 * HighlightIndicators
	 *
	 * A helper method that highlights or un-highlights an indicator
	 * button based on the flag passed in
	 */
    void HighlightIndicator(U32 ke, bool highlight);

	/*
	 * RenderIndicators
	 *
	 * A helper method that displays the touch buttons being
	 * used as physical button indicators.
	 */
    void RenderIndicators(CBlitBuffer & buffer);

	/*
	 * registerEventListeners
	 *
	 * A helper method that registers listeners for screen touch and
	 * physical button events.
	 */
    void RegisterEventListeners(void);

	/*
	 * unregisterEventListeners
	 *
	 * A helper method that unregisters the listeners that were
	 * registered via a call to registerEventListeners
	 */
    void UnregisterEventListeners(void);

	/*
	 * HandleTouchEvent
	 *
	 * This method is called when the touch button event occurs and handles
	 * playing the audio file.
	 */
    void HandleTouchEvent(unsigned int id);
};

#endif  // BUTTON_BUTTONS_INCLUDE_BTNMAINSTATE_H_

