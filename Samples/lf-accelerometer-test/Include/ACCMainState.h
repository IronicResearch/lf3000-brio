#ifndef ACCELEROMETER_ACC_INCLUDE_ACCMAINSTATE_H_
#define ACCELEROMETER_ACC_INCLUDE_ACCMAINSTATE_H_

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file ACCMainState.h
*
* The ACCMainState class is responsible for handling all button and touch events
* and accelerometer readings.  ACCMainState is the only state in which the AccelerometerApp
* can take on.
*
**
* author: 	leapfrog
*  			alindblad - 2/13/13 - Created
*
******************************************************************************/

#include <vector>
#include <GameStateHandler.h>

#include <ButtonMPI.h>
#include <EventMPI.h>
#include <DisplayMPI.h>
#include <AccelerometerMPI.h>

#include <GameStateBase.h>
#include <CText.h>
#include <TouchEventQueue.h>
#include <ButtonEventQueue.h>
#include <BrioOpenGLConfig.h>

static const int kAcceleration = 3;
static const float kLongestSide = 1.42f;
static const float kObjectSize = 20.0f;

struct ColorStruct {
  LeapFrog::Brio::S16 r_;
  LeapFrog::Brio::S16 g_;
  LeapFrog::Brio::S16 b_;
};

/*
 *  ACCMainState class.
 *
 * The only state of the accelerometer app.  This class is responsible
 * for creating the touch button, handling touch and button events.
 */
class ACCMainState : public CGameStateBase, public LeapFrog::Brio::IEventListener {
 public:
  /*
   * Instance
   *
   * The singleton access point
   */
  static ACCMainState* Instance();
  virtual void Suspend();
  virtual void Resume();
  virtual void Enter(CStateData* userData);
  virtual void Update(CGameStateHandler* sh);
  virtual void Exit();
  
 protected:
  ACCMainState();
  ~ACCMainState();
  
  virtual LeapFrog::Brio::tEventStatus Notify( const LeapFrog::Brio::IEventMessage &msgIn );
  
  LeapFrog::Brio::CAccelerometerMPI* accelerometerMPI_;  ///< accelerometer manager
  
  int screenWidth_;
  int screenHeight_;

  LeapFrog::Brio::CTouchEventQueue    touchEventQueue_;	   ///< a queue of all touch events
  LeapFrog::Brio::CButtonEventQueue   buttonEventQueue_;   ///< a queue of all button events
  
  int x_, y_, z_;  ///< accelerometer data
  int gravity_;    ///< directional indicator for gravity
  bool touching_;  ///< an indication if the screen is touched
  
  LeapFrog::Brio::BrioOpenGLConfig *config_;  ///< configuration for OpenGL
  ColorStruct bgColor_; ///< background color
  
  float nearPlane_; ///< indicates the distance to the near plane (OGL)
  float	farPlane_;  ///< indicates the distance to the far plane (OGL)
  
  float xPos_, yPos_, zPos_; ///< position for pyramid indicator object
  float initialZ_;           ///< the initial z position of the object
  float dx_, dy_, dz_;       ///< incremental updates to the position
  int   rotation_;           ///< rotation of the pyramid indicator object
  
  bool accelerometerOn_;     ///< flag indicating if we are using the accelerometer
  
 private:
  /*
   * DoTouchEventLoop
   *
   * Checks for any touch events and then handles an event appropriately
   */
  int DoTouchEventLoop();
  
  /*
   * DoKeyEventLoop
   *
   * Checks for any physial button events and then handles an event appropriately
   * Physical buttons are the native buttons to the device, i.e. the home button
   */
  int DoKeyEventLoop();
  
  /*
   * DoAccEventLoop
   *
   * Updates the incremental change in position to the indicator object
   * based on the most current acceleration data
   */
  int DoAccEventLoop();
  
  /*
   * InitOGL
   *
   * Performs all necessary initialization and setup for OpenGL
   */
  void InitOGL();
  
  /*
   * SetClearColor
   *
   * A helper method that sets the OpenGL clear color
   */
  void SetClearColor(LeapFrog::Brio::S16 r, LeapFrog::Brio::S16 g, LeapFrog::Brio::S16 b, LeapFrog::Brio::S16 a);
  
  /*
   * RenderOGL
   *
   * Renders the pyramid based on accelerometer input affecting it's position
   */
  void RenderOGL();
  
  /*
   * ResetObject
   *
   * A helper method that moves the pyramid indicator back to it's reset position
   */
  void ResetObject();
  
  /*
   * MoveObjectWithGravity
   *
   * A helper method that moves the pyramid indicator based on the current
   * accelerometer data and the gravity of the system
   */
  void MoveObjectWithGravity();
  
  /*
   * MoveObjectIncrementally
   *
   * A helper method that moves the pyramid indicator based on the current
   * accelerometer data
   */
  void MoveObjectIncrementally();
  
  /*
   * init
   *
   * A helper method that performs all initialization required by
   * the touch button and audio used in this example.
   */
  void Init(void);
  
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



#endif // ACCELEROMETER_ACC_INCLUDE_ACCMAINSTATE_H_

