#ifndef BUTTON_BUTTONS_INCLUDE_BUTTONSAPP_H_
#define BUTTON_BUTTONS_INCLUDE_BUTTONSAPP_H_

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file ButtonsApp.h
*
* The Triggered Audio Sample App demonstrates the following capabilities:
*  * add a touchable button with multiple states
*  * play a single audio file based on a touch event trigger
**
* author: 	leapfrog
*  			alindblad - 10/22/12 - created copy from STUB app
*
******************************************************************************/

#include <AppInterface.h>
#include <GameStateHandler.h>
#include <DisplayMPI.h>

/*
 *  ButtonsApp class.
 *
 *  The ButtonsApp class is a direct copy of the STUBApp class.  It
 *  serves as an example of the basic structure and minimum methods and
 *  member variables necessary for an application.
 *
 */
class ButtonsApp : public CAppInterface {
 public:
    /*
     *  ButtonsApp constructor
     *
     *  Creates an app instance.
     *
     *  @param userData (TODO: research userData)
     */
    explicit ButtonsApp(void *userData);

    /*
     *  ButtonsApp destructor
     *
     *  Standard destructor, deallocates memory and cleanup of class object and members when the object is destroyed.
     *
     */
    ~ButtonsApp();

    /*
     * TODO: research Enter()
     *  This is the entry point into your application.  This is called when the app is
     *  instantiated.
     */
    void Enter();

    /*
     * TODO: research Update()
     */
    void Update();

    /*
     * TODO: research Exit()
     */
    void Exit();

    /*
     * TODO: research Suspend()
     */
    void Suspend();

    /*
     * TODO: research Resume()
     */
    void Resume();

    /*
     * TODO: research RequestNewState()
     * @param state TODO:research CGameStateBase
     */
    static void RequestNewState(CGameStateBase* state);

 protected:
    CGameStateHandler *          state_handler_;        ///< TODO: research state_hander roll

    static bool                  new_state_;            ///< tracks whether there has been a state change
    static CGameStateBase*       new_state_requested_;  ///< pointer to the up-coming state object
    static CGameStateBase*       current_state_;        ///< pointer to the current state object

    LeapFrog::Brio::CDisplayMPI  display_mpi_;          ///< used to create the display handle
    LeapFrog::Brio::tDisplayHandle display_handle_;     ///< used for displaying buttons
 private:
    //TODO: is there any reason we would want to copy the app instance?
	/*
	 *  ButtonsApp copy constructor
	 *
	 *	Private and unimplemented to prevent copying
	 */
    ButtonsApp(const ButtonsApp&);

    /*
	 *  operator =
	 *
	 *	Private and unimplemented to prevent copying
	 */
    ButtonsApp& operator =(const ButtonsApp&);
};


#endif  // BUTTON_BUTTONS_INCLUDE_BUTTONSAPP_H_

