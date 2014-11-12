#ifndef LEDSMainState_H_
#define LEDSMainState_H_

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file LEDSApp.h
*
* Some bried to explain the roll of the main state class
*
*
* LeapFrog Apps move from state to state with the app class handling the changes.
* In this example, the MainState is the only state this application enters.  Each
* state must respond to a set of virtual methods that indicate the next course of
* action.  These are: Enter, Exit, Suspend, Resume and Update.
*
* author: 	leapfrog
*  			tmartin - 10/19/12 - cleaned up code and indicated coding style.
*
******************************************************************************/

#include <GameStateHandler.h>
#include <GameStateBase.h>
#include <DisplayMPI.h>
#include <ButtonMPI.h>
#include <EventMPI.h>
#include <TouchEventQueue.h>
#include <ButtonEventQueue.h>
#include "LEDTestApp.h"
#include <BlitBuffer.h>
#include <Hardware/HWControllerMPI.h>
#include <Hardware/HWControllerTypes.h>
#include <EventListener.h>

using namespace LeapFrog::Brio;

class LEDSMainState : public CGameStateBase, public LeapFrog::Brio::IEventListener
{
public:
  LEDSMainState();
  ~LEDSMainState();
  
  virtual LeapFrog::Brio::tEventStatus Notify(const LeapFrog::Brio::IEventMessage &msgIn);

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
  int DoTouchEventLoop();
  int DoKeyEventLoop();
  
  friend class EgAudioListener;
  void ButtonSelected(U16 item);
  
  static LEDSMainState *				state_;
  
  LeapFrog::Brio::U16 				current_selection_;
  
  LeapFrog::Brio::CTouchEventQueue 	touch_event_queue_;
  LeapFrog::Brio::CButtonEventQueue 	button_event_queue_;
  
  //SAMPLE CODE
  void initSample();
  void drawSample();
  void initDisplay();
  void cleanDisplay();
  
  LeapFrog::Brio::CDisplayMPI 		display_manager_;
  int screen_width_, screen_height_;
  
  CBlitBuffer screen_buffer_[2];
  LeapFrog::Brio::tDisplayHandle		display_handle_[2];
  int current_buffer_;
  
  LF::Hardware::HWControllerMPI controllerMPI_;
  int color_;
};



#endif // LEDSMainState_H_

