#ifndef __CAMERAAPP_H__
#define __CAMERAAPP_H__

/******************************************************************************
* Copyright 2014(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file CameraApp.h
*
* The Camera sample app demonstrates the following:
*  * how to setup and use the camera
*  * how to start a video capture session
*  * how to render frames from a video capture session in OpenGL
*
**
* author:  leapfrog
*             alindblad - 1/3/14 - created copy from STUB app
*
******************************************************************************/

#include <AppInterface.h>
#include <GameStateHandler.h>

/*
 *  CameraApp class.
 *
 *  The CameraApp class is a direct copy of the STUBApp class.  It
 *  serves as an example of the basic structure and minimum methods and
 *  member variables necessary for an application.
 *
 */
class CameraApp : public CAppInterface {
public:
  /*!
   * Constructor
   * Creates an app instance.  Instead of allocating resources here
   * do it in the Enter() method.
   */
  CameraApp(void *userData);
  
  /*!
   * Destructor
   */
  ~CameraApp(void);

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
};


#endif // __CAMERAAPP_H__

