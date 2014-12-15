#ifndef ACCELEROMETER_ACC_INCLUDE_ACCAPP_H_
#define ACCELEROMETER_ACC_INCLUDE_ACCAPP_H_

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file ACCApp.h
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
 *  ACCApp class.
 *
 *  The ACCApp class is a direct copy of the STUBApp class.  It
 *  serves as an example of the basic structure and minimum methods and
 *  member variables necessary for an application.
 *
 */
class ACCApp : public CAppInterface
{
public:
	ACCApp(void *userData);
	~ACCApp();

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
   ACCApp(const ACCApp&);

   /*
	 *  operator =
	 *
	 *	Private and unimplemented to prevent copying
	 */
   ACCApp& operator =(const ACCApp&);};


#endif // ACCELEROMETER_ACC_INCLUDE_ACCAPP_H_

