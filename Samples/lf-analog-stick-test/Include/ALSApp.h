#ifndef ANALOG_STICK_ALS_INCLUDE_ALSAPP_H_
#define ANALOG_STICK_ALS_INCLUDE_ALSAPP_H_

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file ALSApp.h
*
* The Accelerometer app demonstrates how to use the accelerometer on the LeapPad
* devices
**
* author: 	leapfrog
*  			alindblad - 2/13/13 - created
*
******************************************************************************/

#include <AppInterface.h>
#include <KeyboardManager.h>
#include <GameStateHandler.h>

/*
 *  ALSApp class.
 *
 *  The ALSApp class is a direct copy of the STUBApp class.  It
 *  serves as an example of the basic structure and minimum methods and
 *  member variables necessary for an application.
 *
 */
class ALSApp : public CAppInterface
{
public:
	ALSApp(void *userData);
	~ALSApp();

	void Enter();
	void Update();
	void Exit();

	void Suspend();
	void Resume();

	static void RequestNewState(CGameStateBase* state);

protected:
    CGameStateHandler *          state_handler_;        ///< manages the state of the app

    static bool                  new_state_;            ///< tracks whether there has been a state change
    static CGameStateBase*       new_state_requested_;  ///< pointer to the up-coming state object

private:
	/*
	 *  ButtonsApp copy constructor
	 *
	 *	Private and unimplemented to prevent copying
	 */
   ALSApp(const ALSApp&);

   /*
	 *  operator =
	 *
	 *	Private and unimplemented to prevent copying
	 */
   ALSApp& operator =(const ALSApp&);};


#endif // ANALOG_STICK_ALS_INCLUDE_ALSAPP_H_

