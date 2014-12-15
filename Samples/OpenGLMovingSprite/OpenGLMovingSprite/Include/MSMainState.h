#ifndef GRAPHICS_OPENGLMOVINGSPRITE_INCLUDE_MSMAINSTATE_H_
#define GRAPHICS_OPENGLMOVINGSPRITE_INCLUDE_MSMAINSTATE_H_

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file MSMainState.h
*
* The MSMainState class is responsible for handling all button and touch events,
* creating the touch button and playing the audio file.  MSMainState is the
* only state in which the OpenGLMovingSpriteApp can take on.
*
**
* author: 	leapfrog
*  			alindblad - 11/01/12 - Created copy from STUB app
*
******************************************************************************/

#include <GameStateBase.h>
#include <ButtonEventQueue.h>
#include <sys/time.h>
#include "MSSprite.h"

namespace LeapFrog {
namespace Brio {
    class BrioOpenGLConfig;
}
}

/*
 *  MSMainState class.
 *
 * The only state of the OpenGLMovingSprite app.  This class is responsible
 * for creating the touch button, handling touch and button events and
 * playing the audio file in response to the trigger of a touch event.
 */
class MSMainState : public CGameStateBase {
 public:
	/*
	 *  OpenGLMovingSpriteApp default constructor
	 *
	 *  Creates an app instance.
	 */
    MSMainState();

	/*
	 *  OpenGLMovingSpriteApp destructor
	 *
	 *  performs class cleanup, i.e.frees all memory allocated
	 */
    ~MSMainState();

	/*
	 * Enter is called when the state first loads.
	 * All initial setup should take place here
	 * Enter is only called once per instance
	 */
    virtual void Enter(CStateData* userData);

	/*
	 * Update is called on every tick
	 * This is the heartbeat of the program
	 */
    virtual void Update(CGameStateHandler* sh);

	/*
	 * Suspend is called when AppManager needs to pause the game
	 * This could be triggered by a system pause from the pause button,
	 * a low battery warning, or by pushing a new app onto the AppManager stack (ie tutorial)
	 */
   virtual void Suspend();

	/*
	 * Resume is called upon returning from the suspended state
	 * This could be triggered by a system un-pause from the pause button,
	 * returning from the low battery warning, or returning from another app that
	 * was previously launched from the AppManager stack
	 */
    virtual void Resume();

	/*
	 * Exit is called when the state is destroyed
	 * All cleanup takes place here
	 * Exit is only called once per instance
	 * This could be triggered by quitting the game, plugging in the USB, turning the power off, etc
	 * Exit would not be called in the case of a catastrophic failure, such as sudden loss of power
	 */
    virtual void Exit();

 protected:
	/*
	 * DoKeyEventLoop
	 *
	 * Checks for any physial button events and then handles an event appropriately
	 * Physical buttons are the native buttons to the device, i.e. the home button
	 */
    int DoKeyEventLoop(void);

    LeapFrog::Brio::CButtonEventQueue   button_event_queue_;   ///< a queue of all button events
    LeapFrog::Brio::BrioOpenGLConfig*   brio_opengl_config_;    ///< connects the egl context and displayMPI
    MSAnimation                         animation_;
    float                               frame_duration_;        ///< a fixed frame duration in seconds
    timespec                            previous_render_time_;  ///< used to determine when to render next frame

 private:
	/*
	 * init
	 *
	 * A helper method that performs all initialization required by
	 * the touch button and audio used in this example.
	 */
    void Init(void);

    /*
     * Determines if the next frame should render based on a prescribed frame
     * rate.
     */
    bool ShouldRenderNextFrame(void);

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
};

#endif  // GRAPHICS_OPENGLMOVINGSPRITE_INCLUDE_MSMAINSTATE_H_

