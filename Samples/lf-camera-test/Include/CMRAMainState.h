#ifndef __CMRAMAINSTATE_H__
#define __CMRAMAINSTATE_H__

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file CMRAMainState.h
*
* The CMRAMainState class is responsible for creating, setting up and
* handling all wand related tracking.  This includes the algorithm, the
* hot spots and the hot spot listener.
*
* author: leapfrog
*          alindblad - 1/3/14 - created class
*
******************************************************************************/

#include <GameStateBase.h>
#include <DisplayMPI.h>
#include <CameraMPI.h>
#include <TouchEventQueue.h>
#include <ButtonEventQueue.h>
#include <Vision/VNVisionMPI.h>

using namespace LeapFrog::Brio;

class CMRAMainState : public CGameStateBase {
public:
  CMRAMainState(void);
  ~CMRAMainState(void);
  
  /*!
   * Enter is called when the state first loads.
   * All initial setup should take place here
   * Enter is only called once per instance
   */
  virtual void Enter(CStateData* userData);
  
  /*!
   * Update is called on every tick
   * This is the heartbeat of the program
   */
  virtual void Update(CGameStateHandler* sh);
  
  /*!
   * Suspend is called when AppManager needs to pause the game
   * This could be triggered by a system pause from the pause button,
   * a low battery warning, or by pushing a new app onto the AppManager
   *  stack (ie tutorial)
   */
  virtual void Suspend(void);
  
  /*!
   * Resume is called upon returning from the suspended state
   * This could be triggered by a system un-pause from the pause button,
   * returning from the low battery warning, or returning from another 
   * app that was previously launched from the AppManager stack
   */
  virtual void Resume(void);
  
  /*!
   * Exit is called when the state is destroyed
   * All cleanup takes place here
   * Exit is only called once per instance
   * This could be triggered by quitting the game, plugging in the USB, 
   * turning the power off, etc  Exit would not be called in the case 
   * of a catastrophic failure, such as sudden loss of power
   */
  virtual void Exit(void);
  
 protected:
  int DoKeyEventLoop(void);  
  void ButtonSelected(U16 item);
  void InitSample(void);
  void CleanDisplay(void);
  void RegisterEventListeners(void);
  void UnRegisterEventListeners(void);
  void SetupDisplaySurface(void);

  LeapFrog::Brio::CButtonEventQueue buttonEventQueue_; 
  LeapFrog::Brio::CDisplayMPI displayManager_;
  int screenWidth_, screenHeight_;
  
  LeapFrog::Brio::tDisplayHandle displayHandle_;
  
  class CameraListener : public IEventListener {
  public:
    CameraListener(void);
    tEventStatus Notify(const IEventMessage& msg);
  private:
    int frameCount_;
    time_t frameTime_;
    LeapFrog::Brio::CCameraMPI cameraMPI_;
  };

  CameraListener cameraListener_;
  LeapFrog::Brio::CCameraMPI cameraMPI_;
  tVideoSurf videoDisplaySurface_;
  tVidCapHndl videoCapture_;
};



#endif // __CMRAMAINSTATE_H__

