#ifndef __CONTROLLER_INCLUDE_CTRLMAINSTATE_H__
#define __CONTROLLER_INCLUDE_CTRLMAINSTATE_H__

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file CTRLMainState.h
*
* The CTRLMainState class is responsible for handling all button and touch events
* and accelerometer readings.  CTRLMainState is the only state in which the CTRLApp
* can take on.
*
**
* author: leapfrog
*           alindblad - 2/13/13 - Created
*
******************************************************************************/

#include <vector>
#include <GameStateHandler.h>

#include <ButtonMPI.h>
#include <EventMPI.h>
#include <DisplayMPI.h>
#include <Hardware/HWControllerMPI.h>

#include <GameStateBase.h>
#include <CText.h>
#include <TouchEventQueue.h>
#include <ButtonEventQueue.h>
#include <RTC.h>
#include <BrioOpenGLConfig.h>

#define MAX_CONTROLLERS 2

static const int kAcceleration = 3;
static const float kLongestSide = 1.42f;
static const float kObjectSize = 20.0f;

struct ColorStruct {
  LeapFrog::Brio::S16 r_;
  LeapFrog::Brio::S16 g_;
  LeapFrog::Brio::S16 b_;
};

/*
 *  CTRLMainState class.
 *
 * The only state of the controller app.  This class is responsible
 * for creating the touch button, handling touch and button events.
 */
class CTRLMainState : public CGameStateBase, public LeapFrog::Brio::IEventListener {
 public:
  /*
   * Instance
   *
   * The singleton access point
   */
  static CTRLMainState* Instance(void);
  virtual void Suspend(void);
  virtual void Resume(void);
  virtual void Enter(CStateData* userData);
  virtual void Update(CGameStateHandler* sh);
  virtual void Exit(void);

  friend class ControllerApp;
  
 protected:
  CTRLMainState(void);
  ~CTRLMainState(void);
  
  virtual LeapFrog::Brio::tEventStatus Notify( const LeapFrog::Brio::IEventMessage &msgIn );
  
  LF::Hardware::HWControllerMPI controller_mpi_;
  
  int screen_width_;
  int screen_height_;
  
  LeapFrog::Brio::CTouchEventQueue touch_event_queue_; ///< a queue of all touch events
  LeapFrog::Brio::CButtonEventQueue button_event_queue_; ///< a queue of all button events
  
  
  LeapFrog::Brio::BrioOpenGLConfig *config_; ///< configuration for OpenGL
  ColorStruct bg_color_; ///< background color
  
  float near_plane_; ///< indicates the distance to the near plane (OGL)
  float far_plane_;  ///< indicates the distance to the far plane (OGL)
  
  struct ControllerData
  {
	  float x_, y_, lx_, ly_;  ///< analog stick data
	  float x_pos_, y_pos_, z_pos_; ///< position for pyramid indicator object
	  float dx_, dy_, dz_; ///< incremental updates to the position
	  int   rotationY_, rotationX_, rotationZ_; ///< rotation of the pyramid indicator object
	  LeapFrog::Brio::S32 ax_,ay_,az_;
	  LeapFrog::Brio::S32 lax_, lay_, laz_;
	  bool analog_stick_on_; ///< flag indicating if we are using the analog stick
  };
  float initial_z_; ///< the initial z position of the object
  float gravity_;
  ControllerData controllerData[MAX_CONTROLLERS];
  int numControllers;
  

  
 private:

  /*
   * DoAccEventLoop
   * DoAnalogStickLoop
   * Updates the incremental change in position to the indicator object
   * based on the most current analog stick or acc data
   */
  int DoAccEventLoop(int i);
  void DoAnalogStickLoop(int i);

  /*
   * InitOGL
   *
   * Performs all necessary initialization and setup for OpenGL
   */
  void InitOGL(void);
  
  /*
   * SetClearColor
   *
   * A helper method that sets the OpenGL clear color
   */
  void SetClearColor(LeapFrog::Brio::S16 r, LeapFrog::Brio::S16 g, LeapFrog::Brio::S16 b, LeapFrog::Brio::S16 a);
  
  /*
   * RenderOGL
   *
   * Renders the pyramid based on analog stick input affecting it's position
   */
  void RenderOGL(void);
  
  /*
   * ResetObject
   *
   * A helper method that moves the pyramid indicator back to it's reset position
   */
  void ResetObject(int i);
  
  /*
   * MoveObjectWithGravity
   *
   * A helper method that moves the pyramid indicator based on the current
   * accelerometer data and the gravity of the system
   */
  void MoveObjectWithGravity(int i);

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
  
  void GetControllers(void);
  float getTheta(float x, float y);

};



#endif // __CONTROLLER_INCLUDE_CTRLMAINSTATE_H__

