#ifndef ACCELEROMETER_ACC_INCLUDE_ACCAPP_H_
#define ACCELEROMETER_ACC_INCLUDE_ACCAPP_H_

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file AccelerometerApp.h
*
* The Accelerometer app demonstrates how to use the accelerometer on the LeapPad
* devices
**
* author:  leapfrog
*             alindblad - 2/13/13 - created
*
******************************************************************************/

#include <AppInterface.h>
#include <KeyboardManager.h>
#include <GameStateHandler.h>

/*
 *  AccelerometerApp class.
 *
 *  The AccelerometerApp class is a direct copy of the STUBApp class.  It
 *  serves as an example of the basic structure and minimum methods and
 *  member variables necessary for an application.
 *
 */
class AccelerometerApp : public CAppInterface
{
public:
  AccelerometerApp(void *userData);
  ~AccelerometerApp();
  
  /*!
   * This is called by the AppManager when the game or title is 
   * launched.  It is recommended that any resource allocation 
   * should be done here rather then in the constructor.
   */
  void Enter(void);
  
  /*!
   * The AppManager calls this function frequently during the life of 
   * the game.  This provides a tick or heart-beat to the game.  
   * Any calls to CAppManager::PushApp() should be called from this 
   * method, or functions called by Update().
   */
  void Update(void);
  
  /*!
   * The AppManager calls this function upon shutting down the game.  
   * The game should handle all logging first, before any other saving 
   * or clean-up.  This will help insulate against problems in the log 
   * files resulting from crashes while saving game data or attempting 
   * to clean up.
   */
  void Exit(void);
  
  /*!
   * Called by AppManager to pause the app and load another state on 
   * the app stack such as a tutorial or the pause screen.
   */
  void Suspend(void);
  
  /*!
   * Called by AppManager when returning to the app from another state 
   * such as a tutorial or the pause screen.
   */
  void Resume(void);
  
  static void RequestNewState(CGameStateBase* state);
  
 protected:
  // manages the state of the app
  CGameStateHandler *stateHandler_; 
  
  // tracks whether there has been a state change
  static bool newState_; 

  // pointer to the up-coming state object
  static CGameStateBase *newStateRequested_; 

  // pointer to the current state object
  static CGameStateBase *currentState_; 
  
 private:
  /*!
   * Explicitly disalbe copy semantics

   */
  AccelerometerApp(const AccelerometerApp&);
  AccelerometerApp& operator =(const AccelerometerApp&);};


#endif // ACCELEROMETER_ACC_INCLUDE_ACCAPP_H_

