#ifndef __CONTROLLER_INCLUDE_CONTROLLERAPP_H__
#define __CONTROLLER_INCLUDE_CONTROLLERAPP_H__

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file ControllerApp.h
*
* The ControllerApp demonstrates how to use the controller clss for
* controller based games
* devices
**
* author:  leapfrog
*            alindblad - 2/7/14 - created
*
******************************************************************************/

#include <AppInterface.h>
#include <KeyboardManager.h>
#include <GameStateHandler.h>

/*
 *  ControllerApp class.
 *
 *  The ControllerApp class is a direct copy of the STUBApp class.  It
 *  serves as an example of the basic structure and minimum methods and
 *  member variables necessary for an application.
 *
 */
class ControllerApp : public CAppInterface
{
public:
  ControllerApp(void *userData);
  ~ControllerApp(void);
  
  void Enter(void);
  void Update(void);
  void Exit(void);
  
  void Suspend(void);
  void Resume(void);
  
  static void RequestNewState(CGameStateBase* state);
  
 protected:
  CGameStateHandler * state_handler_; ///< manages the state of the app
  static bool new_state_; ///< tracks whether there has been a state change
  static CGameStateBase *new_state_requested_; ///< pointer to the up-coming state object
  
 private:
  /*
   *  ButtonsApp copy constructor
   *
   *	Private and unimplemented to prevent copying
   */
  ControllerApp(const ControllerApp&);
  
  /*
   *  operator =
   *
   *	Private and unimplemented to prevent copying
   */
  ControllerApp& operator =(const ControllerApp&);
};

#endif // __CONTROLLER_INCLUDE_CONTROLLERAPP_H__

